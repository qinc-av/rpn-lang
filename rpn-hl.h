/***************************************************
 * file: QInc/Projects/RP42/rpn-lang/rpn-hl.h
 *
 * @file    rpn-hl.h
 * @author  Eric L. Hernes
 * @born_on   Monday, March 25, 2024
 * @copyright (C) Copyright Eric L. Hernes 2024
 * @copyright (C) Copyright Q, Inc. 2024
 *
 * @brief   Swift/C++ interop boundary for rpn-lang.
 *          Only std::string, std::vector, and std::function — no Eigen, no nlohmann.
 */

#pragma once

#include <memory>
#include <string>
#include <vector>
#include <functional>

namespace rpn {
  class Interp;

  struct WordHelp {
    std::string name;
    std::string description;
    std::string category;
    std::vector<std::string> effects;  // one entry per overload
  };
}

// One stack item as returned by RpnInterp::describeStack().
struct StackItem {
  std::string type;     // type_name() — "double", "integer", "vector", etc.
  std::string display;  // operator string() — human display
  std::string deparse;  // deparse() — round-trip rpn expression
  std::string latex;    // to_latex() — for SwiftMath rendering in card views
};

class RpnInterp {
public:
  enum class Result {
    ok,
    parse_error,
    dict_error,
    param_error,
    eval_error,
    compile_error,
    implementation_error,
    cancelled,
  };

  RpnInterp(bool async);
  ~RpnInterp();

  void eval(std::string line, std::function<void(Result)> completionHandler);
  void parseFile(const std::string &path, std::function<void(Result)> completionHandler);

  // C-function-pointer overloads — for Swift async/await bridging (std::function
  // cannot accept Swift closures; these take plain C callbacks + context pointer).
  void eval(const char *line, void (*callback)(int result, void *ctx), void *ctx);
  void parseFile(const char *path, void (*callback)(int result, void *ctx), void *ctx);

  bool validateWord(const std::string &word);
  bool wordExists(const std::string &word);

  std::string status();
  std::vector<std::string> displayStack() const;
  std::vector<StackItem> describeStack();

  // Read-only display state.  Reflects engine state set via ->PRECISION /
  // ->RADIX / ->DEG / ->RAD / ->GRAD words.  Mutation only round-trips through
  // eval() — the engine owns this state.
  //   precision: significant decimal digits, 0–20
  //   radix:     integer display base (2, 8, 10, 16)
  //   angleMode: "DEG" / "RAD" / "GRAD"
  int precision() const;
  int radix() const;
  std::string angleMode() const;

  rpn::WordHelp wordHelp(const std::string &word) const;
  std::vector<std::string> wordList() const;

  void cancel();
  void cancelAll();
  bool isCancelled() const;
  void setProgressHandler(std::function<void(const std::string &message, double fraction)> handler);
  void reportProgress(const std::string &message, double fraction = -1.0);

  // Low-level access for C++ extension callers only — not intended for Swift.
  rpn::Interp &interp();

private:
  // Shared rather than unique ownership: Swift's C++ interop will copy a
  // value-typed RpnInterp when calling const methods (and the default copy
  // ctor is implicitly synthesized as shallow).  Holding _interp by
  // shared_ptr makes those copies safe — they share one engine instead of
  // shallow-copying a raw pointer that the temporary's dtor then deletes.
  std::shared_ptr<rpn::Interp> _interp;
};


/* end of QInc/Projects/RP42/rpn-lang/rpn-hl.h */
