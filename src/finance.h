/***************************************************
 * file: rpn-lang/src/finance.h
 *
 * @brief  stack::Tvm — the time-value-of-money domain type.
 *
 * A `tvm` carries the five TVM variables (n, i, pv, pmt, fv), a
 * begin/end payment-timing flag, and `solveFor` — the enum naming
 * which variable is the computed unknown. The type is pure data plus
 * serialization; the TVM math lives in finance-dict.cpp.
 */
#pragma once

#include "../rpn.h"
#include <string>

namespace stack {

class Tvm : public rpn::Stack::Object {
public:
  enum class SolveFor { none, n, i, pv, pmt, fv };

  double   n     = 0.0;   // number of periods
  double   i     = 0.0;   // interest rate, percent per period
  double   pv    = 0.0;   // present value
  double   pmt   = 0.0;   // payment per period
  double   fv    = 0.0;   // future value
  bool     begin = false; // payments at period start vs end
  SolveFor solveFor = SolveFor::none;

  Tvm() = default;

  virtual std::unique_ptr<rpn::Stack::Object> deep_copy() const override {
    return std::make_unique<Tvm>(*this);
  }

  virtual bool operator==(const rpn::Stack::Object &orhs) const override {
    const auto &rhs = PEEK_CAST(Tvm, orhs);
    return n == rhs.n && i == rhs.i && pv == rhs.pv && pmt == rhs.pmt
        && fv == rhs.fv && begin == rhs.begin && solveFor == rhs.solveFor;
  }

  virtual operator std::string() const override {
    return "TVM[n:" + rpn::to_string(n) + " i:" + rpn::to_string(i)
         + " pv:" + rpn::to_string(pv) + " pmt:" + rpn::to_string(pmt)
         + " fv:" + rpn::to_string(fv) + (begin ? " begin]" : " end]");
  }

  virtual std::string deparse() const override {
    std::string s = num(n) + " " + num(i) + " " + num(pv) + " "
                  + num(pmt) + " " + num(fv) + " "
                  + (begin ? "TRUE" : "FALSE") + " ->TVM";
    switch (solveFor) {
      case SolveFor::n:   s += " SOLVE-N";   break;
      case SolveFor::i:   s += " SOLVE-I";   break;
      case SolveFor::pv:  s += " SOLVE-PV";  break;
      case SolveFor::pmt: s += " SOLVE-PMT"; break;
      case SolveFor::fv:  s += " SOLVE-FV";  break;
      case SolveFor::none: break;
    }
    return s;
  }

  virtual std::string type_name() const override { return "tvm"; }

  virtual nlohmann::json to_json() const override {
    static const char *sf[] = {"none","n","i","pv","pmt","fv"};
    return {{"type", type_name()},
            {"display", (std::string)(*this)},
            {"deparse", deparse()},
            {"data", {{"n",n},{"i",i},{"pv",pv},{"pmt",pmt},{"fv",fv},
                      {"begin",begin},{"solveFor", sf[(int)solveFor]}}}};
  }

private:
  // Round-trip-safe double formatting for deparse: 17 significant
  // digits, and a trailing '.' when the result would otherwise read
  // as an integer literal (so it re-parses as a double).
  static std::string num(double v) {
    auto s = std::format("{:.17g}", v);
    if (s.find_first_not_of("-0123456789") == std::string::npos) s += ".";
    return s;
  }
};

} // namespace stack
