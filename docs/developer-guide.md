# rpn-lang Developer Guide

## Contents

1. [Project Overview](#1-project-overview)
2. [Build and Test](#2-build-and-test)
3. [Architecture Overview](#3-architecture-overview)
4. [The Dictionary System](#4-the-dictionary-system)
5. [Adding a Native Word](#5-adding-a-native-word)
6. [The Interpreter Internals](#6-the-interpreter-internals)
7. [Adding a Control Flow Word](#7-adding-a-control-flow-word)
8. [Stack Types](#8-stack-types)
9. [Adding a New Stack Type](#9-adding-a-new-stack-type)
10. [The Macro System](#10-the-macro-system)
11. [The High-Level Embedding Interface](#11-the-high-level-embedding-interface)

---

## 1. Project Overview

rpn-lang is an embeddable C++ RPN/RPL stack-machine interpreter.  It is
designed to be extended with native C++ words and custom stack types without
modifying the core.

Key properties:
- Words are dispatched by name and validated against the current stack before execution.
- The same word name can have multiple overloaded definitions with different type validators.
- Words are either *native* (C++ functions) or *compiled* (RPN source defined with `: name ... ;`).
- An asynchronous execution queue is available for embedding in event-driven applications.

---

## 2. Build and Test

The library builds as a static archive.  Tests use Catch2 v3.

```bash
# Build the library
cmake -B build -S .
cmake --build build

# Build and run tests
cmake -B build-tests -S tests
cmake --build build-tests
./build-tests/stack-test
./build-tests/runtime-test   # run from the tests/ directory for tests.rpn
```

---

## 3. Architecture Overview

```
rpn::Interp
├── rpn::Stack                   public member: the stack itself
│   └── deque<unique_ptr<Stack::Object>>
├── Interp::Privates             opaque implementation
│   ├── _rtDictionary            multimap: runtime word dispatch
│   ├── _ctDictionary            map: compile-time word dispatch
│   ├── _ctVprogn                stack of Progn under construction
│   ├── _vlocals                 stack of local variable scopes
│   ├── _globalVars              (Phase 1.4) global STO/RCL store
│   ├── _double_decimals         per-instance display precision
│   ├── _int_radix               per-instance integer display base
│   ├── _angleMode               (Phase 1.6) DEG/RAD/GRAD
│   ├── _tracing                 enable debug trace output
│   └── _debugSink               optional callback for trace output
└── async queue                  mutex + condvar + std::async thread
```

### Execution flow

`sync_eval(line)` → `parse(line)` → split on whitespace → for each token:
`Privates::eval(word, rest)` → if compiling: `compiletime_eval` else `runtime_eval`.

`runtime_eval`:
1. If `word` looks like a number → parse and push.
2. Look up `word` in `_rtDictionary` (`word_exists`).
3. Find the matching overload whose validator accepts the current stack (`validate_word`).
4. Call the word's `eval` function.

---

## 4. The Dictionary System

### Two dictionaries

**`_rtDictionary`** (`std::multimap<string, WordDefinition>`)
The runtime dictionary.  Looked up during normal evaluation.  A multimap
because the same word name can have multiple overloads with different validators
(e.g. `+` for doubles, integers, strings, arrays).

**`_ctDictionary`** (`std::map<string, WordDefinition>`)
The compile-time dictionary.  Looked up *instead of* the runtime dictionary
while a word is being compiled (i.e. while `_ctVprogn` is non-empty).
Used for control-flow words like `;`, `NEXT`, `>>` that have different
meaning during compilation than at runtime.

### WordDefinition

```cpp
struct WordDefinition {
  const StackValidator &validator;   // checked before eval is called
  std::function<Result(Interp &rpn, WordContext *ctx, std::string &rest)> eval;
  WordContext *context;              // passed back as ctx; often `this` (Privates*)
};
```

`rest` contains the remainder of the current input line after `word`.  Words
that consume additional tokens (e.g. `."`, `(`) read from `rest` directly.

### Validators

`StrictTypeValidator` — checks exact types at specific stack depths, bottom-up.
Static instances cover the common combinations: `d1_double`, `d2_double_double`,
`d2_integer_double`, etc.  Custom validators can be constructed with a
`vector<size_t>` of `typeid(...).hash_code()` values.

`StackSizeValidator` — only checks depth, not types.  Use `zero`, `one`,
`two`, `three`, or construct with `n`.

Validators are matched against `stack.types()`, which returns the type hashes
of all stack items from TOS downward.

---

## 5. Adding a Native Word

### 1. Declare and implement the function

Use `NATIVE_WORD_DECL` in the appropriate dict source file:

```cpp
NATIVE_WORD_DECL(mydict, my_word) {
  // rpn      — the interpreter; access the stack via rpn.stack
  // ctx      — the WordContext* registered with this word (often nullptr or Privates*)
  // rest     — remainder of current input line
  double val = rpn.stack.pop_as_double();
  rpn.stack.push_double(val * 2.0);
  return rpn::WordDefinition::Result::ok;
}
```

The mangler (`mydict`) is just a prefix to prevent name collisions between
dict files.  `NATIVE_WORD_FN(mydict, my_word)` expands to the actual C++
function name.

### 2. Register it

In your `addXxxWords()` function:

```cpp
void rpn::Interp::addMyWords() {
  addDefinition("MY-WORD", {
    rpn::StrictTypeValidator::d1_double,
    NATIVE_WORD_FN(mydict, my_word),
    nullptr
  });
}
```

Call `addMyWords()` from `Interp::Interp()` in `rpn-interp.cpp`.

### Shortcut macros for common patterns

For words that simply wrap a C function:

```cpp
// Push a constant double
NATIVE_WORD_FN_0_DOUBLE(mydict, my_pi, M_PI)

// Unary double → double
static double my_func(double a) { return a * a; }
NATIVE_WORD_FN_1_NUMBER(mydict, my_func)

// Binary (double,double) → double
static double my_add(double a, double b) { return a + b; }
NATIVE_WORD_FN_2_NUMBER(mydict, my_add)
```

Then register with `ADD_NATIVE_1_NUMBER_WDEF` / `ADD_NATIVE_2_NUMBER_WDEF`
to automatically create overloads for all double/integer combinations:

```cpp
// Registers 4 overloads: d/d, d/i, i/d, i/i — dispatching to double or
// integer function as appropriate
ADD_NATIVE_2_NUMBER_WDEF(mydict, *this, "+", my_add, my_iadd, nullptr);
```

---

## 6. The Interpreter Internals

This section covers the compile-time machinery.  Read this before touching
`rpn-interp.cpp` or implementing Phase 1 control-flow features.

### The `Progn` struct

`Progn` is the internal representation of a compilable unit — a word definition,
a loop body, a lambda, or a conditional branch.  It inherits from both
`rpn::WordContext` and `rpn::Stack::Object`, so it can be stored on the stack
(as a lambda) or used as the `ctx` pointer in a `WordDefinition`.

Key fields:
```cpp
struct Progn {
  rpn::Interp::Privates &_p;               // back-reference to interpreter
  std::vector<std::string> _wordlist;       // tokens to evaluate
  std::shared_ptr<var_dict_t> _locals;      // local variable scope
  CompileType _type;                        // ct_worddef / ct_forloop / ct_lambda / ...
  std::string _ident;                       // name (word def) or loop variable (FOR)
};
```

A Progn is built word-by-word during compilation, then either:
- **Registered** in `_rtDictionary` (word definitions via `:`).
- **Pushed onto the stack** (lambdas via `<< ... >>`).
- **Executed immediately** (top-level loops and conditionals).
- **Stored in a parent Progn's `_locals`** (nested control flow).

### The compile stack: `_ctVprogn`

`_ctVprogn` is a `std::vector<Progn>` that acts as a compilation stack.
When `_ctVprogn` is non-empty, the interpreter is in compile mode.

`start_compile(type, needIdent)` pushes a new Progn.
`end_compile(progp, type)` pops it, verifying the type matches.

In compile mode, `Privates::eval` routes to `compiletime_eval` instead of
`runtime_eval`.  `compiletime_eval`:

1. If `_needIdent` is true and the current Progn has no `_ident` yet →
   the next token becomes the identifier (used for `: wordname` and `FOR varname`).
2. Check the compile-time dictionary (`_ctDictionary`) — if found, execute the
   CT word immediately (these are the control-flow words).
3. Otherwise, if the token is a number or exists in `_rtDictionary`, add it
   to `_ctVprogn.back()._wordlist` for later execution.

### Nesting

Nested control flow (a loop inside a word def, or an `IF` inside a loop) works
by having multiple Progns on `_ctVprogn` simultaneously.  The innermost Progn
is always `_ctVprogn.back()`.

When an inner Progn completes (e.g. `NEXT` ends a FOR loop), `end_compile`
pops it.  If another Progn remains on the stack (we are nested), the completed
Progn is stored in the *parent* Progn's `_locals` using its memory address as
a string key, and that key string is added to the parent's wordlist:

```cpp
// from ct_NEXT handler (the established pattern):
std::string word = std::to_string((uint64_t)progp);
p->_ctVprogn.back()._locals->emplace(word, progp);
p->_ctVprogn.back().addWord(word);
```

When the parent Progn eventually executes (`eval_lambda`), it encounters this
address-as-string in its wordlist, finds it in `_locals`, and dispatches it.

### `eval_lambda` — the execution engine

`eval_lambda` is called for `ct_worddef` and `ct_lambda` Progns.  It walks
`_wordlist` token by token:

1. Check `_locals` for the token (handles loop variables, nested Progns, lambdas).
2. If the local is a Progn:
   - `ct_lambda` type → **push it onto the stack** (use `EXEC` to evaluate).
   - Any other type → **execute it** (nested loops, word sub-programs).
3. If not a local, call `_p.eval(word, rest)` — normal runtime dispatch.

### Local variable scopes: `_vlocals`

`_vlocals` is a `std::vector<shared_ptr<var_dict_t>>` that acts as a stack of
scopes.  On entry to a Progn, its `_locals` map is pushed; on exit it is popped.
`find_local_variable` searches from innermost to outermost scope.

FOR loop iteration variables are stored in `_locals` and updated each iteration:

```cpp
(*_locals)[_ident] = std::make_unique<StDouble>(start);
```

---

## 7. Adding a Control Flow Word

This is the most important section for Phase 1 work.  The established pattern
is the `FOR` / `NEXT` implementation.  Follow it exactly for new constructs.

### The FOR loop as the reference pattern

`FOR` is a runtime word (registered in `_rtDictionary`).  When evaluated at
top level, it calls `start_compile(ct_forloop, true)` — `true` means the next
token will be the loop variable name (`_ident`).

`FOR` is *also* registered in `_ctDictionary` as `ct_FOR` so that nested loops
(a FOR inside a word def) are handled — `ct_FOR` calls `start_compile` again,
pushing another Progn onto `_ctVprogn`.

`NEXT` is only in `_ctDictionary` (it has no runtime meaning outside a
compile).  Its handler:

```cpp
NATIVE_WORD_DECL(private, ct_NEXT) {
  Progn *progp = nullptr;
  rv = p->end_compile(progp, ct_forloop);   // pop the FOR's Progn
  if (rv == ok) {
    if (p->_ctVprogn.size() == 0) {
      // Top level: execute immediately
      rv = progp->eval(rpn);
      delete progp;
    } else {
      // Nested: store in parent, add reference to parent's wordlist
      std::string word = std::to_string((uint64_t)progp);
      p->_ctVprogn.back()._locals->emplace(word, progp);
      p->_ctVprogn.back().addWord(word);
    }
  }
  return rv;
}
```

### Template for a new control-flow word

```
1. Add a new CompileType value (e.g. ct_ifblock).

2. Add an eval_xxx() method to Progn.
   Implement the runtime semantics there.
   Call it from Progn::eval() in the switch statement.

3. Write the opening word (e.g. IF):
   - Runtime version: registered in _rtDictionary.
     calls start_compile(ct_ifblock, false).
   - Compile-time version: registered in _ctDictionary.
     also calls start_compile(ct_ifblock, false) to allow nesting.

4. Write the closing word (e.g. END):
   - Registered only in _ctDictionary.
   - Calls end_compile(progp, ct_ifblock).
   - Top level: execute or push immediately.
   - Nested: store in parent _locals, add address key to parent wordlist.

5. Write any intermediate words (e.g. THEN, ELSE):
   - Registered only in _ctDictionary.
   - Finalize one sub-Progn, start another.
   - Store sub-Progns in the current Progn's _locals under stable keys
     (e.g. "__true", "__false") so eval_xxx can find them.

6. Register everything in add_private_words().

7. Write tests before implementing (see tests/runtime-test.cpp).
```

### IF / THEN / ELSE design sketch

The if-block Progn collects two sub-Progns in its `_locals`:

- `_locals["__true"]`  — words between `THEN` and `ELSE` / `END`
- `_locals["__false"]` — words between `ELSE` and `END` (empty if no ELSE)

`eval_ifblock`:

```cpp
rpn::WordDefinition::Result Progn::eval_ifblock(rpn::Interp &rpn) {
  bool cond = rpn.stack.pop_as_boolean();
  auto key = cond ? "__true" : "__false";
  auto it = _locals->find(key);
  if (it != _locals->end()) {
    auto *branch = dynamic_cast<Progn*>(it->second.get());
    if (branch) return branch->eval(rpn);
  }
  return rpn::WordDefinition::Result::ok;
}
```

Compile-time words:

- `IF` → `start_compile(ct_ifblock, false)`, also start collecting the
  true-branch immediately (push another Progn for `ct_lambda`).
- `THEN` → `end_compile` the true-branch lambda, store as `"__true"` in
  the if-block's `_locals`.  Start false-branch Progn.
- `ELSE` → `end_compile` false-branch (if any), store as `"__false"`.
- `END` → `end_compile(progp, ct_ifblock)`.  Top-level: execute.  Nested:
  store in parent.

---

## 8. Stack Types

All stack values inherit from `rpn::Stack::Object`.  The required interface:

```cpp
class MyType : public rpn::Stack::Object {
public:
  virtual std::unique_ptr<Object> deep_copy() const override;
  virtual operator std::string() const override; // human display
  virtual std::string deparse() const override;  // lossless RPN round-trip
  virtual bool operator==(const Object &) const override;
  // Optional but expected:
  virtual std::string to_latex() const override; // LaTeX math-mode string
  virtual std::string to_text() const override;  // plain text (defaults to operator string())
  virtual bool operator>(const Object &) const override;
  virtual bool operator<(const Object &) const override;
};
```

### Display contract

| Method | Contract |
|---|---|
| `operator string()` | Human-readable display.  Used by `peek_for_display` (via `to_latex`). |
| `to_latex()` | LaTeX math-mode string.  Default wraps `operator string()` in `\text{}`.  Override for proper math rendering. |
| `to_text()` | Plain text.  Defaults to `operator string()`. |
| `deparse()` | RPN string that, when evaluated, recreates this object on the stack.  Must round-trip. |

### Casting

Use `PEEK_CAST(Type, obj)` to downcast from `Stack::Object&`.  It performs
a `dynamic_cast` and throws `std::bad_cast` on failure (caught by `eval()`
and reported as `param_error`).

### Type identity

Validators use `typeid(MyType).hash_code()`.  `stack.types()` returns a
`vector<size_t>` of hashes from TOS downward.  New types need corresponding
`StrictTypeValidator` instances if they will be used in validator patterns.

---

## 9. Adding a New Stack Type

The preferred pattern is **multiple inheritance**: inherit from both
`rpn::Stack::Object` and your existing domain class.  This means the stack
object *is* the domain object — no wrapping, no unwrapping.

### The MI extension pattern

Suppose you have an existing domain class:

```cpp
// Your existing domain type — no changes required
class Widget {
public:
  Widget(const std::string &name, double value) : _name(name), _value(value) {}
  const std::string &name()  const { return _name; }
  double             value() const { return _value; }
  bool operator==(const Widget &rhs) const {
    return _name == rhs._name && _value == rhs._value;
  }
private:
  std::string _name;
  double      _value;
};
```

The stack adapter uses MI to make it a first-class stack object:

```cpp
// In your dict header or .cpp file:
namespace stack {

class Widget : public rpn::Stack::Object, public ::Widget {
public:
  Widget(const std::string &name, double value) : ::Widget(name, value) {}
  Widget(const ::Widget &w) : ::Widget(w) {}

  // Required interface:
  virtual std::unique_ptr<rpn::Stack::Object> deep_copy() const override {
    return std::make_unique<Widget>(*this);
  }
  virtual operator std::string() const override {
    return "Widget{" + name() + ":" + std::to_string(value()) + "}";
  }
  virtual std::string deparse() const override {
    // RPN that recreates this object — adjust to match your ->WIDGET word
    return std::to_string(value()) + " \"" + name() + "\" ->WIDGET";
  }
  virtual bool operator==(const rpn::Stack::Object &orhs) const override {
    const auto &rhs = PEEK_CAST(const Widget, orhs);
    return ::Widget::operator==(rhs);
  }
};

} // namespace stack
```

The key benefit: any word that receives a `stack::Widget` from the stack can
pass it directly to any API that accepts a `::Widget &` — no cast or unwrap
needed because the object already IS-A `::Widget`.

### Step-by-step checklist

1. **Define the adapter class** — in your dict header or `.cpp` file, not in
   `rpn.h`.  Inherit `rpn::Stack::Object` first, then your domain type.

2. **Implement the required interface** — `deep_copy`, `operator string`,
   `deparse`, `operator==`.  Add `operator>` / `operator<` if the type will
   be compared or sorted.

3. **Add a validator** if the type needs to appear in word signatures:

   ```cpp
   namespace mydict_validator {
     extern const rpn::StrictTypeValidator d1_widget;
   }
   const rpn::StrictTypeValidator mydict_validator::d1_widget(
     {typeid(stack::Widget).hash_code()}, "d1_widget");
   ```

4. **Add conversion words** — `->WIDGET` to construct from stack items,
   `WIDGET->` to explode back to primitives.

5. **Add a type alias** if widely used:

   ```cpp
   using StWidget = stack::Widget;
   ```

### Real-world example

`stack::Complex` in `src/math-dict.cpp` uses this exact pattern, inheriting
both `rpn::Stack::Object` and `std::complex<double>`.  The color-math
extension in `etc/colorcalc-rpn.h` shows it at scale: `stack::Rgb`,
`stack::XYZ`, `stack::Lab`, etc., each inheriting their CIE/RGB domain class
alongside `rpn::Stack::Object`.

---

## 10. The Macro System

The macros in `rpn.h` reduce boilerplate for the common word-implementation
patterns.  They are intentionally kept rather than replaced with templates —
they are more readable at the use site, and template errors in this context are
not more debuggable than macro errors.

### Naming

`NATIVE_WORD_FN(mangler, op)` expands to `mangler##_func_##op` — a plain C++
function name.  The mangler prevents collisions between dict files that might
both define a function called e.g. `add`.

### Implementation macros

| Macro | Use |
|---|---|
| `NATIVE_WORD_DECL(mangler, fn)` | Declare/define a word function signature |
| `NATIVE_WORD_FN_0_DOUBLE(mangler, fn, val)` | Push a constant double |
| `NATIVE_WORD_FN_1_NUMBER(mangler, fn)` | pop double → call fn → push double |
| `NATIVE_WORD_FN_1_INTEGER(mangler, fn)` | pop integer → call fn → push integer |
| `NATIVE_WORD_FN_2_NUMBER(mangler, fn)` | pop 2 doubles → call fn(nos,tos) → push double |
| `NATIVE_WORD_FN_2_INTEGER(mangler, fn)` | pop 2 integers → call fn(nos,tos) → push integer |

### Registration macros

| Macro | Use |
|---|---|
| `NATIVE_WORD_WDEF(mangler, validator, w, ptr)` | Build a `WordDefinition` literal |
| `ADD_NATIVE_2_NUMBER_WDEF(mangler, r, sym, dfn, ifn, ptr)` | Register 4 overloads (d/d, d/i, i/d, i/i) |
| `ADD_NATIVE_1_NUMBER_WDEF(mangler, r, sym, dfn, ifn, ptr)` | Register 2 overloads (d, i) |
| `ADD_NATIVE_3_NUMBER_WDEF(mangler, r, sym, dfn, ifn, ptr)` | Register 8 overloads (all d/i triples) |

`math-dict.cpp` defines local aliases (`ADD_MATH_BINARY_NUMBER_WDEF` etc.) that
fix the mangler and `ptr` to reduce further repetition in that file.

---

## 11. The High-Level Embedding Interface

`rpn-hl.h` / `rpn-hl.cpp` provide a simpler C++ class (`RpnInterp`) and an
Objective-C wrapper (`RPNInterp` as `NSObject`) for embedding in
Swift/ObjC applications.

The C++ `RpnInterp`:

```cpp
RpnInterp interp;
interp.eval("3 4 +");
auto stack = interp.displayStack();  // vector of strings
```

`displayStack()` calls `peek_for_display` on each item, which returns the
`to_latex()` string.  RP-42 renders these with a LaTeX engine.

For direct access to the full API, use `rpn::Interp` directly.  The two
integration levels:

**High-level:** Instantiate `rpn::Interp`, call `sync_eval(string)` or
`eval(string, completionHandler)`.  Read results via `stack.peek_for_display(n)`.

**Low-level:** Same as above, plus: `addDefinition` (native C++ words),
`addCompiledWord` (RPN source), `validateWord`, `wordExists`, direct stack
manipulation via `rpn.stack.*`.

### Debug tracing

```cpp
rpn::Interp interp(false);
interp.setDebugSink([](const std::string &msg) {
  std::cerr << "[rpn] " << msg << "\n";
});
interp.sync_eval("TRUE TRACE");   // enable tracing
interp.sync_eval("3 4 +");
interp.sync_eval("FALSE TRACE");  // disable
```

### Threading

`rpn::Interp(true)` launches a background thread with a work queue.  Use
`eval(line, completionHandler)` to post work; the completion handler is called
from the background thread.  The stack is not thread-safe — do not access
`rpn.stack` from the embedding thread while async evaluation is in progress.
Use `sync_eval` from a single thread for single-threaded embedding.
