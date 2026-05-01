// swift/RpnLang.swift — Swift wrapper for RpnLangCXX
@_exported import RpnLangCXX

public struct StackItem {
    public let type: String
    public let display: String
    public let deparse: String
    public let latex: String
}

// MARK: - Internal bridging (Unmanaged + C function pointer)

private final class CompletionContext {
    let completion: (RpnInterp.Result) -> Void
    init(_ c: @escaping (RpnInterp.Result) -> Void) { completion = c }
}

private func completionCallback(result: Int32, ctx: UnsafeMutableRawPointer?) {
    let box = Unmanaged<CompletionContext>.fromOpaque(ctx!).takeRetainedValue()
    box.completion(RpnInterp.Result(rawValue: result) ?? .implementation_error)
}

// MARK: - Public Swift API

public extension RpnInterp {

    // Closure-based — the real bridge. All chicanery lives here.
    mutating func eval(_ line: String, completion: @escaping (RpnInterp.Result) -> Void) {
        let ctx = Unmanaged.passRetained(CompletionContext(completion)).toOpaque()
        eval(line, completionCallback, ctx)
    }

    // async/await — built on top of the closure overload.
    mutating func eval(_ line: String) async -> RpnInterp.Result {
        await withCheckedContinuation { continuation in
            eval(line) { continuation.resume(returning: $0) }
        }
    }

    mutating func stackItems() -> [StackItem] {
        describeStack().map {
            StackItem(type: String($0.type),
                      display: String($0.display),
                      deparse: String($0.deparse),
                      latex: String($0.latex))
        }
    }
}
