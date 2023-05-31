/***************************************************
 * file: github/elh/rpn-cnc/rpn-dict.cpp
 *
 * @file    rpn-dict.cpp
 * @author  Eric L. Hernes
 * @version V1.0
 * @born_on   Saturday, May 27, 2023
 * @copyright (C) Copyright Eric L. Hernes 2023
 * @copyright (C) Copyright Q, Inc. 2023
 *
 * @brief   An Eric L. Hernes Signature Series C++ module
 *
 */

#include "rpn.h"

#include <typeinfo>
#include <cmath>

static void addStackWords(rpn::Runtime &rpn);
static void addMathWords(rpn::Runtime &rpn);

void
rpn::Runtime::addInternalWords(WordContext *wcp) {
  addStackWords(*this);
  addMathWords(*this);
}

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
  return a/b;
}
static int64_t idivide(int64_t a, int64_t b) {
  return a/b;
}
MATH_BINARY_FUNC(divide);
MATH_BINARY_INTEGER_FUNC(idivide);

static double inverse(double a) {
  return 1./a;
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

MATH_UNARY_FUNC(sqrt);
MATH_BINARY_FUNC(pow);
static int64_t ipow(int64_t a, int64_t b) {
  return (int64_t)pow(a,b);
}
MATH_BINARY_INTEGER_FUNC(ipow);

static double cos_deg(double a) {
  return cos(deg_to_rad(a));
}
MATH_UNARY_FUNC(cos_deg);

static double acos_deg(double a) {
  return rad_to_deg(acos(a));
}
MATH_UNARY_FUNC(acos_deg);

static double sin_deg(double a) {
  return sin(deg_to_rad(a));
}
MATH_UNARY_FUNC(sin_deg);

static double asin_deg(double a) {
  return rad_to_deg(asin(a));
}
MATH_UNARY_FUNC(asin_deg);

static double tan_deg(double a) {
  return tan(deg_to_rad(a));
}
MATH_UNARY_FUNC(tan_deg);

static double atan_deg(double a) {
  return rad_to_deg(atan(a));
}
MATH_UNARY_FUNC(atan_deg);

static double ln2(double a) {
  return log(a)/0.69314718056; // ln(2)
}
MATH_UNARY_FUNC(ln2);

static double atan2_deg(double a, double b) {
  return rad_to_deg(atan2(a,b));
}
MATH_BINARY_FUNC(atan2_deg);

static int64_t imin(int64_t a, int64_t b) {
  return std::min(a,b);
}
static int64_t imax(int64_t a, int64_t b) {
  return std::max(a,b);
}
MATH_BINARY_FUNC(fmin);
MATH_BINARY_FUNC(fmax);
MATH_BINARY_FUNC(imin);
MATH_BINARY_FUNC(imax);

MATH_BINARY_FUNC(hypot);
MATH_UNARY_FUNC(round);
MATH_UNARY_FUNC(exp);
MATH_UNARY_FUNC(ceil);
MATH_UNARY_FUNC(floor);
MATH_UNARY_FUNC(log);
MATH_UNARY_FUNC(log10);

MATH_GENERATE(pi, M_PI);
MATH_GENERATE(e, M_E);
MATH_GENERATE(rand, rand());
MATH_GENERATE(rand48, drand48());

void
addMathWords(rpn::Runtime &rpn) {
  ADD_MATH_BINARY_NUMBER_WDEF(rpn, "+", add, iadd);
  ADD_MATH_BINARY_NUMBER_WDEF(rpn, "-", subtract, isubtract);
  ADD_MATH_BINARY_NUMBER_WDEF(rpn, "*", multiply, imultiply);
  ADD_MATH_BINARY_NUMBER_WDEF(rpn, "/", divide, idivide);
  ADD_MATH_BINARY_NUMBER_WDEF(rpn, "^", pow, ipow);
  ADD_MATH_BINARY_NUMBER_WDEF(rpn, "HYPOT", hypot, hypot);
  ADD_MATH_BINARY_NUMBER_WDEF(rpn, "ATAN2", atan2_deg, atan2_deg);
  ADD_MATH_BINARY_NUMBER_WDEF(rpn, "MIN", fmin, imin);
  ADD_MATH_BINARY_NUMBER_WDEF(rpn, "MAX", fmax, imax);

  ADD_MATH_UNARY_NUMBER_WDEF(rpn, "INV", inverse, inverse);
  ADD_MATH_UNARY_NUMBER_WDEF(rpn, "SQ", square, square);
  ADD_MATH_UNARY_NUMBER_WDEF(rpn, "SQRT", sqrt, sqrt);
  ADD_MATH_UNARY_NUMBER_WDEF(rpn, "COS", cos_deg, cos_deg);
  ADD_MATH_UNARY_NUMBER_WDEF(rpn, "SIN", sin_deg, sin_deg);
  ADD_MATH_UNARY_NUMBER_WDEF(rpn, "TAN", tan_deg, tan_deg);
  ADD_MATH_UNARY_NUMBER_WDEF(rpn, "ACOS", acos_deg, acos_deg);
  ADD_MATH_UNARY_NUMBER_WDEF(rpn, "ASIN", asin_deg, asin_deg);
  ADD_MATH_UNARY_NUMBER_WDEF(rpn, "EXP", exp, exp);
  ADD_MATH_UNARY_NUMBER_WDEF(rpn, "LN", log, log);
  ADD_MATH_UNARY_NUMBER_WDEF(rpn, "LN2", ln2, ln2);
  ADD_MATH_UNARY_NUMBER_WDEF(rpn, "LOG", log10, log10);

  // these don't really make sense on Integers, but maybe we should
  // allow it anyway?
  rpn.addDefinition("ROUND", MATH_WORD_WDEF(rpn::StrictTypeValidator::d1_double, round));
  rpn.addDefinition("CEIL", MATH_WORD_WDEF(rpn::StrictTypeValidator::d1_double, ceil));
  rpn.addDefinition("FLOOR", MATH_WORD_WDEF(rpn::StrictTypeValidator::d1_double, floor));

  rpn.addDefinition("k_PI", MATH_CONSTANT_WDEF(pi));
  rpn.addDefinition("k_E", MATH_CONSTANT_WDEF(e));
  rpn.addDefinition("RAND", MATH_CONSTANT_WDEF(rand));
  rpn.addDefinition("RAND48", MATH_CONSTANT_WDEF(rand48));

  //  rpn.addDefinition("LSHIFT", MATH_BINARY_DEF(lshift)); // integer
  //  rpn.addDefinition("RSHIFT", MATH_BINARY_DEF(rshift)); // integer

}

/*
 *
 * EVAL
 * HYPOTn
 * MAXn
 * MINn
 * NEG
 *
 * binary ops
 * lshift
 * rshift
 * and
 * or
 * not
 * xor
 */

/****************************************
 * conditionals
 *
 * IF
 * IFTE
 * EQ?
 */

/****************************************
 * memory operations
 *
 * STO
 * RCL
 */

/****************************************
 * type conversion words
 *
 */

/* */

#define STACK_OP(op) NATIVE_WORD_FN(stack,op)

#define STACK_OP_FUNC(op)							\
  static rpn::WordDefinition::Result STACK_OP(op)(rpn::Runtime &rpn, rpn::WordContext *ctx, std::string &rest) { \
    rpn.stack.op();							\
    return rpn::WordDefinition::Result::ok;				\
  }

#define STACK_OPn_FUNC(op)						\
  static rpn::WordDefinition::Result STACK_OP(op)(rpn::Runtime &rpn, rpn::WordContext *ctx, std::string &rest) { \
    int64_t n=rpn.stack.pop_integer();					\
    rpn.stack.op(n);							\
    return rpn::WordDefinition::Result::ok;				\
  }

STACK_OP_FUNC(drop);
STACK_OP_FUNC(clear);
STACK_OP_FUNC(swap);
STACK_OP_FUNC(rollu);
STACK_OP_FUNC(rolld);
STACK_OP_FUNC(over);
STACK_OP_FUNC(dup);
STACK_OP_FUNC(rotu);
STACK_OP_FUNC(rotd);
STACK_OP_FUNC(print);

STACK_OPn_FUNC(dropn);
STACK_OPn_FUNC(dupn);
STACK_OPn_FUNC(nipn);
STACK_OPn_FUNC(pick);
STACK_OPn_FUNC(rolldn);
STACK_OPn_FUNC(rollun);
STACK_OPn_FUNC(tuckn);

// depth is special because we push the value back on the stack
static rpn::WordDefinition::Result STACK_OP(depth)(rpn::Runtime &rpn, rpn::WordContext *ctx, std::string &rest) {
  rpn.stack.push_integer(rpn.stack.depth());
  return rpn::WordDefinition::Result::ok;
}

#define ADD_STACK_OP(r, symbol, vv, func)				\
  r.addDefinition(symbol, NATIVE_WORD_WDEF(stack, rpn::StackSizeValidator::vv, func, nullptr))

void addStackWords(rpn::Runtime &rpn) {
  ADD_STACK_OP(rpn, "DROP", one, drop);
  ADD_STACK_OP(rpn, "CLEAR", zero, clear);
  ADD_STACK_OP(rpn, "DEPTH", zero, depth);
  ADD_STACK_OP(rpn, "SWAP", two, swap);
  ADD_STACK_OP(rpn, "ROLLU", zero, rollu);
  ADD_STACK_OP(rpn, "ROLLD", zero, rolld);
  ADD_STACK_OP(rpn, "OVER", two, over);
  ADD_STACK_OP(rpn, "DUP", one, dup);
  ADD_STACK_OP(rpn, "ROTU", three, rotu);
  ADD_STACK_OP(rpn, "ROTD", three, rotd);
  ADD_STACK_OP(rpn, "DROPN", ntos, dropn);
  ADD_STACK_OP(rpn, "DUPN", ntos, dupn);
  ADD_STACK_OP(rpn, "NIPN", ntos, nipn);
  ADD_STACK_OP(rpn, "PICK", ntos, pick);
  ADD_STACK_OP(rpn, "ROLLDN", ntos, rolldn);
  ADD_STACK_OP(rpn, "ROLLUN", ntos, rollun);
  ADD_STACK_OP(rpn, "TUCKN", ntos, tuckn);
  ADD_STACK_OP(rpn, ".S", zero, print);
}

/* end of github/elh/rpn-cnc/rpn-dict.cpp */
