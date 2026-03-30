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

#include <string>
#include <vector>
#include <functional>
#include "rpn-wordhelp.h"

namespace rpn {
  class Interp;
}

// One stack item as returned by RpnInterp::describeStack().
struct StackItem {
  std::string type;     // type_name() — "double", "integer", "vector", etc.
  std::string display;  // operator string() — human display
  std::string deparse;  // deparse() — round-trip rpn expression
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
  std::vector<std::string> displayStack();
  std::vector<StackItem> describeStack();

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
  rpn::Interp *_interp;
};


/* end of QInc/Projects/RP42/rpn-lang/rpn-hl.h */
