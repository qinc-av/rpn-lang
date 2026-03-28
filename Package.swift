// swift-tools-version: 5.9
//
// Package.swift — rpn-lang SwiftPM package
//
// Status: SKELETON — not yet buildable.  See TODOs below.
//
// Architecture:
//   Swift → (ObjC bridge) → RpnInterp @interface (rpn-hl.mm)
//                         → C++ library (rpn-interp.cpp, etc.)
//
// The ObjC @interface RpnInterp in rpn-hl.h IS the Swift bridge.
// No C ABI wrapper is needed.
//
// The current bridging header in RP-42 does:
//   #import <rpn_lang/rpn-hl.h>
// which exposes the @interface directly to Swift.
//
// For SwiftPM, the public header for the module map must be ObjC-only.
// rpn-hl.h includes rpn.h (C++) unconditionally, which cannot appear in
// a SwiftPM clang module.  A separate rpn-hl-objc.h is needed that
// declares only the ObjC interface and imports only Foundation.
//
// TODO:
//   1. Create src/rpn-hl-objc.h — ObjC-only public header:
//        #import <Foundation/Foundation.h>
//        typedef NS_ENUM(NSInteger, RpnResult) { rpn_ok, rpn_parse_error, ... };
//        @interface RpnInterp : NSObject
//        - (id) init;
//        - (void) eval:(NSString*)line completionHandler:(void(^)(RpnResult))completionHandler;
//        ... (full interface from rpn-hl.h #if __OBJC__ section)
//        @end
//      This header has no C++ includes and is safe for a module map.
//
//   2. Add .gitignore entry for .build/ (SwiftPM build dir).
//
//   3. Verify Catch2 / test targets don't interfere with SwiftPM.
//
//   4. On first build: resolve header search path for third_party/ (nlohmann/json).
//      nlohmann is header-only so it can be vendored under Sources/ or referenced
//      via headerSearchPath.
//
//   5. Confirm cxx20 support on all target platforms (iOS 16+, macOS 13+).

import PackageDescription

let sources: [String] = [
    "rpn-stack.cpp",
    "rpn-interp.cpp",
    "types-dict.cpp",
    "math-dict.cpp",
    "stack-dict.cpp",
    "logic-dict.cpp",
    "fraction-dict.cpp",
    "fraction.cpp",
    "timecode-dict.cpp",
    "matrix-dict.cpp",
    "geometry.cpp",
    "geometry-dict.cpp",
    "keypad-dict.cpp",
    "rpn-stdlib.cpp",
    "rpn-hl.cpp",
    "rpn-hl.mm",   // ObjC++ bridge — must be compiled as ObjC++
].map { "src/" + $0 }

let package = Package(
    name: "rpn-lang",
    platforms: [
        .iOS(.v16),
        .macOS(.v13),
    ],
    products: [
        .library(name: "RpnLang", targets: ["RpnLang"]),
    ],
    targets: [
        .target(
            name: "RpnLang",
            path: ".",
            sources: sources,
            // TODO: replace with src/rpn-hl-objc.h once created (see TODO #1 above).
            // publicHeadersPath must be relative to path (".") and contain only
            // ObjC-compatible headers (no C++ includes).
            publicHeadersPath: "swiftpm-include",  // TODO: create this directory
            cxxSettings: [
                // Root dir for rpn.h, rpn-hl.h
                .headerSearchPath("."),
                // vendored nlohmann/json and other third-party headers
                .headerSearchPath("third_party"),
            ],
            linkerSettings: [
                .linkedFramework("Foundation"),
            ]
        ),
    ],
    cxxLanguageStandard: .cxx20
)
