# Plan: color-dict / ColorCalc Integration

## Status: Deferred — design needed

## Context

`libs/color-dict.{h,cpp}` (in the RP42 project) implements `ColorCalc`, which
adds color science words (`->RGB`, `->XYZ`, `->Lab`, color space conversions,
quantisation, delta-E, etc.) to the rpn-lang interpreter. It is currently wired
in via ObjC++ (`rp42-misc.mm`) and is not active.

## The Core Problem

`color-dict.h` includes `libQiColor/QiColor.h` and defines its own stack types
(`stack::Rgb`, `stack::XYZ`, `stack::Lab`, `stack::Mx3`, `stack::Vec3`, etc.)
using the MI pattern (`class stack::Foo : public rpn::Stack::Object, public q::Foo`).

The same pattern — and some of the same type names (`stack::Mx3`, `stack::Vec3`) —
now exists in `rpn-lang` itself (`rpn-matrix.h`). These are exact duplicates
originating from `libQiColor`.

Wiring `ColorCalc` into the interpreter currently causes:
- Name collisions: `stack::Mx3` and `stack::Vec3` defined in both `rpn-matrix.h`
  and `color-dict.h`
- Potential ODR violations if both translation units are linked

## Open Questions

- Should `rpn-lang`'s `stack::Mx3` / `stack::Vec3` be the canonical definitions,
  with `color-dict.h` removing its duplicates and using `rpn-matrix.h` instead?
- Or should `libQiColor` be the source of truth, with `rpn-lang` depending on it
  for these types?
- Are there other type overlaps beyond `Mx3` and `Vec3`?
- Does `libQiColor` need to become a SwiftPM dependency of rpn-lang, or does
  it stay as an RP42-side dependency only?

## Prerequisite

Resolve the type duplication strategy before any implementation.
The swift-interop work (see `docs/swift-interop-plan.md`) leaves a hook in
`makeRpnInterp()` for future ColorCalc registration once this is resolved.
