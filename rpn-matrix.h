/***************************************************
 * file: qinc/rpn-lang/rpn-matrix.h
 *
 * @file    rpn-matrix.h
 * @author  Eric L. Hernes
 * @born_on   Sunday, March 29, 2026
 * @copyright (C) Copyright Eric L. Hernes 2026
 * @copyright (C) Copyright Q, Inc. 2026
 *
 * @brief   stack::Vector and stack::Matrix type definitions (Eigen backend).
 *
 * Embedders that need matrix/vector types should include this header in
 * addition to rpn.h.  rpn.h itself never gains an Eigen dependency.
 *
 */

#pragma once

#include "rpn.h"
#include <Eigen/Dense>
#include <cmath>
#include <format>

namespace stack {

// ---------------------------------------------------------------------------
// stack::Vector — N-dimensional real column vector
// ---------------------------------------------------------------------------
class Vector : public rpn::Stack::Object {
public:
  explicit Vector(size_t n) : _v(Eigen::VectorXd::Zero(n > 0 ? (int)n : 1)) {}
  Vector(const std::vector<double> &vals) : _v((int)(vals.empty() ? 1 : vals.size())) {
    for (int i = 0; i < (int)vals.size(); i++) _v(i) = vals[i];
  }
  explicit Vector(const Eigen::VectorXd &v) : _v(v) {}
  Vector(const Vector &other) : _v(other._v) {}
  virtual ~Vector() {}

  size_t size() const { return (size_t)_v.size(); }
  double get(size_t i) const { return _v((int)i); }
  void set(size_t i, double v) { _v((int)i) = v; }
  const Eigen::VectorXd &vec() const { return _v; }
  Eigen::VectorXd &vec() { return _v; }

  virtual bool operator==(const rpn::Stack::Object &orhs) const override {
    const auto &rhs = PEEK_CAST(Vector, orhs);
    if (_v.size() != rhs._v.size()) return false;
    return _v.isApprox(rhs._v, 0.0);  // exact comparison
  }
  virtual bool operator>(const rpn::Stack::Object &) const override { return false; }
  virtual bool operator<(const rpn::Stack::Object &) const override { return false; }
  virtual std::unique_ptr<rpn::Stack::Object> deep_copy() const override {
    return std::make_unique<Vector>(*this);
  }
  virtual operator std::string() const override {
    std::string rv = "[";
    for (int i = 0; i < _v.size(); i++) {
      if (i > 0) rv += " ";
      rv += rpn::to_string(_v(i));
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
    for (int i = 0; i < _v.size(); i++) {
      if (i > 0) rv += " ";
      rv += dp(_v(i));
    }
    rv += " " + std::to_string(_v.size()) + " ->VEC";
    return rv;
  }
  virtual std::string to_latex() const override {
    std::string rv = "\\begin{pmatrix}";
    for (int i = 0; i < _v.size(); i++) {
      if (i > 0) rv += "\\\\";
      rv += rpn::to_string(_v(i));
    }
    rv += "\\end{pmatrix}";
    return rv;
  }
  virtual std::string type_name() const override { return "vector"; }
  virtual nlohmann::json to_json() const override {
    nlohmann::json data = nlohmann::json::array();
    for (int i = 0; i < _v.size(); i++) data.push_back(_v(i));
    return {{"type", type_name()}, {"display", (std::string)(*this)},
            {"deparse", deparse()}, {"data", data}};
  }
private:
  Eigen::VectorXd _v;
};

// ---------------------------------------------------------------------------
// stack::Matrix — N×M real matrix
// ---------------------------------------------------------------------------
class Matrix : public rpn::Stack::Object {
public:
  explicit Matrix(size_t rows, size_t cols)
      : _m(Eigen::MatrixXd::Zero((int)rows, (int)cols)) {}
  explicit Matrix(const Eigen::MatrixXd &m) : _m(m) {}
  Matrix(const Matrix &other) : _m(other._m) {}
  virtual ~Matrix() {}

  size_t rows() const { return (size_t)_m.rows(); }
  size_t cols() const { return (size_t)_m.cols(); }
  double get(size_t r, size_t c) const { return _m((int)r, (int)c); }
  void set(size_t r, size_t c, double v) { _m((int)r, (int)c) = v; }
  const Eigen::MatrixXd &mat() const { return _m; }
  Eigen::MatrixXd &mat() { return _m; }

  virtual bool operator==(const rpn::Stack::Object &orhs) const override {
    const auto &rhs = PEEK_CAST(Matrix, orhs);
    if (_m.rows() != rhs._m.rows() || _m.cols() != rhs._m.cols()) return false;
    return _m.isApprox(rhs._m, 0.0);  // exact comparison
  }
  virtual bool operator>(const rpn::Stack::Object &) const override { return false; }
  virtual bool operator<(const rpn::Stack::Object &) const override { return false; }
  virtual std::unique_ptr<rpn::Stack::Object> deep_copy() const override {
    return std::make_unique<Matrix>(*this);
  }
  virtual operator std::string() const override {
    std::string rv = "[";
    for (int r = 0; r < _m.rows(); r++) {
      rv += "[";
      for (int c = 0; c < _m.cols(); c++) {
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
    for (int r = 0; r < _m.rows(); r++)
      for (int c = 0; c < _m.cols(); c++) {
        if (r > 0 || c > 0) rv += " ";
        rv += dp(_m(r, c));
      }
    rv += " " + std::to_string(_m.rows());
    rv += " " + std::to_string(_m.cols());
    rv += " ->MATRIX";
    return rv;
  }
  virtual std::string to_latex() const override {
    std::string rv = "\\begin{pmatrix}";
    for (int r = 0; r < _m.rows(); r++) {
      if (r > 0) rv += "\\\\";
      for (int c = 0; c < _m.cols(); c++) {
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
    for (int r = 0; r < _m.rows(); r++) {
      nlohmann::json row = nlohmann::json::array();
      for (int c = 0; c < _m.cols(); c++) row.push_back(_m(r, c));
      data.push_back(row);
    }
    return {{"type", type_name()}, {"display", (std::string)(*this)},
            {"deparse", deparse()}, {"data", data}};
  }
private:
  Eigen::MatrixXd _m;
};

} // namespace stack

/* end of qinc/rpn-lang/rpn-matrix.h */
