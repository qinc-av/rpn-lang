// swift-tools-version: 5.9
//
// Package.swift — rpn-lang SwiftPM package
//
// Architecture:
//   RpnLangCXX  — clang target: all C++ sources, rpn-hl.h public surface
//   RpnLang     — Swift target: async/await wrapper, idiomatic Swift types
//
// Swift 5.9+ C++ interoperability — no ObjC bridge, no bridging header.
// module.modulemap at repo root exposes only rpn-hl.h + rpn-wordhelp.h to Swift.

import PackageDescription

let cppSources: [String] = [
    "rpn-stack.cpp",
    "rpn-interp.cpp",
    "types-dict.cpp",
    "math-dict.cpp",
    "stack-dict.cpp",
    "logic-dict.cpp",
    "fraction-dict.cpp",
    "fraction.cpp",
    "timecode-dict.cpp",
    "vecmx3.cpp",
    "matrix-dict.cpp",
    "mx3-dict.cpp",
    "vec3-dict.cpp",
    "stats-dict.cpp",
    "finance-dict.cpp",
    "geometry.cpp",
    "geometry-dict.cpp",
    "keypad-dict.cpp",
    "rpn-stdlib.cpp",
    "rpn-hl.cpp",
].map { "src/" + $0 }

let package = Package(
    name: "rpn-lang",
    platforms: [
      .iOS(.v17),
      .macOS(.v14),
    ],
    products: [
        .library(name: "RpnLang",    targets: ["RpnLang"]),
        // C++ target also exposed as a product so external C++ targets
        // (e.g. RpnColorCXX in rpn-color) can depend on it directly for
        // header visibility — not just transitively through RpnLang.
        .library(name: "RpnLangCXX", targets: ["RpnLangCXX"]),
    ],
    targets: [
        // C++ library — module.modulemap at repo root controls what Swift sees
        .target(
            name: "RpnLangCXX",
            path: ".",
            sources: cppSources,
            publicHeadersPath: "include",
            cxxSettings: [
                .headerSearchPath("."),
                .headerSearchPath("third_party"),
            ]
        ),
        // Swift wrapper — async/await, idiomatic types
        .target(
            name: "RpnLang",
            dependencies: ["RpnLangCXX"],
            path: "swift",
            swiftSettings: [
              .interoperabilityMode(.Cxx)
            ]
        ),
    ],
    cxxLanguageStandard: .cxx20
)
