# rpn-lang Development Plan

## Status

Phase 0 complete. Phase 1 complete. Phase 2 complete. Phase 3 complete. Phase 2.3 retrofitted and complete. RPL standard library complete. C++ word audit complete. Phase 4 complete. CAS spun out to cas-plan.md. Units spun out to units-plan.md. Starting Phase 6 (Testing).

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

## Phase 2 — API Completeness and Embedding Quality (Complete)

| # | Task | Status |
|---|---|---|
| 2.1 | Word introspection / documentation API — `setWordCategory`, `addWordMetadata`, `wordHelp`, `wordList`; `effects` auto-derived from validators; exposed through C++ and ObjC HL. | Done |
| 2.2 | Display / deparse contract — formalized 4-way contract (`operator string`, `deparse`, `to_text`, `to_latex`); full-precision deparse on all built-in types; `thread_local` display globals; `TRUE`/`FALSE` words; DEPARSE round-trip tests. | Done |
| 2.3 | Compiled word validation — `( stack-effect )` comment parsed at compile time. Three-tier behavior: all registered type names → `StrictTypeValidator`; HP48-style variable names → `StackSizeValidator(n)` (arity only); zero inputs → `StackSizeValidator::zero`. Type registry in `Privates`; `registerType()` public API for embedder types. Dynamic validator pools own heap-allocated validators. Future: `name:type` annotation syntax (e.g. `cx:double`) for combined doc + typed validation. | Done |
| 2.4 | Threading / cancel / progress — `std::atomic<bool> _cancelRequested` checked at each word boundary and loop iteration; `cancel()`, `cancelAll()` (drains queue), `isCancelled()`; `setProgressHandler` / `reportProgress`; `Result::cancelled`; full C++ and ObjC HL exposure. | Done |
| 2.5 | HL interface completeness — `displayStack`, `wordHelp`, `wordList`, `describeStack` (returns full `to_json()` descriptor array) in both C++ and ObjC HL layers. | Done |

---

## Phase 3 — Extended Types and Operations (Complete)

### 3.0 Type Identity (Complete)

`virtual std::string type_name() const` added to `Stack::Object` (default `"any"`). Implemented on all concrete types: `double`, `integer`, `boolean`, `string`, `name`, `object`, `array`, `json`, `vec3`, `fraction`, `timecode`, `complex`. All `to_json()` overrides delegate to `type_name()`. Used by the Phase 2.3 type registry.

### 3.1 JSON Words (Complete)

`StJson` type (`class stack::Json : public rpn::Stack::Object, public nlohmann::json`) — a JSON value as a first-class stack type.  `to_json()` virtual method on `Stack::Object` returns a `{type, display, deparse, data}` descriptor; implemented on all built-in types including StFraction, StTimecode, StComplex.  nlohmann/json v3.11.3 vendored under `third_party/`.

- `->JSON` — pops any value, pushes `StJson` holding the `data` field of its descriptor.
- `JSON->` — unpacks a `StJson`: array → elements (as StJson) + count; object → (value, key) pairs + count; scalar → native type.  Analogous to `ARRAY->` / `OBJ->`.
- `describeStack()` on HL layer — returns full descriptor array for UI consumption.

---

## RPL Standard Library (Complete)

Hybrid architecture: C++ primitives + compiled RPL words.  The stdlib is `src/rpn-stdlib.cpp`, loaded by `addStdlibWords()` at the end of the Interp constructor.  Each word uses `( stack-effect )` comments for typed or arity-based validators via Phase 2.3.

**C++ primitives added to support stdlib:** `MOD`, `ABS`, `GAMMA`, `LGAMMA`, `NaN`.

**Implemented words:**

| Word | Category | Definition / notes |
|---|---|---|
| `SINH` | math | `DUP EXP SWAP CHS EXP - 2. /` |
| `COSH` | math | `DUP EXP SWAP CHS EXP + 2. /` |
| `TANH` | math | `DUP SINH SWAP COSH /` |
| `ASINH` | math | `DUP DUP * 1. + SQRT + LN` |
| `ACOSH` | math | `DUP DUP * 1. - SQRT + LN` |
| `ATANH` | math | `DUP 1. + SWAP CHS 1. + / LN 2. /` |
| `SQ` | math | `DUP *` — migrated from C++ |
| `HYPOT` | math | `SQ SWAP SQ + SQRT` — migrated from C++ |
| `!` / `FACT` | math | `1. + GAMMA` — generalizes to real args via gamma function |
| `nCr` | math | `OVER OVER - ! SWAP ! * SWAP ! SWAP /` |
| `nPr` | math | `OVER OVER - ! SWAP DROP SWAP ! SWAP /` |
| `GCD` | math | Euclidean via `BEGIN`/`UNTIL` with `IF`/`ELSE`/`END` for b=0 base case |
| `LCM` | math | `OVER OVER GCD / *` |
| `DUP2` | stack | `OVER OVER` — migrated from C++ (`sync_eval`) |
| `DROP2` | stack | `DROP DROP` — migrated from C++ (`sync_eval`) |
| `VEC3->{xy}` | geometry | `VEC3-> DROP ->VEC3y SWAP ->VEC3x +` — migrated from C++ |
| `->{xy}` | geometry | `->VEC3x SWAP ->VEC3y +` — migrated from C++ |

**C++ word audit:** Complete.  Remaining C++ words that could theoretically move to RPL (`CHS`, `INV`, `D->R`, `R->D`, `LN2`) were left in C++ because they support both integer and double inputs; moving them would silently drop integer type handling.  `->VEC3x/y/z` remain in C++ (need `std::nan` directly).  All other simple composites have been migrated.

---

## Phase 4 — Advanced Mathematics (Complete)

| # | Task | Complexity | Notes |
|---|---|---|---|
| 4.0 | `v_numbertype` word audit: review all existing word definitions and replace patterns that use `StackSizeValidator` + `pop_as_{integer,double}` with no real type checking, or duplicate `d1_integer` / `d1_double` registrations for the same word, with `d1_number` / `d2_number_number` validators where both integer and double are semantically acceptable. Keep `d1_integer` where strictly integer semantics are required (binary ops, radix, wordsize). Validator infrastructure already in place. **Done.** Added `ADD_NATIVE_1_FLOAT_WDEF` / `ADD_NATIVE_2_FLOAT_WDEF` macros; collapsed trig, transcendentals, ATAN2, `->VEC3x/y/z`, `->PRECISION`, `->RADIX`. | S | Done |
| 4.1 | Binary ops enhancement: RLEFT, RRIGHT (rotate), STWS. **Done.** RLEFT/RRIGHT circular rotate within wordsize; STWS/RCWS HP48 aliases for ->WORDSIZE/WORDSIZE->. | S | Done |
| 4.2 | Matrix type: `stack::Matrix` (NxM), `->MATRIX`, `+`, `-`, `*`, determinant, transpose, inverse, scale. Connect to `StVec3`. `stack::Matrix` / `stack::Vector` in `src/rpn-matrix-types.h` (Techsoft `matrix.h` backend); core words in `src/matrix-dict.cpp`. Backend stays on Techsoft until Phase 4.5 Eigen migration. **Done.** | L | Done |
| 4.3 | Literal vector/matrix entry: `[`/`]` collects into `stack::Vector`; `{`/`}` collects into `stack::Object`. Non-conformant elements → `StArray`. **Done.** | M | Done |
| 4.4 | `stack::Vec3` and `stack::Mx3` completeness via MI on `q::Vec3`/`q::Mx3`. **Done.** | S | Done |
| 4.5 | Eigen migration: migrate matrix backend from Techsoft Matrix TCL Lite (`src/matrix.h`) to Eigen (header-only, actively maintained). Add EIGENVAL and full decomposition (`SelfAdjointEigenSolver` / `EigenSolver`). DET, INV, TRANS already in 4.2; 4.5 adds EIGENVAL. Promote `src/rpn-matrix-types.h` to public `rpn-matrix.h` at repo root (Eigen dep stays out of `rpn.h`). **Done.** | L | Done |
| 4.6 | Statistics: MEAN, VARIANCE, STDDEV, LINFIT, CORRELATION on Array. OLS (ordinary least squares) regression: beta estimators, t-stats, TSS/RSS/ESS, adjusted R², degrees of freedom, fitted values, residuals. **Done.** | M+ | Done |
| 4.7 | CAS: symbolic differentiation, integration, simplification. Spun out to `docs/cas-plan.md`. | XL | See cas-plan.md |

---

## Phase 5 — Units

Spun out to `docs/units-plan.md`.

---

## Phase 6 — Testing

### 6.1 Structured Validator Tests

`tests/validator-tests.h` has ~200 hand-written cases as a static map. Wrap into parameterized Catch2 `TEST_CASE`s with descriptions so failures are identifiable.

### 6.2 Comprehensive Word Tests

TDD for all Phase 1 control flow (write tests before implementation). Test error paths. `DEPARSE` round-trip tests. Per-dict coverage.

One pre-existing failure in `runtime-test.cpp` test case 17: the `tests.rpn` parse test uses a relative file path (`"tests.rpn"`) that only resolves when the test binary is run from the `tests/` directory. Not a code bug.

---

## Documentation

See `docs/developer-guide.md`.

The word reference section will be substantially auto-generated once Phase 2.1 (word introspection API) is complete.

---

## Build & Infrastructure

| Item | Notes |
|---|---|
| CMake structure | `src/CMakeLists.txt` defines the `rpn-lang` static library. Top-level `CMakeLists.txt` does `add_subdirectory(src)` + `add_subdirectory(tests)`. `tests/CMakeLists.txt` links against `rpn-lang` target (does not recompile sources). **Keep `src/CMakeLists.txt` and `Package.swift` in sync when adding/removing source files.** |
| CMakePresets | `CMakePresets.json` at repo root. Presets: `debug` (Ninja, `build/`), `release` (Ninja, `build-release/`), `xcode` (Xcode generator, `build-xcode/`). Usage: `cmake --preset debug && cmake --build --preset debug`. |
| SwiftPM package | `Package.swift` skeleton — not yet buildable. Architecture: Swift C++ interop (Swift 5.9+) imports `RpnInterp` directly as a C++ class — no ObjC bridge, no `.mm`. The RP-42 Xcode project continues to use `rpn-hl.mm` (ObjC++ bridge) independently. **Remaining work:** create `include/` with a public header; `rpn-hl.h` exposes `rpn::WordHelp` and `nlohmann::json` return types which Swift's C++ importer may not handle cleanly — a thin `RpnInterp.h` wrapping only `std::string`/`std::vector` types may be needed. |
| SIOF / static init | All validator statics and global `rpn::Interp` instances in test/consumer code must use construct-on-first-use (Meyers singleton) or be initialized after `main()` starts. Namespace-scope validator statics in library `.cpp` files are safe as long as they are not referenced by other static initializers outside the library. |

---

## Possible Future Work

Items considered but not scheduled.  Revisit if requirements emerge.

| Item | Notes |
|---|---|
| WASM build target | Technically feasible; no current use case. |
| C ABI wrapper (`rpn_c.h`) | Would enable non-C++ bindings; likely needed as part of SwiftPM packaging (see Build & Infrastructure). |
| MCP custom tool registration | `rpn::McpServer` currently exposes a fixed tool set (`eval`, `stack`, `clear`, `load_file`, `word_list`, `word_help`). Embedders with custom word extensions should be able to register additional MCP tools with their own names, descriptions, and input schemas — so Claude sees them as first-class tools rather than having to go through `eval`. API sketch: `McpServer::addTool(name, description, inputSchema, handler)`. |
| MCP TCP bridge via socat | Apps that embed `rpn::McpServer` on a TCP port can be reached from Claude Desktop / Claude Code using `socat - TCP:localhost:<port>` as the MCP command. No code changes needed; document this pattern in the developer guide. |

---

## Open Design Decisions

| Topic | Decision / Status |
|---|---|
| C++ macros vs templates | Keep macros. They serve their purpose; a hybrid would require two mental models simultaneously with no net gain. |
| `TStackObject<T>` retention | Removed. MI (inherit `rpn::Stack::Object` + domain class) is the extension path. Documented in developer guide §9. |
| STO/RCL lookup precedence | HP48 convention: variables shadow dictionary words. Implemented. |
| Trig mode propagation | `AngleMode` enum in `rpn.h`; public `angleMode()`/`setAngleMode()` on `Interp`; math words call `rpn.angleMode()`. Implemented. |
| JSON type vs JSON words | `StJson` (MI: `Stack::Object + nlohmann::json`) as first-class type. `to_json()` returns full `{type,display,deparse,data}` descriptor on all types; `->JSON` / `JSON->` for stack interop. |
| Double → Number rename | Low urgency; no strong reason to rename. Leave as-is. |
| CAS library | Research phase. Evaluate SymEngine and GiNaC before any implementation. **Dependency note:** SymEngine uses Eigen for its numeric layer; GiNaC does not. The Eigen migration (Phase 4.5) is therefore a prerequisite for a SymEngine-based CAS. If GiNaC is chosen instead, 4.5 remains a prerequisite only for eigenvalue support. Resolve CAS library choice before starting 4.7. |
| Type overlap consolidation | Keep all three tiers. `StArray` / `StObject` are computation types — they hold live `Stack::Object` instances with full type fidelity (an `StFraction` in an array stays an `StFraction`). `StJson` is an interchange type — for `describeStack()`, file I/O, and external APIs. `StVector` / `StMatrix` are numeric-only linear algebra types. `->JSON` / `JSON->` are the explicit (lossy) crossing points. Symbolic arrays (Phase 4.4 CAS) are `StArray` of CAS expression objects — `StJson` cannot represent them. |
| Stack object naming convention | Resolved. All `using StX` aliases removed. Use `stack::X` everywhere. `stack::Complex` moved to `rpn.h`. `stack::Vector` / `stack::Matrix` moved to `src/rpn-matrix-types.h` (stays in `src/` until Phase 4.2 Eigen migration removes `matrix.h` dependency, then moves to `rpn.h`). `StVec3` unchanged — will be refactored to `stack::Vec3` in Phase 4.6 (vecmx.h rework). |
| Number type / integer coalescing | `v_numbertype` sentinel (value 1) added to `StrictTypeValidator`; matches `stack::Double` or `stack::Integer`. Registered as `"number"` in type registry. `d1_number` and `d2_number_number` validators added. `stack::Integer` is retained for words that require strictly integer semantics (binary ops, radix, wordsize). Word audit complete (Phase 4.0). |
| Header split for embedders | Two tiers. **Core types** live in `rpn.h` (Double, Integer, Boolean, String, Name, Object, Array, Json, Complex, Vec3) — only stdlib + vendored nlohmann deps, always available. **Domain types** each own their header and may include whatever they need; embedders opt in explicitly: `src/fraction.h` (q::Fraction dep), `src/timecode.h` (q::Timecode dep), `rpn-matrix.h` (Eigen — Phase 4.2), `rpn-cas.h` (SymEngine/GiNaC — Phase 4.4). `rpn.h` never gains a heavy dep. |
| UI / SwiftUI alignment | `describeStack()` returns a full JSON snapshot each call — sufficient for reactive UI. Stable per-object IDs are **not needed**: virtually every interesting word pops and pushes a new object anyway, so IDs would change on each operation regardless. Stack rearrangement ops (SWAP, ROT, etc.) are the only true in-place moves, but for a small calculator stack the cost of a full re-render on those is negligible. `stack::Object` is effectively immutable once pushed (nothing modifies objects after they land on the stack); making the stack hold `unique_ptr<const Object>` enforces this invariant. **Implemented:** `_stack` changed to `deque<unique_ptr<const Object>>`; `pop()` returns `unique_ptr<const Object>`; `peek()` returns `const Object &`; `PEEK_CAST` and `POP_CAST` both produce `const obtype&` automatically. |
| WHILE loop design | Single-block: all body+condition before WHILE/UNTIL; `__until` key distinguishes WHILE (exit when false) from UNTIL (exit when true). At-least-one-iteration semantics for UNTIL. Implemented. |
| FOR STEP design | `FOR ... n STEP` — body leaves step on TOS each iteration; STEP pops it. `_step = NaN` sentinel marks step-from-stack mode. NEXT = fixed step 1. Implemented. |
| StName vs StString for variables | `StName` (`'identifier'` literal) for variable names; `StString` (`"content"` literal) for data. `is_valid_name()` prevents shadowing. Implemented. |
| Runaway / deadlocked interpreter | Addressed in Phase 2.4. `cancel()` / `cancelAll()` interrupt running evals; `isCancelled()` for native long-running words. Watchdog / iteration-counter debug helpers not implemented — low priority. |
| Emacs calc feature gaps | Audited. Low-hanging fruit (hyperbolic trig, combinatorics) → RPL stdlib. Needs new types: FIX/SCI/ENG display modes, MAP/REDUCE on arrays, inf/nan as stack values, financial functions, probability distributions, curve fitting, polynomial ops. CAS gaps (symbolic algebra) → Phase 4.4. Units → Phase 5. |
| RPL stdlib architecture | Hybrid: C++ primitives + RPL compiled words. Stdlib loaded at Interp init from embedded raw string in `src/rpn-stdlib.cpp`. Typed via Phase 2.3 stack-effect comments. |

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
