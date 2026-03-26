# rpn-lang Development Plan

## Status

Phase 0 complete. Phase 1 complete. Starting Phase 2.

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

## Phase 1 — Control Flow and Variables (Complete)

| # | Task | Status |
|---|---|---|
| 1.1 | `<< ... >>` lambda syntax and `EXEC` — implemented in Phase 0.4. | Done |
| 1.2 | `IF <cond> THEN <true> [ELSE <false>] END`. HP48-style: condition already on stack, IF branches. True/false branches stored in `_locals` under `__true`/`__false`. | Done |
| 1.3 | `BEGIN <body> <cond> WHILE REPEAT` and `BEGIN <body> UNTIL`. Single-block design; `__until` key in `_locals` distinguishes the two forms. | Done |
| 1.4 | `STO` / `RCL` / `VARS` / `PURGE`. HP48 convention: variables shadow dictionary words. Added `StName` type (`'identifier'` literal syntax) and `"string"` literal syntax with parser-level grouping. `is_valid_name()` prevents shadowing operators or numeric literals. | Done |
| 1.5 | `FOR ... NEXT` (fixed step 1) and `FOR ... n STEP` (runtime step from TOS each iteration). Negative step for countdown loops. `_step = NaN` sentinel marks step-from-stack mode. | Done |
| 1.6 | `AngleMode` enum (`degrees`/`radians`/`gradians`) in `rpn.h`. Public `angleMode()`/`setAngleMode()` on `Interp`. `->DEG` / `->RAD` / `->GRAD` set mode; `ANGLEMODE` queries it. All trig words (`SIN` `COS` `TAN` `ASIN` `ACOS` `ATAN` `ATAN2`) rewritten as mode-aware `NATIVE_WORD_DECL`s using `to_radians()`/`from_radians()` helpers. | Done |
| 1.7 | `binaryWordsize()` / `setBinaryWordsize()` (clamped 1–64) on `Interp`. `AND` `OR` `XOR` `NEG` apply wordsize bitmask. `LSHIFT` / `RSHIFT` (logical unsigned, masked). `->WORDSIZE` / `WORDSIZE->`. Default wordsize 64 = no-op mask. | Done |

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

**Deadlocked / runaway interpreter** — all loop constructs (`eval_forloop`, `eval_whileloop`)
are unbounded by design; protection is the embedder's responsibility.  Currently the embedder
has no way to interrupt a running evaluation from another thread.  Address holistically here:

1. `std::atomic<bool> _cancelRequested` in `Privates`, checked at the top of each loop
   iteration.  Public API: `Interp::cancel()` sets the flag; `parse()` clears it on entry.
2. `cancelAll()`: set cancel flag AND drain the pending request queue so no further
   work starts.
3. Long-running word progress / heartbeat callback — required for CNC probing; also lets
   the embedder implement its own timeout strategy without a hard-coded limit.
4. Consider debug helpers: iteration counter accessible via callback, last-executed-word
   in `_status`, or a watchdog-friendly `ping()` that returns only when the interpreter
   is idle.

**Complexity:** M (design decision first).

### 2.5 High-Level Interface Completeness

- Expand `RpnInterp` HL layer to expose stack peek and word introspection
  (prerequisite for 2.1 metadata reaching Swift/ObjC callers).

**Complexity:** S–M.

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

## Possible Future Work

Items considered but not scheduled.  Revisit if requirements emerge.

| Item | Notes |
|---|---|
| WASM build target | Technically feasible; no current use case. |
| C ABI wrapper (`rpn_c.h`) | Would enable non-C++ bindings; no active requirement. |

---

## Open Design Decisions

| Topic | Decision / Status |
|---|---|
| C++ macros vs templates | Keep macros. They serve their purpose; a hybrid would require two mental models simultaneously with no net gain. |
| `TStackObject<T>` retention | Removed. MI (inherit `rpn::Stack::Object` + domain class) is the extension path. Documented in developer guide §9. |
| STO/RCL lookup precedence | HP48 convention: variables shadow dictionary words. Implemented. |
| Trig mode propagation | `AngleMode` enum in `rpn.h`; public `angleMode()`/`setAngleMode()` on `Interp`; math words call `rpn.angleMode()`. Implemented. |
| JSON type vs JSON words | Words-only approach on existing Array/Object. No new type. |
| Double → Number rename | Low urgency; no strong reason to rename. Leave as-is. |
| CAS library | Research phase. Evaluate SymEngine and GiNaC before any implementation. |
| WHILE loop design | Single-block: all body+condition before WHILE/UNTIL; `__until` key distinguishes WHILE (exit when false) from UNTIL (exit when true). At-least-one-iteration semantics for UNTIL. Implemented. |
| FOR STEP design | `FOR ... n STEP` — body leaves step on TOS each iteration; STEP pops it. `_step = NaN` sentinel marks step-from-stack mode. NEXT = fixed step 1. Implemented. |
| StName vs StString for variables | `StName` (`'identifier'` literal) for variable names; `StString` (`"content"` literal) for data. `is_valid_name()` prevents shadowing. Implemented. |
| Runaway / deadlocked interpreter | Deferred to Phase 2.4. Embedder has no interrupt mechanism yet; see Phase 2.4 notes. |

---

## UI Tasks (Dependent on Library Stability)

These live in the RP-42 SwiftUI project and Machina Nexum, not in rpn-lang itself.  Library prerequisites noted.

**RP-42:**
- Per-type stack views (needs Phase 2.2 display contract)
- Long-press word documentation (needs Phase 2.1 introspection API)
- State indicators for DEG/RAD/GRAD, binary wordsize, radix (Phase 1.6, 1.7, 0.1 complete — library-side ready)
- Drag-and-drop stack manipulation
- Object editing vs object viewing modes
- Emacs-calc keybinding feel

**Machina Nexum:**
- Reuse RP-42 UI components
- CNC words: probe operations, jog/control, read machine coordinates
- Long-running word callback protocol (needs Phase 2.4 before CNC words can be implemented)
