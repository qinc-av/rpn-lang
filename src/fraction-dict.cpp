/***************************************************
 * file: QInc/Projects/color-calc/src/libs/rpn-lang/src/fraction-dict.cpp
 *
 * @file    fraction-dict.cpp
 * @author  Eric L. Hernes
 * @version V1.0
 * @born_on   Wednesday, March 13, 2024
 * @copyright (C) Copyright Eric L. Hernes 2024
 * @copyright (C) Copyright Q, Inc. 2024
 *
 * @brief   An Eric L. Hernes Signature Series C++ module
 *
 */

#include "../rpn.h"
#include <cmath>
#include "fraction.h"


NATIVE_WORD_DECL(fraction, to_frac_ii) {
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  auto denom = rpn.stack.pop_integer();
  auto num = rpn.stack.pop_integer();
  rpn.stack.push(stack::Fraction(num, denom));
  return rv;
}

NATIVE_WORD_DECL(fraction, to_frac_d) {
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  auto dec = rpn.stack.pop_as_double(); // should already be double
  rpn.stack.push(stack::Fraction(dec));
  return rv;
}

NATIVE_WORD_DECL(fraction, obj_to) {
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  auto ofrac = rpn.stack.pop();
  const auto &frac = POP_CAST(stack::Fraction,ofrac);
  rpn.stack.push_integer(frac._numerator);
  rpn.stack.push_integer(frac._denominator);
  return rv;
}

NATIVE_WORD_DECL(fraction, to_float) {
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  auto ofrac = rpn.stack.pop();
  const auto &frac = POP_CAST(stack::Fraction,ofrac);
  rpn.stack.push_double(frac);
  return rv;
}

NATIVE_WORD_DECL(fraction, inv_f) {
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  auto ofrac = rpn.stack.pop();
  const auto &frac = POP_CAST(stack::Fraction,ofrac);
  rpn.stack.push(stack::Fraction(frac._denominator,frac._numerator));
  return rv;
}

NATIVE_WORD_DECL(fraction, add_ff) {
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  auto o2 = rpn.stack.pop();
  const auto &f2 = POP_CAST(stack::Fraction,o2);
  auto o1 = rpn.stack.pop();
  const auto &f1 = POP_CAST(stack::Fraction,o1);
  rpn.stack.push(stack::Fraction(f1 + f2));
  return rv;
}

NATIVE_WORD_DECL(fraction, add_fn) {
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  auto o2 = rpn.stack.pop();
  const auto &f2 = POP_CAST(stack::Fraction,o2);
  auto d1 = rpn.stack.pop_as_double();
  rpn.stack.push(stack::Fraction(d1 + f2));
  return rv;
}

NATIVE_WORD_DECL(fraction, add_nf) {
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  auto d2 = rpn.stack.pop_as_double();
  auto o1 = rpn.stack.pop();
  const auto &f1 = POP_CAST(stack::Fraction,o1);
  rpn.stack.push(stack::Fraction(f1 + d2));
  return rv;
}

NATIVE_WORD_DECL(fraction, sub_ff) {
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  auto o2 = rpn.stack.pop();
  const auto &f2 = POP_CAST(stack::Fraction,o2);
  auto o1 = rpn.stack.pop();
  const auto &f1 = POP_CAST(stack::Fraction,o1);
  rpn.stack.push(stack::Fraction(f1 - f2));
  return rv;
}

NATIVE_WORD_DECL(fraction, sub_fn) {
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  auto o2 = rpn.stack.pop();
  const auto &f2 = POP_CAST(stack::Fraction,o2);
  auto d1 = rpn.stack.pop_as_double();
  rpn.stack.push(stack::Fraction(d1 - f2));
  return rv;
}

NATIVE_WORD_DECL(fraction, sub_nf) {
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  auto d2 = rpn.stack.pop_as_double();
  auto o1 = rpn.stack.pop();
  const auto &f1 = POP_CAST(stack::Fraction,o1);
  rpn.stack.push(stack::Fraction(f1 - d2));
  return rv;
}

NATIVE_WORD_DECL(fraction, mult_fn) {
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  auto o2 = rpn.stack.pop();
  const auto &f2 = POP_CAST(stack::Fraction,o2);
  auto d1 = rpn.stack.pop_as_double();
  rpn.stack.push(stack::Fraction(d1 * f2));
  return rv;
}

NATIVE_WORD_DECL(fraction, mult_nf) {
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  auto d2 = rpn.stack.pop_as_double();
  auto o1 = rpn.stack.pop();
  const auto &f1 = POP_CAST(stack::Fraction,o1);
  rpn.stack.push(stack::Fraction(f1 * d2));
  return rv;
}

NATIVE_WORD_DECL(fraction, mult_ff) {
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  auto o2 = rpn.stack.pop();
  const auto &f2 = POP_CAST(stack::Fraction,o2);
  auto o1 = rpn.stack.pop();
  const auto &f1 = POP_CAST(stack::Fraction,o1);
  rpn.stack.push(stack::Fraction(f1 * f2));
  return rv;
}


NATIVE_WORD_DECL(fraction, divide_fn) {
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  auto o2 = rpn.stack.pop();
  const auto &f2 = POP_CAST(stack::Fraction,o2);
  auto d1 = rpn.stack.pop_as_double();
  rpn.stack.push(stack::Fraction(d1 / f2));
  return rv;
}

NATIVE_WORD_DECL(fraction, divide_nf) {
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  auto d2 = rpn.stack.pop_as_double();
  auto o1 = rpn.stack.pop();
  const auto &f1 = POP_CAST(stack::Fraction,o1);
  rpn.stack.push(stack::Fraction(f1 / d2));
  return rv;
}

NATIVE_WORD_DECL(fraction, divide_ff) {
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  auto o2 = rpn.stack.pop();
  const auto &f2 = POP_CAST(stack::Fraction,o2);
  auto o1 = rpn.stack.pop();
  const auto &f1 = POP_CAST(stack::Fraction,o1);
  rpn.stack.push(stack::Fraction(f1 / f2));
  return rv;
}

NATIVE_WORD_DECL(fraction, sq_f) {
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  auto o = rpn.stack.pop();
  const auto &f = POP_CAST(stack::Fraction,o);
  rpn.stack.push(stack::Fraction(f * f));
  return rv;
}

NATIVE_WORD_DECL(fraction, sqrt_f) {
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  auto o = rpn.stack.pop();
  const auto &f = POP_CAST(stack::Fraction,o);
  rpn.stack.push(stack::Fraction(std::sqrt(f)));
  return rv;
}

NATIVE_WORD_DECL(fraction, pow_fn) {
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  auto o2 = rpn.stack.pop();
  const auto &f2 = POP_CAST(stack::Fraction,o2);
  auto d1 = rpn.stack.pop_as_double();
  rpn.stack.push(stack::Fraction(std::pow(d1, double(f2))));
  return rv;
}

NATIVE_WORD_DECL(fraction, pow_nf) {
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  auto d2 = rpn.stack.pop_as_double();
  auto o1 = rpn.stack.pop();
  const auto &f1 = POP_CAST(stack::Fraction,o1);
  rpn.stack.push(stack::Fraction(std::pow(double(f1), d2)));
  return rv;
}

NATIVE_WORD_DECL(fraction, pow_ff) {
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  auto o2 = rpn.stack.pop();
  double d2 = POP_CAST(stack::Fraction,o2);
  auto o1 = rpn.stack.pop();
  double d1 = POP_CAST(stack::Fraction,o1);
  double result = std::pow(d1,d2);
  rpn.stack.push(stack::Fraction(result));
  return rv;
}

NATIVE_WORD_DECL(fraction, neg_f) {
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  auto o1 = rpn.stack.pop();
  const auto &f1 = POP_CAST(stack::Fraction,o1);
  rpn.stack.push(stack::Fraction(f1.neg()));
  return rv;
}

const rpn::StrictTypeValidator frac_validator::d1_frac({typeid(stack::Fraction).hash_code()}, "d1_frac");
const rpn::StrictTypeValidator frac_validator::d2_frac_frac({typeid(stack::Fraction).hash_code(),typeid(stack::Fraction).hash_code()}, "d2_frac_frac");
const rpn::StrictTypeValidator frac_validator::d2_int_frac({typeid(stack::Fraction).hash_code(),typeid(stack::Integer).hash_code()}, "d2_int_frac");
const rpn::StrictTypeValidator frac_validator::d2_double_frac({typeid(stack::Fraction).hash_code(),typeid(stack::Double).hash_code()}, "d2_double_frac");
const rpn::StrictTypeValidator frac_validator::d2_frac_int({typeid(stack::Integer).hash_code(),typeid(stack::Fraction).hash_code()}, "d2_frac_int");
const rpn::StrictTypeValidator frac_validator::d2_frac_double({typeid(stack::Double).hash_code(),typeid(stack::Fraction).hash_code()}, "d2_frac_double");
const rpn::StrictTypeValidator frac_validator::d5_frac_double_double_double_double({
    typeid(stack::Double).hash_code(),typeid(stack::Double).hash_code(),typeid(stack::Double).hash_code(),typeid(stack::Double).hash_code(),
      typeid(stack::Fraction).hash_code()}, "d5_frac_double_double_double_double");

#define ADD_FRAC_NUM_WORD(rpn, word_token, method)			\
  rpn.addDefinition(word_token, NATIVE_WORD_WDEF(fraction, frac_validator::d2_int_frac, method##_fn, nullptr)); \
  rpn.addDefinition(word_token, NATIVE_WORD_WDEF(fraction, frac_validator::d2_double_frac, method##_fn, nullptr)); \
  rpn.addDefinition(word_token, NATIVE_WORD_WDEF(fraction, frac_validator::d2_frac_int, method##_nf, nullptr)); \
  rpn.addDefinition(word_token, NATIVE_WORD_WDEF(fraction, frac_validator::d2_frac_double, method##_nf, nullptr)); \
  rpn.addDefinition(word_token, NATIVE_WORD_WDEF(fraction, frac_validator::d2_frac_frac, method##_ff, nullptr))

void
rpn::Interp::addFractionWords() {
  rpn::Interp &rpn = *this; // in case we want to move this out someday
  setWordCategory("fraction");
  registerType("fraction", typeid(stack::Fraction).hash_code());

  rpn.addDefinition("->FRAC", NATIVE_WORD_WDEF(fraction, rpn::StrictTypeValidator::d2_integer_integer, to_frac_ii, nullptr));
  rpn.addDefinition("->FRAC", NATIVE_WORD_WDEF(fraction, rpn::StrictTypeValidator::d1_double, to_frac_d, nullptr));
  rpn.addDefinition("->FLOAT", NATIVE_WORD_WDEF(fraction, frac_validator::d1_frac, to_float, nullptr));
  rpn.addDefinition("EVAL", NATIVE_WORD_WDEF(fraction, frac_validator::d1_frac, to_float, nullptr));
  rpn.addDefinition("OBJ->", NATIVE_WORD_WDEF(fraction, frac_validator::d1_frac, obj_to, nullptr));
  rpn.addDefinition("INV", NATIVE_WORD_WDEF(fraction, frac_validator::d1_frac, inv_f, nullptr));
  rpn.addDefinition("NEG", NATIVE_WORD_WDEF(fraction, frac_validator::d1_frac, neg_f, nullptr));
  rpn.addDefinition("SQ", NATIVE_WORD_WDEF(fraction, frac_validator::d1_frac, sq_f, nullptr));
  rpn.addDefinition("SQRT", NATIVE_WORD_WDEF(fraction, frac_validator::d1_frac, sqrt_f, nullptr));

  ADD_FRAC_NUM_WORD(rpn, "+", add);
  ADD_FRAC_NUM_WORD(rpn, "-", sub);
  ADD_FRAC_NUM_WORD(rpn, "*", mult);
  ADD_FRAC_NUM_WORD(rpn, "/", divide);
  ADD_FRAC_NUM_WORD(rpn, "^", pow);

  addWordMetadata("->FRAC",   "Create a fraction from integer numerator and denominator, or approximate a double as a fraction.");
  addWordMetadata("->FLOAT",  "Convert a fraction to double.");
  addWordMetadata("OBJ->",    "Explode a fraction to its integer numerator and denominator.");
  addWordMetadata("EVAL",     "Evaluate a fraction to its double value.");
  addWordMetadata("INV",      "Reciprocal of a fraction.");
  addWordMetadata("NEG",      "Negate a fraction.");
  addWordMetadata("SQ",       "Square a fraction.");
  addWordMetadata("SQRT",     "Square root of a fraction (returns a fraction approximation).");
}

/* end of QInc/Projects/color-calc/src/libs/rpn-lang/src/fraction-dict.cpp */
