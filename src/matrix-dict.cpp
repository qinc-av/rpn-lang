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
 * @brief   StVector and StMatrix words for rpn-lang.
 *
 * Types (stack::Vector, stack::Matrix) are defined locally here, backed by
 * math::matrix<double> (Techsoft Matrix TCL Lite v1.13, src/matrix.h).
 * Validators use a local matrix_validator:: namespace (same pattern as
 * stack::Complex in math-dict.cpp).
 *
 */

#include "../rpn.h"
#include "matrix.h"
#include <cmath>
#include <format>

// ---------------------------------------------------------------------------
// stack::Vector — N-dimensional real column vector (N×1 matrix internally)
// ---------------------------------------------------------------------------
namespace stack {

class Vector : public rpn::Stack::Object {
public:
  explicit Vector(size_t n) : _m(n > 0 ? n : 1, 1) { _m = 0.0; }
  Vector(const std::vector<double> &vals) : _m(vals.empty() ? 1 : vals.size(), 1) {
    for (size_t i = 0; i < vals.size(); i++) _m(i, 0) = vals[i];
  }
  Vector(const Vector &other) : _m(other._m) {}
  explicit Vector(const math::matrix<double> &m) : _m(m) {}
  virtual ~Vector() {}

  size_t size() const { return _m.RowNo(); }
  double get(size_t i) const { return _m(i, 0); }
  void set(size_t i, double v) { _m(i, 0) = v; }
  const math::matrix<double> &mat() const { return _m; }
  math::matrix<double> &mat() { return _m; }

  virtual bool operator==(const rpn::Stack::Object &orhs) const override {
    const auto &rhs = PEEK_CAST(const Vector, orhs);
    if (_m.RowNo() != rhs._m.RowNo()) return false;
    for (size_t i = 0; i < _m.RowNo(); i++)
      if (_m(i, 0) != rhs._m(i, 0)) return false;
    return true;
  }
  virtual bool operator>(const rpn::Stack::Object &) const override { return false; }
  virtual bool operator<(const rpn::Stack::Object &) const override { return false; }
  virtual std::unique_ptr<rpn::Stack::Object> deep_copy() const override {
    return std::make_unique<Vector>(*this);
  }
  virtual operator std::string() const override {
    std::string rv = "[";
    for (size_t i = 0; i < _m.RowNo(); i++) {
      if (i > 0) rv += " ";
      rv += rpn::to_string(_m(i, 0));
    }
    rv += "]";
    return rv;
  }
  virtual std::string deparse() const override {
    auto dp = [](double v) {
      auto s = std::format("{:.17g}", v);
      if (s.find_first_not_of("-0123456789") == std::string::npos) s += ".";
      return s;
    };
    std::string rv;
    for (size_t i = 0; i < _m.RowNo(); i++) {
      if (i > 0) rv += " ";
      rv += dp(_m(i, 0));
    }
    rv += " " + std::to_string(_m.RowNo()) + " ->VEC";
    return rv;
  }
  virtual std::string to_latex() const override {
    std::string rv = "\\begin{pmatrix}";
    for (size_t i = 0; i < _m.RowNo(); i++) {
      if (i > 0) rv += "\\\\";
      rv += rpn::to_string(_m(i, 0));
    }
    rv += "\\end{pmatrix}";
    return rv;
  }
  virtual std::string type_name() const override { return "vector"; }
  virtual nlohmann::json to_json() const override {
    nlohmann::json data = nlohmann::json::array();
    for (size_t i = 0; i < _m.RowNo(); i++) data.push_back(_m(i, 0));
    return {{"type", type_name()}, {"display", (std::string)(*this)},
            {"deparse", deparse()}, {"data", data}};
  }
private:
  math::matrix<double> _m;
};

// ---------------------------------------------------------------------------
// stack::Matrix — N×M real matrix
// ---------------------------------------------------------------------------
class Matrix : public rpn::Stack::Object {
public:
  explicit Matrix(size_t rows, size_t cols) : _m(rows, cols) { _m = 0.0; }
  explicit Matrix(const math::matrix<double> &m) : _m(m) {}
  Matrix(const Matrix &other) : _m(other._m) {}
  virtual ~Matrix() {}

  size_t rows() const { return _m.RowNo(); }
  size_t cols() const { return _m.ColNo(); }
  double get(size_t r, size_t c) const { return _m(r, c); }
  void set(size_t r, size_t c, double v) { _m(r, c) = v; }
  const math::matrix<double> &mat() const { return _m; }
  math::matrix<double> &mat() { return _m; }

  virtual bool operator==(const rpn::Stack::Object &orhs) const override {
    const auto &rhs = PEEK_CAST(const Matrix, orhs);
    if (_m.RowNo() != rhs._m.RowNo() || _m.ColNo() != rhs._m.ColNo()) return false;
    for (size_t r = 0; r < _m.RowNo(); r++)
      for (size_t c = 0; c < _m.ColNo(); c++)
        if (_m(r, c) != rhs._m(r, c)) return false;
    return true;
  }
  virtual bool operator>(const rpn::Stack::Object &) const override { return false; }
  virtual bool operator<(const rpn::Stack::Object &) const override { return false; }
  virtual std::unique_ptr<rpn::Stack::Object> deep_copy() const override {
    return std::make_unique<Matrix>(*this);
  }
  virtual operator std::string() const override {
    std::string rv = "[";
    for (size_t r = 0; r < _m.RowNo(); r++) {
      rv += "[";
      for (size_t c = 0; c < _m.ColNo(); c++) {
        if (c > 0) rv += " ";
        rv += rpn::to_string(_m(r, c));
      }
      rv += "]";
    }
    rv += "]";
    return rv;
  }
  virtual std::string deparse() const override {
    auto dp = [](double v) {
      auto s = std::format("{:.17g}", v);
      if (s.find_first_not_of("-0123456789") == std::string::npos) s += ".";
      return s;
    };
    std::string rv;
    for (size_t r = 0; r < _m.RowNo(); r++)
      for (size_t c = 0; c < _m.ColNo(); c++) {
        if (r > 0 || c > 0) rv += " ";
        rv += dp(_m(r, c));
      }
    rv += " " + std::to_string(_m.RowNo());
    rv += " " + std::to_string(_m.ColNo());
    rv += " ->MATRIX";
    return rv;
  }
  virtual std::string to_latex() const override {
    std::string rv = "\\begin{pmatrix}";
    for (size_t r = 0; r < _m.RowNo(); r++) {
      if (r > 0) rv += "\\\\";
      for (size_t c = 0; c < _m.ColNo(); c++) {
        if (c > 0) rv += "&";
        rv += rpn::to_string(_m(r, c));
      }
    }
    rv += "\\end{pmatrix}";
    return rv;
  }
  virtual std::string type_name() const override { return "matrix"; }
  virtual nlohmann::json to_json() const override {
    nlohmann::json data = nlohmann::json::array();
    for (size_t r = 0; r < _m.RowNo(); r++) {
      nlohmann::json row = nlohmann::json::array();
      for (size_t c = 0; c < _m.ColNo(); c++) row.push_back(_m(r, c));
      data.push_back(row);
    }
    return {{"type", type_name()}, {"display", (std::string)(*this)},
            {"deparse", deparse()}, {"data", data}};
  }
private:
  math::matrix<double> _m;
};

} // namespace stack

// Local aliases — use ::stack:: to disambiguate from std::stack (matrix.h pulls in using namespace std)
using StVector = ::stack::Vector;
using StMatrix = ::stack::Matrix;

// ---------------------------------------------------------------------------
// Local validators (same pattern as math_validator::d1_complex in math-dict.cpp)
// ---------------------------------------------------------------------------
namespace matrix_validator {
  const rpn::StrictTypeValidator d1_vector({typeid(StVector).hash_code()}, "d1_vector");
  const rpn::StrictTypeValidator d1_matrix({typeid(StMatrix).hash_code()}, "d1_matrix");
  const rpn::StrictTypeValidator d2_vector_vector({typeid(StVector).hash_code(), typeid(StVector).hash_code()}, "d2_vector_vector");
  const rpn::StrictTypeValidator d2_matrix_matrix({typeid(StMatrix).hash_code(), typeid(StMatrix).hash_code()}, "d2_matrix_matrix");
  const rpn::StrictTypeValidator d2_matrix_vector({typeid(StMatrix).hash_code(), typeid(StVector).hash_code()}, "d2_matrix_vector");
  const rpn::StrictTypeValidator d2_vector_matrix({typeid(StVector).hash_code(), typeid(StMatrix).hash_code()}, "d2_vector_matrix");
  const rpn::StrictTypeValidator d2_vector_double( {typeid(StVector).hash_code(), typeid(StDouble).hash_code()},  "d2_vector_double");
  const rpn::StrictTypeValidator d2_vector_integer({typeid(StVector).hash_code(), typeid(StInteger).hash_code()}, "d2_vector_integer");
  const rpn::StrictTypeValidator d2_double_vector( {typeid(StDouble).hash_code(),  typeid(StVector).hash_code()}, "d2_double_vector");
  const rpn::StrictTypeValidator d2_integer_vector({typeid(StInteger).hash_code(), typeid(StVector).hash_code()}, "d2_integer_vector");
  const rpn::StrictTypeValidator d2_matrix_double( {typeid(StMatrix).hash_code(), typeid(StDouble).hash_code()},  "d2_matrix_double");
  const rpn::StrictTypeValidator d2_matrix_integer({typeid(StMatrix).hash_code(), typeid(StInteger).hash_code()}, "d2_matrix_integer");
  const rpn::StrictTypeValidator d2_double_matrix( {typeid(StDouble).hash_code(),  typeid(StMatrix).hash_code()}, "d2_double_matrix");
  const rpn::StrictTypeValidator d2_integer_matrix({typeid(StInteger).hash_code(), typeid(StMatrix).hash_code()}, "d2_integer_matrix");
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
    rpn.stack.push(StVector(std::vector<double>{}));
    return rpn::WordDefinition::Result::ok;
  }
  std::vector<double> vals(n);
  for (size_t i = n; i > 0; i--)
    vals[i - 1] = rpn.stack.pop_as_double();
  rpn.stack.push(StVector(vals));
  return rpn::WordDefinition::Result::ok;
}

// VEC->  ( vec -- v0 v1 ... vN-1 n )
NATIVE_WORD_DECL(matrix, vector_to) {
  auto sv = rpn.stack.pop();
  const auto &v = PEEK_CAST(const StVector, *sv);
  for (size_t i = 0; i < v.size(); i++)
    rpn.stack.push_double(v.get(i));
  rpn.stack.push_integer((int64_t)v.size());
  return rpn::WordDefinition::Result::ok;
}

// SIZE  ( vec -- n )
NATIVE_WORD_DECL(matrix, vec_size) {
  auto sv = rpn.stack.pop();
  const auto &v = PEEK_CAST(const StVector, *sv);
  rpn.stack.push_integer((int64_t)v.size());
  return rpn::WordDefinition::Result::ok;
}

// VDOT  ( v1 v2 -- dot )  NOS · TOS
NATIVE_WORD_DECL(matrix, vec_dot) {
  auto sv1 = rpn.stack.pop();  // TOS
  auto sv2 = rpn.stack.pop();  // NOS
  const auto &v1 = PEEK_CAST(const StVector, *sv1);
  const auto &v2 = PEEK_CAST(const StVector, *sv2);
  if (v1.size() != v2.size()) return rpn::WordDefinition::Result::param_error;
  double dot = 0.0;
  for (size_t i = 0; i < v1.size(); i++) dot += v2.get(i) * v1.get(i);
  rpn.stack.push_double(dot);
  return rpn::WordDefinition::Result::ok;
}

// VNORM  ( vec -- norm )  L2 norm
NATIVE_WORD_DECL(matrix, vec_norm) {
  auto sv = rpn.stack.pop();
  const auto &v = PEEK_CAST(const StVector, *sv);
  double norm = 0.0;
  for (size_t i = 0; i < v.size(); i++) norm += v.get(i) * v.get(i);
  rpn.stack.push_double(std::sqrt(norm));
  return rpn::WordDefinition::Result::ok;
}

// +  ( v1 v2 -- v )  element-wise NOS + TOS
NATIVE_WORD_DECL(matrix, vec_add) {
  auto sv1 = rpn.stack.pop();  // TOS
  auto sv2 = rpn.stack.pop();  // NOS
  const auto &v1 = PEEK_CAST(const StVector, *sv1);
  const auto &v2 = PEEK_CAST(const StVector, *sv2);
  if (v1.size() != v2.size()) return rpn::WordDefinition::Result::param_error;
  rpn.stack.push(StVector(v2.mat() + v1.mat()));
  return rpn::WordDefinition::Result::ok;
}

// -  ( v1 v2 -- v )  element-wise NOS - TOS
NATIVE_WORD_DECL(matrix, vec_sub) {
  auto sv1 = rpn.stack.pop();  // TOS
  auto sv2 = rpn.stack.pop();  // NOS
  const auto &v1 = PEEK_CAST(const StVector, *sv1);
  const auto &v2 = PEEK_CAST(const StVector, *sv2);
  if (v1.size() != v2.size()) return rpn::WordDefinition::Result::param_error;
  rpn.stack.push(StVector(v2.mat() - v1.mat()));
  return rpn::WordDefinition::Result::ok;
}

// *  ( vec scalar -- vec )  scale, TOS = scalar
NATIVE_WORD_DECL(matrix, vec_scale_double) {
  double s = rpn.stack.pop_as_double();
  auto sv = rpn.stack.pop();
  auto &v = PEEK_CAST(StVector, *sv);
  v.mat() *= s;
  rpn.stack.push(v);
  return rpn::WordDefinition::Result::ok;
}

// *  ( scalar vec -- vec )  scale, NOS = scalar
NATIVE_WORD_DECL(matrix, scale_double_vec) {
  auto sv = rpn.stack.pop();   // TOS: vec
  double s = rpn.stack.pop_as_double();  // NOS: scalar
  auto &v = PEEK_CAST(StVector, *sv);
  v.mat() *= s;
  rpn.stack.push(v);
  return rpn::WordDefinition::Result::ok;
}

// VEC->COLVEC  ( vec -- matrix )  N×1
NATIVE_WORD_DECL(matrix, vec_to_colvec) {
  auto sv = rpn.stack.pop();
  const auto &v = PEEK_CAST(const StVector, *sv);
  rpn.stack.push(StMatrix(v.mat()));
  return rpn::WordDefinition::Result::ok;
}

// VEC->ROWVEC  ( vec -- matrix )  1×N
NATIVE_WORD_DECL(matrix, vec_to_rowvec) {
  auto sv = rpn.stack.pop();
  auto &v = PEEK_CAST(StVector, *sv);
  rpn.stack.push(StMatrix(~v.mat()));
  return rpn::WordDefinition::Result::ok;
}

// COLVEC->VEC  ( matrix -- vec )  requires N×1
NATIVE_WORD_DECL(matrix, colvec_to_vec) {
  auto sm = rpn.stack.pop();
  const auto &m = PEEK_CAST(const StMatrix, *sm);
  if (m.cols() != 1) return rpn::WordDefinition::Result::param_error;
  rpn.stack.push(StVector(m.mat()));
  return rpn::WordDefinition::Result::ok;
}

// VEC3->VEC  ( vec3 -- vec )  3-element vector
NATIVE_WORD_DECL(matrix, vec3_to_vec) {
  auto sv3 = rpn.stack.pop();
  const auto &v3 = PEEK_CAST(const StVec3, *sv3);
  std::vector<double> vals = {v3._x, v3._y, v3._z};
  rpn.stack.push(StVector(vals));
  return rpn::WordDefinition::Result::ok;
}

// VEC->VEC3  ( vec -- vec3 )  requires size == 3
NATIVE_WORD_DECL(matrix, vec_to_vec3) {
  auto sv = rpn.stack.pop();
  const auto &v = PEEK_CAST(const StVector, *sv);
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
  StMatrix m(rows, cols);
  for (size_t r = rows; r > 0; r--)
    for (size_t c = cols; c > 0; c--)
      m.set(r - 1, c - 1, rpn.stack.pop_as_double());
  rpn.stack.push(m);
  return rpn::WordDefinition::Result::ok;
}

// MATRIX->  ( matrix -- v00 v01 ... vRC rows cols )
NATIVE_WORD_DECL(matrix, matrix_to) {
  auto sm = rpn.stack.pop();
  const auto &m = PEEK_CAST(const StMatrix, *sm);
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
  const auto &m = PEEK_CAST(const StMatrix, *sm);
  rpn.stack.push_integer((int64_t)m.rows());
  return rpn::WordDefinition::Result::ok;
}

// COLS  ( matrix -- n )
NATIVE_WORD_DECL(matrix, mat_cols) {
  auto sm = rpn.stack.pop();
  const auto &m = PEEK_CAST(const StMatrix, *sm);
  rpn.stack.push_integer((int64_t)m.cols());
  return rpn::WordDefinition::Result::ok;
}

// +  ( m1 m2 -- m )  NOS + TOS; dimensions must match
NATIVE_WORD_DECL(matrix, mat_add) {
  auto sm1 = rpn.stack.pop();  // TOS
  auto sm2 = rpn.stack.pop();  // NOS
  const auto &m1 = PEEK_CAST(const StMatrix, *sm1);
  const auto &m2 = PEEK_CAST(const StMatrix, *sm2);
  if (m1.rows() != m2.rows() || m1.cols() != m2.cols())
    return rpn::WordDefinition::Result::param_error;
  rpn.stack.push(StMatrix(m2.mat() + m1.mat()));
  return rpn::WordDefinition::Result::ok;
}

// -  ( m1 m2 -- m )  NOS - TOS
NATIVE_WORD_DECL(matrix, mat_sub) {
  auto sm1 = rpn.stack.pop();  // TOS
  auto sm2 = rpn.stack.pop();  // NOS
  const auto &m1 = PEEK_CAST(const StMatrix, *sm1);
  const auto &m2 = PEEK_CAST(const StMatrix, *sm2);
  if (m1.rows() != m2.rows() || m1.cols() != m2.cols())
    return rpn::WordDefinition::Result::param_error;
  rpn.stack.push(StMatrix(m2.mat() - m1.mat()));
  return rpn::WordDefinition::Result::ok;
}

// *  ( m1 m2 -- m )  matrix multiply NOS × TOS; NOS.cols == TOS.rows
NATIVE_WORD_DECL(matrix, mat_mul) {
  auto sm1 = rpn.stack.pop();  // TOS
  auto sm2 = rpn.stack.pop();  // NOS
  const auto &m1 = PEEK_CAST(const StMatrix, *sm1);
  const auto &m2 = PEEK_CAST(const StMatrix, *sm2);
  if (m2.cols() != m1.rows()) return rpn::WordDefinition::Result::param_error;
  rpn.stack.push(StMatrix(m2.mat() * m1.mat()));
  return rpn::WordDefinition::Result::ok;
}

// *  ( matrix vec -- vec )  matrix-vector multiply NOS × TOS
NATIVE_WORD_DECL(matrix, mat_vec_mul) {
  auto sv = rpn.stack.pop();   // TOS: vector
  auto sm = rpn.stack.pop();   // NOS: matrix
  const auto &v = PEEK_CAST(const StVector, *sv);
  const auto &m = PEEK_CAST(const StMatrix, *sm);
  if (m.cols() != v.size()) return rpn::WordDefinition::Result::param_error;
  rpn.stack.push(StVector(m.mat() * v.mat()));
  return rpn::WordDefinition::Result::ok;
}

// *  ( matrix scalar -- matrix )  TOS = scalar
NATIVE_WORD_DECL(matrix, mat_scale_double) {
  double s = rpn.stack.pop_as_double();
  auto sm = rpn.stack.pop();
  auto &m = PEEK_CAST(StMatrix, *sm);
  m.mat() *= s;
  rpn.stack.push(m);
  return rpn::WordDefinition::Result::ok;
}

// *  ( scalar matrix -- matrix )  NOS = scalar
NATIVE_WORD_DECL(matrix, scale_double_mat) {
  auto sm = rpn.stack.pop();   // TOS: matrix
  double s = rpn.stack.pop_as_double();  // NOS: scalar
  auto &m = PEEK_CAST(StMatrix, *sm);
  m.mat() *= s;
  rpn.stack.push(m);
  return rpn::WordDefinition::Result::ok;
}

// DET  ( matrix -- d )  determinant; square matrix required
NATIVE_WORD_DECL(matrix, mat_det) {
  auto sm = rpn.stack.pop();
  const auto &m = PEEK_CAST(const StMatrix, *sm);
  if (m.rows() != m.cols()) return rpn::WordDefinition::Result::param_error;
  rpn.stack.push_double(m.mat().Det());
  return rpn::WordDefinition::Result::ok;
}

// TRANS  ( matrix -- matrix )  transpose
NATIVE_WORD_DECL(matrix, mat_trans) {
  auto sm = rpn.stack.pop();
  auto &m = PEEK_CAST(StMatrix, *sm);
  rpn.stack.push(StMatrix(~m.mat()));
  return rpn::WordDefinition::Result::ok;
}

// INV  ( matrix -- matrix )  inverse; square non-singular required
NATIVE_WORD_DECL(matrix, mat_inv) {
  auto sm = rpn.stack.pop();
  auto &m = PEEK_CAST(StMatrix, *sm);
  if (m.rows() != m.cols()) return rpn::WordDefinition::Result::param_error;
  rpn.stack.push(StMatrix(m.mat().Inv()));
  return rpn::WordDefinition::Result::ok;
}

// IDENTITY  ( n -- matrix )  n×n identity matrix
NATIVE_WORD_DECL(matrix, mat_identity) {
  size_t n = (size_t)rpn.stack.pop_as_integer();
  if (n == 0) return rpn::WordDefinition::Result::param_error;
  StMatrix m(n, n);
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
  rpn.addDefinition("*", MV_WDEF(d2_double_vector,  vec_scale_double));  // TOS=double, pop double first
  rpn.addDefinition("*", MV_WDEF(d2_integer_vector, vec_scale_double));  // TOS=int,    pop as double first
  rpn.addDefinition("*", MV_WDEF(d2_vector_double,  scale_double_vec));  // TOS=vector, pop vec first
  rpn.addDefinition("*", MV_WDEF(d2_vector_integer, scale_double_vec));  // TOS=vector, pop vec first

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
  rpn.addDefinition("*", MV_WDEF(d2_vector_matrix,  mat_vec_mul));  // TOS=vec, NOS=matrix (M v *)
  rpn.addDefinition("*", MV_WDEF(d2_double_matrix,  mat_scale_double));  // TOS=double, pop double first
  rpn.addDefinition("*", MV_WDEF(d2_integer_matrix, mat_scale_double));  // TOS=int,    pop as double first
  rpn.addDefinition("*", MV_WDEF(d2_matrix_double,  scale_double_mat));  // TOS=matrix, pop mat first
  rpn.addDefinition("*", MV_WDEF(d2_matrix_integer, scale_double_mat));  // TOS=matrix, pop mat first

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
