/***************************************************
 * file: qinc/rpn-lang/src/matrix-dict.cpp
 *
 * @file    matrix-dict.cpp
 * @author  Eric L. Hernes
 * @version V1.0
 * @born_on   Saturday, March 28, 2026
 * @copyright (C) Copyright Eric L. Hernes 2026
 * @copyright (C) Copyright Q, Inc. 2026
 *
 * @brief   ::stack::Vector and ::stack::Matrix words for rpn-lang.
 *
 * Types (stack::Vector, stack::Matrix) are defined locally here, backed by
 * math::matrix<double> (Techsoft Matrix TCL Lite v1.13, src/matrix.h).
 * Validators use a local matrix_validator:: namespace (same pattern as
 * stack::Complex in math-dict.cpp).
 *
 */

#include "rpn-matrix-types.h"

// stack::Vector and stack::Matrix are defined in rpn-matrix-types.h
// Use ::stack::Vector / ::stack::Matrix to disambiguate from std::stack
// (matrix.h pulls in using namespace std).


// ---------------------------------------------------------------------------
// Local validators (same pattern as math_validator::d1_complex in math-dict.cpp)
// ---------------------------------------------------------------------------
namespace matrix_validator {
  const rpn::StrictTypeValidator d1_vector({typeid(::stack::Vector).hash_code()}, "d1_vector");
  const rpn::StrictTypeValidator d1_matrix({typeid(::stack::Matrix).hash_code()}, "d1_matrix");
  const rpn::StrictTypeValidator d2_vector_vector({typeid(::stack::Vector).hash_code(), typeid(::stack::Vector).hash_code()}, "d2_vector_vector");
  const rpn::StrictTypeValidator d2_matrix_matrix({typeid(::stack::Matrix).hash_code(), typeid(::stack::Matrix).hash_code()}, "d2_matrix_matrix");
  const rpn::StrictTypeValidator d2_vector_matrix({typeid(::stack::Matrix).hash_code(), typeid(::stack::Vector).hash_code()}, "d2_vector_matrix");
  const rpn::StrictTypeValidator d2_matrix_vector({typeid(::stack::Vector).hash_code(), typeid(::stack::Matrix).hash_code()}, "d2_matrix_vector");
  const rpn::StrictTypeValidator d2_double_vector( {typeid(::stack::Vector).hash_code(), typeid(::stack::Double).hash_code()},  "d2_double_vector");
  const rpn::StrictTypeValidator d2_integer_vector({typeid(::stack::Vector).hash_code(), typeid(::stack::Integer).hash_code()}, "d2_integer_vector");
  const rpn::StrictTypeValidator d2_vector_double( {typeid(::stack::Double).hash_code(),  typeid(::stack::Vector).hash_code()}, "d2_vector_double");
  const rpn::StrictTypeValidator d2_vector_integer({typeid(::stack::Integer).hash_code(), typeid(::stack::Vector).hash_code()}, "d2_vector_integer");
  const rpn::StrictTypeValidator d2_double_matrix( {typeid(::stack::Matrix).hash_code(), typeid(::stack::Double).hash_code()},  "d2_double_matrix");
  const rpn::StrictTypeValidator d2_integer_matrix({typeid(::stack::Matrix).hash_code(), typeid(::stack::Integer).hash_code()}, "d2_integer_matrix");
  const rpn::StrictTypeValidator d2_matrix_double( {typeid(::stack::Double).hash_code(),  typeid(::stack::Matrix).hash_code()}, "d2_matrix_double");
  const rpn::StrictTypeValidator d2_matrix_integer({typeid(::stack::Integer).hash_code(), typeid(::stack::Matrix).hash_code()}, "d2_matrix_integer");
}

#define MV_WDEF(validator, fn) { matrix_validator::validator, NATIVE_WORD_FN(matrix, fn), nullptr }
#define MAT_FUNC(op)           NATIVE_WORD_FN(matrix, op)

// ---------------------------------------------------------------------------
// Vector words
// ---------------------------------------------------------------------------

// ->VEC  ( v0 v1 ... vN-1 n -- vec )
// TOS = n (integer count); below = n doubles (bottom = v[0], TOS-1 = v[N-1])
NATIVE_WORD_DECL(matrix, to_vector) {
  size_t n = (size_t)rpn.stack.pop_as_integer();
  if (n == 0) {
    rpn.stack.push(::stack::Vector(std::vector<double>{}));
    return rpn::WordDefinition::Result::ok;
  }
  std::vector<double> vals(n);
  for (size_t i = n; i > 0; i--)
    vals[i - 1] = rpn.stack.pop_as_double();
  rpn.stack.push(::stack::Vector(vals));
  return rpn::WordDefinition::Result::ok;
}

// VEC->  ( vec -- v0 v1 ... vN-1 n )
NATIVE_WORD_DECL(matrix, vector_to) {
  auto sv = rpn.stack.pop();
  const auto &v = PEEK_CAST(const ::stack::Vector, *sv);
  for (size_t i = 0; i < v.size(); i++)
    rpn.stack.push_double(v.get(i));
  rpn.stack.push_integer((int64_t)v.size());
  return rpn::WordDefinition::Result::ok;
}

// SIZE  ( vec -- n )
NATIVE_WORD_DECL(matrix, vec_size) {
  auto sv = rpn.stack.pop();
  const auto &v = PEEK_CAST(const ::stack::Vector, *sv);
  rpn.stack.push_integer((int64_t)v.size());
  return rpn::WordDefinition::Result::ok;
}

// VDOT  ( v1 v2 -- dot )  NOS · TOS
NATIVE_WORD_DECL(matrix, vec_dot) {
  auto sv1 = rpn.stack.pop();  // TOS
  auto sv2 = rpn.stack.pop();  // NOS
  const auto &v1 = PEEK_CAST(const ::stack::Vector, *sv1);
  const auto &v2 = PEEK_CAST(const ::stack::Vector, *sv2);
  if (v1.size() != v2.size()) return rpn::WordDefinition::Result::param_error;
  double dot = 0.0;
  for (size_t i = 0; i < v1.size(); i++) dot += v2.get(i) * v1.get(i);
  rpn.stack.push_double(dot);
  return rpn::WordDefinition::Result::ok;
}

// VNORM  ( vec -- norm )  L2 norm
NATIVE_WORD_DECL(matrix, vec_norm) {
  auto sv = rpn.stack.pop();
  const auto &v = PEEK_CAST(const ::stack::Vector, *sv);
  double norm = 0.0;
  for (size_t i = 0; i < v.size(); i++) norm += v.get(i) * v.get(i);
  rpn.stack.push_double(std::sqrt(norm));
  return rpn::WordDefinition::Result::ok;
}

// +  ( v1 v2 -- v )  element-wise NOS + TOS
NATIVE_WORD_DECL(matrix, vec_add) {
  auto sv1 = rpn.stack.pop();  // TOS
  auto sv2 = rpn.stack.pop();  // NOS
  const auto &v1 = PEEK_CAST(const ::stack::Vector, *sv1);
  const auto &v2 = PEEK_CAST(const ::stack::Vector, *sv2);
  if (v1.size() != v2.size()) return rpn::WordDefinition::Result::param_error;
  rpn.stack.push(::stack::Vector(v2.mat() + v1.mat()));
  return rpn::WordDefinition::Result::ok;
}

// -  ( v1 v2 -- v )  element-wise NOS - TOS
NATIVE_WORD_DECL(matrix, vec_sub) {
  auto sv1 = rpn.stack.pop();  // TOS
  auto sv2 = rpn.stack.pop();  // NOS
  const auto &v1 = PEEK_CAST(const ::stack::Vector, *sv1);
  const auto &v2 = PEEK_CAST(const ::stack::Vector, *sv2);
  if (v1.size() != v2.size()) return rpn::WordDefinition::Result::param_error;
  rpn.stack.push(::stack::Vector(v2.mat() - v1.mat()));
  return rpn::WordDefinition::Result::ok;
}

// *  ( vec scalar -- vec )  scale, TOS = scalar
NATIVE_WORD_DECL(matrix, vec_scale_double) {
  double s = rpn.stack.pop_as_double();
  auto sv = rpn.stack.pop();
  auto &v = PEEK_CAST(::stack::Vector, *sv);
  v.mat() *= s;
  rpn.stack.push(v);
  return rpn::WordDefinition::Result::ok;
}

// *  ( scalar vec -- vec )  scale, NOS = scalar
NATIVE_WORD_DECL(matrix, scale_double_vec) {
  auto sv = rpn.stack.pop();   // TOS: vec
  double s = rpn.stack.pop_as_double();  // NOS: scalar
  auto &v = PEEK_CAST(::stack::Vector, *sv);
  v.mat() *= s;
  rpn.stack.push(v);
  return rpn::WordDefinition::Result::ok;
}

// VEC->COLVEC  ( vec -- matrix )  N×1
NATIVE_WORD_DECL(matrix, vec_to_colvec) {
  auto sv = rpn.stack.pop();
  const auto &v = PEEK_CAST(const ::stack::Vector, *sv);
  rpn.stack.push(::stack::Matrix(v.mat()));
  return rpn::WordDefinition::Result::ok;
}

// VEC->ROWVEC  ( vec -- matrix )  1×N
NATIVE_WORD_DECL(matrix, vec_to_rowvec) {
  auto sv = rpn.stack.pop();
  auto &v = PEEK_CAST(::stack::Vector, *sv);
  rpn.stack.push(::stack::Matrix(~v.mat()));
  return rpn::WordDefinition::Result::ok;
}

// COLVEC->VEC  ( matrix -- vec )  requires N×1
NATIVE_WORD_DECL(matrix, colvec_to_vec) {
  auto sm = rpn.stack.pop();
  const auto &m = PEEK_CAST(const ::stack::Matrix, *sm);
  if (m.cols() != 1) return rpn::WordDefinition::Result::param_error;
  rpn.stack.push(::stack::Vector(m.mat()));
  return rpn::WordDefinition::Result::ok;
}

// VEC3->VEC  ( vec3 -- vec )  3-element vector
NATIVE_WORD_DECL(matrix, vec3_to_vec) {
  auto sv3 = rpn.stack.pop();
  const auto &v3 = PEEK_CAST(const StVec3, *sv3);
  std::vector<double> vals = {v3._x, v3._y, v3._z};
  rpn.stack.push(::stack::Vector(vals));
  return rpn::WordDefinition::Result::ok;
}

// VEC->VEC3  ( vec -- vec3 )  requires size == 3
NATIVE_WORD_DECL(matrix, vec_to_vec3) {
  auto sv = rpn.stack.pop();
  const auto &v = PEEK_CAST(const ::stack::Vector, *sv);
  if (v.size() != 3) return rpn::WordDefinition::Result::param_error;
  rpn.stack.push(StVec3(v.get(0), v.get(1), v.get(2)));
  return rpn::WordDefinition::Result::ok;
}

// ---------------------------------------------------------------------------
// Matrix words
// ---------------------------------------------------------------------------

// ->MATRIX  ( v00 v01 ... vRC rows cols -- matrix )
// rows and cols are integers; elements in row-major order, TOS = last element
NATIVE_WORD_DECL(matrix, to_matrix) {
  size_t cols = (size_t)rpn.stack.pop_as_integer();
  size_t rows = (size_t)rpn.stack.pop_as_integer();
  if (rows == 0 || cols == 0) return rpn::WordDefinition::Result::param_error;
  ::stack::Matrix m(rows, cols);
  for (size_t r = rows; r > 0; r--)
    for (size_t c = cols; c > 0; c--)
      m.set(r - 1, c - 1, rpn.stack.pop_as_double());
  rpn.stack.push(m);
  return rpn::WordDefinition::Result::ok;
}

// MATRIX->  ( matrix -- v00 v01 ... vRC rows cols )
NATIVE_WORD_DECL(matrix, matrix_to) {
  auto sm = rpn.stack.pop();
  const auto &m = PEEK_CAST(const ::stack::Matrix, *sm);
  for (size_t r = 0; r < m.rows(); r++)
    for (size_t c = 0; c < m.cols(); c++)
      rpn.stack.push_double(m.get(r, c));
  rpn.stack.push_integer((int64_t)m.rows());
  rpn.stack.push_integer((int64_t)m.cols());
  return rpn::WordDefinition::Result::ok;
}

// ROWS  ( matrix -- n )
NATIVE_WORD_DECL(matrix, mat_rows) {
  auto sm = rpn.stack.pop();
  const auto &m = PEEK_CAST(const ::stack::Matrix, *sm);
  rpn.stack.push_integer((int64_t)m.rows());
  return rpn::WordDefinition::Result::ok;
}

// COLS  ( matrix -- n )
NATIVE_WORD_DECL(matrix, mat_cols) {
  auto sm = rpn.stack.pop();
  const auto &m = PEEK_CAST(const ::stack::Matrix, *sm);
  rpn.stack.push_integer((int64_t)m.cols());
  return rpn::WordDefinition::Result::ok;
}

// +  ( m1 m2 -- m )  NOS + TOS; dimensions must match
NATIVE_WORD_DECL(matrix, mat_add) {
  auto sm1 = rpn.stack.pop();  // TOS
  auto sm2 = rpn.stack.pop();  // NOS
  const auto &m1 = PEEK_CAST(const ::stack::Matrix, *sm1);
  const auto &m2 = PEEK_CAST(const ::stack::Matrix, *sm2);
  if (m1.rows() != m2.rows() || m1.cols() != m2.cols())
    return rpn::WordDefinition::Result::param_error;
  rpn.stack.push(::stack::Matrix(m2.mat() + m1.mat()));
  return rpn::WordDefinition::Result::ok;
}

// -  ( m1 m2 -- m )  NOS - TOS
NATIVE_WORD_DECL(matrix, mat_sub) {
  auto sm1 = rpn.stack.pop();  // TOS
  auto sm2 = rpn.stack.pop();  // NOS
  const auto &m1 = PEEK_CAST(const ::stack::Matrix, *sm1);
  const auto &m2 = PEEK_CAST(const ::stack::Matrix, *sm2);
  if (m1.rows() != m2.rows() || m1.cols() != m2.cols())
    return rpn::WordDefinition::Result::param_error;
  rpn.stack.push(::stack::Matrix(m2.mat() - m1.mat()));
  return rpn::WordDefinition::Result::ok;
}

// *  ( m1 m2 -- m )  matrix multiply NOS × TOS; NOS.cols == TOS.rows
NATIVE_WORD_DECL(matrix, mat_mul) {
  auto sm1 = rpn.stack.pop();  // TOS
  auto sm2 = rpn.stack.pop();  // NOS
  const auto &m1 = PEEK_CAST(const ::stack::Matrix, *sm1);
  const auto &m2 = PEEK_CAST(const ::stack::Matrix, *sm2);
  if (m2.cols() != m1.rows()) return rpn::WordDefinition::Result::param_error;
  rpn.stack.push(::stack::Matrix(m2.mat() * m1.mat()));
  return rpn::WordDefinition::Result::ok;
}

// *  ( matrix vec -- vec )  matrix-vector multiply NOS × TOS
NATIVE_WORD_DECL(matrix, mat_vec_mul) {
  auto sv = rpn.stack.pop();   // TOS: vector
  auto sm = rpn.stack.pop();   // NOS: matrix
  const auto &v = PEEK_CAST(const ::stack::Vector, *sv);
  const auto &m = PEEK_CAST(const ::stack::Matrix, *sm);
  if (m.cols() != v.size()) return rpn::WordDefinition::Result::param_error;
  rpn.stack.push(::stack::Vector(m.mat() * v.mat()));
  return rpn::WordDefinition::Result::ok;
}

// *  ( matrix scalar -- matrix )  TOS = scalar
NATIVE_WORD_DECL(matrix, mat_scale_double) {
  double s = rpn.stack.pop_as_double();
  auto sm = rpn.stack.pop();
  auto &m = PEEK_CAST(::stack::Matrix, *sm);
  m.mat() *= s;
  rpn.stack.push(m);
  return rpn::WordDefinition::Result::ok;
}

// *  ( scalar matrix -- matrix )  NOS = scalar
NATIVE_WORD_DECL(matrix, scale_double_mat) {
  auto sm = rpn.stack.pop();   // TOS: matrix
  double s = rpn.stack.pop_as_double();  // NOS: scalar
  auto &m = PEEK_CAST(::stack::Matrix, *sm);
  m.mat() *= s;
  rpn.stack.push(m);
  return rpn::WordDefinition::Result::ok;
}

// DET  ( matrix -- d )  determinant; square matrix required
NATIVE_WORD_DECL(matrix, mat_det) {
  auto sm = rpn.stack.pop();
  const auto &m = PEEK_CAST(const ::stack::Matrix, *sm);
  if (m.rows() != m.cols()) return rpn::WordDefinition::Result::param_error;
  rpn.stack.push_double(m.mat().Det());
  return rpn::WordDefinition::Result::ok;
}

// TRANS  ( matrix -- matrix )  transpose
NATIVE_WORD_DECL(matrix, mat_trans) {
  auto sm = rpn.stack.pop();
  auto &m = PEEK_CAST(::stack::Matrix, *sm);
  rpn.stack.push(::stack::Matrix(~m.mat()));
  return rpn::WordDefinition::Result::ok;
}

// INV  ( matrix -- matrix )  inverse; square non-singular required
NATIVE_WORD_DECL(matrix, mat_inv) {
  auto sm = rpn.stack.pop();
  auto &m = PEEK_CAST(::stack::Matrix, *sm);
  if (m.rows() != m.cols()) return rpn::WordDefinition::Result::param_error;
  rpn.stack.push(::stack::Matrix(m.mat().Inv()));
  return rpn::WordDefinition::Result::ok;
}

// IDENTITY  ( n -- matrix )  n×n identity matrix
NATIVE_WORD_DECL(matrix, mat_identity) {
  size_t n = (size_t)rpn.stack.pop_as_integer();
  if (n == 0) return rpn::WordDefinition::Result::param_error;
  ::stack::Matrix m(n, n);
  m.mat().Identity();
  rpn.stack.push(m);
  return rpn::WordDefinition::Result::ok;
}

// ---------------------------------------------------------------------------
// addMatrixWords
// ---------------------------------------------------------------------------

void
rpn::Interp::addMatrixWords() {
  rpn::Interp &rpn(*this);
  setWordCategory("vector");

  // ------- Vector construction / decomposition -------
  rpn.addDefinition("->VEC",  {rpn::StackSizeValidator::ntos, MAT_FUNC(to_vector), nullptr});
  rpn.addDefinition("VEC->",  MV_WDEF(d1_vector, vector_to));
  rpn.addDefinition("SIZE",   MV_WDEF(d1_vector, vec_size));

  // ------- Vector arithmetic -------
  rpn.addDefinition("+", MV_WDEF(d2_vector_vector,  vec_add));
  rpn.addDefinition("-", MV_WDEF(d2_vector_vector,  vec_sub));
  rpn.addDefinition("*", MV_WDEF(d2_vector_double,  vec_scale_double));  // TOS=double, pop double first
  rpn.addDefinition("*", MV_WDEF(d2_vector_integer, vec_scale_double));  // TOS=int,    pop as double first
  rpn.addDefinition("*", MV_WDEF(d2_double_vector,  scale_double_vec));  // TOS=vector, pop vec first
  rpn.addDefinition("*", MV_WDEF(d2_integer_vector, scale_double_vec));  // TOS=vector, pop vec first

  // ------- Vector operations -------
  rpn.addDefinition("VDOT",  MV_WDEF(d2_vector_vector, vec_dot));
  rpn.addDefinition("VNORM", MV_WDEF(d1_vector,        vec_norm));

  // ------- Vector ↔ Matrix interop -------
  rpn.addDefinition("VEC->COLVEC", MV_WDEF(d1_vector, vec_to_colvec));
  rpn.addDefinition("VEC->ROWVEC", MV_WDEF(d1_vector, vec_to_rowvec));
  rpn.addDefinition("COLVEC->VEC", MV_WDEF(d1_matrix, colvec_to_vec));

  // ------- Vector ↔ VEC3 interop -------
  rpn.addDefinition("VEC3->VEC", {rpn::StrictTypeValidator::d1_vec3, MAT_FUNC(vec3_to_vec), nullptr});
  rpn.addDefinition("VEC->VEC3", MV_WDEF(d1_vector, vec_to_vec3));

  addWordMetadata("->VEC",       "Build a vector from N doubles. TOS=N; below = elements bottom→TOS = v[0]...v[N-1].");
  addWordMetadata("VEC->",       "Decompose vector: pushes v[0]...v[N-1] then N.");
  addWordMetadata("SIZE",        "Push element count of a vector.");
  addWordMetadata("VDOT",        "Dot product of two equal-length vectors (NOS · TOS).");
  addWordMetadata("VNORM",       "L2 (Euclidean) norm of a vector.");
  addWordMetadata("VEC->COLVEC", "Convert a vector to an N×1 column-vector matrix.");
  addWordMetadata("VEC->ROWVEC", "Convert a vector to a 1×N row-vector matrix.");
  addWordMetadata("COLVEC->VEC", "Convert an N×1 matrix to a vector.");
  addWordMetadata("VEC3->VEC",   "Convert a VEC3 to a 3-element vector.");
  addWordMetadata("VEC->VEC3",   "Convert a 3-element vector to a VEC3.");

  // ------- Matrix construction / decomposition -------
  setWordCategory("matrix");

  rpn.addDefinition("->MATRIX", {rpn::StackSizeValidator::two, MAT_FUNC(to_matrix), nullptr});
  rpn.addDefinition("MATRIX->", MV_WDEF(d1_matrix, matrix_to));
  rpn.addDefinition("ROWS",     MV_WDEF(d1_matrix, mat_rows));
  rpn.addDefinition("COLS",     MV_WDEF(d1_matrix, mat_cols));

  // ------- Matrix arithmetic -------
  rpn.addDefinition("+", MV_WDEF(d2_matrix_matrix,  mat_add));
  rpn.addDefinition("-", MV_WDEF(d2_matrix_matrix,  mat_sub));
  rpn.addDefinition("*", MV_WDEF(d2_matrix_matrix,  mat_mul));
  rpn.addDefinition("*", MV_WDEF(d2_matrix_vector,  mat_vec_mul));  // TOS=vec, NOS=matrix (M v *)
  rpn.addDefinition("*", MV_WDEF(d2_matrix_double,  mat_scale_double));  // TOS=double, pop double first
  rpn.addDefinition("*", MV_WDEF(d2_matrix_integer, mat_scale_double));  // TOS=int,    pop as double first
  rpn.addDefinition("*", MV_WDEF(d2_double_matrix,  scale_double_mat));  // TOS=matrix, pop mat first
  rpn.addDefinition("*", MV_WDEF(d2_integer_matrix, scale_double_mat));  // TOS=matrix, pop mat first

  // ------- Matrix operations -------
  rpn.addDefinition("DET",      MV_WDEF(d1_matrix, mat_det));
  rpn.addDefinition("TRANS",    MV_WDEF(d1_matrix, mat_trans));
  rpn.addDefinition("INV",      MV_WDEF(d1_matrix, mat_inv));
  rpn.addDefinition("IDENTITY", {rpn::StackSizeValidator::one, MAT_FUNC(mat_identity), nullptr});

  addWordMetadata("->MATRIX", "Build N×M matrix. Stack: v[0][0]...v[N-1][M-1] rows cols (row-major, TOS=last element).");
  addWordMetadata("MATRIX->", "Decompose matrix: pushes elements row-major, then rows, then cols.");
  addWordMetadata("ROWS",     "Number of rows of a matrix.");
  addWordMetadata("COLS",     "Number of columns of a matrix.");
  addWordMetadata("DET",      "Determinant of a square matrix.");
  addWordMetadata("TRANS",    "Transpose of a matrix.");
  addWordMetadata("INV",      "Inverse of a square non-singular matrix.");
  addWordMetadata("IDENTITY", "Push an N×N identity matrix.");

  setWordCategory(""); // reset
}

/* end of qinc/rpn-lang/src/matrix-dict.cpp */
