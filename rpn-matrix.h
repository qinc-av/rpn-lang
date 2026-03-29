/***************************************************
 * file: qinc/rpn-lang/rpn-matrix.h
 *
 * @file    rpn-matrix.h
 * @author  Eric L. Hernes
 * @born_on   Sunday, March 29, 2026
 * @copyright (C) Copyright Eric L. Hernes 2026
 * @copyright (C) Copyright Q, Inc. 2026
 *
 * @brief   stack::Vector, stack::Matrix, stack::Vec3, stack::Mx3 type definitions.
 *
 * Embedders that need matrix/vector types should include this header in
 * addition to rpn.h.  rpn.h itself never gains an Eigen or vecmx3 dependency.
 *
 */

#pragma once

#include "rpn.h"
#include "vecmx3.h"
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

// ---------------------------------------------------------------------------
// stack::Vec3 — 3D vector (q::Vec3 backend for math; Stack::Object for stack)
// ---------------------------------------------------------------------------
class Vec3 : public rpn::Stack::Object, public q::Vec3 {
public:
  Vec3(double x=std::nan(""), double y=std::nan(""), double z=std::nan(""))
    : q::Vec3(x, y, z) {}
  Vec3(const Vec3 &other) : q::Vec3(other) {}
  virtual ~Vec3() {}

  virtual bool operator==(const rpn::Stack::Object &orhs) const override {
    const auto &rhs = PEEK_CAST(Vec3, orhs);
    double x = (*this)[0], y = (*this)[1], z = (*this)[2];
    double rx = rhs[0], ry = rhs[1], rz = rhs[2];
    return (((x == rx) || (std::isnan(x) && std::isnan(rx))) &&
            ((y == ry) || (std::isnan(y) && std::isnan(ry))) &&
            ((z == rz) || (std::isnan(z) && std::isnan(rz))));
  }
  virtual std::unique_ptr<rpn::Stack::Object> deep_copy() const override {
    return std::make_unique<Vec3>(*this);
  }
  virtual operator std::string() const override {
    double x = (*this)[0], y = (*this)[1], z = (*this)[2];
    std::string rv = "<";
    if (!std::isnan(x)) { rv += " x:"; rv += rpn::to_string(x); }
    if (!std::isnan(y)) { rv += " y:"; rv += rpn::to_string(y); }
    if (!std::isnan(z)) { rv += " z:"; rv += rpn::to_string(z); }
    rv += " >";
    return rv;
  }
  virtual std::string deparse() const override {
    auto dp = [](double v) {
      auto s = std::format("{:.17g}", v);
      if (s.find_first_not_of("-0123456789") == std::string::npos) s += ".";
      return s;
    };
    return dp((*this)[0]) + " " + dp((*this)[1]) + " " + dp((*this)[2]) + " ->VEC3";
  }
  virtual std::string to_latex() const override {
    double x = (*this)[0], y = (*this)[1], z = (*this)[2];
    std::string rv = "[";
    if (!std::isnan(x)) { rv += rpn::to_string(x); rv += "_x"; }
    if (!std::isnan(y)) { rv += rpn::to_string(y); rv += "_y"; }
    if (!std::isnan(z)) { rv += rpn::to_string(z); rv += "_z"; }
    rv += "]";
    return rv;
  }
  virtual std::string type_name() const override { return "vec3"; }
  virtual nlohmann::json to_json() const override {
    double x = (*this)[0], y = (*this)[1], z = (*this)[2];
    return {{"type", type_name()}, {"display", (std::string)(*this)},
            {"deparse", deparse()}, {"data", {{"x", x}, {"y", y}, {"z", z}}}};
  }
};

// ---------------------------------------------------------------------------
// stack::Mx3 — 3×3 matrix (q::Mx3 backend; distinct from stack::Matrix)
// ---------------------------------------------------------------------------
class Mx3 : public rpn::Stack::Object, public q::Mx3 {
public:
  Mx3() : q::Mx3() {}
  Mx3(const q::Vec3 &r1, const q::Vec3 &r2, const q::Vec3 &r3) : q::Mx3(r1, r2, r3) {}
  Mx3(double e11, double e12, double e13,
      double e21, double e22, double e23,
      double e31, double e32, double e33)
    : q::Mx3(e11, e12, e13, e21, e22, e23, e31, e32, e33) {}
  Mx3(const Mx3 &other) : q::Mx3(other) {}
  virtual ~Mx3() {}

  virtual bool operator==(const rpn::Stack::Object &orhs) const override {
    const auto &rhs = PEEK_CAST(Mx3, orhs);
    for (int r = 0; r < 3; r++)
      for (int c = 0; c < 3; c++)
        if ((*this)(r,c) != rhs(r,c)) return false;
    return true;
  }
  virtual std::unique_ptr<rpn::Stack::Object> deep_copy() const override {
    return std::make_unique<Mx3>(*this);
  }
  virtual operator std::string() const override {
    std::string rv = "[[";
    for (int r = 0; r < 3; r++) {
      if (r > 0) rv += "][";
      for (int c = 0; c < 3; c++) {
        if (c > 0) rv += " ";
        rv += rpn::to_string((*this)(r, c));
      }
    }
    rv += "]]";
    return rv;
  }
  virtual std::string deparse() const override {
    auto dp = [](double v) {
      auto s = std::format("{:.17g}", v);
      if (s.find_first_not_of("-0123456789") == std::string::npos) s += ".";
      return s;
    };
    std::string rv;
    for (int r = 0; r < 3; r++)
      for (int c = 0; c < 3; c++) {
        if (r > 0 || c > 0) rv += " ";
        rv += dp((*this)(r, c));
      }
    rv += " ->MX3";
    return rv;
  }
  virtual std::string to_latex() const override {
    std::string rv = "\\begin{pmatrix}";
    for (int r = 0; r < 3; r++) {
      if (r > 0) rv += "\\\\";
      for (int c = 0; c < 3; c++) {
        if (c > 0) rv += "&";
        rv += rpn::to_string((*this)(r, c));
      }
    }
    rv += "\\end{pmatrix}";
    return rv;
  }
  virtual std::string type_name() const override { return "mx3"; }
  virtual nlohmann::json to_json() const override {
    nlohmann::json data = nlohmann::json::array();
    for (int r = 0; r < 3; r++) {
      nlohmann::json row = nlohmann::json::array();
      for (int c = 0; c < 3; c++) row.push_back((*this)(r, c));
      data.push_back(row);
    }
    return {{"type", type_name()}, {"display", (std::string)(*this)},
            {"deparse", deparse()}, {"data", data}};
  }
};

} // namespace stack

/* end of qinc/rpn-lang/rpn-matrix.h */
