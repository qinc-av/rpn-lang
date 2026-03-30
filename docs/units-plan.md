# Plan: Units of Measure

## Status
Deferred — design needed.

## Context
Add physical unit tracking to numeric values (e.g., `3.0 kg`, `9.8 m/s^2`).

## Open Questions
- New stack type `stack::Quantity` wrapping a double + unit, or attach units as metadata?
- Unit representation: string annotation vs. structured dimension vector (SI base units)?
- How do arithmetic words handle unit propagation and validation (m + kg → error)?
- Conversion words: `->SI`, `->IMPERIAL`, or unit-to-unit `m->ft`?
- How does DEPARSE/EVAL round-trip a Quantity?
- Scope: just SI, or arbitrary user-defined unit systems?

## Prerequisite
Settle on the unit representation model before writing any code.
