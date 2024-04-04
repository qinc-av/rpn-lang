/***************************************************
 * file: QInc/Projects/rpn-lang/src/fraction.h
 *
 * @file    fraction.h
 * @author  Eric L. Hernes
 * @born_on   Friday, April 30, 2010
 * @copyright (C) Copyright Eric L. Hernes 2024
 * @copyright (C) Copyright Q, Inc. 2024
 *
 * @brief   An Eric L. Hernes Signature Series C/C++ header
 *
 * $Id$
 */

#pragma once

#include <cmath>
#include <string>

namespace q {
class Fraction {
 public:
  Fraction(int64_t n, int64_t d) : _numerator(n), _denominator(d) {};
  Fraction(double v);
  Fraction(const Fraction &o) : _numerator(o._numerator), _denominator(o._denominator) {};

  Fraction operator+(const Fraction &rhs) const {
    // not very smart, brute force
    return Fraction(_numerator*rhs._denominator + rhs._numerator*_denominator,
		    _denominator * rhs._denominator);
  }
  Fraction operator+(const double &rhs) const {
    return Fraction(_numerator + rhs*_denominator,
		    _denominator);
  }

  Fraction operator-(const Fraction &rhs) const {
    return Fraction(_numerator*rhs._denominator - rhs._numerator*_denominator,
		    _denominator * rhs._denominator);
  }
  Fraction operator-(const double &rhs) const {
    return Fraction(_numerator - rhs*_denominator,
		    _denominator);
  }

  Fraction operator*(const Fraction &rhs) const {
    return Fraction(_numerator*rhs._numerator,
		    _denominator * rhs._denominator);
  }
  Fraction operator*(const double &rhs) const {
    return Fraction(_numerator*rhs,
		    _denominator);
  }

  Fraction operator/(const Fraction &rhs) const {
    return *this * rhs.reciprocal();
  }
  Fraction operator/(const double &rhs) const {
    return Fraction(_numerator,
		    _denominator*rhs);
  }

  Fraction neg() const {
    return Fraction(- _numerator, _denominator);
  }

  Fraction reciprocal() const {
    return Fraction(_denominator,_numerator);
  }

  operator double() const {
    double rv = std::nan("");
    if (_denominator == 0) {
      rv = (_numerator < 0) ? -INFINITY : INFINITY;
    } else {
      rv = double(_numerator)/double(_denominator);
    }
    return rv;
  }

  virtual bool operator==(const Fraction &rhs) const {
    auto v = double(*this);
    auto vr = double(rhs);
    return std::abs(v-vr) < s_precision;
  }
  
  std::string to_string() const { return std::to_string(_numerator) + "/" + std::to_string(_denominator); };

  int64_t _numerator;
  int64_t _denominator;
  static double s_precision;
};
}

q::Fraction operator+(const double &lhs, const q::Fraction &rhs);
q::Fraction operator-(const double &lhs, const q::Fraction &rhs);
q::Fraction operator*(const double &lhs, const q::Fraction &rhs);
q::Fraction operator/(const double &lhs, const q::Fraction &rhs);

#ifdef _RPN_LANG_RPN_H_
namespace stack {
  class Fraction : public rpn::Stack::Object, public q::Fraction {
  public:
    Fraction() = delete;
    Fraction(int64_t n, int64_t d) : q::Fraction(n,d) {}
    Fraction(const q::Fraction &f) : q::Fraction(f) {}
    virtual bool operator==(const rpn::Stack::Object &orhs) const override {
      auto &rhs = PEEK_CAST(const Fraction, orhs);
      return ((const q::Fraction &)*this) == ((const q::Fraction &)rhs);
    }
    virtual std::unique_ptr<rpn::Stack::Object> deep_copy() const override { return std::make_unique<Fraction>(*this); };
    virtual operator std::string() const override {
      return q::Fraction::to_string();
    }
  private:
};

} // namespace stack

namespace frac_validator {
  extern const rpn::StrictTypeValidator d1_frac;
  extern const rpn::StrictTypeValidator d2_frac_frac;
  extern const rpn::StrictTypeValidator d2_frac_int;
  extern const rpn::StrictTypeValidator d2_frac_double;
  extern const rpn::StrictTypeValidator d2_int_frac;
  extern const rpn::StrictTypeValidator d2_double_frac;
  extern const rpn::StrictTypeValidator d5_int_int_int_int_frac;
}
#endif

/* end of QInc/Projects/rpn-lang/src/fraction.h */
