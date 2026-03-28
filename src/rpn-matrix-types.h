/***************************************************
 * file: qinc/rpn-lang/src/rpn-matrix-types.h
 *
 * @file    rpn-matrix-types.h
 * @author  Eric L. Hernes
 * @born_on   Sunday, March 29, 2026
 * @copyright (C) Copyright Eric L. Hernes 2026
 * @copyright (C) Copyright Q, Inc. 2026
 *
 * @brief   stack::Vector and stack::Matrix type definitions.
 *
 * Kept in src/ (not rpn.h) because the backing store uses
 * math::matrix<double> from src/matrix.h, which is an internal
 * dependency.  After Phase 4.2 (Eigen migration) these will move
 * to rpn.h.
 *
 * Note: matrix.h pulls in <using namespace std>, so references to
 * stack::Vector and stack::Matrix in files that include this header
 * should use ::stack::Vector / ::stack::Matrix to avoid ambiguity
 * with std::stack.
 */

#pragma once

#include "../rpn.h"
#include "matrix.h"
#include <cmath>
#include <format>

namespace stack {

// ---------------------------------------------------------------------------
// stack::Vector — N-dimensional real column vector (N×1 matrix internally)
// ---------------------------------------------------------------------------
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
