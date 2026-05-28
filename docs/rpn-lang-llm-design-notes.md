# rpn-lang + LLM/MCP: Design Notes

A working document capturing the design philosophy for exposing rpn-lang
through an MCP interface, and the role we expect LLMs to play in interacting
with rpn-lang-based applications.

---

## 1. The role of rpn-lang in our systems

rpn-lang is positioned as **the interaction layer between an actor (human,
script, or LLM agent) and a stateful domain** — not as a primary authoring
medium for large systems.

This matches where stack languages have historically succeeded:

- **HP calculators**: RPN as the interface between fingers and arithmetic.
- **PostScript**: a wire format emitted by drawing software, rarely
  handwritten.
- **Forth in embedded work**: interactive prompt for talking to silicon
  during bring-up and debug.
- **G-code on CNC**: streaming command language, machine-generated with
  human tweaks.

The common thread: stack languages are excellent **glue between an actor
and a domain with state**. They are poor as a primary medium for large-scale
application authoring.

The shape of our target applications fits this niche cleanly:

- **5-channel LMS colorimeter**: an interactive measurement instrument with
  state (current integration time, channel gains, calibration, last reading)
  that wants compact command sequences.
- **CNC pendant / FluidNC controller**: jog axes, set offsets, run canned
  cycles, query state — again, an actor commanding a stateful machine.

In both cases, the stack itself usefully *is* the working memory of the
interaction: the last measurement, the current axis, the pending offset.
State that would be variables in a procedural language is "what's on the
stack right now," which matches how operators actually think mid-task.

## 2. The "write-only at the wrong granularity" principle

Stack code at the line level is genuinely write-only — debugging
`OVER SWAP ROT DUP` three words after the fact is brutal, for humans and
LLMs alike. Both fail at simulating non-lexical, positional stack state
across a long sequence.

But stack code at the **named-word level** is readable indefinitely.
`: SUM-OF-SQUARES ( a b -- n ) DUP * SWAP DUP * + ;` is fine forever.

**Implication for design discipline:**

- Factor mercilessly into small named words.
- Treat each word as the atomic unit of code.
- When editing, don't surgically patch — replace the whole word.
- Anchor every edit to known stack states at word boundaries.
- If a word is hard to edit, it's too big. Break it.

This discipline matches both:

- **How humans should write stack code** (per Chuck Moore, Leo Brodie, and
  fifty years of Forth practice).
- **How LLMs work best**: regenerate small self-contained artifacts with
  known interfaces, rather than reasoning about non-local edits to stateful
  sequences.

## 3. Why rpn-lang's typed stack-effect mechanism matters

rpn-lang has an extensive stack data-type matching mechanism, and the
stack-effect comments are tightly linked to validation logic. This is
unusual and important.

In traditional Forth, `( a b -- sum )` is a human convention — nothing
enforces it, and drift between comment and code is a major bug source. In
rpn-lang, the signature is **load-bearing**: a verifiable interface contract.

This unlocks several capabilities that matter for LLM-assisted use:

### Verification as both filter and feedback signal

The model can propose a word, and the checker can mechanically reject it
with precise diagnostics ("your signature claims `( a b -- sum )` but your
body leaves two items on the stack"). This is a tight generate-verify loop
that catches errors at the type level before execution.

### Type-directed synthesis

If signatures encode types (not just arity), then composing words becomes
type-directed search. A signature like `( n:int s:string -- s:string )`
tells the model not just shape but intent. The set of valid word sequences
between two stack states is constrained by the type system — many
combinations are statically invalid and can be pruned before the LLM ever
sees them.

This is analogous to Hindley-Milner type inference doing the heavy lifting
in ML-family languages. We get something similar in a stack language, which
is rare.

### Safe refactoring guarantees

Replacing a word with another of the same signature is type-safe by
construction. Callers' assumptions about stack shape can't be silently
broken.

## 4. What we want the MCP interface to expose

Given the above, the MCP tool surface should make rpn-lang's structure
machine-readable, not just dump a flat word list.

### Core tools

- **`vocabulary_query`** — return words filtered by signature. "What words
  produce a `vector:3d`?" or "What words consume `n:int n:int`?" gives the
  model a much smaller search space than the full dictionary.
- **`define_word`** — take a name, stack-effect signature, body, and
  docstring. Run the stack-effect checker before accepting. Reject with
  precise error messages on type mismatch.
- **`compose_bridge`** — given a current stack state and a target stack
  state, return candidate word sequences. Pre-filter to type-valid
  sequences; the LLM ranks by semantic plausibility.
- **`test_word`** — run a word against example input stacks and report
  resulting stacks. Cheap semantic verification beyond type-checking.
- **`run_sequence`** — execute a sequence against current state, return
  resulting stack. The primary "operator" tool.

### Design choices worth being explicit about

- **No in-place editing of words.** Replacement only. This matches both the
  factoring discipline and LLM strengths — they regenerate well, they patch
  badly.
- **Signature-first definition workflow.** The model declares the signature
  before writing the body. The body is type-checked against it. This matches
  how good typed-functional programmers work and gives the LLM a contract
  to satisfy.
- **Soft cap on word body length.** Warn or reject definitions over some
  small word count, forcing the factoring discipline. A 20-word definition
  is a smell.
- **Named canned sequences are first-class.** They are the real
  "programs" in our world — the measurement procedures, the canned cycles,
  the macros. The LLM should be able to invoke them, list them, and compose
  new ones from them.

## 5. What we are NOT asking the LLM to do

A clear-eyed scoping of the LLM's role saves a lot of wasted effort.

We are **not** asking the LLM to:

- Become a fluent Forth programmer.
- Write large rpn-lang applications from scratch.
- Author idiomatic deep-stack manipulation.
- Replace the procedural code (Swift, C++) that does the heavy lifting
  underneath.

We **are** asking the LLM to:

- Translate natural-language intent into short command sequences against
  the available vocabulary ("measure the white point on the center patch
  and store it as the reference" → a few words against the colorimeter
  domain).
- Compose existing words (including named canned sequences) to accomplish
  stated goals.
- Read the typed vocabulary and recognize when a problem doesn't map well
  to the available primitives.
- Propose new named procedures when the user is accumulating a repeated
  pattern.

This is closer to "translate English into a shell pipeline" or "into a SQL
query" than to "write me a program in language X." Current models handle
the former well in-context, with no fine-tuning required, provided the
domain vocabulary is exposed clearly.

## 6. The "low-resource language" question, briefly

Empirically, LLM competence in a programming language correlates with that
language's share of training data. Stack languages (Forth, PostScript) sit
far down the corpus curve. The literature on low-resource programming
languages suggests fine-tuning becomes useful at scales of tens of thousands
of validated function-docstring pairs for small models, while larger models
benefit more from in-context learning with translation examples and
few-shot prompts.

For rpn-lang specifically, **fine-tuning is the wrong investment**. The
right investment is:

1. A clean MCP that exposes typed signatures and semantic descriptions.
2. A handful of canonical example sessions in the system prompt — "here's
   how you'd accomplish a measurement," "here's how you'd zero an axis."
3. A tight verification path: model proposes a sequence, system shows what
   it would do or what's on the stack after, user/agent confirms.
4. First-class treatment of accumulated named procedures as the real
   authored artifacts.

In-context structure beats fine-tuned model knowledge for our use case,
because we're not asking the model to know rpn-lang — we're asking it to
operate a well-described instrument whose command language happens to be
stack-based.

## 7. The expected accumulation pattern

The actual authored artifacts will be **named procedures**, accumulated
over time:

- 3 to 10 words long, each.
- Single-purpose, with a clear signature and docstring.
- Composable: new procedures freely call existing ones.
- Domain-specific: measurement procedures for the colorimeter, canned
  cycles for the CNC, etc.

A working system will probably have on the order of 30-100 such named
procedures per domain. That is the real "program," and it's the right
granularity for stack code: small enough to read at a glance, large enough
to capture meaningful operations.

This is also exactly the corpus an LLM needs to be useful — both as
building blocks for new requests and as examples of idiomatic factoring in
the domain.

## 8. Architecture summary

| Layer                  | Language        | Role                                  |
|------------------------|-----------------|---------------------------------------|
| Heavy lifting          | Swift, C++      | Application logic, hardware I/O, math |
| Interaction layer      | rpn-lang        | Commands against stateful domain      |
| Named procedures       | rpn-lang        | Accumulated domain vocabulary         |
| Agent dispatch         | LLM via MCP     | NL intent → command sequences         |
| Verification           | rpn-lang types  | Type-check before execution           |

Each tool in its native role. The stack language earns its keep at the
interaction layer without being asked to do work it's bad at.
