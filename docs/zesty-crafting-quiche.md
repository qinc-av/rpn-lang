# Plan: String Literals, StName, STO/RCL, and Collection Literals

## Status: COMPLETE

Both phases are implemented. All 5386 assertions pass.

---

## Phase 1 — String Literals, StName, STO/RCL ✓

**Outcome:** `"hello world"` pushes a string; `'x'` pushes a name; `42. 'x' STO` stores a value; `."` stays as backward-compat alias.

**Files changed:**
- `rpn.h` — `stack::Name` class; `StString::deparse()` now returns `"..."` format
- `src/rpn-interp.cpp` — `nextWord` groups `"..."` tokens; `runtime_eval` / `compiletime_eval` detect string and name literals; STO/RCL/PURGE accept `StName` or `StString` via `name_from_tos()`; VARS pushes `StName`
- `tests/runtime-test.cpp` — literals and STO/RCL test cases

---

## Phase 4.3 — Collection Literals via `stack::Marker` ✓

**Outcome:** `[ 1. 2. 3. ]` pushes a `stack::Vector`; `{ "x" 1. "y" 2. }` pushes a `stack::Object`.

**Key design decisions:**
- `stack::Marker` type in `rpn.h` — holds a string label, used as a stack sentinel
- `MARK` / `FIND-MARK` as user-accessible Forth-inspired primitives
- `[` / `{` push markers; `]` / `}` are stdlib compiled words that call `FIND-MARK` then collect
- `->OBJ` order swapped to `"key" value ->OBJ` (key-before-value, consistent with literal syntax)
- `->OBJECT` added as explicit n-field word (`"k1" v1 .. "kn" vn n ->OBJECT`) used by `}`; kept separate from `->OBJ` to avoid dispatch ambiguity when TOS is a numeric value vs. a count
- `<`/`>` for Array literals NOT implemented — ambiguous with comparison operators

**Files changed:**
- `rpn.h` — `stack::Marker` class; `addMarkerWords()` declaration
- `src/types-dict.cpp` — `MARK`, `FIND-MARK`, `[`, `{`, `->OBJECT`; `->OBJ` validator/order fixed; `addMarkerWords()`
- `src/rpn-stdlib.cpp` — `]`, `}` compiled words
- `src/rpn-interp.cpp` — `addMarkerWords()` call in constructor
- `tests/runtime-test.cpp` — marker and collection literals test case

---

## Verification

```
cmake --build build-tests
./build-tests/stack-test
./build-tests/runtime-test
```
