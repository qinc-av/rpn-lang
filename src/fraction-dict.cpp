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

using StFraction = stack::Fraction;

NATIVE_WORD_DECL(fraction, to_frac_ii) {
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  auto denom = rpn.stack.pop_integer();
  auto num = rpn.stack.pop_integer();
  rpn.stack.push(StFraction(num, denom));
  return rv;
}

NATIVE_WORD_DECL(fraction, to_frac_d) {
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  auto dec = rpn.stack.pop_as_double(); // should already be double
  rpn.stack.push(StFraction(dec));
  return rv;
}

NATIVE_WORD_DECL(fraction, obj_to) {
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  auto ofrac = rpn.stack.pop();
  const auto &frac = POP_CAST(StFraction,ofrac);
  rpn.stack.push_integer(frac._numerator);
  rpn.stack.push_integer(frac._denominator);
  return rv;
}

NATIVE_WORD_DECL(fraction, frac_to) {
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  auto ofrac = rpn.stack.pop();
  const auto &frac = POP_CAST(StFraction,ofrac);
  rpn.stack.push_double(frac);
  return rv;
}

NATIVE_WORD_DECL(fraction, inv_f) {
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  auto ofrac = rpn.stack.pop();
  const auto &frac = POP_CAST(StFraction,ofrac);
  rpn.stack.push(StFraction(frac._denominator,frac._numerator));
  return rv;
}

NATIVE_WORD_DECL(fraction, add_ff) {
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  auto o2 = rpn.stack.pop();
  const auto &f2 = POP_CAST(StFraction,o2);
  auto o1 = rpn.stack.pop();
  const auto &f1 = POP_CAST(StFraction,o1);
  rpn.stack.push(StFraction(f1 + f2));
  return rv;
}

NATIVE_WORD_DECL(fraction, add_fn) {
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  auto o2 = rpn.stack.pop();
  const auto &f2 = POP_CAST(StFraction,o2);
  auto d1 = rpn.stack.pop_as_double();
  rpn.stack.push(StFraction(d1 + f2));
  return rv;
}

NATIVE_WORD_DECL(fraction, add_nf) {
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  auto d2 = rpn.stack.pop_as_double();
  auto o1 = rpn.stack.pop();
  const auto &f1 = POP_CAST(StFraction,o1);
  rpn.stack.push(StFraction(f1 + d2));
  return rv;
}

NATIVE_WORD_DECL(fraction, sub_ff) {
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  auto o2 = rpn.stack.pop();
  const auto &f2 = POP_CAST(StFraction,o2);
  auto o1 = rpn.stack.pop();
  const auto &f1 = POP_CAST(StFraction,o1);
  rpn.stack.push(StFraction(f1 - f2));
  return rv;
}

NATIVE_WORD_DECL(fraction, sub_fn) {
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  auto o2 = rpn.stack.pop();
  const auto &f2 = POP_CAST(StFraction,o2);
  auto d1 = rpn.stack.pop_as_double();
  rpn.stack.push(StFraction(d1 - f2));
  return rv;
}

NATIVE_WORD_DECL(fraction, sub_nf) {
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  auto d2 = rpn.stack.pop_as_double();
  auto o1 = rpn.stack.pop();
  const auto &f1 = POP_CAST(StFraction,o1);
  rpn.stack.push(StFraction(f1 - d2));
  return rv;
}

NATIVE_WORD_DECL(fraction, mult_fn) {
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  auto o2 = rpn.stack.pop();
  const auto &f2 = POP_CAST(StFraction,o2);
  auto d1 = rpn.stack.pop_as_double();
  rpn.stack.push(StFraction(d1 * f2));
  return rv;
}

NATIVE_WORD_DECL(fraction, mult_nf) {
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  auto d2 = rpn.stack.pop_as_double();
  auto o1 = rpn.stack.pop();
  const auto &f1 = POP_CAST(StFraction,o1);
  rpn.stack.push(StFraction(f1 * d2));
  return rv;
}

NATIVE_WORD_DECL(fraction, mult_ff) {
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  auto o2 = rpn.stack.pop();
  const auto &f2 = POP_CAST(StFraction,o2);
  auto o1 = rpn.stack.pop();
  const auto &f1 = POP_CAST(StFraction,o1);
  rpn.stack.push(StFraction(f1 * f2));
  return rv;
}


NATIVE_WORD_DECL(fraction, divide_fn) {
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  auto o2 = rpn.stack.pop();
  const auto &f2 = POP_CAST(StFraction,o2);
  auto d1 = rpn.stack.pop_as_double();
  rpn.stack.push(StFraction(d1 / f2));
  return rv;
}

NATIVE_WORD_DECL(fraction, divide_nf) {
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  auto d2 = rpn.stack.pop_as_double();
  auto o1 = rpn.stack.pop();
  const auto &f1 = POP_CAST(StFraction,o1);
  rpn.stack.push(StFraction(f1 / d2));
  return rv;
}

NATIVE_WORD_DECL(fraction, divide_ff) {
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  auto o2 = rpn.stack.pop();
  const auto &f2 = POP_CAST(StFraction,o2);
  auto o1 = rpn.stack.pop();
  const auto &f1 = POP_CAST(StFraction,o1);
  rpn.stack.push(StFraction(f1 / f2));
  return rv;
}

NATIVE_WORD_DECL(fraction, sq_f) {
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  auto o = rpn.stack.pop();
  const auto &f = POP_CAST(StFraction,o);
  rpn.stack.push(StFraction(f * f));
  return rv;
}

NATIVE_WORD_DECL(fraction, sqrt_f) {
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  auto o = rpn.stack.pop();
  const auto &f = POP_CAST(StFraction,o);
  rpn.stack.push(StFraction(std::sqrt(f)));
  return rv;
}

NATIVE_WORD_DECL(fraction, pow_fn) {
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  auto o2 = rpn.stack.pop();
  const auto &f2 = POP_CAST(StFraction,o2);
  auto d1 = rpn.stack.pop_as_double();
  rpn.stack.push(StFraction(std::pow(d1, double(f2))));
  return rv;
}

NATIVE_WORD_DECL(fraction, pow_nf) {
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  auto d2 = rpn.stack.pop_as_double();
  auto o1 = rpn.stack.pop();
  const auto &f1 = POP_CAST(StFraction,o1);
  rpn.stack.push(StFraction(std::pow(double(f1), d2)));
  return rv;
}

NATIVE_WORD_DECL(fraction, pow_ff) {
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  auto o2 = rpn.stack.pop();
  double d2 = POP_CAST(StFraction,o2);
  auto o1 = rpn.stack.pop();
  double d1 = POP_CAST(StFraction,o1);
  double result = std::pow(d1,d2);
  rpn.stack.push(StFraction(result));
  return rv;
}

NATIVE_WORD_DECL(fraction, neg_f) {
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  auto o1 = rpn.stack.pop();
  const auto &f1 = POP_CAST(StFraction,o1);
  rpn.stack.push(StFraction(f1.neg()));
  return rv;
}

const rpn::StrictTypeValidator frac_validator::d1_frac({typeid(StFraction).hash_code()});
const rpn::StrictTypeValidator frac_validator::d2_frac_frac({typeid(StFraction).hash_code(),typeid(StFraction).hash_code()});
const rpn::StrictTypeValidator frac_validator::d2_frac_int({typeid(StFraction).hash_code(),typeid(StInteger).hash_code()});
const rpn::StrictTypeValidator frac_validator::d2_frac_double({typeid(StFraction).hash_code(),typeid(StDouble).hash_code()});
const rpn::StrictTypeValidator frac_validator::d2_int_frac({typeid(StInteger).hash_code(),typeid(StFraction).hash_code()});
const rpn::StrictTypeValidator frac_validator::d2_double_frac({typeid(StDouble).hash_code(),typeid(StFraction).hash_code()});
const rpn::StrictTypeValidator frac_validator::d5_int_int_int_int_frac({
    typeid(StInteger).hash_code(),typeid(StInteger).hash_code(),typeid(StInteger).hash_code(),typeid(StInteger).hash_code(),
      typeid(StFraction).hash_code()});

#define ADD_FRAC_NUM_WORD(rpn, word_token, method)			\
  rpn.addDefinition(word_token, NATIVE_WORD_WDEF(fraction, frac_validator::d2_frac_int, method##_fn, nullptr)); \
  rpn.addDefinition(word_token, NATIVE_WORD_WDEF(fraction, frac_validator::d2_frac_double, method##_fn, nullptr)); \
  rpn.addDefinition(word_token, NATIVE_WORD_WDEF(fraction, frac_validator::d2_int_frac, method##_nf, nullptr)); \
  rpn.addDefinition(word_token, NATIVE_WORD_WDEF(fraction, frac_validator::d2_double_frac, method##_nf, nullptr)); \
  rpn.addDefinition(word_token, NATIVE_WORD_WDEF(fraction, frac_validator::d2_frac_frac, method##_ff, nullptr))

void
rpn::Interp::addFractionWords() {
  rpn::Interp &rpn = *this; // in case we want to move this out someday
  
  rpn.addDefinition("->FRAC", NATIVE_WORD_WDEF(fraction, rpn::StrictTypeValidator::d2_integer_integer, to_frac_ii, nullptr));
  rpn.addDefinition("->FRAC", NATIVE_WORD_WDEF(fraction, rpn::StrictTypeValidator::d1_double, to_frac_d, nullptr));
  rpn.addDefinition("->DOUBLE", NATIVE_WORD_WDEF(fraction, frac_validator::d1_frac, frac_to, nullptr));
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
}

/* end of QInc/Projects/color-calc/src/libs/rpn-lang/src/fraction-dict.cpp */
