/***************************************************
 * file: QInc/Projects/rpn-lang/src/timecode.h
 *
 * @file    timecode.h
 * @author  Eric L. Hernes
 * @born_on   Thursday, March 14, 2024
 * @copyright (C) Copyright Eric L. Hernes 2024
 * @copyright (C) Copyright Q, Inc. 2024
 *
 * @brief   An Eric L. Hernes Signature Series C/C++ header
 *
 * $Id$
 */

#pragma once

#include "fraction.h"

namespace q {
  class Timecode {
  public:
    Timecode(int64_t frames, const q::Fraction &fr);
    Timecode(int64_t h, int64_t m, int64_t s, int64_t f, const q::Fraction &fr);
    Timecode(const Timecode &rhs);
    void normalize();

    bool operator==(const Timecode &rhs) const;
    std::string to_string() const;
    Timecode operator+(const Timecode &rhs) const;
    Timecode operator-(const Timecode &rhs) const;
    Timecode operator+(int64_t &rhs) const;
    Timecode operator-(int64_t &rhs) const;

    int64_t to_frames() const;

    //  private: // do we really care about privatizing these

    int _day; // really just for addition carry
    int _hour;
    int _minute;
    int _second;
    int _frame;

    q::Fraction _frameRate;
  };
}

#ifdef _RPN_LANG_RPN_H_
namespace stack {
  class Timecode : public q::Timecode, public rpn::Stack::Object {
public:
    Timecode() = delete;
    Timecode(const q::Timecode &tc) : q::Timecode(tc) {}
    virtual ~Timecode() {};
    virtual bool operator==(const Object &orhs) const override {
      const stack::Timecode &rhs = PEEK_CAST(stack::Timecode,orhs);
      return ((const q::Timecode &)*this) == ((const q::Timecode &)rhs);
    };

    virtual std::unique_ptr<rpn::Stack::Object> deep_copy() const override { return std::make_unique<Timecode>(*this); };
    virtual operator std::string() const override {
      return q::Timecode::to_string();
    }
    virtual std::string deparse() const override {
      // ->TC expects d2_frac_double: frame count as double, frame rate as Fraction.
      // ->FRAC expects d2_integer_integer; use 0d prefix to force integer parsing.
      return std::to_string(to_frames()) + ". " +
             "0d" + std::to_string(_frameRate._numerator) + " " +
             "0d" + std::to_string(_frameRate._denominator) + " ->FRAC ->TC";
    }
    virtual std::string type_name() const override { return "timecode"; }
    virtual nlohmann::json to_json() const override {
      return {{"type",type_name()},{"display",(std::string)(*this)},{"deparse",deparse()},
              {"data",{{"frames",to_frames()},
                       {"frame_rate",{{"numerator",_frameRate._numerator},
                                      {"denominator",_frameRate._denominator}}}}}};
    }
    // default to_latex()
  };
} // namespace stack

namespace timecode_validator {
  extern const rpn::StrictTypeValidator d1_tc;
  extern const rpn::StrictTypeValidator d2_tc_tc;
  extern const rpn::StrictTypeValidator d2_tc_double;
  extern const rpn::StrictTypeValidator d2_double_tc;
}
#endif

/* end of QInc/Projects/rpn-lang/src/timecode.h */
