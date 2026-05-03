/***************************************************
 * file: qinc/rpn-lang/src/mx3-dict.cpp
 *
 * @file    mx3-dict.cpp
 * @author  Eric L. Hernes
 * @born_on   Sunday, March 29, 2026
 * @copyright (C) Copyright Eric L. Hernes 2026
 * @copyright (C) Copyright Q, Inc. 2026
 *
 * @brief   Mx3 words: ->MX3, MX3->, MX3ID, MX3DIAG, *, and overloads for DET/TRANS/INV.
 *
 * Stack::Mx3 wraps q::Mx3 (vecmx3.h).  All operations create new Mx3 objects.
 * DET, TRANS, and INV are overloaded in matrix-dict.cpp to dispatch on type.
 *
 */

#include "../rpn-matrix.h"

#define MX3_WDEF(validator, fn) { rpn::StrictTypeValidator::validator, NATIVE_WORD_FN(mx3m, fn), nullptr }

// ---------------------------------------------------------------------------
// ->MX3  ( e11 e12 e13 e21 e22 e23 e31 e32 e33 -- mx3 )  9 numbers, row-major
// ---------------------------------------------------------------------------
NATIVE_WORD_DECL(mx3m, to_mx3) {
  double e33 = rpn.stack.pop_as_double();
  double e32 = rpn.stack.pop_as_double();
  double e31 = rpn.stack.pop_as_double();
  double e23 = rpn.stack.pop_as_double();
  double e22 = rpn.stack.pop_as_double();
  double e21 = rpn.stack.pop_as_double();
  double e13 = rpn.stack.pop_as_double();
  double e12 = rpn.stack.pop_as_double();
  double e11 = rpn.stack.pop_as_double();
  rpn.stack.push(stack::Mx3(e11, e12, e13, e21, e22, e23, e31, e32, e33));
  return rpn::WordDefinition::Result::ok;
}

// ---------------------------------------------------------------------------
// MX3->  ( mx3 -- e11 e12 e13 e21 e22 e23 e31 e32 e33 )  explode, row-major
// ---------------------------------------------------------------------------
NATIVE_WORD_DECL(mx3m, mx3_to) {
  auto sm = rpn.stack.pop();
  const auto &m = PEEK_CAST(stack::Mx3, *sm);
  for (int r = 0; r < 3; r++)
    for (int c = 0; c < 3; c++)
      rpn.stack.push_double(m(r, c));
  return rpn::WordDefinition::Result::ok;
}

// ---------------------------------------------------------------------------
// MX3ID  ( -- mx3 )  push identity
// ---------------------------------------------------------------------------
NATIVE_WORD_DECL(mx3m, mx3id) {
  rpn.stack.push(stack::Mx3());
  return rpn::WordDefinition::Result::ok;
}

// ---------------------------------------------------------------------------
// MX3DIAG  ( vec3 -- mx3 )  diagonal matrix from Vec3
// ---------------------------------------------------------------------------
NATIVE_WORD_DECL(mx3m, mx3diag) {
  auto sv = rpn.stack.pop();
  const auto &v = PEEK_CAST(stack::Vec3, *sv);
  rpn.stack.push(stack::Mx3(
    v[0], 0., 0.,
    0., v[1], 0.,
    0., 0., v[2]));
  return rpn::WordDefinition::Result::ok;
}

// ---------------------------------------------------------------------------
// *  ( mx3 vec3 -- vec3 )  matrix × vector
// ---------------------------------------------------------------------------
NATIVE_WORD_DECL(mx3m, mx3_mul_vec3) {
  auto sv = rpn.stack.pop();
  auto sm = rpn.stack.pop();
  const auto &v = PEEK_CAST(stack::Vec3, *sv);
  const auto &m = PEEK_CAST(stack::Mx3, *sm);
  q::Vec3 result = m.q::Mx3::operator*(static_cast<const q::Vec3 &>(v));
  rpn.stack.push(stack::Vec3(result[0], result[1], result[2]));
  return rpn::WordDefinition::Result::ok;
}

// ---------------------------------------------------------------------------
// *  ( m1 m2 -- m3 )  matrix × matrix  (NOS=m1, TOS=m2 → m1*m2)
// ---------------------------------------------------------------------------
NATIVE_WORD_DECL(mx3m, mx3_mul_mx3) {
  auto sm2 = rpn.stack.pop();
  auto sm1 = rpn.stack.pop();
  const auto &m2 = PEEK_CAST(stack::Mx3, *sm2);
  const auto &m1 = PEEK_CAST(stack::Mx3, *sm1);
  q::Mx3 r = m1.q::Mx3::operator*(static_cast<const q::Mx3 &>(m2));
  rpn.stack.push(stack::Mx3(
    r(0,0), r(0,1), r(0,2),
    r(1,0), r(1,1), r(1,2),
    r(2,0), r(2,1), r(2,2)));
  return rpn::WordDefinition::Result::ok;
}

// ---------------------------------------------------------------------------
// +  ( m1 m2 -- m3 )  element-wise add (NOS + TOS).  q::Mx3 has no operator+.
// ---------------------------------------------------------------------------
NATIVE_WORD_DECL(mx3m, mx3_add) {
  auto sm2 = rpn.stack.pop();   // TOS
  auto sm1 = rpn.stack.pop();   // NOS
  const auto &m2 = PEEK_CAST(stack::Mx3, *sm2);
  const auto &m1 = PEEK_CAST(stack::Mx3, *sm1);
  stack::Mx3 out;
  for (int r = 0; r < 3; r++)
    for (int c = 0; c < 3; c++)
      out(r, c) = m1(r, c) + m2(r, c);
  rpn.stack.push(out);
  return rpn::WordDefinition::Result::ok;
}

// ---------------------------------------------------------------------------
// -  ( m1 m2 -- m3 )  element-wise subtract (NOS - TOS).
// ---------------------------------------------------------------------------
NATIVE_WORD_DECL(mx3m, mx3_sub) {
  auto sm2 = rpn.stack.pop();   // TOS
  auto sm1 = rpn.stack.pop();   // NOS
  const auto &m2 = PEEK_CAST(stack::Mx3, *sm2);
  const auto &m1 = PEEK_CAST(stack::Mx3, *sm1);
  stack::Mx3 out;
  for (int r = 0; r < 3; r++)
    for (int c = 0; c < 3; c++)
      out(r, c) = m1(r, c) - m2(r, c);
  rpn.stack.push(out);
  return rpn::WordDefinition::Result::ok;
}

// ---------------------------------------------------------------------------
// *  ( mx3 scalar -- mx3 )  TOS=scalar (handles both double and integer).
// ---------------------------------------------------------------------------
NATIVE_WORD_DECL(mx3m, mx3_mul_scalar) {
  double s = rpn.stack.pop_as_double();
  auto sm = rpn.stack.pop();
  const auto &m = PEEK_CAST(stack::Mx3, *sm);
  q::Mx3 r = m.q::Mx3::operator*(s);
  rpn.stack.push(stack::Mx3(
    r(0,0), r(0,1), r(0,2),
    r(1,0), r(1,1), r(1,2),
    r(2,0), r(2,1), r(2,2)));
  return rpn::WordDefinition::Result::ok;
}

// ---------------------------------------------------------------------------
// *  ( scalar mx3 -- mx3 )  NOS=scalar.
// ---------------------------------------------------------------------------
NATIVE_WORD_DECL(mx3m, scalar_mul_mx3) {
  auto sm = rpn.stack.pop();   // TOS: mx3
  double s = rpn.stack.pop_as_double();   // NOS: scalar
  const auto &m = PEEK_CAST(stack::Mx3, *sm);
  q::Mx3 r = m.q::Mx3::operator*(s);
  rpn.stack.push(stack::Mx3(
    r(0,0), r(0,1), r(0,2),
    r(1,0), r(1,1), r(1,2),
    r(2,0), r(2,1), r(2,2)));
  return rpn::WordDefinition::Result::ok;
}

// ---------------------------------------------------------------------------
// /  ( mx3 scalar -- mx3 )  TOS=scalar; element-wise divide (m * 1/s).
// Returns param_error on s == 0.
// ---------------------------------------------------------------------------
NATIVE_WORD_DECL(mx3m, mx3_div_scalar) {
  double s = rpn.stack.pop_as_double();
  if (s == 0.) return rpn::WordDefinition::Result::param_error;
  auto sm = rpn.stack.pop();
  const auto &m = PEEK_CAST(stack::Mx3, *sm);
  q::Mx3 r = m.q::Mx3::operator*(1.0 / s);
  rpn.stack.push(stack::Mx3(
    r(0,0), r(0,1), r(0,2),
    r(1,0), r(1,1), r(1,2),
    r(2,0), r(2,1), r(2,2)));
  return rpn::WordDefinition::Result::ok;
}

// ---------------------------------------------------------------------------
// /  ( scalar mx3 -- mx3 )  NOS=scalar; convenience for s · inv(m).
// Singularity not checked here — matches mx3_inv (matrix-dict.cpp); a
// singular input yields NaN-laden output.
// ---------------------------------------------------------------------------
NATIVE_WORD_DECL(mx3m, scalar_div_mx3) {
  auto sm = rpn.stack.pop();   // TOS: mx3
  double s = rpn.stack.pop_as_double();   // NOS: scalar
  const auto &m = PEEK_CAST(stack::Mx3, *sm);
  q::Mx3 inv = m.q::Mx3::inverse();
  q::Mx3 r   = inv.q::Mx3::operator*(s);
  rpn.stack.push(stack::Mx3(
    r(0,0), r(0,1), r(0,2),
    r(1,0), r(1,1), r(1,2),
    r(2,0), r(2,1), r(2,2)));
  return rpn::WordDefinition::Result::ok;
}

// local validators
namespace mx3_validator {
  const rpn::StackSizeValidator nine{9};
  const rpn::StrictTypeValidator d2_mx3_mx3(
    {typeid(stack::Mx3).hash_code(), typeid(stack::Mx3).hash_code()}, "d2_mx3_mx3");

  // ctor order is {TOS, NOS}; suffix in name reads bottom→top.
  const rpn::StrictTypeValidator d2_mx3_double(
    {typeid(stack::Double).hash_code(),  typeid(stack::Mx3).hash_code()},     "d2_mx3_double");
  const rpn::StrictTypeValidator d2_mx3_integer(
    {typeid(stack::Integer).hash_code(), typeid(stack::Mx3).hash_code()},     "d2_mx3_integer");
  const rpn::StrictTypeValidator d2_double_mx3(
    {typeid(stack::Mx3).hash_code(),     typeid(stack::Double).hash_code()},  "d2_double_mx3");
  const rpn::StrictTypeValidator d2_integer_mx3(
    {typeid(stack::Mx3).hash_code(),     typeid(stack::Integer).hash_code()}, "d2_integer_mx3");
}

// ---------------------------------------------------------------------------
// addMx3Words
// ---------------------------------------------------------------------------
void
rpn::Interp::addMx3Words() {
  setWordCategory("mx3");

  addDefinition("->MX3",   { mx3_validator::nine,       NATIVE_WORD_FN(mx3m, to_mx3),      nullptr });
  addDefinition("MX3->",   MX3_WDEF(d1_mx3,              mx3_to));
  addDefinition("OBJ->",   MX3_WDEF(d1_mx3,              mx3_to));
  addDefinition("MX3ID",   { rpn::StackSizeValidator::zero, NATIVE_WORD_FN(mx3m, mx3id),   nullptr });
  addDefinition("MX3DIAG", MX3_WDEF(d1_vec3,              mx3diag));

  addDefinition("+",       { mx3_validator::d2_mx3_mx3,     NATIVE_WORD_FN(mx3m, mx3_add),       nullptr });
  addDefinition("-",       { mx3_validator::d2_mx3_mx3,     NATIVE_WORD_FN(mx3m, mx3_sub),       nullptr });
  addDefinition("*",       MX3_WDEF(d2_mx3_vec3,            mx3_mul_vec3));
  addDefinition("*",       { mx3_validator::d2_mx3_mx3,     NATIVE_WORD_FN(mx3m, mx3_mul_mx3),   nullptr });
  addDefinition("*",       { mx3_validator::d2_mx3_double,  NATIVE_WORD_FN(mx3m, mx3_mul_scalar),nullptr });
  addDefinition("*",       { mx3_validator::d2_mx3_integer, NATIVE_WORD_FN(mx3m, mx3_mul_scalar),nullptr });
  addDefinition("*",       { mx3_validator::d2_double_mx3,  NATIVE_WORD_FN(mx3m, scalar_mul_mx3),nullptr });
  addDefinition("*",       { mx3_validator::d2_integer_mx3, NATIVE_WORD_FN(mx3m, scalar_mul_mx3),nullptr });

  addDefinition("/",       { mx3_validator::d2_mx3_double,  NATIVE_WORD_FN(mx3m, mx3_div_scalar),nullptr });
  addDefinition("/",       { mx3_validator::d2_mx3_integer, NATIVE_WORD_FN(mx3m, mx3_div_scalar),nullptr });
  addDefinition("/",       { mx3_validator::d2_double_mx3,  NATIVE_WORD_FN(mx3m, scalar_div_mx3),nullptr });
  addDefinition("/",       { mx3_validator::d2_integer_mx3, NATIVE_WORD_FN(mx3m, scalar_div_mx3),nullptr });

  addWordMetadata("->MX3",   "Create a 3×3 matrix from 9 doubles (row-major: e11..e33).");
  addWordMetadata("MX3->",   "Explode a 3×3 matrix to 9 doubles (row-major).");
  addWordMetadata("MX3ID",   "Push the 3×3 identity matrix.");
  addWordMetadata("MX3DIAG", "Create a diagonal 3×3 matrix from a Vec3.");
  addWordMetadata("+",       "Element-wise add: Mx3 + Mx3 → Mx3.");
  addWordMetadata("-",       "Element-wise subtract: Mx3 - Mx3 → Mx3.");
  addWordMetadata("*",       "Multiply: Mx3 × Vec3 → Vec3; Mx3 × Mx3 → Mx3; Mx3 × scalar → Mx3.");
  addWordMetadata("/",       "Mx3 ÷ scalar → element-wise; scalar ÷ Mx3 → s · inv(Mx3).");

  setWordCategory("");
}

/* end of qinc/rpn-lang/src/mx3-dict.cpp */
