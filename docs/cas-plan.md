# Plan: CAS (Computer Algebra System)

## Status
Deferred — requires significant research before design.

## Context
Add symbolic algebra capabilities to rpn-lang. This is a large, open-ended feature
that needs scoping before implementation begins.

## Open Questions
- Which CAS backend? Options: GiNaC, SymEngine, FLINT, or hand-rolled subset.
- How does a symbolic expression live on the stack? New `stack::Expr` type?
- How do symbolic and numeric types interoperate (auto-promotion)?
- What words are in scope for v1: simplify, expand, factor, diff, integrate?
- How does CAS interact with the existing variable system (STO/RCL)?

## Prerequisite
Research CAS library options and decide on backend before writing any code.
