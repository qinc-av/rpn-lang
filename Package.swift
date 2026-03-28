// swift-tools-version: 5.9
//
// Package.swift — rpn-lang SwiftPM package
//
// Status: SKELETON — not yet buildable. See TODOs below.
//
// Architecture:
//   Swift --[C++ interop]--> RpnInterp (C++ class in rpn-hl.h)
//                        --> C++ library (rpn-interp.cpp, etc.)
//
// Swift 5.9+ C++ interoperability is used directly — no ObjC bridge,
// no bridging header, no .mm files.  The C++ RpnInterp class (the
// #else branch of rpn-hl.h) is imported directly into Swift.
// Consuming Swift targets set .interoperabilityMode(.cxx) in their
// own swiftSettings.
//
// The RP-42 Xcode project uses rpn-hl.mm (ObjC++ bridge) for its own
// reasons; that is separate from and unaffected by this package.
//
// TODOs before this builds:
//   1. Create swiftpm-include/ directory with a public header.
//      rpn-hl.h currently exposes rpn::WordHelp and nlohmann::json
//      as return types, which Swift's C++ importer will attempt to
//      import.  Options:
//        a. Use rpn-hl.h directly and see how far Swift's importer gets
//           (nlohmann is heavily templated — likely problematic).
//        b. Create swiftpm-include/RpnInterp.h as a thin Swift-facing
//           header that wraps only std::string / std::vector return types
//           and hides rpn::WordHelp / nlohmann::json behind opaque types
//           or simplified equivalents.
//
//   2. Decide whether the Swift layer calls RpnInterp directly or wraps
//      it in a @MainActor Swift class for async/UI use.
//
//   3. Verify CXX_STANDARD 20 is honoured on all target platforms.

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
            publicHeadersPath: "swiftpm-include", // TODO: create — see TODO #1 above
            cxxSettings: [
                .headerSearchPath("."),            // rpn.h, rpn-hl.h
                .headerSearchPath("third_party"),  // nlohmann/json
            ]
        ),
    ],
    cxxLanguageStandard: .cxx20
)
