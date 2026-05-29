/***************************************************
 * file: qinc/rpn-lang/src/vec3-dict.cpp
 *
 * @file    vec3-dict.cpp
 * @author  Eric L. Hernes
 * @born_on   Sunday, March 29, 2026
 * @copyright (C) Copyright Eric L. Hernes 2026
 * @copyright (C) Copyright Q, Inc. 2026
 *
 * @brief   Vec3 math words: DOT3, CROSS3, NORM3, VEC3->MX3COL, VEC3->MX3ROW.
 *
 */

#include "../rpn-matrix.h"
#include <cmath>

#define VEC3_WDEF(validator, fn) { rpn::StrictTypeValidator::validator, NATIVE_WORD_FN(vec3m, fn), nullptr }

// ---------------------------------------------------------------------------
// DOT3  ( v1 v2 -- dot )  dot product
// NOS = v1, TOS = v2
// ---------------------------------------------------------------------------
NATIVE_WORD_DECL(vec3m, dot3) {
  auto s2 = rpn.stack.pop();
  auto s1 = rpn.stack.pop();
  const auto &v2 = PEEK_CAST(stack::Vec3, *s2);
  const auto &v1 = PEEK_CAST(stack::Vec3, *s1);
  double dot = v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2];
  rpn.stack.push_double(dot);
  return rpn::WordDefinition::Result::ok;
}

// ---------------------------------------------------------------------------
// CROSS3  ( v1 v2 -- v3 )  cross product
// NOS = v1, TOS = v2; result = v1 × v2
// ---------------------------------------------------------------------------
NATIVE_WORD_DECL(vec3m, cross3) {
  auto s2 = rpn.stack.pop();
  auto s1 = rpn.stack.pop();
  const auto &v2 = PEEK_CAST(stack::Vec3, *s2);
  const auto &v1 = PEEK_CAST(stack::Vec3, *s1);
  rpn.stack.push(stack::Vec3(
    v1[1]*v2[2] - v1[2]*v2[1],
    v1[2]*v2[0] - v1[0]*v2[2],
    v1[0]*v2[1] - v1[1]*v2[0]));
  return rpn::WordDefinition::Result::ok;
}

// ---------------------------------------------------------------------------
// NORM3  ( v -- norm )  Euclidean norm
// ---------------------------------------------------------------------------
NATIVE_WORD_DECL(vec3m, norm3) {
  auto s = rpn.stack.pop();
  const auto &v = PEEK_CAST(stack::Vec3, *s);
  rpn.stack.push_double(std::sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]));
  return rpn::WordDefinition::Result::ok;
}

// ---------------------------------------------------------------------------
// *  ( vec3 scalar -- vec3 )  TOS=scalar
// ---------------------------------------------------------------------------
NATIVE_WORD_DECL(vec3m, vec3_mul_scalar) {
  double s = rpn.stack.pop_as_double();
  auto sv = rpn.stack.pop();
  const auto &v = PEEK_CAST(stack::Vec3, *sv);
  rpn.stack.push(stack::Vec3(v[0]*s, v[1]*s, v[2]*s));
  return rpn::WordDefinition::Result::ok;
}

// ---------------------------------------------------------------------------
// *  ( scalar vec3 -- vec3 )  NOS=scalar
// ---------------------------------------------------------------------------
NATIVE_WORD_DECL(vec3m, scalar_mul_vec3) {
  auto sv = rpn.stack.pop();
  double s = rpn.stack.pop_as_double();
  const auto &v = PEEK_CAST(stack::Vec3, *sv);
  rpn.stack.push(stack::Vec3(v[0]*s, v[1]*s, v[2]*s));
  return rpn::WordDefinition::Result::ok;
}

// ---------------------------------------------------------------------------
// addVec3Dictionary
// ---------------------------------------------------------------------------
void
rpn::Interp::addVec3Dictionary() {
  if (_alreadyRegistered("vec3")) return;
  setWordCategory("vec3");

  addDefinition("DOT3",   {rpn::StrictTypeValidator::d2_vec3_vec3, NATIVE_WORD_FN(vec3m, dot3),   nullptr, "", rpn::StackEffect{{{"a", "vec3"}, {"b", "vec3"}}, {{"dot", "number"}}}});
  addDefinition("CROSS3", {rpn::StrictTypeValidator::d2_vec3_vec3, NATIVE_WORD_FN(vec3m, cross3), nullptr, "", rpn::StackEffect{{{"a", "vec3"}, {"b", "vec3"}}, {{"cross", "vec3"}}}});
  addDefinition("NORM3",  {rpn::StrictTypeValidator::d1_vec3,      NATIVE_WORD_FN(vec3m, norm3),  nullptr, "", rpn::StackEffect{{{"v", "vec3"}}, {{"norm", "number"}}}});

  // Vec3 +/- live in types-dict.cpp; scalar * was missing — fill the gap.
  addDefinition("*", {rpn::StrictTypeValidator::d2_vec3_double,  NATIVE_WORD_FN(vec3m, vec3_mul_scalar), nullptr, "", rpn::StackEffect{{{"v", "vec3"}, {"s", "number"}},  {{"scaled", "vec3"}}}});
  addDefinition("*", {rpn::StrictTypeValidator::d2_vec3_integer, NATIVE_WORD_FN(vec3m, vec3_mul_scalar), nullptr, "", rpn::StackEffect{{{"v", "vec3"}, {"s", "integer"}}, {{"scaled", "vec3"}}}});
  addDefinition("*", {rpn::StrictTypeValidator::d2_double_vec3,  NATIVE_WORD_FN(vec3m, scalar_mul_vec3), nullptr, "", rpn::StackEffect{{{"s", "number"},  {"v", "vec3"}}, {{"scaled", "vec3"}}}});
  addDefinition("*", {rpn::StrictTypeValidator::d2_integer_vec3, NATIVE_WORD_FN(vec3m, scalar_mul_vec3), nullptr, "", rpn::StackEffect{{{"s", "integer"}, {"v", "vec3"}}, {{"scaled", "vec3"}}}});

  addWordMetadata("DOT3",   "Dot product of two Vec3 vectors.");
  addWordMetadata("CROSS3", "Cross product: `v1 v2 CROSS3` → v1×v2.");
  addWordMetadata("NORM3",  "Euclidean norm (length) of a Vec3.");
  // * for Vec3 scaling is an extension overload; math-dict owns the metadata.

  setWordCategory("");
}

/* end of qinc/rpn-lang/src/vec3-dict.cpp */
