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
    dependencies: [
        // Eigen 3.4.0 as a SwiftPM package (header-only, re-published as
        // the elhernes/eigen fork).  URL-based so external consumers can
        // resolve; when this package is built under RP42 or another
        // umbrella that pins eigen locally via XCLocalSwiftPackageReference,
        // the local path reference overrides this URL by package-name match.
        //
        // CMake builds of this same source tree continue to use the
        // vendored third_party/Eigen tree — this dep is SwiftPM-only.
        .package(url: "git@github.com:elhernes/eigen.git", branch: "main"),
    ],
    targets: [
        // C++ library — module.modulemap at repo root controls what Swift sees
        .target(
            name: "RpnLangCXX",
            dependencies: [
                .product(name: "CEigen", package: "eigen"),
            ],
            path: ".",
            sources: cppSources,
            publicHeadersPath: "include",
            cxxSettings: [
                .headerSearchPath("."),
                // third_party still needed for nlohmann (vendored).  The
                // vendored Eigen subdirectory is also reachable through
                // this path but is shadowed by CEigen's publicHeadersPath,
                // which appears earlier on the include search order.
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
