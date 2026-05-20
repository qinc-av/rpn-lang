# Plan: color-dict / ColorCalc Integration — Superseded

## Status: Superseded

The original plan — wiring an in-RP42 `libs/color-dict` into the
interpreter through an ObjC++ bridge (`rp42-misc.mm`), and resolving
`stack::Mx3` / `stack::Vec3` type duplication first — was not carried
out as written. It is kept here only as a record of how the problem
was actually resolved.

## What happened instead

color-dict became `rpn-color/`, a standalone SwiftPM package consumed
by RP42 as a git submodule. Its C++ target (`RpnColorCXX`) compiles
`color-dict.cpp` plus the `libQiColor` backing library (a nested
submodule); its modulemap exposes a single Swift entry point,
`addColorDictionary(RpnInterp)`. There is no ObjC bridge and no `.mm`
anywhere — RP42 is pure Swift consuming SwiftPM packages.

## Type-duplication resolution

`rpn-lang` owns the canonical `stack::Mx3` / `stack::Vec3`
(`rpn-matrix.h`). color-dict keeps its own qicolor-backed
`q::color::Mx3` / `q::color::Vec3` and bridges to rpn-lang's
`q::Mx3` / `q::Vec3` with element-wise `to_color()` / `to_stack()`
helpers — the two namespaces have identical APIs but cannot share a
dependency. No ODR collision: the types live in distinct packages
with distinct namespaces.

For current detail see RP42's `CLAUDE.md` and the `rpn-color/`
package's own README and docs.
