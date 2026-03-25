# rpn-lang Development Plan

## Status

Phase 0 complete. Starting Phase 1.

---

## Background

rpn-lang is an embeddable C++ RPN/RPL stack-machine interpreter.  It is
embedded in RP-42 (SwiftUI iOS/iPadOS/macOS calculator) and planned for
Machina Nexum (CNC controller).  The mindmap `rpn-lang.xmind` is the
source of most of these tasks.

---

## Phase 0 — Technical Debt (Complete)

| # | Task | Status |
|---|---|---|
| 0.1 | Move `sk_double_decimals`, `sk_double_precision`, `_sk_int_radix` from file-scope statics into `Interp::Privates`. Update precision/radix words to use per-instance state. Keep globals in sync for `rpn::to_string()` free functions until Phase 2.2. | Done |
| 0.2 | Remove all `printf` from library code. Added `_debugSink` callback and `_trace()` helper to `Privates`. Added `rpn::Interp::setDebugSink()` to public API. Error status goes into `_status`; tracing goes through the sink. | Done |
| 0.3 | Stack type audit. Fixed `StInteger::operator string()` (now respects radix). Fixed `StInteger::to_latex()` (subscript only when radix ≠ 10). Implemented `StObject::deparse()`. Implemented `object_to` (`OBJ->`), `add_array_any`, `add_any_array`. Cleaned up `peek_for_display()`. Fixed pre-existing stack-test failure. | Done |
| 0.4 | Implemented `addCompiledWord`. Added `<< ... >>` lambda syntax (`ct_lambda`). Added `EXEC` word. Fixed `Progn::deparse()` and `Progn::to_latex()`. Modified `eval_lambda` to push `ct_lambda` Progns rather than execute them. | Done |

---

## Phase 1 — Control Flow and Variables

### 1.1 PROGN / anonymous executable on stack
`<< ... >>` syntax and `EXEC` were implemented in Phase 0.4.  Complete.

### 1.2 IF / THEN / ELSE

**Pattern to follow:** `ct_NEXT` in `rpn-interp.cpp`.

HP48 / Forth syntax:
```
IF <condition> THEN <true-branch> END
IF <condition> THEN <true-branch> ELSE <false-branch> END
```

Steps:
1. Add `ct_ifblock` to `CompileType` enum.
2. Register `IF` in `_ctDictionary`: calls `start_compile(ct_ifblock, false)`.
3. Register `THEN` in `_ctDictionary`: ends the condition capture (not needed if condition is evaluated before IF — see note below), starts true-branch compile.
4. Register `ELSE` in `_ctDictionary`: ends true-branch, starts false-branch.
5. Register `END` in `_ctDictionary`: finalizes.  At top level, evaluates the if-block immediately; when nested, stores in parent `_locals` with address key.

**Design note:** In HP48/Forth, `IF` consumes a boolean already on the stack (the condition is evaluated before `IF` is reached). `IF` itself just branches. So the compile structure is: collect true-branch words, optionally collect false-branch words, at runtime pop a boolean and dispatch.

Implementation approach: an if-block Progn stores two sub-Progns in its `_locals` under canonical keys `"__true"` and `"__false"`.  Its `eval()` case pops a boolean, evaluates the appropriate branch.

**Dependencies:** 0.4 (lambda/Progn infrastructure).
**Complexity:** M.

### 1.3 WHILE / UNTIL Loops

Syntax:
```
BEGIN <body> <condition> WHILE REPEAT    ( loop while true )
BEGIN <body> UNTIL                       ( loop until true )
```

`ct_whileloop` enum value and `eval_whileloop` stub already exist. Implement `eval_whileloop` and `eval_untilloop`.

**Dependencies:** 1.2 (pattern clarified by IF work).
**Complexity:** M.

### 1.4 Global Variables — STO / RCL

1. Add `var_dict_t _globalVars` to `Interp::Privates`.
2. Implement `STO`: pop a string (name) and a value, store in `_globalVars`.
3. Implement `RCL`: pop a string (name), push stored value.
4. In `runtime_eval`, check `_globalVars` after dictionary lookup fails.
5. Implement `VARS`: push Array of all variable names.
6. Implement `PURGE`: remove a named variable.

**Design decision:** HP48 convention — variables shadow dictionary words (look up variables before dictionary). Forth convention is the reverse. Decision: **HP48 convention** (variables shadow words).

**Dependencies:** 0.1 (Privates owns state).
**Complexity:** S.

### 1.5 FOR Loop — STEP Support

`ct_STEP` and `NATIVE_WORD_DECL(private, ct_STEP)` exist but are `#ifdef notyet`.

1. Uncomment and enable `ct_STEP`.
2. Add `_step` field to `Progn` (default 1.0).
3. `ct_STEP` at compile time: store the step value in `_step`.
4. `eval_forloop`: use `_step` in the loop condition and increment; support negative step for countdown loops.

**Dependencies:** None (isolated change).
**Complexity:** S.

### 1.6 Trig Mode — DEG / RAD / GRAD

1. Add `enum class AngleMode { degrees, radians, gradians }` to `Privates`.
2. Add `AngleMode _angleMode = AngleMode::degrees` to `Privates`.
3. Register `->DEG`, `->RAD`, `->GRAD` words that set the mode.
4. Register `DEG->`, `RAD->`, `GRAD->` words that push the current mode as a string.
5. Refactor all trig functions in `math-dict.cpp` to read `_angleMode` from the interpreter context.

**Open question:** trig words currently take no `ctx` with an `Interp::Privates*`.  Options:
- Pass the interpreter reference (already available as `rpn`) and downcast `rpn.m_p`.
- Keep a module-level pointer to current `Privates` (fragile).
- Recommended: access via `rpn.m_p->_angleMode` directly in each trig word — `m_p` is accessible since it's public on `Interp`.

**Dependencies:** 0.1.
**Complexity:** M (mechanical but touches all trig functions).

### 1.7 Binary Wordsize Mode

1. Add `int _binaryWordsize = 64` to `Privates`.
2. Register `->WORDSIZE` / `WORDSIZE->` words.
3. Apply a bitmask on integer results in binary operations (`AND`, `OR`, `XOR`, `NEG`).
4. Implement `LSHIFT` and `RSHIFT` (stubs commented out in `math-dict.cpp`).

**Dependencies:** 0.1.
**Complexity:** S.

---

## Phase 2 — API Completeness and Embedding Quality

### 2.1 Word Introspection / Documentation API

Add to `WordDefinition`:
- `std::string description`
- `std::string stack_effect`  (Forth-style: `( n1 n2 -- n3 )`)
- `std::string category`

Add words: `WORD-HELP`, `WORD-EFFECT`.
Expand `rpn-hl.h` to expose word help via the high-level interface.

This enables RP-42 long-press documentation and the word reference in this developer guide.

**Complexity:** L (mechanical — every word registration needs touching).

### 2.2 Display vs. Deparse Contract

Formalize and enforce:
- `operator string()` = human display string
- `deparse()` = lossless RPN that round-trips through `EVAL`
- `to_text()` = plain text (defaults to `operator string()`)
- `to_latex()` = LaTeX math mode

Verify `DEPARSE` → `EVAL` round-trips for all built-in types.
Remove the `TODO Phase 2.2` globals once display flows properly through the interpreter.

**Complexity:** M.

### 2.3 Compiled Word Validation

Currently all user-defined words get `StackSizeValidator::zero`, bypassing type checking.  Allow validator specification, either:
- Parse `( stack-effect )` comment to generate a validator.
- Or explicit validator in `addCompiledWord`.

**Complexity:** M.

### 2.4 Threading / Embedding Review

- Document the threading contract.
- Add `cancelAll()` to drain the queue.
- Design a long-running word progress callback (required for CNC probing in Machina Nexum).

**Complexity:** M (design decision first).

### 2.5 Foreign Language / C ABI

- Expand `RpnInterp` HL layer to expose stack peek and word introspection.
- Plan a C ABI wrapper (`rpn_c.h`) for non-C++ bindings.
- WASM deferred to Phase 4.

**Complexity:** M.

---

## Phase 3 — Extended Types and Operations

### 3.1 JSON Words

Add `->JSON` (serialize TOS to JSON string) and `JSON->` (parse JSON string into Array/Object/primitives). Use existing Array/Object types — no new type needed.

**Complexity:** S–M.

### 3.2 General Vector / Matrix

`stack::Matrix` type (NxM). `->MATRIX` word. Connect to `StVec3`. Basic operations: `+`, `-`, `*`, determinant, transpose, inverse.

**Complexity:** L. Prerequisite for Phase 4 statistics and linear algebra.

### 3.3 Algebraic Entry Integration

`shunting-yard.cpp` already exists but is disconnected. Wire it into `eval_mathexpr` and register an infix delimiter in the CT dictionary.

**Complexity:** M.

### 3.4 Unit Conversions

New `units-dict.cpp`. Conversion words for length, mass, temperature, pressure, volume, angle. Pure conversion words — no new type.

**Complexity:** S.

---

## Phase 4 — Advanced Mathematics

| # | Task | Complexity | Notes |
|---|---|---|---|
| 4.1 | Statistics: MEAN, VARIANCE, STDDEV, LINFIT, CORRELATION on Array | M | No new type |
| 4.2 | Linear Algebra: DET, INV, TRANS, EIGENVAL on Matrix | L | Requires 3.2 |
| 4.3 | Binary ops enhancement: RLEFT, RRIGHT (rotate), STWS | S | Builds on 1.7 |
| 4.4 | CAS: symbolic differentiation, integration, simplification | XL | Research; evaluate SymEngine/GiNaC first |

---

## Phase 5 — Testing

### 5.1 Structured Validator Tests

`tests/validator-tests.h` has ~200 hand-written cases as a static map. Wrap into parameterized Catch2 `TEST_CASE`s with descriptions so failures are identifiable.

### 5.2 Comprehensive Word Tests

TDD for all Phase 1 control flow (write tests before implementation). Test error paths. `DEPARSE` round-trip tests. Per-dict coverage.

One pre-existing failure in `runtime-test.cpp` test case 17: the `tests.rpn` parse test uses a relative file path (`"tests.rpn"`) that only resolves when the test binary is run from the `tests/` directory. Not a code bug.

---

## Documentation

See `docs/developer-guide.md`.

The word reference section will be substantially auto-generated once Phase 2.1 (word introspection API) is complete.

---

## Open Design Decisions

| Topic | Decision / Status |
|---|---|
| C++ macros vs templates | Keep macros. They serve their purpose; a hybrid would require two mental models simultaneously with no net gain. |
| `TStackObject<T>` retention | Keep. It is the documented extension path for embedder custom types; removing it would break the public API. |
| STO/RCL lookup precedence | HP48 convention: variables shadow dictionary words. |
| Trig mode propagation | Access via `rpn.m_p->_angleMode` in math words (m_p is public on Interp). |
| JSON type vs JSON words | Words-only approach on existing Array/Object. No new type. |
| Double → Number rename | Low urgency; no strong reason to rename. Leave as-is. |
| CAS library | Research phase. Evaluate SymEngine and GiNaC before any implementation. |
| WHILE loop variable precedence | `while` checks TOS boolean, does not pop it before body — to be confirmed against HP48 semantics. |

---

## UI Tasks (Dependent on Library Stability)

These live in the RP-42 SwiftUI project and Machina Nexum, not in rpn-lang itself.  Library prerequisites noted.

**RP-42:**
- Per-type stack views (needs Phase 2.2 display contract)
- Long-press word documentation (needs Phase 2.1 introspection API)
- State indicators for DEG/RAD/GRAD, binary wordsize, radix (needs Phase 1.6, 1.7, 0.1)
- Drag-and-drop stack manipulation
- Object editing vs object viewing modes
- Emacs-calc keybinding feel

**Machina Nexum:**
- Reuse RP-42 UI components
- CNC words: probe operations, jog/control, read machine coordinates
- Long-running word callback protocol (needs Phase 2.4 before CNC words can be implemented)
