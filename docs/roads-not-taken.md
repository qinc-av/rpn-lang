# rpn-lang — Roads Not Taken (Now or Yet)

A companion to `rpn-lang-llm-design-notes.md`. Captures the design
directions explored in conversation but deliberately parked, along with
the rationale for parking each — so they're not lost and so the bar for
revisiting them is on the record.

The general posture: rpn-lang's current shape works for its current
embedders (RP42 today; a colorimeter and a CNC pendant on the
near-horizon). Speculative infrastructure for hypothetical consumers
gets parked until a real consumer surfaces its need; well-scoped
documentation moves stay on the table because they pay off either way.

---

## Active vs. parked

Two pieces are sized and ready to scope on demand, **not** in this
document because they're the road we're *taking* next:

- **`canFire` predicate + greyed-out keyword grid** (~2–3 days).
  Uses today's `StrictTypeValidator`. Surfaces inputs visually; the
  long-press tooltip surfaces "wants" details when greyed.
- **Named-typed stack effects in metadata** (~1 week including dict
  population). `( cashflows:vector rate:number -- npv:number )` becomes
  the canonical signature form — type carries the shape, name carries
  the role. Documentation layer only; dispatch unchanged.

Everything below this line is *not* on the immediate path.

---

## 1. Static body verifier for user-defined words

Verify that a `:`-defined word's body actually produces the stack
effect its signature declares. Walk the tokens, look up each word's
validator-resolved overload, simulate inputs/outputs on a virtual stack,
confirm the body ends with the declared outputs.

**What it enables:**
- Definition-time type errors instead of runtime ones.
- Real refactor safety — replace a word with another of the same
  signature, callers are provably unaffected.
- Type-directed program synthesis (`compose_bridge`-style tooling) over
  a verified user-word corpus.
- "What signature does this body actually have?" inference for
  un-annotated user words.

**Why parked:**
- No demonstrated need today. Users defining 3–10-word procedures
  rarely produce signature/body mismatches that runtime errors don't
  catch in seconds.
- Real compiler work: control-flow handling (see §2), overload
  resolution, fixpoint pass for forward references / recursion.
- Pays off most clearly *with* a programmatic consumer of typed
  signatures (LLM tooling, refactor automation). Without that
  consumer, the value is "fewer typos in user words" — a real but
  bounded win.

**Trigger to revisit:**
A second real embedder building substantial named-procedure
vocabularies, *or* MCP / LLM tooling proving itself useful against the
native dictionary and demanding the same surface for user words.

---

## 2. Branch convergence & variable-arity loops in the verifier

The classical static-stack-effect verifier requires both arms of
`IF/THEN/ELSE` to leave the same shape and loops to be stack-neutral.
That rule excludes genuinely useful patterns:

- **Divergent branches** — `IF cond THEN happy-path ELSE error-path END`
  where the two arms produce different stack shapes (e.g., a result vs.
  an error code + flag).
- **Variable-arity loops** — `FOR ... NEXT` that fans out N items onto
  the stack, e.g. `bolt-circle ( n diam phase -- p₁ p₂ … pₙ )`.

**Why parked:**
- These are bundled with §1; no verifier yet means no enforcement
  question to answer yet.
- The pragmatic resolution, when it does come, is *soft typing*:
  - Words with declared signatures get verified where possible.
  - Words whose body contains a non-verifiable construct are recorded
    as "typed but unverified" — still typed at the *interface*
    (inputs/outputs), just not statically proven against their body.
  - Words without a declared signature are "untyped" — verifier
    doesn't touch them; they're opaque to type-directed tools.
- Worth noting: the variable-arity-loop case usually has a cleaner
  *idiomatic* fix — return one `array:array` of N items instead of N
  loose items. That matches the existing pattern for `AMORT`,
  `DEP-SL`, etc. Recommendation, not enforcement.

**Trigger to revisit:** Same as §1.

---

## 3. Dual-outcome / union-type signatures

The notation question for words whose output shape varies by path:

```
( amount:number from-year:int to-year:int -- adjusted:number | error:string )
```

**Why parked:**
- No corpus of "this word returns either X or Y" in the current
  dictionary — most words consistently return one shape, with
  `param_error` etc. signaled via the `WordDefinition::Result` return
  code rather than via the stack.
- Composition tooling would need to handle the variant — every
  downstream word in a sequence would have to disambiguate.
- Result codes already serve as the "this path failed" signal cleanly
  enough.

**Trigger to revisit:** A real word in any embedder that genuinely
needs to communicate a variant result on the stack itself.

---

## 4. Locals — `LOCALS|`-style binding inside word bodies

Named stack effects suggest a natural next step: bind the names as
*local variables* inside the body, so the user can write
`( a:number b:number -- sum:number ) a b + ;` instead of pure-stack
juggling. Forth dialects have `LOCALS|`; many people find them clearer.

**Why parked:**
- Pure ergonomic enhancement, not a capability gain. The signature
  documentation lands without this.
- Distinct from the bigger type system work — could be added
  independently, but probably worth doing after enough user words
  exist to see whether the named-stack approach is actually painful.
- Risk of cultural drift: locals make stack code look like procedural
  code with extra steps. Rare; usually a sign to factor smaller.

**Trigger to revisit:** Real user feedback that pure-stack notation is
the friction point in writing named procedures — not before.

---

## 5. The category-stamping inconsistency

`addDefinition` stamps `_wordMetadata[word].category` on the *first*
definition of a name. `addWordMetadata` doesn't override it. For words
that gain their canonical metadata from a different dict than their
first definition site, the surfaced category is wrong:

- `OBJ->` is owned (by description) by `types-dict` but defined first
  by `math-dict`'s complex overload → displayed category is "math".

**Why parked:**
- Cosmetic; the description (the high-visibility field) is now correct
  after the owner-writes / extender-skips cleanup.
- Fix is small and well-scoped: let `addWordMetadata` overwrite the
  category when the canonical-owner writes it (either by adding an
  explicit "this is the owner" flag, or by treating any non-empty
  `_currentCategory` at metadata-write time as authoritative).

**Trigger to revisit:** Either a UI surface that filters by category
(today's keyword-grid groups by category already — worth checking that
it surfaces words correctly even when their category-stamp is "wrong"),
or any pass where the cosmetic fix is convenient to fold in.

---

## 6. `compose_bridge` MCP tool

"Given current stack state and target stack state, return candidate
word sequences." The most ambitious MCP tool from
`rpn-lang-llm-design-notes.md` §4.

**Why parked:**
- Program synthesis under a type system. Search space explodes for
  any non-trivial gap.
- Will disappoint at the description level — users will type hard
  goals and lose faith.
- Needs the static verifier (§1) to filter type-invalid candidates
  before the LLM ever sees them.

**Trigger to revisit:** §1 lands, MCP layer proves itself with the
single-step suggester (`available_words`), there's appetite for the
multi-step search.

---

## 7. `available_words` MCP tool (single-step suggester)

"What words can fire right now, given the current stack?" Reachable —
shares the `canFire` predicate from the planned grey-out work.

**Why parked (lightly):**
- Not parked on capability — the predicate is the same one greying
  out the keyword grid. Parked on *plumbing*: nobody's standing up an
  MCP layer yet.
- Could ship literally the day someone wires up an MCP server; the
  data layer is ready.

**Trigger to revisit:** First MCP-using embedder lands (CNC pendant
agent, colorimeter helper, RP42 voice assistant — whatever).

---

## 8. Persistent named procedures

The LLM design notes treat named procedures as *the* authored artifact
(§7) — but `:`-defined words today only exist for the lifetime of the
`rpn::Interp` instance. Where do procedures *live* between sessions?

Open questions:
- Storage format — source text only, or serialized AST with verifier
  state attached?
- Location — per-instance (file alongside the controller), per-user
  (export/import), per-app (bundled with RP42)?
- Reload — at startup, on-demand, lazy?
- Conflict resolution — what happens if a user-defined `MAX` shadows
  a native word, or if a procedure depends on a dict that's no
  longer registered?

**Why parked:**
- Not a current friction point — RP42 users aren't building
  substantial procedure libraries (yet).
- Real design work whose answers depend heavily on the embedder
  (CNC's needs differ from a colorimeter's).

**Trigger to revisit:** The first embedder where users *want* to
preserve procedures across sessions — likely the CNC pendant.

---

## 9. CMake-based test build for rpn-color

Currently rpn-color's deparse round-trip coverage lives in an XCTest
target (`Tests/RpnColorTests/`). Adding a CMake + Catch2 path would
match rpn-lang's per-domain test pattern and unblock a future C++-only
consumer.

**Why parked:**
- No C++-only consumer of rpn-color exists. Per the SwiftPM-vs-CMake
  conversation, building speculative infrastructure for a hypothetical
  consumer is the path with the worst track record.
- The XCTest target works, runs in <1s, covers all ten color types.

**Trigger to revisit:** First C++-only rpn-color consumer announces
itself (e.g., a CNC-pendant build that doesn't want Swift). At that
point the test-target port is mechanical — the tests drive the engine
at the RPN-eval level, so XCTest → Catch2 is largely
`XCTAssertEqual` → `REQUIRE`.

---

## 10. Variable-arity-loops → array-return refactor pattern

The Forth tradition is "push N items onto the stack." The modern
typed-stack tradition (Factor and others) is "collect N items into an
array, push the array." The latter is dramatically easier on every
downstream surface — signatures, type checking, MCP tools, composition.

**Why parked (as a hard recommendation):**
- It's a cultural shift, not a code change. Existing words like
  `bolt-circle` work fine in the loose-stack form.
- The named-typed signatures (planned work) implicitly encourage the
  array form — words with `array:array` outputs read better than
  variadic ones.
- Forcing the rule would be disruptive; offering it as guidance is
  cheap and lands naturally as the typed-signature corpus grows.

**Trigger to revisit:** Active guidance to add to whatever style guide
rpn-lang gets when the LLM/MCP layer materializes — both for humans
and for any LLM proposing new word definitions.

---

## Summary table

| # | Topic | Where the value lives | Why parked |
|---|---|---|---|
| 1 | Static body verifier | Definition-time errors; LLM type-directed search | No verifier consumer yet |
| 2 | Branch / loop verification | Coverage of §1 across more patterns | Bundled with §1 |
| 3 | Union-type signatures | Words with variant outputs | No corpus needing it |
| 4 | LOCALS-style bindings | Author ergonomics | Pure ergonomics; revisit on real friction |
| 5 | Category-stamping fix | Correct category display for shared words | Cosmetic; description already correct |
| 6 | `compose_bridge` MCP tool | Multi-step program synthesis | Hard; gated on §1 + simpler MCP first |
| 7 | `available_words` MCP tool | Single-step suggestion | Data ready; needs MCP plumbing |
| 8 | Persistent named procedures | Cross-session procedure libraries | Embedder-dependent; no friction yet |
| 9 | rpn-color CMake test build | C++-only consumer support | No such consumer |
| 10 | Array-return refactor pattern | Cleaner signatures + composition | Cultural; encourage when typed sigs land |

---

## The shape of the bet

This document and `rpn-lang-llm-design-notes.md` between them encode a
single posture: **rpn-lang is at the interaction layer, not the
authoring layer; its type system earns its keep at the user-facing
edges (UI affordances, MCP tools, documentation) more than at the
authoring edges (verifier, locals).** Investments that pay off in the
former are on the table; investments that pay off mostly in the
latter are parked until something downstream concrete demands them.

If that posture turns out wrong — if the colorimeter or CNC work
generates a flood of authored user procedures, or if an MCP integration
demands the static verifier sooner than expected — the parked items
above are designed to be picked up individually, in roughly the order
the table lists them. Nothing here forecloses anything else.
