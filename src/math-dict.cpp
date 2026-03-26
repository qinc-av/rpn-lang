/***************************************************
 * file: qinc/rpn-lang/src/math-dict.cpp
 *
 * @file    math-dict.cpp
 * @author  Eric L. Hernes
 * @version V1.0
 * @born_on   Monday, June 12, 2023
 * @copyright (C) Copyright Eric L. Hernes 2023
 * @copyright (C) Copyright Q, Inc. 2023
 *
 * @brief   An Eric L. Hernes Signature Series C++ module
 *
 */

#define _USE_MATH_DEFINES // for MSVC
#define _CRT_RAND_S

#include "../rpn.h"

#include <cmath>
#include <limits>
#include <complex>

#if defined (_MSC_VER)
#include <stdlib.h>
double drand48() {
  unsigned int rv=0;
  rand_s(&rv);
  return double(rv)/double(UINT_MAX);
}
#endif

/****************************************
 * math types
 */
namespace stack {
  class Complex : public rpn::Stack::Object, public std::complex<double>  {
  public:
    Complex() = delete;
    Complex(double re, double im) : std::complex<double>(re,im) {}
    Complex(const Complex &cx) : std::complex<double>(cx) {}
    Complex(const std::complex<double> &cx) : std::complex<double>(cx) {}
    virtual bool operator==(const rpn::Stack::Object &orhs) const override {
      auto &rhs = PEEK_CAST(const Complex, orhs);
      return ((const std::complex<double> &)*this) == ((const std::complex<double> &)rhs);
    }
    virtual std::unique_ptr<rpn::Stack::Object> deep_copy() const override { return std::make_unique<Complex>(*this); };
    virtual operator std::string() const override {
      std:: string rv = rpn::to_string(this->real());
      if (this->imag()>0) {
	rv += "+";
      }
      rv += rpn::to_string(this->imag());
      rv += "i";
      return rv;
    }
    virtual std::string deparse() const override {
      std::string rv;
      rv += std::to_string(this->real()) + " ";
      rv += std::to_string(this->imag()) + " ->COMPLEX";
      return rv;
    }
  // default to_text()
    virtual std::string to_latex() const override {
      return (std::string)(*this);
    }
  private:
  };

} // namespace stack
namespace math_validator {
  extern const rpn::StrictTypeValidator d1_complex;
}
const rpn::StrictTypeValidator math_validator::d1_complex({typeid(stack::Complex).hash_code()}, "d1_complex");

/****************************************
 * math words
 */
#define MATH_FUNC(op) NATIVE_WORD_FN(math,op)
#define MATH_GENERATE(fn, val) NATIVE_WORD_FN_0_DOUBLE(math, fn, val)
#define MATH_UNARY_FUNC(fn) NATIVE_WORD_FN_1_NUMBER(math, fn)
#define MATH_UNARY_INTEGER_FUNC(fn) NATIVE_WORD_FN_1_INTEGER(math, fn)
#define MATH_BINARY_FUNC(fn) NATIVE_WORD_FN_2_NUMBER(math, fn)
#define MATH_BINARY_INTEGER_FUNC(fn) NATIVE_WORD_FN_2_INTEGER(math, fn)

#define MATH_CONSTANT_WDEF(w) NATIVE_WORD_WDEF(math, rpn::StackSizeValidator::zero, w, nullptr)
#define MATH_WORD_WDEF(validator,w) NATIVE_WORD_WDEF(math, validator, w, nullptr)

// common case for binary function that converts to double except for
// when both parameters are integers
#define ADD_MATH_BINARY_NUMBER_WDEF(r, symbol, double_func, integer_func) \
  ADD_NATIVE_2_NUMBER_WDEF(math, r, symbol, double_func, integer_func, nullptr)

#define ADD_MATH_UNARY_NUMBER_WDEF(r, symbol, double_func, integer_func) \
  ADD_NATIVE_1_NUMBER_WDEF(math, r, symbol, double_func, integer_func, nullptr)

static double deg_to_rad(const double &deg) {
  return deg * (M_PI / 180.);
}
static double rad_to_deg(const double &rad) {
  return rad * 180. / M_PI;
}

// Convert an angle value from the current angle mode to radians.
static double to_radians(double a, rpn::AngleMode mode) {
  switch (mode) {
    case rpn::AngleMode::degrees:  return a * (M_PI / 180.);
    case rpn::AngleMode::radians:  return a;
    case rpn::AngleMode::gradians: return a * (M_PI / 200.);
  }
  return a;
}
// Convert an angle value from radians to the current angle mode.
static double from_radians(double a, rpn::AngleMode mode) {
  switch (mode) {
    case rpn::AngleMode::degrees:  return a * (180. / M_PI);
    case rpn::AngleMode::radians:  return a;
    case rpn::AngleMode::gradians: return a * (200. / M_PI);
  }
  return a;
}

static double multiply(double a, double b) {
  return a*b;
}
static int64_t imultiply(int64_t a, int64_t b) {
  return a*b;
}
MATH_BINARY_FUNC(multiply);
MATH_BINARY_INTEGER_FUNC(imultiply);

static double add(double a, double b) {
  return a+b;
}
static int64_t iadd(int64_t a, int64_t b) {
  return a+b;
}
MATH_BINARY_FUNC(add);
MATH_BINARY_INTEGER_FUNC(iadd);

static double subtract(double a, double b) {
  return a-b;
}
static int64_t isubtract(int64_t a, int64_t b) {
  return a-b;
}
MATH_BINARY_FUNC(subtract);
MATH_BINARY_INTEGER_FUNC(isubtract);

static double divide(double a, double b) {
  double rv = std::nan("");
  if (b==0) {
    rv = (a>0) ? INFINITY : -INFINITY;
  } else {
    rv = a/b;
  }
  return rv;
}

static int64_t idivide(int64_t a, int64_t b) {
  int64_t rv = 0;
  if (b!=0) {
    rv = a/b;
  } else {
    rv = (a>0) ? std::numeric_limits<int64_t>::max() : -std::numeric_limits<int64_t>::max();
  }
  return rv;
}
MATH_BINARY_FUNC(divide);
MATH_BINARY_INTEGER_FUNC(idivide);

static double inverse(double a) {
  double rv=std::nan("");
  if (a==0) {
    rv = (a>0) ? INFINITY : -INFINITY;
  } else {
    rv = 1./a;
  }
  return rv;
}
MATH_UNARY_FUNC(inverse);

static double square(double a) {
  return a*a;
}
static int64_t isquare(int64_t a) {
  return a*a;
}
MATH_UNARY_FUNC(square);
MATH_UNARY_INTEGER_FUNC(isquare);

MATH_BINARY_FUNC(pow);
static int64_t ipow(int64_t a, int64_t b) {
  return (int64_t)pow(a,b);
}
MATH_BINARY_INTEGER_FUNC(ipow);

// Angle-mode-aware trig words.  Each reads rpn.angleMode() at call time so the
// mode can be changed between evaluations without re-registering words.

NATIVE_WORD_DECL(math, trig_cos) {
  double a = rpn.stack.pop_as_double();
  rpn.stack.push_double(::cos(to_radians(a, rpn.angleMode())));
  return rpn::WordDefinition::Result::ok;
}
NATIVE_WORD_DECL(math, trig_acos) {
  double a = rpn.stack.pop_as_double();
  rpn.stack.push_double(from_radians(::acos(a), rpn.angleMode()));
  return rpn::WordDefinition::Result::ok;
}
NATIVE_WORD_DECL(math, trig_sin) {
  double a = rpn.stack.pop_as_double();
  rpn.stack.push_double(::sin(to_radians(a, rpn.angleMode())));
  return rpn::WordDefinition::Result::ok;
}
NATIVE_WORD_DECL(math, trig_asin) {
  double a = rpn.stack.pop_as_double();
  rpn.stack.push_double(from_radians(::asin(a), rpn.angleMode()));
  return rpn::WordDefinition::Result::ok;
}
NATIVE_WORD_DECL(math, trig_tan) {
  double a = rpn.stack.pop_as_double();
  rpn.stack.push_double(::tan(to_radians(a, rpn.angleMode())));
  return rpn::WordDefinition::Result::ok;
}
NATIVE_WORD_DECL(math, trig_atan) {
  double a = rpn.stack.pop_as_double();
  rpn.stack.push_double(from_radians(::atan(a), rpn.angleMode()));
  return rpn::WordDefinition::Result::ok;
}

static double ln2(double a) {
  return log(a)/log(2.);
}
MATH_UNARY_FUNC(ln2);

// atan2: stack order is ( x y -- angle ), i.e. y=TOS, x=TOS-1 → atan2(y,x)
NATIVE_WORD_DECL(math, trig_atan2) {
  double y = rpn.stack.pop_as_double();
  double x = rpn.stack.pop_as_double();
  rpn.stack.push_double(from_radians(::atan2(y, x), rpn.angleMode()));
  return rpn::WordDefinition::Result::ok;
}

static int64_t imin(int64_t a, int64_t b) {
  return std::min(a,b);
}
static int64_t imax(int64_t a, int64_t b) {
  return std::max(a,b);
}
MATH_BINARY_FUNC(fmin);
MATH_BINARY_FUNC(fmax);
MATH_BINARY_INTEGER_FUNC(imin);
MATH_BINARY_INTEGER_FUNC(imax);

MATH_BINARY_FUNC(hypot);
MATH_UNARY_FUNC(round);
MATH_UNARY_FUNC(exp);
MATH_UNARY_FUNC(ceil);
MATH_UNARY_FUNC(floor);
MATH_UNARY_FUNC(log);
MATH_UNARY_FUNC(log10);

MATH_UNARY_FUNC(deg_to_rad);
MATH_UNARY_FUNC(rad_to_deg);

MATH_GENERATE(pi, M_PI);
MATH_GENERATE(e, M_E);
MATH_GENERATE(rand, rand());

NATIVE_WORD_DECL(math,quadratic) {
  double c = rpn.stack.pop_as_double();
  double b = rpn.stack.pop_as_double();
  double a = rpn.stack.pop_as_double();

  double sq = b*b - 4*a*c;
  if (sq<0) {
    std::complex<double> csq(sq, 0.);
    auto x1 = (-b + sqrt(csq))/(2*a);
    auto x2 = (-b - sqrt(csq))/(2*a);
    rpn.stack.push(stack::Complex(x1));
    rpn.stack.push(stack::Complex(x2));
  } else {
    double x1 = (-b + sqrt(sq))/(2*a);
    double x2 = (-b - sqrt(sq))/(2*a);
    rpn.stack.push_double(x1);
    rpn.stack.push_double(x2);
  }
  return rpn::WordDefinition::Result::ok;
}

NATIVE_WORD_DECL(math,to_complex) {
  double im = rpn.stack.pop_as_double();
  double re = rpn.stack.pop_as_double();
  rpn.stack.push(stack::Complex(re, im));
  return rpn::WordDefinition::Result::ok;
}

NATIVE_WORD_DECL(math,complex_to) {
  auto sob = rpn.stack.pop();
  const auto &cx = PEEK_CAST(stack::Complex,*sob);
  rpn.stack.push_double(cx.real());
  rpn.stack.push_double(cx.imag());
  return rpn::WordDefinition::Result::ok;
}

NATIVE_WORD_DECL(math,sqrt) {
  double x = rpn.stack.pop_as_double();
  if (x<0) {
    std::complex<double> cx(x, 0.);
    rpn.stack.push(stack::Complex(sqrt(cx)));
  } else {
    rpn.stack.push_double(sqrt(x));
  }
  return rpn::WordDefinition::Result::ok;
}

#ifdef WIN32
double drand48() {
  return (double)rand()/32767.;
}
#endif

MATH_GENERATE(drand, drand48());

static double change_sign(double x) {
  return -1. * x;
}
MATH_UNARY_FUNC(change_sign);
static int64_t ichange_sign(int64_t x) {
  return -1 * x;
}
MATH_UNARY_INTEGER_FUNC(ichange_sign);

void
rpn::Interp::addMathWords() {
  rpn::Interp &rpn(*this);

  ADD_MATH_BINARY_NUMBER_WDEF(rpn, "+", add, iadd);
  ADD_MATH_BINARY_NUMBER_WDEF(rpn, "-", subtract, isubtract);
  ADD_MATH_BINARY_NUMBER_WDEF(rpn, "*", multiply, imultiply);
  ADD_MATH_BINARY_NUMBER_WDEF(rpn, "/", divide, idivide);
  ADD_MATH_BINARY_NUMBER_WDEF(rpn, "^", pow, ipow);
  ADD_MATH_BINARY_NUMBER_WDEF(rpn, "HYPOT", hypot, hypot);
  // ATAN2 accepts any two numeric types; both paths use the mode-aware body.
  rpn.addDefinition("ATAN2", { rpn::StrictTypeValidator::d2_double_double,   NATIVE_WORD_FN(math, trig_atan2), nullptr });
  rpn.addDefinition("ATAN2", { rpn::StrictTypeValidator::d2_double_integer,  NATIVE_WORD_FN(math, trig_atan2), nullptr });
  rpn.addDefinition("ATAN2", { rpn::StrictTypeValidator::d2_integer_double,  NATIVE_WORD_FN(math, trig_atan2), nullptr });
  rpn.addDefinition("ATAN2", { rpn::StrictTypeValidator::d2_integer_integer, NATIVE_WORD_FN(math, trig_atan2), nullptr });
  ADD_MATH_BINARY_NUMBER_WDEF(rpn, "MIN", fmin, imin);
  ADD_MATH_BINARY_NUMBER_WDEF(rpn, "MAX", fmax, imax);

  ADD_MATH_UNARY_NUMBER_WDEF(rpn, "INV", inverse, inverse);
  ADD_MATH_UNARY_NUMBER_WDEF(rpn, "SQ", square, isquare);
  ADD_MATH_UNARY_NUMBER_WDEF(rpn, "SQRT", sqrt, sqrt);
  // Mode-aware trig words: both double and integer inputs accepted (pop_as_double handles cast).
#define ADD_TRIG_WDEF(sym, fn) \
  rpn.addDefinition(sym, { rpn::StrictTypeValidator::d1_double,  NATIVE_WORD_FN(math, fn), nullptr }); \
  rpn.addDefinition(sym, { rpn::StrictTypeValidator::d1_integer, NATIVE_WORD_FN(math, fn), nullptr })
  ADD_TRIG_WDEF("COS",  trig_cos);
  ADD_TRIG_WDEF("SIN",  trig_sin);
  ADD_TRIG_WDEF("TAN",  trig_tan);
  ADD_TRIG_WDEF("ACOS", trig_acos);
  ADD_TRIG_WDEF("ASIN", trig_asin);
  ADD_TRIG_WDEF("ATAN", trig_atan);
#undef ADD_TRIG_WDEF
  ADD_MATH_UNARY_NUMBER_WDEF(rpn, "EXP", exp, exp);
  ADD_MATH_UNARY_NUMBER_WDEF(rpn, "LN", log, log);
  ADD_MATH_UNARY_NUMBER_WDEF(rpn, "LN2", ln2, ln2);
  ADD_MATH_UNARY_NUMBER_WDEF(rpn, "LOG", log10, log10);
  ADD_MATH_UNARY_NUMBER_WDEF(rpn, "CHS", change_sign, ichange_sign);

  ADD_MATH_UNARY_NUMBER_WDEF(rpn, "D->R", deg_to_rad, deg_to_rad);
  ADD_MATH_UNARY_NUMBER_WDEF(rpn, "R->D", rad_to_deg, rad_to_deg);

  // these don't really make sense on Integers, but maybe we should
  // allow it anyway?
  addDefinition("ROUND", MATH_WORD_WDEF(rpn::StrictTypeValidator::d1_double, round));
  addDefinition("CEIL", MATH_WORD_WDEF(rpn::StrictTypeValidator::d1_double, ceil));
  addDefinition("FLOOR", MATH_WORD_WDEF(rpn::StrictTypeValidator::d1_double, floor));

  ADD_NATIVE_3_NUMBER_WDEF(math, rpn, "QUAD", quadratic, quadratic, nullptr);
  ADD_NATIVE_2_NUMBER_WDEF(math, rpn, "->COMPLEX", to_complex, to_complex, nullptr);
  addDefinition("OBJ->", MATH_WORD_WDEF(math_validator::d1_complex, complex_to));

  addDefinition("k_PI", MATH_CONSTANT_WDEF(pi));
  addDefinition("k_E", MATH_CONSTANT_WDEF(e));
  addDefinition("RAND", MATH_CONSTANT_WDEF(rand));
  addDefinition("DRAND", MATH_CONSTANT_WDEF(drand));

  // Angle mode: ->DEG / ->RAD / ->GRAD set the mode; ANGLEMODE queries it.
  rpn.addDefinition("->DEG",  { rpn::StackSizeValidator::zero, [](rpn::Interp &rpn, rpn::WordContext *, std::string &) {
    rpn.setAngleMode(rpn::AngleMode::degrees);
    return rpn::WordDefinition::Result::ok;
  }, nullptr });
  rpn.addDefinition("->RAD",  { rpn::StackSizeValidator::zero, [](rpn::Interp &rpn, rpn::WordContext *, std::string &) {
    rpn.setAngleMode(rpn::AngleMode::radians);
    return rpn::WordDefinition::Result::ok;
  }, nullptr });
  rpn.addDefinition("->GRAD", { rpn::StackSizeValidator::zero, [](rpn::Interp &rpn, rpn::WordContext *, std::string &) {
    rpn.setAngleMode(rpn::AngleMode::gradians);
    return rpn::WordDefinition::Result::ok;
  }, nullptr });
  rpn.addDefinition("ANGLEMODE", { rpn::StackSizeValidator::zero, [](rpn::Interp &rpn, rpn::WordContext *, std::string &) {
    switch (rpn.angleMode()) {
      case rpn::AngleMode::degrees:  rpn.stack.push_string("DEG");  break;
      case rpn::AngleMode::radians:  rpn.stack.push_string("RAD");  break;
      case rpn::AngleMode::gradians: rpn.stack.push_string("GRAD"); break;
    }
    return rpn::WordDefinition::Result::ok;
  }, nullptr });

  //  rpn.addDefinition("LSHIFT", MATH_BINARY_DEF(lshift)); // integer
  //  rpn.addDefinition("RSHIFT", MATH_BINARY_DEF(rshift)); // integer

}

/* end of qinc/rpn-lang/src/math-dict.cpp */
