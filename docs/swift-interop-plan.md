# Plan: Swift/C++ Direct Interop

## Status: In progress

## Goal

Replace the ObjC++ bridge layer with direct Swift 5.9+ C++ interoperability.
`RpnInterp` (C++ class in `rpn-hl.h`) is imported directly into Swift — no ObjC,
no bridging header, no `.mm` files.

Additionally, the RP42 project must be able to extend the interpreter with new
native C++ words (e.g. `ColorCalc` dict) without going through ObjC++.

---

## Current State

- **rpn-lang `Package.swift`**: skeleton, marked "not yet buildable". Sources listed;
  `publicHeadersPath = "include"` (symlinks to real headers). C++20.
- **rpn-lang `include/`**: symlinks — `rpn.h`, `rpn-hl.h`, `rpn-matrix.h`,
  `vecmx3.h`, `Eigen/`, `nlohmann/`. To be deleted.
- **rpn-lang `rpn-hl.h`**: dual-mode header; ObjC++ branch disabled
  (`#if __OBJC__ && 0`). C++ class `RpnInterp` is active. **Problematic API:**
  `describeStack()` and `wordHelp()` return `nlohmann::json` — not importable by
  Swift's C++ importer. `eval()` and `setProgressHandler()` take `std::function`.
- **RP42 Xcode project**: `SWIFT_OBJC_INTEROP_MODE = objcxx`, bridging header
  present (mostly commented out). `RP42App.swift` already imports `RpnLang` and
  calls C++ directly (`RpnInterp(false)`, `std.string(...)`, `displayStack()`).
- **`rp42-misc.mm`**: `addColorDictionary(RpnInterp*)` extends the interpreter via
  ObjC++ accessor. Currently unused (commented out).

---

## Problems to Solve

1. **`nlohmann::json` in public API** — not importable by Swift. Replace with plain
   C++ structs containing `std::string` fields.
2. **`std::function` callbacks** — `eval()` callback needs verification with Swift.
3. **Xcode project still in ObjC++ interop mode** — must switch to C++ interop mode.
4. **Extension mechanism** — RP42 needs to add native C++ words. Cannot go through
   the HL layer: native words require `rpn::Interp` and concrete `stack::` types.
   Must stay in C++.
5. **`include/` and module map** — pulls Eigen/nlohmann into Swift's importer.
   Delete `include/`; use explicit `module.modulemap`.

---

## Architecture Decision: No JSON at the C++ boundary

Instead of serialising `nlohmann::json` to `std::string` and parsing it in Swift,
expose plain C++ types that Swift can import directly. `StackItem` is a new struct
in `rpn-hl.h`; `rpn::WordHelp` already has only `std::string` and
`std::vector<std::string>` members — it moves to `rpn-hl.h` and is used directly.

```cpp
// rpn-hl.h
#include <string>
#include <vector>

namespace rpn {
  struct WordHelp {           // moved here from rpn.h
    std::string name;
    std::string description;
    std::string category;
    std::vector<std::string> effects;  // one per overload
  };
}

struct StackItem {
    std::string type;
    std::string display;
    std::string deparse;
};
```

`RpnInterp` API becomes:
```cpp
std::vector<StackItem>   describeStack() const;
rpn::WordHelp            wordHelp(const std::string &word) const;
std::vector<std::string> wordList() const;       // unchanged
std::vector<std::string> displayStack() const;   // unchanged
```

No JSON round-trip anywhere in the interface. `nlohmann::json` stays inside
`rpn-hl.cpp` as an implementation detail. `rpn.h` retains a `using` alias or
forward reference so internal library code is unaffected.

---

## Architecture Decision: Two-target Package.swift

A Swift target inside rpn-lang wraps the C++ types into idiomatic Swift and is
what consumers `import`. This keeps RP42 Swift code clean and makes rpn-lang
self-contained as a package.

```
RpnLangCXX  — clang target: all .cpp sources, rpn-hl.h public header
RpnLang     — Swift target: wraps RpnLangCXX, exposes Swift types
```

```swift
// Package.swift
targets: [
  .target(
    name: "RpnLangCXX",
    path: ".",
    sources: sources,         // all .cpp files
    publicHeadersPath: ".",   // module.modulemap at repo root
    cxxSettings: [
      .headerSearchPath("."),
      .headerSearchPath("third_party"),
    ]
  ),
  .target(
    name: "RpnLang",
    dependencies: ["RpnLangCXX"],
    path: "swift",            // new swift/ directory in rpn-lang
    swiftSettings: [.interoperabilityMode(.cxx)]
  ),
]
products: [
  .library(name: "RpnLang", targets: ["RpnLang"]),
]
```

`module.modulemap` at repo root (used by `RpnLangCXX`):
```
module RpnLangCXX {
    header "rpn-hl.h"
    export *
}
```

The `swift/` directory contains Swift source files that wrap `RpnLangCXX` types.
Kept minimal — type bridging and async/await wrapping only. `@Observable` and
higher-level model design are the consuming app's responsibility.

The interpreter is always created in async mode (`RpnInterp(true)`). The Swift
layer wraps the `std::function` completion handler into Swift `async/await` via
`withCheckedContinuation`:

```swift
// swift/RpnLang.swift
import RpnLangCXX

public struct StackItem {
    public let type: String
    public let display: String
    public let deparse: String
}

// rpn.WordHelp is imported directly from C++ — no Swift wrapper needed.
// Consumers access .name, .category, .description, .effects as std:: types;
// the extension below converts to Swift String/[String] at the call site.

public extension RpnInterp {
    func eval(_ line: String) async -> RpnInterp.Result {
        await withCheckedContinuation { continuation in
            eval(std.string(line)) { result in
                continuation.resume(returning: result)
            }
        }
    }

    func stackItems() -> [StackItem] {
        describeStack().map {
            StackItem(type: String($0.type),
                      display: String($0.display),
                      deparse: String($0.deparse))
        }
    }
}
```

If `std::function` cannot accept a Swift closure (determined in Task 1a spike),
the fallback is a C-linkage free function with a context pointer — never `evalSync()`:

```cpp
// rpn-hl.h (fallback only if std::function fails)
extern "C" {
  typedef void (*RpnCompletionFn)(int result, void *ctx);
  void RpnInterp_eval(RpnInterp *interp, const char *line,
                      RpnCompletionFn fn, void *ctx);
}
```

Swift wraps this with `@convention(c)` and `withCheckedContinuation`:

```swift
func eval(_ line: String) async -> RpnInterp.Result {
    await withCheckedContinuation { continuation in
        let ctx = Unmanaged.passRetained(continuation as AnyObject).toOpaque()
        RpnInterp_eval(self, line, { result, ctx in
            let c = Unmanaged<AnyObject>.fromOpaque(ctx!).takeRetainedValue()
            // resume continuation with result
        }, ctx)
    }
}
```

Async execution is non-negotiable; only the callback mechanism changes.

Delete `include/` entirely.

---

## Architecture Decision: Extension Pattern

Native word extensions require `rpn::Interp &` and the full `stack::` type
hierarchy — well below `RpnInterp`. Swift stays out of this entirely.

**Pattern:** RP42 defines a C++ factory that configures the interpreter before
handing it to Swift:

```cpp
// rp42-interp.cpp (in RP42 project)
#include "rpn-hl.h"
#include "rpn.h"
#include "color-dict.h"

RpnInterp *makeRpnInterp() {
    auto *i = new RpnInterp(false);
    add_colorWords(i->interp());   // low-level, C++ only
    return i;
}
```

`RpnInterp` exposes `rpn::Interp &interp()` for C++ callers only (not exposed
to Swift via the module map).

---

## Tasks

### Task 1a — Spike: verify `std::function` callback from Swift

**Prerequisite for Task 2 async wrapper design.**

Get `Package.swift` to a minimally buildable state (even with current `rpn-hl.h`),
then write a small Swift test that passes a closure to `eval()`:

```swift
let rpn = RpnInterp(true)  // async
rpn.eval(std.string("1. 2. +")) { result in
    print(result)
}
```

If this compiles and the closure fires: proceed with `withCheckedContinuation` wrapper.
If not: design the C-linkage fallback before proceeding with Task 1.

### Task 1 — Fix `rpn-hl.h`: replace JSON with C++ structs

**Files:** `rpn-hl.h`, `src/rpn-hl.cpp`

- Remove `#include "nlohmann/json.hpp"` from `rpn-hl.h`; keep in `rpn-hl.cpp` only
- Remove `#if __OBJC__` branch entirely
- Move `rpn::WordHelp` struct from `rpn.h` into `rpn-hl.h`; add `using` alias in
  `rpn.h` so internal library code is unaffected
- Add `StackItem` struct to `rpn-hl.h`
- Change `describeStack()` → `std::vector<StackItem>`
- Change `wordHelp()` → `rpn::WordHelp` (no wrapper struct needed)
- Add `rpn::Interp &interp()` accessor for C++ extension callers
- Keep `std::function` in `eval()` / `setProgressHandler()`
- **Spike first**: verify Swift can pass a closure to `std::function<void(Result)>`
  before committing to the async wrapper design (see Task 1a)

### Task 2 — Fix `Package.swift`, add `module.modulemap`, add `swift/` target

**Files:** `Package.swift`, `module.modulemap` (new), `swift/RpnLang.swift` (new)

- Rewrite `Package.swift` with `RpnLangCXX` + `RpnLang` targets (see above)
- Create `module.modulemap` at repo root
- Create `swift/` directory with Swift wrapper (see above)
- Delete `include/` directory

### Task 3 — Update Xcode project build settings

**File:** `RP42.xcodeproj` (edit via Xcode UI)

- Remove `SWIFT_OBJC_BRIDGING_HEADER`
- Change `SWIFT_OBJC_INTEROP_MODE` from `objcxx` → `cxx`
- Verify `CLANG_CXX_LANGUAGE_STANDARD = gnu++20` still set
- Confirm rpn-lang referenced as local SwiftPM package

### Task 4 — Add C++ factory in RP42

**Files:** new `RP42/rp42-interp.cpp`

- Write `rp42-interp.cpp` with `makeRpnInterp()` factory — creates `RpnInterp(true)`
  (async) with no extensions for now:
  ```cpp
  // rp42-interp.cpp
  #include "rpn-hl.h"

  RpnInterp *makeRpnInterp() {
      return new RpnInterp(true);
  }
  ```
- Add `rp42-interp.cpp` to the RP42 Xcode target
- Delete `rp42-misc.mm`
- **Note:** color-dict / ColorCalc integration is deferred to `docs/color-dict-plan.md`.
  The factory is designed to accept future extension calls before returning.

### Task 5 — Update `RP42App.swift` and callers

**File:** `RP42/RP42App.swift` and any other Swift files

- `import RpnLang`
- Call `makeRpnInterp()` instead of `RpnInterp(false)`
- Use `rpn.stack()` → `[StackItem]` instead of `displayStack()`
- `wordHelp()` returns `rpn.WordHelp` directly — access `.name`, `.category`,
  `.description`, `.effects` as C++ types and convert to Swift at the call site
- Use `await rpn.eval(cmd)` — async/await via Swift wrapper
- Stack updates read after `await` returns (completion handler has fired by then)

### Task 6 — Clean up ObjC++ artifacts

- Delete `RP42-Bridging-Header.h`
- Confirm `rp42-misc.mm` removed (Task 4)
- Update `development-plan.md` Build & Infrastructure section

---

## Verification

```bash
# SwiftPM build
swift build

# Xcode build
xcodebuild -project ../RP42.xcodeproj -scheme RP42 -destination 'platform=macOS'

# C++ tests still pass
cmake --build --preset debug
build/tests/stack-test && build/tests/runtime-test
```

---

## Open Questions

- **`std::function` from Swift** — resolved by Task 1a spike. If it fails, use
  C-linkage callback + context pointer as fallback. `evalSync()` is not an option;
  async execution is required.
- Should `makeRpnInterp()` have `extern "C"` linkage for cleaner Swift import, or
  is C++ linkage sufficient?
- color-dict / ColorCalc integration deferred — see `docs/color-dict-plan.md`.
  The deeper issue is type duplication between `libQiColor` and `rpn-lang`
  (`stack::Mx3`, `stack::Vec3` and others exist in both); that must be resolved
  before wiring ColorCalc into the interpreter.
