/***************************************************
 * file: QInc/Projects/color-calc/src/libs/rpn-lang/src/fraction.cpp
 *
 * @file    fraction.cpp
 * @author  Eric L. Hernes
 * @version V1.0
 * @born_on   Thursday, Friday, April 30, 2010
 * @copyright (C) Copyright Eric L. Hernes 2010-2024
 * @copyright (C) Copyright Q, Inc. 2010-2024
 *
 * @brief   An Eric L. Hernes Signature Series C++ module
 *
 */

#include "fraction.h"

/****************************************
 * fraction methods
 */

q::Fraction
operator+(const double &lhs, const q::Fraction &rhs) {
  // order doesn't matter, just use the Fraction operator
  return rhs + lhs;
}

q::Fraction
operator-(const double &lhs, const q::Fraction &rhs) {
  return rhs + lhs;
}

q::Fraction
operator*(const double &lhs, const q::Fraction &rhs) {
  // order doesn't matter, just use the Fraction operator
  return rhs * lhs;
}

q::Fraction
operator/(const double &lhs, const q::Fraction &rhs) {
  // invert and multiply
  return rhs.reciprocal() * lhs;
}

double q::Fraction::s_precision = 0.00000000001;

//
// http://mathforum.org/library/drmath/view/51886.html
//
q::Fraction::Fraction(double val) {    // find nearest fraction
  int sign = (val < 0.) ? -1 : 1;
  val = fabs(val);
  int intPart = (int)val;
  val -= (double)intPart;
  Fraction low(0, 1);           // "A" = 0/1
  Fraction high(1, 1);          // "B" = 1/1
  for (int i = 0; i < 100; ++i) {
    double testLow = low._denominator * val - low._numerator;
    double testHigh = high._numerator - high._denominator * val;
    if (testHigh < s_precision * high._denominator)
      break; // high is answer
    if (testLow < s_precision * low._denominator) {  // low is answer
      high = low;
      break;
    }
    if (i & 1) {  // odd step: add multiple of low to high
      double test = testHigh / testLow;
      int count = (int)test;    // "N"
      int64_t num = (count + 1) * low._numerator + high._numerator;
      int64_t denom = (count + 1) * low._denominator + high._denominator;
      if ((num > 0x8000) ||
	  (denom > 0x10000))
	break;
      high._numerator = num - low._numerator;  // new "A"
      high._denominator = denom - low._denominator;
      low._numerator = num;             // new "B"
      low._denominator = denom;
    } else {  // even step: add multiple of high to low
      double test = testLow / testHigh;
      int count = (int)test;     // "N"
      int64_t num = low._numerator + (count + 1) * high._numerator;
      int64_t denom = low._denominator + (count + 1) * high._denominator;
      if ((num > 0x10000) ||
	  (denom > 0x10000))
	break;
      low._numerator = num - high._numerator;  // new "A"
      low._denominator = denom - high._denominator;
      high._numerator = num;            // new "B"
      high._denominator = denom;
    }
  }
  _numerator = (intPart * high._denominator + high._numerator) * sign;
  _denominator = high._denominator;

  //  Fraction r(intPart, 1) + high;
  //  _numerator = r._numerator;
  //  _denominator = r._denominator;
}

/* end of QInc/Projects/color-calc/src/libs/rpn-lang/src/fraction.cpp */
