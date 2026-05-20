/***************************************************
 * file: rpn-lang/src/finance-dict.cpp
 *
 * @brief  Financial words: TVM (time value of money) plus the
 *         stack::Tvm constructors, solvers, setters, and AMORT.
 *         See docs/finance-plan.md.
 */
#include "../rpn.h"
#include "../rpn-matrix.h"
#include "finance.h"
#include <cmath>
#include <functional>

// ---------------------------------------------------------------------------
// TVM math helpers (anonymous namespace)
// ---------------------------------------------------------------------------
namespace {
  using ::stack::Tvm;

  inline double rate(const Tvm &t) { return t.i / 100.0; }
  inline double gmode(const Tvm &t) { return t.begin ? 1.0 : 0.0; }

  // present-value annuity factor; equals n when r == 0
  double annuity_factor(double n, double r) {
    if (r == 0.0) return n;
    return (1.0 - std::pow(1.0 + r, -n)) / r;
  }

  // residual of the TVM equation — zero when the five vars are consistent
  double tvm_residual(double n, double r, double pv, double pmt,
                      double fv, double g) {
    double A = annuity_factor(n, r);
    double D = std::pow(1.0 + r, -n);
    return pv + (1.0 + r * g) * pmt * A + fv * D;
  }

  double solve_pv(const Tvm &t) {
    double r = rate(t), D = std::pow(1.0 + r, -t.n);
    return -((1.0 + r * gmode(t)) * t.pmt * annuity_factor(t.n, r) + t.fv * D);
  }

  double solve_fv(const Tvm &t) {
    double r = rate(t), D = std::pow(1.0 + r, -t.n);
    return -(t.pv + (1.0 + r * gmode(t)) * t.pmt * annuity_factor(t.n, r)) / D;
  }

  double solve_pmt(const Tvm &t) {
    double r = rate(t), D = std::pow(1.0 + r, -t.n);
    double kA = (1.0 + r * gmode(t)) * annuity_factor(t.n, r);
    return -(t.pv + t.fv * D) / kA;
  }

  // returns NaN when there is no real solution (log of a non-positive)
  double solve_n(const Tvm &t) {
    double r = rate(t);
    if (r == 0.0) {
      return (t.pmt == 0.0) ? std::nan("") : -(t.pv + t.fv) / t.pmt;
    }
    double c = (1.0 + r * gmode(t)) * t.pmt / r;
    double denom = t.fv - c;
    if (denom == 0.0) return std::nan("");
    double D = -(t.pv + c) / denom;
    if (D <= 0.0) return std::nan("");
    return -std::log(D) / std::log(1.0 + r);
  }

  // Bracketed root-finder: scan [lo,hi] in `steps` intervals for a sign
  // change, then converge with secant + bisection fallback. Returns NaN
  // if no bracket is found or it fails to converge.
  double solve_root(const std::function<double(double)> &f,
                    double lo, double hi, int steps = 200,
                    int max_iter = 100, double tol = 1e-9) {
    double a = lo, fa = f(a), b = a, fb = fa;
    bool bracketed = false;
    double step = (hi - lo) / steps;
    for (int k = 1; k <= steps; ++k) {
      b = lo + k * step;
      fb = f(b);
      if (std::isfinite(fa) && std::isfinite(fb) && fa * fb <= 0.0) {
        bracketed = true;
        break;
      }
      a = b; fa = fb;
    }
    if (!bracketed) return std::nan("");
    for (int it = 0; it < max_iter; ++it) {
      if (std::abs(fb) < tol) return b;
      double c;
      if (fb != fa) {                       // secant step
        c = b - fb * (b - a) / (fb - fa);
      } else {
        c = std::nan("");
      }
      if (!std::isfinite(c) || c <= std::min(a,b) || c >= std::max(a,b)) {
        c = 0.5 * (a + b);                  // bisection fallback
        // Bracket collapsed to one representable value — b is the best
        // estimate (it carries the smaller residual). Returning here
        // avoids spinning out max_iter and yielding NaN.
        if (c == a || c == b) return b;
      }
      double fc = f(c);
      if (fa * fc <= 0.0) { b = c; fb = fc; }
      else                { a = c; fa = fc; }
    }
    return (std::abs(fb) < 1e-6) ? b : std::nan("");
  }

  // returns the rate as a percent, or NaN if it cannot be found
  double solve_i_percent(const Tvm &t) {
    double g = gmode(t);
    auto f = [&](double r) {
      return tvm_residual(t.n, r, t.pv, t.pmt, t.fv, g);
    };
    // periodic rates from -99% to +100% per period cover every
    // realistic case; r = 0 is handled inside annuity_factor.
    double r = solve_root(f, -0.99, 1.0);
    return std::isnan(r) ? std::nan("") : r * 100.0;
  }

  // Re-derive the solveFor field in place from the other four.
  // No-op when solveFor == none.
  void tvm_resolve(Tvm &t) {
    switch (t.solveFor) {
      case Tvm::SolveFor::none: break;
      case Tvm::SolveFor::n:    t.n   = solve_n(t);          break;
      case Tvm::SolveFor::i:    t.i   = solve_i_percent(t);  break;
      case Tvm::SolveFor::pv:   t.pv  = solve_pv(t);         break;
      case Tvm::SolveFor::pmt:  t.pmt = solve_pmt(t);        break;
      case Tvm::SolveFor::fv:   t.fv  = solve_fv(t);         break;
    }
  }

  // Apply a value to one field of a popped tvm: if that field is the
  // current solveFor, clear solveFor; otherwise re-derive solveFor.
  void tvm_set_field(Tvm &t, Tvm::SolveFor field, double value) {
    switch (field) {
      case Tvm::SolveFor::n:   t.n   = value; break;
      case Tvm::SolveFor::i:   t.i   = value; break;
      case Tvm::SolveFor::pv:  t.pv  = value; break;
      case Tvm::SolveFor::pmt: t.pmt = value; break;
      case Tvm::SolveFor::fv:  t.fv  = value; break;
      case Tvm::SolveFor::none: break;
    }
    if (t.solveFor == field) t.solveFor = Tvm::SolveFor::none;
    else                     tvm_resolve(t);
  }
} // anonymous namespace

// ---------------------------------------------------------------------------
// Local validators
// ---------------------------------------------------------------------------
namespace finance_validator {
  const rpn::StrictTypeValidator d1_tvm(
    { typeid(::stack::Tvm).hash_code() }, "d1_tvm");

  // ->TVM ( n i pv pmt fv begin -- tvm ) : TOS=boolean, the five
  // below are numbers. _types[0] is TOS.
  const rpn::StrictTypeValidator d6_tvm_args(
    { typeid(::stack::Boolean).hash_code(),
      rpn::StrictTypeValidator::v_numbertype,
      rpn::StrictTypeValidator::v_numbertype,
      rpn::StrictTypeValidator::v_numbertype,
      rpn::StrictTypeValidator::v_numbertype,
      rpn::StrictTypeValidator::v_numbertype },
    "d6_number_number_number_number_number_boolean");

  // ( tvm x -- tvm ) : TOS = number, NOS = tvm
  const rpn::StrictTypeValidator d2_tvm_double(
    { rpn::StrictTypeValidator::v_numbertype,
      typeid(::stack::Tvm).hash_code() },
    "d2_tvm_number");
}

// ---------------------------------------------------------------------------
// TVM  ( -- tvm )   blank time-value-of-money object
// ---------------------------------------------------------------------------
NATIVE_WORD_DECL(finance, tvm_new) {
  rpn.stack.push(::stack::Tvm{});
  return rpn::WordDefinition::Result::ok;
}

// ---------------------------------------------------------------------------
// ->TVM  ( n i pv pmt fv begin -- tvm )
// ---------------------------------------------------------------------------
NATIVE_WORD_DECL(finance, to_tvm) {
  bool   begin = rpn.stack.pop_boolean();
  double fv    = rpn.stack.pop_as_double();
  double pmt   = rpn.stack.pop_as_double();
  double pv    = rpn.stack.pop_as_double();
  double i     = rpn.stack.pop_as_double();
  double n     = rpn.stack.pop_as_double();
  ::stack::Tvm t;
  t.n = n; t.i = i; t.pv = pv; t.pmt = pmt; t.fv = fv; t.begin = begin;
  rpn.stack.push(t);
  return rpn::WordDefinition::Result::ok;
}

// ---------------------------------------------------------------------------
// TVM->  ( tvm -- n i pv pmt fv begin )
// ---------------------------------------------------------------------------
NATIVE_WORD_DECL(finance, tvm_explode) {
  auto sv = rpn.stack.pop();
  const auto &t = POP_CAST(::stack::Tvm, sv);
  rpn.stack.push_double(t.n);
  rpn.stack.push_double(t.i);
  rpn.stack.push_double(t.pv);
  rpn.stack.push_double(t.pmt);
  rpn.stack.push_double(t.fv);
  rpn.stack.push_boolean(t.begin);
  return rpn::WordDefinition::Result::ok;
}

// ---------------------------------------------------------------------------
// SOLVE-PV / SOLVE-FV / SOLVE-PMT / SOLVE-N  ( tvm -- tvm )
//   name a field the dependent unknown and compute it
// ---------------------------------------------------------------------------
NATIVE_WORD_DECL(finance, solve_pv_w) {
  auto sv = rpn.stack.pop();
  ::stack::Tvm t = POP_CAST(::stack::Tvm, sv);
  t.pv = solve_pv(t);
  t.solveFor = ::stack::Tvm::SolveFor::pv;
  rpn.stack.push(t);
  return rpn::WordDefinition::Result::ok;
}
NATIVE_WORD_DECL(finance, solve_fv_w) {
  auto sv = rpn.stack.pop();
  ::stack::Tvm t = POP_CAST(::stack::Tvm, sv);
  t.fv = solve_fv(t);
  t.solveFor = ::stack::Tvm::SolveFor::fv;
  rpn.stack.push(t);
  return rpn::WordDefinition::Result::ok;
}
NATIVE_WORD_DECL(finance, solve_pmt_w) {
  auto sv = rpn.stack.pop();
  ::stack::Tvm t = POP_CAST(::stack::Tvm, sv);
  t.pmt = solve_pmt(t);
  t.solveFor = ::stack::Tvm::SolveFor::pmt;
  rpn.stack.push(t);
  return rpn::WordDefinition::Result::ok;
}
NATIVE_WORD_DECL(finance, solve_n_w) {
  auto sv = rpn.stack.pop();
  ::stack::Tvm t = POP_CAST(::stack::Tvm, sv);
  double n = solve_n(t);
  if (std::isnan(n)) { rpn.stack.push(t); return rpn::WordDefinition::Result::param_error; }
  t.n = n;
  t.solveFor = ::stack::Tvm::SolveFor::n;
  rpn.stack.push(t);
  return rpn::WordDefinition::Result::ok;
}

// ---------------------------------------------------------------------------
// SOLVE-I  ( tvm -- tvm )
// ---------------------------------------------------------------------------
NATIVE_WORD_DECL(finance, solve_i_w) {
  auto sv = rpn.stack.pop();
  ::stack::Tvm t = POP_CAST(::stack::Tvm, sv);
  double i = solve_i_percent(t);
  if (std::isnan(i)) { rpn.stack.push(t); return rpn::WordDefinition::Result::param_error; }
  t.i = i;
  t.solveFor = ::stack::Tvm::SolveFor::i;
  rpn.stack.push(t);
  return rpn::WordDefinition::Result::ok;
}

// ---------------------------------------------------------------------------
// TVM-N / TVM-I / TVM-PV / TVM-PMT / TVM-FV  ( tvm x -- tvm )
// ---------------------------------------------------------------------------
static rpn::WordDefinition::Result
tvm_set_word(rpn::Interp &rpn, ::stack::Tvm::SolveFor field) {
  double value = rpn.stack.pop_as_double();
  auto sv = rpn.stack.pop();
  ::stack::Tvm t = POP_CAST(::stack::Tvm, sv);
  tvm_set_field(t, field, value);
  rpn.stack.push(t);
  return rpn::WordDefinition::Result::ok;
}
NATIVE_WORD_DECL(finance, tvm_set_n)   { return tvm_set_word(rpn, ::stack::Tvm::SolveFor::n);   }
NATIVE_WORD_DECL(finance, tvm_set_i)   { return tvm_set_word(rpn, ::stack::Tvm::SolveFor::i);   }
NATIVE_WORD_DECL(finance, tvm_set_pv)  { return tvm_set_word(rpn, ::stack::Tvm::SolveFor::pv);  }
NATIVE_WORD_DECL(finance, tvm_set_pmt) { return tvm_set_word(rpn, ::stack::Tvm::SolveFor::pmt); }
NATIVE_WORD_DECL(finance, tvm_set_fv)  { return tvm_set_word(rpn, ::stack::Tvm::SolveFor::fv);  }

// ---------------------------------------------------------------------------
// TVM-BEGIN / TVM-END  ( tvm -- tvm )
// ---------------------------------------------------------------------------
NATIVE_WORD_DECL(finance, tvm_begin) {
  auto sv = rpn.stack.pop();
  ::stack::Tvm t = POP_CAST(::stack::Tvm, sv);
  t.begin = true;  tvm_resolve(t);
  rpn.stack.push(t);
  return rpn::WordDefinition::Result::ok;
}
NATIVE_WORD_DECL(finance, tvm_end) {
  auto sv = rpn.stack.pop();
  ::stack::Tvm t = POP_CAST(::stack::Tvm, sv);
  t.begin = false; tvm_resolve(t);
  rpn.stack.push(t);
  return rpn::WordDefinition::Result::ok;
}

// ---------------------------------------------------------------------------
// AMORT  ( tvm -- tvm obj )
//   full amortization schedule as a column-oriented stack::Object
// ---------------------------------------------------------------------------
NATIVE_WORD_DECL(finance, amort) {
  const auto &t = PEEK_CAST(::stack::Tvm, rpn.stack.peek(1));  // leave tvm on stack
  int periods = (int)std::llround(t.n);
  if (periods < 1) return rpn::WordDefinition::Result::param_error;

  double r = t.i / 100.0;
  std::vector<double> period, interest, principal, balance;
  double bal = t.pv;
  double totInt = 0.0, totPrin = 0.0;
  for (int k = 1; k <= periods; ++k) {
    double intr = bal * r;
    double prin = (-t.pmt) - intr;
    bal -= prin;
    period.push_back((double)k);
    interest.push_back(intr);
    principal.push_back(prin);
    balance.push_back(bal);
    totInt  += intr;
    totPrin += prin;
  }

  ::stack::Object obj;
  obj.add_value("period",          ::stack::Vector(period));
  obj.add_value("interest",        ::stack::Vector(interest));
  obj.add_value("principal",       ::stack::Vector(principal));
  obj.add_value("balance",         ::stack::Vector(balance));
  obj.add_value("total-interest",  ::stack::Double(totInt));
  obj.add_value("total-principal", ::stack::Double(totPrin));
  rpn.stack.push(obj);
  return rpn::WordDefinition::Result::ok;
}

// ---------------------------------------------------------------------------
// addFinanceWords
// ---------------------------------------------------------------------------
void
rpn::Interp::addFinanceWords() {
  setWordCategory("finance");

  addDefinition("TVM", { rpn::StackSizeValidator::zero,
                         NATIVE_WORD_FN(finance, tvm_new), nullptr });

  addDefinition("->TVM", { finance_validator::d6_tvm_args,
                           NATIVE_WORD_FN(finance, to_tvm), nullptr });
  addDefinition("TVM->", { finance_validator::d1_tvm,
                           NATIVE_WORD_FN(finance, tvm_explode), nullptr });

  addDefinition("SOLVE-PV",  { finance_validator::d1_tvm, NATIVE_WORD_FN(finance, solve_pv_w),  nullptr });
  addDefinition("SOLVE-FV",  { finance_validator::d1_tvm, NATIVE_WORD_FN(finance, solve_fv_w),  nullptr });
  addDefinition("SOLVE-PMT", { finance_validator::d1_tvm, NATIVE_WORD_FN(finance, solve_pmt_w), nullptr });
  addDefinition("SOLVE-N",   { finance_validator::d1_tvm, NATIVE_WORD_FN(finance, solve_n_w),   nullptr });
  addDefinition("SOLVE-I",   { finance_validator::d1_tvm, NATIVE_WORD_FN(finance, solve_i_w),   nullptr });

  addDefinition("TVM-N",   { finance_validator::d2_tvm_double, NATIVE_WORD_FN(finance, tvm_set_n),   nullptr });
  addDefinition("TVM-I",   { finance_validator::d2_tvm_double, NATIVE_WORD_FN(finance, tvm_set_i),   nullptr });
  addDefinition("TVM-PV",  { finance_validator::d2_tvm_double, NATIVE_WORD_FN(finance, tvm_set_pv),  nullptr });
  addDefinition("TVM-PMT", { finance_validator::d2_tvm_double, NATIVE_WORD_FN(finance, tvm_set_pmt), nullptr });
  addDefinition("TVM-FV",  { finance_validator::d2_tvm_double, NATIVE_WORD_FN(finance, tvm_set_fv),  nullptr });
  addDefinition("TVM-BEGIN", { finance_validator::d1_tvm, NATIVE_WORD_FN(finance, tvm_begin), nullptr });
  addDefinition("TVM-END",   { finance_validator::d1_tvm, NATIVE_WORD_FN(finance, tvm_end),   nullptr });
  addDefinition("AMORT",     { finance_validator::d1_tvm, NATIVE_WORD_FN(finance, amort),     nullptr });

  addWordMetadata("TVM",      "Push a blank time-value-of-money object.");
  addWordMetadata("->TVM",    "Build a tvm. `n i pv pmt fv begin ->TVM`.");
  addWordMetadata("TVM->",    "Decompose a tvm into `n i pv pmt fv begin`.");
  addWordMetadata("SOLVE-PV",  "Solve a tvm for present value.");
  addWordMetadata("SOLVE-FV",  "Solve a tvm for future value.");
  addWordMetadata("SOLVE-PMT", "Solve a tvm for payment.");
  addWordMetadata("SOLVE-N",   "Solve a tvm for the number of periods.");
  addWordMetadata("SOLVE-I",   "Solve a tvm for the periodic interest rate.");
  addWordMetadata("TVM-N",   "Set a tvm's period count; re-solves the unknown.");
  addWordMetadata("TVM-I",   "Set a tvm's periodic rate; re-solves the unknown.");
  addWordMetadata("TVM-PV",  "Set a tvm's present value; re-solves the unknown.");
  addWordMetadata("TVM-PMT", "Set a tvm's payment; re-solves the unknown.");
  addWordMetadata("TVM-FV",  "Set a tvm's future value; re-solves the unknown.");
  addWordMetadata("TVM-BEGIN", "Set a tvm to begin-of-period payments.");
  addWordMetadata("TVM-END",   "Set a tvm to end-of-period payments.");
  addWordMetadata("AMORT",     "Amortization schedule of a tvm → object {period,interest,principal,balance,total-interest,total-principal}.");

  setWordCategory("");
}

/* end of rpn-lang/src/finance-dict.cpp */
