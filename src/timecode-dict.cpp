/***************************************************
 * file: QInc/Projects/color-calc/src/libs/rpn-lang/src/timecode-dict.cpp
 *
 * @file    timecode-dict.cpp
 * @author  Eric L. Hernes
 * @version V1.0
 * @born_on   Wednesday, March 13, 2024
 * @copyright (C) Copyright Eric L. Hernes 2024
 * @copyright (C) Copyright Q, Inc. 2024
 *
 * @brief   An Eric L. Hernes Signature Series C++ module
 *
 */

#include "../rpn.h"
#include "fraction.h"
#include "timecode.h"

/*
 * Reference: https://github.com/bradcordeiro/libedl/blob/master/src/timecode.cpp
 */
static bool
isDropFrame(unsigned &ifr, const q::Fraction &fr) {
  ifr = std::ceil(double(fr));
  bool isDF = (std::floor(double(fr)) != double(fr)); // it's probably more nuanced than this.
  return isDF;
}

q::Timecode::Timecode(int64_t h, int64_t m, int64_t s, int64_t f, const q::Fraction &fr) : _day(0), _hour(h), _minute(m), _second(s), _frame(f), _frameRate(fr) {
  normalize();
}

int64_t maxFrames(const q::Fraction &fr) {
  q::Timecode maxtc(23,59,59,std::ceil(double(fr))-1, fr);
  return maxtc.to_frames();
}

q::Timecode::Timecode(int64_t frameInput, const q::Fraction &fr) : _day(0), _hour(0), _minute(0), _second(0), _frame(0), _frameRate(fr) {
  unsigned ifr;
  bool isDF = isDropFrame(ifr, _frameRate);

  int nominal_fps = ifr;
  int dropCount = nominal_fps / 15;

  int frameLimit = maxFrames(fr);
  int framesPerMin = 60 * nominal_fps;
  int framesPer10Min = framesPerMin * 10;
  int framesPerHour = framesPer10Min * 6;

  if (frameInput >= frameLimit) {
    frameInput %= frameLimit;
  }

  if (isDF) {
    framesPerMin -= (nominal_fps / 15);
    framesPer10Min = framesPerMin * 10 + dropCount;
    framesPerHour = framesPer10Min * 6;
  }

  _hour = frameInput / framesPerHour;
  frameInput %= framesPerHour;

  if (isDF) {
    // By subtracting two frames while calculating minutes and adding them
    // back in afterward, you avoid landing on an invalid dropframe frame
    // (frame 0 on any minute except every tenth minute)
    int ten_minute = frameInput / framesPer10Min;
    frameInput %= framesPer10Min;
    frameInput -= dropCount;
    int unit_minute = frameInput / framesPerMin;
    frameInput %= framesPerMin;
    _minute = ten_minute * 10 + unit_minute;
    frameInput += dropCount;
  } else {
    _minute = (frameInput / framesPer10Min);
    frameInput %= framesPer10Min;
    _minute += frameInput / framesPerMin;
    frameInput %= framesPerMin;
  }

  _second = frameInput / nominal_fps;
  frameInput %= nominal_fps;

  _frame = frameInput;
}

q::Timecode::Timecode(const Timecode &rhs) : _day(rhs._day), _hour(rhs._hour), _minute(rhs._minute), _second(rhs._second), _frame(rhs._frame), _frameRate(rhs._frameRate) {
  normalize();
}

void
q::Timecode::normalize() {
  int64_t ifr = std::ceil(double(_frameRate));
  int64_t carry = std::floor(_frame/ifr);
  _frame = _frame%ifr;

  _second += carry;
  carry = std::floor(_second/60.);
  _second = _second%60;

  _minute += carry;
  carry = std::floor(_minute/60.);
  _minute = _minute%60;

  _hour += carry;
  carry = std::floor(_hour/24.);
  _hour = _hour%24;

  _day += carry; // do we care about this?
}

bool
q::Timecode::operator==(const q::Timecode &rhs) const {
  return (_hour == rhs._hour &&
	  _minute == rhs._minute &&
	  _second == rhs._second &&
	  _frame == rhs._frame &&
	  _frameRate == rhs._frameRate);
}

q::Timecode
q::Timecode::operator+(const q::Timecode &rhs) const {
  return rhs;
}

q::Timecode
q::Timecode::operator-(const q::Timecode &rhs) const {
  return *this;
}

q::Timecode
q::Timecode::operator+(int64_t &rhs) const {
  return *this;
}

q::Timecode
q::Timecode::operator-(int64_t &rhs) const {
  return *this;
}

int64_t
q::Timecode::to_frames() const {
  unsigned ifr;
  bool isDF = isDropFrame(ifr, _frameRate);
  int nominal_fps = ifr;
  int framesPerMin = 60 * nominal_fps;
  int framesPer10Min = framesPerMin * 10;
  int framesPerHour = framesPer10Min * 6;
  int totalFrames = 0;

  if (isDF) {
    int dropCount = nominal_fps / 15;
    framesPerMin -= dropCount;
    framesPer10Min = (framesPerMin * 10) + dropCount;
    framesPerHour = framesPer10Min * 6;
  }

  totalFrames += _hour * framesPerHour;
  totalFrames += (_minute % 10) * framesPerMin;
  totalFrames += (_minute / 10) * framesPer10Min;
  totalFrames += _second * nominal_fps;
  totalFrames += _frame;

  return totalFrames;
}

std::string
q::Timecode::to_string() const {
  double fr = _frameRate;
  bool isDF = (std::floor(fr) != fr); // it's probably more nuanced than this.
  char tmp[64];
  snprintf(tmp, sizeof(tmp), "TC/%s %02d:%02d:%02d%c%02d @ %2.2f", (isDF?"DF":"NDF"), _hour, _minute, _second, (isDF?';':':'), _frame, double(_frameRate));
  return tmp;
}

NATIVE_WORD_DECL(timecode, to_tc_if) {
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  auto frame = rpn.stack.pop_integer();
  auto ofrac = rpn.stack.pop();
  const auto &fr = POP_CAST(stack::Fraction,ofrac);
  rpn.stack.push(stack::Timecode(q::Timecode(frame,fr)));
  return rv;
}

NATIVE_WORD_DECL(timecode, to_tc_iiiif) {
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  auto frame = rpn.stack.pop_integer();
  auto second = rpn.stack.pop_integer();
  auto minute = rpn.stack.pop_integer();
  auto hour = rpn.stack.pop_integer();
  auto ofrac = rpn.stack.pop();
  const auto &fr = POP_CAST(stack::Fraction,ofrac);
  rpn.stack.push(stack::Timecode(q::Timecode(hour,minute,second,frame,fr)));
  return rv;
}

NATIVE_WORD_DECL(timecode, to_frames) {
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  auto otc = rpn.stack.pop();
  const auto &tc = POP_CAST(stack::Timecode,otc);
  rpn.stack.push_integer(tc.to_frames());
  return rv;
}

NATIVE_WORD_DECL(timecode, framerate) {
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  auto otc = rpn.stack.pop();
  const auto &tc = POP_CAST(stack::Timecode,otc);
  rpn.stack.push(stack::Fraction(tc._frameRate));
  return rv;
}

void
rpn::Interp::addTimecodeWords() {
  rpn::Interp &rpn = *this; // in case we want to move this out someday
  rpn.addDefinition("->TC", NATIVE_WORD_WDEF(timecode, frac_validator::d2_int_frac, to_tc_if, nullptr));
  rpn.addDefinition("->TC", NATIVE_WORD_WDEF(timecode, frac_validator::d5_int_int_int_int_frac, to_tc_iiiif, nullptr));
  rpn.addDefinition("FR", NATIVE_WORD_WDEF(timecode, timecode_validator::d1_tc, framerate, nullptr));
  rpn.addDefinition("->FRAMES", NATIVE_WORD_WDEF(timecode, timecode_validator::d1_tc, to_frames, nullptr));
}

const rpn::StrictTypeValidator timecode_validator::d1_tc({typeid(stack::Timecode).hash_code()}, "d1_tc");
const rpn::StrictTypeValidator timecode_validator::d2_tc_tc({typeid(stack::Timecode).hash_code(),typeid(stack::Timecode).hash_code()}, "d2_tc_tc");
const rpn::StrictTypeValidator timecode_validator::d2_int_tc({typeid(StInteger).hash_code(),typeid(stack::Timecode).hash_code()}, "d2_int_tc");
const rpn::StrictTypeValidator timecode_validator::d2_tc_int({typeid(stack::Timecode).hash_code(),typeid(StInteger).hash_code()}, "d2_tc_int");

/* end of QInc/Projects/color-calc/src/libs/rpn-lang/src/timecode-dict.cpp */
