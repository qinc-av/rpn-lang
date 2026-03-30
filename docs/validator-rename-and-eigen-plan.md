# Plan: Eigen Migration Notes + Validator Convention Rename

## Status: Complete

---

## Task A — Update development-plan.md (doc only, no code)

### Change 1: Phase 4.2 row

**Find:**
```
| 4.2 | Linear Algebra: DET, INV, TRANS, EIGENVAL on Matrix | L | Requires 4.0 |
```

**Replace with:**
```
| 4.2 | Linear Algebra: EIGENVAL and full decomposition on Matrix. **Prerequisite: migrate matrix backend from Techsoft Matrix TCL Lite (`src/matrix.h`) to Eigen** (header-only, actively maintained, provides eigenvalue decomposition via `SelfAdjointEigenSolver` / `EigenSolver`). DET, INV, TRANS already implemented in 4.0; 4.2 adds EIGENVAL and the Eigen migration. | L | Requires 4.0; Eigen migration is also prerequisite for 4.4 |
```

### Change 2: Phase 4.4 row

**Find:**
```
| 4.4 | CAS: symbolic differentiation, integration, simplification. Evaluate SymEngine/GiNaC. Algebraic entry (`'expr'` literal syntax, implied multiplication, function calls) is a thin wrapper over the CAS parser — implement here, not separately. Remove `src/shunting-yard.cpp`. | XL | Research first |
```

**Replace with:**
```
| 4.4 | CAS: symbolic differentiation, integration, simplification. Evaluate SymEngine/GiNaC. Algebraic entry (`'expr'` literal syntax, implied multiplication, function calls) is a thin wrapper over the CAS parser — implement here, not separately. Remove `src/shunting-yard.cpp`. **Note: SymEngine uses Eigen for its numeric layer; the Eigen migration from 4.2 is a prerequisite.** | XL | Requires 4.2 (Eigen migration); Research first |
```

### Change 3: Open Design Decisions — CAS library row

**Find:**
```
| CAS library | Research phase. Evaluate SymEngine and GiNaC before any implementation. |
```

**Replace with:**
```
| CAS library | Research phase. Evaluate SymEngine and GiNaC before any implementation. **Dependency note:** SymEngine uses Eigen for its numeric layer; GiNaC does not. The Eigen migration (Phase 4.2) is therefore a prerequisite for a SymEngine-based CAS. If GiNaC is chosen instead, 4.2 remains a prerequisite only for eigenvalue support. Resolve CAS library choice before starting 4.4. |
```

---

## Task B — Rename validators to stack-effect order

### Why

Current naming is **TOS-first**: `d2_double_integer` means TOS=double, NOS=integer.
Stack-effect notation is **bottom-to-TOS**: `( integer double -- )`.
These are backwards relative to each other. Consequences:
- `wordHelp` effects display in wrong order (NOS and TOS swapped for all asymmetric words)
- `parse_input_types` builds compiled-word validators in TOS-first order, meaning a compiled word comment `( integer double -- )` creates a validator requiring TOS=integer — opposite of HP48 convention. Latent bug; no compiled words currently use asymmetric typed effects.
- Caused real swap bugs in Phase 4.0 matrix-dict.cpp.

### New convention

`d2_X_Y` means NOS=X, TOS=Y — matching `( X Y -- )`. Read the name exactly as the left side of the stack-effect comment. The `_types[]` array constructor order stays **TOS-first** (index 0 = TOS) — runtime matching is unchanged. Only identifier names change.

**Side-effect bonus:** `input_types()` strips the `dN_` prefix and replaces `_` with spaces. After the rename it automatically produces correct stack-effect-order strings for all native word `wordHelp` output. No separate regeneration step needed.

### Files to touch

1. `rpn.h` — `static const` declarations in `rpn::StrictTypeValidator`; `ADD_NATIVE_2_NUMBER_WDEF` and `ADD_NATIVE_3_NUMBER_WDEF` macro bodies
2. `src/rpn-interp.cpp` — `const rpn::StrictTypeValidator rpn::StrictTypeValidator::dN_...` definitions; fix `parse_input_types` (see Step 5)
3. `src/matrix-dict.cpp` — `matrix_validator::` namespace declarations + all `addDefinition` calls
4. `src/math-dict.cpp` — any `addDefinition` calls using asymmetric built-in validators (check ATAN2 overloads)
5. `docs/developer-guide.md` — add validator naming convention note to Section 5

### Rename table

#### Built-in validators — `rpn.h` + `src/rpn-interp.cpp`

All `d1_*` (unary) and symmetric names are **unchanged**.

| Old name (TOS-first) | New name (stack-effect order) |
|---|---|
| `d2_double_integer` | `d2_integer_double` |
| `d2_integer_double` | `d2_double_integer` |
| `d2_double_vec3` | `d2_vec3_double` |
| `d2_vec3_double` | `d2_double_vec3` |
| `d2_integer_vec3` | `d2_vec3_integer` |
| `d2_vec3_integer` | `d2_integer_vec3` |
| `d2_array_any` | `d2_any_array` |
| `d2_any_array` | `d2_array_any` |
| `d2_string_any` | `d2_any_string` |
| `d2_any_string` | `d2_string_any` |
| `d2_object_any` | `d2_any_object` |
| `d2_any_object` | `d2_object_any` |
| `d3_integer_double_double` | `d3_double_double_integer` |
| `d3_double_double_integer` | `d3_integer_double_double` |
| `d3_double_integer_integer` | `d3_integer_integer_double` |
| `d3_integer_integer_double` | `d3_double_integer_integer` |
| `d3_any_any_boolean` | `d3_boolean_any_any` |
| `d3_boolean_any_any` | `d3_any_any_boolean` |
| `d3_object_string_any` | `d3_any_string_object` |
| `d3_string_any_object` | `d3_object_any_string` |
| `d4_double_double_double_integer` | `d4_integer_double_double_double` |
| `d4_integer_double_double_double` | `d4_double_double_double_integer` |

Palindromes (reversed suffix = same): `d3_double_integer_double`, `d3_integer_double_integer` — **unchanged**.

#### Local `matrix_validator::` — `src/matrix-dict.cpp`

| Old name | New name |
|---|---|
| `d2_matrix_vector` | `d2_vector_matrix` |
| `d2_vector_matrix` | `d2_matrix_vector` |
| `d2_vector_double` | `d2_double_vector` |
| `d2_vector_integer` | `d2_integer_vector` |
| `d2_double_vector` | `d2_vector_double` |
| `d2_integer_vector` | `d2_vector_integer` |
| `d2_matrix_double` | `d2_double_matrix` |
| `d2_matrix_integer` | `d2_integer_matrix` |
| `d2_double_matrix` | `d2_matrix_double` |
| `d2_integer_matrix` | `d2_matrix_integer` |

`math_validator::d1_complex`, `timecode_validator::*`, `frac_validator::*` — check each; rename any asymmetric ones using the same rule.

### Critical: collision-safe rename procedure

Many pairs are mutual swaps (e.g. `d2_double_integer` ↔ `d2_integer_double`). A sequential find-and-replace will corrupt the code. Use a **two-phase approach**:

**Phase 1** — rename all asymmetric names to a temp prefix `_NEW_` in all files:
```
d2_double_integer  →  d2_NEW_integer_double
d2_integer_double  →  d2_NEW_double_integer
... (all asymmetric pairs)
```

**Phase 2** — strip `_NEW_` everywhere.

### Step 5 — Fix `parse_input_types`

After the rename, `parse_input_types` (rpn-interp.cpp ~line 314) reads a comment like `( integer double -- )` left-to-right, building `_types = [integer, double]`. Under the new convention that means `_types[0]=integer` = TOS=integer, but the comment intends TOS=double. Fix: **reverse the types vector** before constructing the validator:

```cpp
// After collecting tokens left-to-right:
std::reverse(types.begin(), types.end());
return types;
```

This makes compiled RPL words with typed stack-effect comments behave correctly in HP48 convention. Currently safe to add because no compiled word uses asymmetric fully-typed effects; the fix is latent-bug prevention.

### Step 6 — Developer guide update (Section 5)

Insert after "### 2. Register it" code block, before "### Shortcut macros":

---
**Validator naming convention**

`dN_X_..._Y` names read **left-to-right in stack-effect order** (bottom → TOS), matching the left side of a `( X ... Y -- )` comment:

- `d2_integer_double` → NOS=integer, TOS=double → `( integer double -- )`
- `d2_vector_double` → NOS=vector, TOS=double → `( vector double -- )`
- `d3_double_double_integer` → bottom=double, mid=double, TOS=integer → `( double double integer -- )`

The `_types[]` constructor array is indexed **TOS-first** (index 0 = TOS) — the reverse of the name order. Always construct as `{TOS_type, ..., NOS_type}` while naming `d2_NOS_TOS`. The same convention applies to local validators in dict namespaces (`matrix_validator::`, `math_validator::`, etc.).

Compiled RPL words with fully-typed stack-effect comments follow the same convention: `( integer double -- result )` generates a validator requiring NOS=integer, TOS=double.

---

### Step 7 — Verification

```bash
cmake --build build-tests
./build-tests/stack-test
./build-tests/runtime-test
```

The validator test case (`TEST_CASE("validators", "strict-type")`) directly exercises every built-in validator with a stack generator string. After the rename all 5324 assertions must remain green. The generator strings in the test file must also be updated to match the new names.

**Note:** The validator test generator strings themselves encode which stack state satisfies each validator. After the rename the generator strings need to be swapped to match — e.g. the entry for `d2_integer_double` (new: NOS=integer, TOS=double) should use `"2_ 3.1415"` (push integer first, then double on top), which was previously the entry for old `d2_double_integer`. The test file is the ground truth for verifying the rename is correct.
