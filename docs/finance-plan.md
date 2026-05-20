# Plan: Financial Package (TVM, AMORT, IRR, …)

## Status

Designed — ready for implementation planning. Brainstormed 2026-05-19.

## Overview

A built-in financial-calculator dictionary for rpn-lang: time-value-of-money
solving, amortization, cash-flow analysis (NPV / IRR), interest-rate
conversion, depreciation, percentage math, and historical inflation
adjustment. It fills the "financial functions" gap noted in
`development-plan.md`'s Emacs-calc audit.

The package is **built into rpn-lang** (like `stats-dict`), not a separate
SwiftPM submodule — financial functions are general calculator
functionality, not app-specific.

## Scope

**v1 includes:**

- TVM — a `stack::Tvm` object plus a five-variable solver
- AMORT — amortization schedule
- NPV / IRR — cash-flow analysis over a `stack::Vector`
- Interest-rate conversion — nominal↔effective, continuous↔effective,
  Fisher real↔nominal
- Depreciation — straight-line, sum-of-years-digits, declining-balance
- Percentage math — `%`, `%CHG`, `%T`, markup / margin
- Historical inflation — embedded US CPI-U annual table + adjustment words

**Deferred (v2+):**

- Bonds (price / yield) — needs a calendar date type rpn-lang lacks
- Exchange rates / currency — belongs with the Phase 5 Units-of-measure
  concept (a currency is a unit); a finance-local currency type would
  conflict with that later work
- Non-US or loadable CPI index data — v1 embeds a US snapshot only

## Placement

| File | Contents |
|---|---|
| `src/finance.h` | the `stack::Tvm` domain type — opt-in header, pure math, no external dependency (pattern of `fraction.h` / `timecode.h`) |
| `src/finance-dict.cpp` | all words; the embedded US CPI-U table; the internal root-finder |
| `rpn.h` | declares `void addFinanceWords();` |
| `src/rpn-interp.cpp` | calls `addFinanceWords()` in the `Interp` constructor, near `addStatsWords()` |
| `src/CMakeLists.txt`, `Package.swift` | both gain `finance-dict.cpp` — keep in sync |
| `tests/runtime-test.cpp` | `[finance]` test group + `stack::Tvm` deparse round-trip case |

All C++, one dictionary — not a hybrid RPL-stdlib split: the TVM and
cash-flow formulas are interrelated and the root-finder must be C++.

## The `stack::Tvm` type

A `tvm` is a stack value with **seven fields**:

| Field | Type | Meaning |
|---|---|---|
| `n` | double | number of periods (fractional allowed) |
| `i` | double | interest rate, **percent per period** |
| `pv` | double | present value |
| `pmt` | double | payment per period |
| `fv` | double | future value |
| `begin` | bool | payments at period start (annuity due) vs end; default end |
| `solveFor` | enum `{none,n,i,pv,pmt,fv}` | which field is the computed unknown |

**Self-consistency invariant:** whenever `solveFor != none`, that field
always holds the TVM-equation solution of the other four. Every
constructor and setter produces a consistent object.

- A setter on an **input** field re-derives the `solveFor` field.
- A setter on the **`solveFor` field itself** sets it and clears
  `solveFor` to `none`.
- A `SOLVE-X` word sets `solveFor := X` and computes it.

Immutability: every operation returns a **new** `stack::Tvm`. "Modify and
recalculate" is "produce a new consistent object and replace on the
stack" — there is no in-place mutation. This is what lets the future
RP42 UI detail card edit a field and re-render live: the card calls a
setter word, gets back a new consistent `tvm`, and the `solveFor`
invariant means the dependent field is already updated.

`Stack::Object` overrides:

- `type_name()` → `"tvm"`
- `operator std::string()` → `TVM[n:360 i:0.5 pv:200000 pmt:-1199.10 fv:0 end]`
- `deparse()` → `360 0.5 200000 -1199.10 0 FALSE ->TVM`, followed by
  ` SOLVE-<X>` when `solveFor != none` — round-trips to an equal object
  (the `SOLVE` re-derives the already-consistent field, idempotently)
- `to_json()` → `{type, display, deparse, data:{n,i,pv,pmt,fv,begin,solveFor}}`
  — the legitimate interchange path; the RP42 UI detail card reads this
- `operator double()` → `NaN` (a TVM has no scalar value)
- `to_latex()`, `operator==` (field-wise), `deep_copy()` — standard

The TVM equation:

```
0 = pv + (1 + r·g)·pmt·(1 − (1+r)^−n)/r + fv·(1+r)^−n
```

where `r = i/100` (decimal periodic rate) and `g = 1` for begin-mode
else `0`. The `r = 0` case degenerates to the linear `0 = pv + pmt·n + fv`.
Closed-form solutions exist for `n`, `pv`, `pmt`, `fv`; `i` requires
iteration (see The solver).

## Word set

~34 words, all registered under a `finance` word category. Each ships an
`addWordMetadata` stack-effect string the RP42 UI surfaces on long-press.

### TVM

| Word | Stack effect | Notes |
|---|---|---|
| `TVM` | `( -- tvm )` | blank object (all 0, end, `solveFor` none) |
| `->TVM` | `( n i pv pmt fv begin -- tvm )` | full constructor; `deparse` emits this |
| `TVM-N` `TVM-I` `TVM-PV` `TVM-PMT` `TVM-FV` | `( tvm x -- tvm' )` | set an input field; re-derives the solve target |
| `TVM-BEGIN` `TVM-END` | `( tvm -- tvm' )` | payment timing |
| `SOLVE-N` `SOLVE-I` `SOLVE-PV` `SOLVE-PMT` `SOLVE-FV` | `( tvm -- tvm' )` | name that field the unknown and compute it |
| `TVM->` | `( tvm -- n i pv pmt fv begin )` | decompose (inverse of `->TVM`) |
| `AMORT` | `( tvm -- tvm obj )` | full amortization schedule (see Structured results) |

Setters and solvers return only the object — the solved value shows on
the object's display / card. There are no per-field getter words in v1;
`TVM->` covers extraction.

### Cash flow

| Word | Stack effect | Notes |
|---|---|---|
| `NPV` | `( cashflows rate -- npv )` | `cashflows` = `stack::Vector`, element 0 = t₀ |
| `IRR` | `( cashflows -- irr )` | iterative; returns percent per period |

### Interest-rate conversion

| Word | Stack effect | Notes |
|---|---|---|
| `NOM->EFF` `EFF->NOM` | `( rate periods -- rate' )` | nominal ↔ effective annual; `periods` = compoundings/year |
| `CONT->EFF` `EFF->CONT` | `( rate -- rate' )` | continuous-nominal ↔ effective (no period count — continuous is the limit) |
| `REAL->NOM` `NOM->REAL` | `( rate inflation -- rate' )` | Fisher equation `(1+nom) = (1+real)(1+infl)` |

Continuous compounding lives **only** here — the `stack::Tvm` solver
stays discrete-periodic, mirroring how HP financial calculators handle
it. `CONT->EFF` computes `eff = (e^(cont/100) − 1)·100`; the user feeds
the result to `TVM-I`.

### Depreciation

| Word | Stack effect | Notes |
|---|---|---|
| `DEP-SL` `DEP-SOYD` | `( cost salvage life -- obj )` | straight-line / sum-of-years-digits |
| `DEP-DB` | `( cost salvage life factor -- obj )` | declining-balance; `factor` e.g. `2.0` for double-declining |

### Percentage

| Word | Stack effect |
|---|---|
| `%` | `( base rate -- base·rate/100 )` |
| `%CHG` | `( old new -- percent-change )` |
| `%T` | `( total amount -- percent-of-total )` |
| `MU-COST` `MU-PRICE` | `( cost price -- markup% / margin% )` |

(No `%`-family words exist in `math-dict` or the RPL stdlib today — this
family is all new, no collisions.)

### Inflation

| Word | Stack effect | Notes |
|---|---|---|
| `CPI` | `( year -- index )` | US CPI-U annual index; `param_error` if the year is outside the table |
| `INFL-ADJUST` | `( amount from-year to-year -- adjusted )` | `amount · CPI(to) / CPI(from)` |

The CPI-U annual-average table (≈110 values, public-domain BLS data) is a
static array embedded in `finance-dict.cpp`, carrying a "current through
`<year>`" marker. It goes stale one value per year; refresh by editing
the array. US-only for v1.

The word-based interface (`CPI` / `INFL-ADJUST` on plain numbers) is a
deliberate v1 choice, not the conceptual model — see *Out of scope /
future work* for the units model these words will eventually fold into.

## Structured results — `stack::Object`, never `stack::Json`

`AMORT` and the `DEP-*` words return a **column-oriented `stack::Object`**:
named columns, each column a `stack::Vector`; rows are positional (period
index). This preserves full type fidelity and lets a column be pulled out
(`obj "interest" GET`) and fed straight to the `stats` words. `stack::Json`
is **never** produced by a finance word — it is an interchange type for
the outside world only.

```
AMORT  obj = { "period"          <Vector 1..n>
               "interest"        <Vector>
               "principal"       <Vector>
               "balance"         <Vector>
               "total-interest"  <Double>
               "total-principal" <Double> }

DEP-*  obj = { "period"       <Vector>
               "depreciation" <Vector>
               "accumulated"  <Vector>
               "book"         <Vector> }
```

`AMORT` reads the `tvm`'s current field values — the caller should solve
the `tvm` first.

## Conventions

- **Rates are percent-per-period everywhere** — `tvm.i`, the `NPV` / `IRR`
  rate argument, the `IRR` result, and every conversion word. Internally
  divided by 100. Annual↔periodic is the user's `5 12 /` or a conversion
  word.
- **Cash-flow signs** — money received is positive, money paid is
  negative. Documented in word help, not enforced (the equations are
  sign-agnostic given consistent signs).

## The solver

`SOLVE-I` and `IRR` need root-finding; everything else is closed-form. A
single internal static helper in `finance-dict.cpp`: Newton-Raphson with a
bisection fallback and a bracket scan for the initial sign change, a
~100-iteration cap and ~1e-9 tolerance. It is not exposed as a general
`SOLVE` word in v1.

## Error handling

`rpn::WordDefinition::Result::param_error` (never an exception across the
boundary) for:

- solver non-convergence (`SOLVE-I`, `IRR`)
- `IRR` cash flows with no sign change (no real IRR exists)
- `CPI` / `INFL-ADJUST` year outside the embedded table
- degenerate inputs — `life ≤ 0`, empty cash-flow vector, etc.

Multiple-IRR is a documented limitation: `IRR` returns the first root it
brackets.

## Testing

A `[finance]` group in `tests/runtime-test.cpp`:

- all five `SOLVE-*` directions, cross-checked against known HP-12C
  reference values
- begin vs end mode; the `i = 0` degenerate case
- `AMORT` schedule columns and totals
- `NPV` / `IRR` against textbook cases; the `IRR` no-solution path
- the six interest-conversion words
- the three depreciation methods
- `CPI` lookup, `INFL-ADJUST`, and the out-of-range `param_error`
- `stack::Tvm` added to the `deparse` round-trip `TEST_CASE` (project
  discipline for every new `stack::Object` subclass)

## Out of scope / future work

- **Bonds (price, yield)** — needs a calendar date type rpn-lang lacks.
- **Money as units (currency + inflation)** — exchange rates and historical
  inflation are the same problem: a *unit conversion* between members of a
  money-unit family (`USD`↔`EUR`, `USD_1923`↔`USD_2024`), the conversion
  factor coming from data (a rate table, a CPI series). Both belong with
  the Phase 5 Units-of-measure concept. v1 ships exchange rates not at all,
  and inflation as words (`CPI` / `INFL-ADJUST`) on plain numbers — a
  pragmatic interface, not the conceptual model. This is a concrete design
  input for Phase 5 Units: that feature must handle a non-dimensional,
  *data-driven* sub-kind of unit — most likely a `Quantity` carrying a
  vintage / epoch rather than ≈110 literal `USD_<year>` unit names. When
  Units lands, `INFL-ADJUST` becomes sugar over unit conversion (or is
  superseded); the embedded CPI table is exactly the data that version
  needs, so it is not wasted, and a plain-number `CPI` lookup stays useful
  regardless.
- **Non-US / loadable CPI data** — v1 embeds a US snapshot; a loadable
  multi-region table is future work.
- **A data-frame type** — rpn-lang has no R-style type with named rows
  *and* columns. The column-oriented `stack::Object` covers finance
  schedules because their rows are positional, but a true data-frame
  would also serve `stats`. Related: the existing `OLS` word returns a
  `stack::Json` and should move to `stack::Object` — a separate cleanup,
  not part of this package.
