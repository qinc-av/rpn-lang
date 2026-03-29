/***************************************************
 * file: qinc/rpn-lang/src/logic-dict.cpp
 *
 * @file    logic-dict.cpp
 * @author  Eric L. Hernes
 * @version V1.0
 * @born_on   Monday, June 12, 2023
 * @copyright (C) Copyright Eric L. Hernes 2023
 * @copyright (C) Copyright Q, Inc. 2023
 *
 * @brief   An Eric L. Hernes Signature Series C++ module
 *
 */

#include "../rpn.h"

NATIVE_WORD_DECL(logic, ifte) {
  bool s1 = rpn.stack.pop_boolean();
  rpn.stack.nipn(s1 ? 1 : 2);
  return rpn::WordDefinition::Result::ok;
}

NATIVE_WORD_DECL(logic, equal) {
  auto s1 = rpn.stack.pop();
  auto s2 = rpn.stack.pop();
  try {
    bool val = (*s1 == *s2);
    rpn.stack.push_boolean(val);
  } catch (...) {
    rpn.stack.push_boolean(false);
  }
  return rpn::WordDefinition::Result::ok;
}

NATIVE_WORD_DECL(logic, not_equal) {
  auto s1 = rpn.stack.pop();
  auto s2 = rpn.stack.pop();
  try {
    bool val = !(*s1 == *s2);
    rpn.stack.push_boolean(val);
  } catch (...) {
    rpn.stack.push_boolean(false);
  }
  return rpn::WordDefinition::Result::ok;
}

NATIVE_WORD_DECL(logic, greater) {
  auto s1 = rpn.stack.pop();
  auto s2 = rpn.stack.pop();
  rpn.stack.push_boolean(*s2 > *s1);
  return rpn::WordDefinition::Result::ok;
}

NATIVE_WORD_DECL(logic, greater_eq) {
  auto s1 = rpn.stack.pop();
  auto s2 = rpn.stack.pop();
  rpn.stack.push_boolean(!(*s2 < *s1));
  return rpn::WordDefinition::Result::ok;
}

NATIVE_WORD_DECL(logic, less) {
  auto s1 = rpn.stack.pop();
  auto s2 = rpn.stack.pop();
  rpn.stack.push_boolean(*s2 < *s1);
  return rpn::WordDefinition::Result::ok;
}

NATIVE_WORD_DECL(logic, less_eq) {
  auto s1 = rpn.stack.pop();
  auto s2 = rpn.stack.pop();
  rpn.stack.push_boolean(!(*s2 > *s1));
  return rpn::WordDefinition::Result::ok;
}

NATIVE_WORD_DECL(logic, l_not) {
  auto s1 = rpn.stack.pop_boolean();
  rpn.stack.push_boolean(!s1);
  return rpn::WordDefinition::Result::ok;
}

NATIVE_WORD_DECL(logic, l_and) {
  auto s1 = rpn.stack.pop_boolean();
  auto s2 = rpn.stack.pop_boolean();
  rpn.stack.push_boolean(s1 && s2);
  return rpn::WordDefinition::Result::ok;
}

NATIVE_WORD_DECL(logic, l_or) {
  auto s1 = rpn.stack.pop_boolean();
  auto s2 = rpn.stack.pop_boolean();
  rpn.stack.push_boolean(s1 || s2);
  return rpn::WordDefinition::Result::ok;
}

// Returns a bitmask for the current wordsize.  Wordsize 64 → all bits set (no-op mask).
static int64_t wordsize_mask(int ws) {
  if (ws >= 64) return -1LL;
  return (int64_t)((1ULL << ws) - 1);
}

NATIVE_WORD_DECL(logic, b_or) {
  auto s1 = rpn.stack.pop_integer();
  auto s2 = rpn.stack.pop_integer();
  rpn.stack.push_integer((s1 | s2) & wordsize_mask(rpn.binaryWordsize()));
  return rpn::WordDefinition::Result::ok;
}
NATIVE_WORD_DECL(logic, b_and) {
  auto s1 = rpn.stack.pop_integer();
  auto s2 = rpn.stack.pop_integer();
  rpn.stack.push_integer((s1 & s2) & wordsize_mask(rpn.binaryWordsize()));
  return rpn::WordDefinition::Result::ok;
}
NATIVE_WORD_DECL(logic, b_xor) {
  auto s1 = rpn.stack.pop_integer();
  auto s2 = rpn.stack.pop_integer();
  rpn.stack.push_integer((s1 ^ s2) & wordsize_mask(rpn.binaryWordsize()));
  return rpn::WordDefinition::Result::ok;
}
NATIVE_WORD_DECL(logic, b_neg) {
  auto s1 = rpn.stack.pop_integer();
  rpn.stack.push_integer(~s1 & wordsize_mask(rpn.binaryWordsize()));
  return rpn::WordDefinition::Result::ok;
}

// Logical (unsigned) left shift; result masked to wordsize.
NATIVE_WORD_DECL(logic, b_lshift) {
  auto shift = rpn.stack.pop_as_integer();
  auto value = rpn.stack.pop_as_integer();
  int64_t mask = wordsize_mask(rpn.binaryWordsize());
  rpn.stack.push_integer((int64_t)((uint64_t)value << shift) & mask);
  return rpn::WordDefinition::Result::ok;
}
// Logical (unsigned) right shift on the masked value.
NATIVE_WORD_DECL(logic, b_rshift) {
  auto shift = rpn.stack.pop_as_integer();
  auto value = rpn.stack.pop_as_integer();
  uint64_t mask = (uint64_t)wordsize_mask(rpn.binaryWordsize());
  rpn.stack.push_integer((int64_t)(((uint64_t)value & mask) >> shift));
  return rpn::WordDefinition::Result::ok;
}

// Circular rotate left by n bits within the current wordsize.
NATIVE_WORD_DECL(logic, b_rleft) {
  int n = (int)(rpn.stack.pop_as_integer() % rpn.binaryWordsize());
  if (n < 0) n += rpn.binaryWordsize();
  int64_t value = rpn.stack.pop_as_integer();
  int ws = rpn.binaryWordsize();
  uint64_t mask = (uint64_t)wordsize_mask(ws);
  uint64_t uval = (uint64_t)value & mask;
  int64_t result = n == 0 ? (int64_t)uval : (int64_t)(((uval << n) | (uval >> (ws - n))) & mask);
  rpn.stack.push_integer(result);
  return rpn::WordDefinition::Result::ok;
}

// Circular rotate right by n bits within the current wordsize.
NATIVE_WORD_DECL(logic, b_rright) {
  int n = (int)(rpn.stack.pop_as_integer() % rpn.binaryWordsize());
  if (n < 0) n += rpn.binaryWordsize();
  int64_t value = rpn.stack.pop_as_integer();
  int ws = rpn.binaryWordsize();
  uint64_t mask = (uint64_t)wordsize_mask(ws);
  uint64_t uval = (uint64_t)value & mask;
  int64_t result = n == 0 ? (int64_t)uval : (int64_t)(((uval >> n) | (uval << (ws - n))) & mask);
  rpn.stack.push_integer(result);
  return rpn::WordDefinition::Result::ok;
}

NATIVE_WORD_DECL(logic, push_true) {
  rpn.stack.push_boolean(true);
  return rpn::WordDefinition::Result::ok;
}

NATIVE_WORD_DECL(logic, push_false) {
  rpn.stack.push_boolean(false);
  return rpn::WordDefinition::Result::ok;
}

void
rpn::Interp::addLogicWords() {
  setWordCategory("logic");
  //    IF
  //    IFTE
  //    EQ?
  addDefinition("IFTE", NATIVE_WORD_WDEF(logic, rpn::StrictTypeValidator::d3_any_any_boolean, ifte, nullptr));
  addDefinition("==", NATIVE_WORD_WDEF(logic, rpn::StackSizeValidator::two, equal, nullptr));
  addDefinition(">", NATIVE_WORD_WDEF(logic, rpn::StackSizeValidator::two, greater, nullptr));
  addDefinition(">=", NATIVE_WORD_WDEF(logic, rpn::StackSizeValidator::two, greater_eq, nullptr));
  addDefinition("<", NATIVE_WORD_WDEF(logic, rpn::StackSizeValidator::two, less, nullptr));
  addDefinition("<=", NATIVE_WORD_WDEF(logic, rpn::StackSizeValidator::two, less_eq, nullptr));
  addDefinition("!=", NATIVE_WORD_WDEF(logic, rpn::StackSizeValidator::two, not_equal, nullptr));

  addDefinition("NOT", NATIVE_WORD_WDEF(logic, rpn::StrictTypeValidator::d1_boolean, l_not, nullptr));
  addDefinition("AND", NATIVE_WORD_WDEF(logic, rpn::StrictTypeValidator::d2_boolean_boolean, l_and, nullptr));
  addDefinition("OR", NATIVE_WORD_WDEF(logic, rpn::StrictTypeValidator::d2_boolean_boolean, l_or, nullptr));

  addDefinition("NEG",    NATIVE_WORD_WDEF(logic, rpn::StrictTypeValidator::d1_integer,     b_neg,    nullptr));
  addDefinition("AND",    NATIVE_WORD_WDEF(logic, rpn::StrictTypeValidator::d2_integer_integer, b_and, nullptr));
  addDefinition("OR",     NATIVE_WORD_WDEF(logic, rpn::StrictTypeValidator::d2_integer_integer, b_or,  nullptr));
  addDefinition("XOR",    NATIVE_WORD_WDEF(logic, rpn::StrictTypeValidator::d2_integer_integer, b_xor, nullptr));
  // LSHIFT/RSHIFT: value must be integer; shift count may be integer or double (4 vs 0x04).
  // LSHIFT/RSHIFT: value must be integer (TOS-1); shift count may be integer or double (TOS).
  addDefinition("LSHIFT", NATIVE_WORD_WDEF(logic, rpn::StrictTypeValidator::d2_integer_integer, b_lshift, nullptr));
  addDefinition("LSHIFT", NATIVE_WORD_WDEF(logic, rpn::StrictTypeValidator::d2_integer_double,  b_lshift, nullptr));
  addDefinition("RSHIFT", NATIVE_WORD_WDEF(logic, rpn::StrictTypeValidator::d2_integer_integer, b_rshift, nullptr));
  addDefinition("RSHIFT", NATIVE_WORD_WDEF(logic, rpn::StrictTypeValidator::d2_integer_double,  b_rshift, nullptr));
  addDefinition("RLEFT",  NATIVE_WORD_WDEF(logic, rpn::StrictTypeValidator::d2_integer_integer, b_rleft,  nullptr));
  addDefinition("RLEFT",  NATIVE_WORD_WDEF(logic, rpn::StrictTypeValidator::d2_integer_double,  b_rleft,  nullptr));
  addDefinition("RRIGHT", NATIVE_WORD_WDEF(logic, rpn::StrictTypeValidator::d2_integer_integer, b_rright, nullptr));
  addDefinition("RRIGHT", NATIVE_WORD_WDEF(logic, rpn::StrictTypeValidator::d2_integer_double,  b_rright, nullptr));

  // Binary wordsize: ->WORDSIZE / STWS sets (TOS integer, 1–64); WORDSIZE-> / RCWS queries.
  addDefinition("->WORDSIZE", { rpn::StrictTypeValidator::d1_integer,
    [](rpn::Interp &rpn, rpn::WordContext *, std::string &) {
      rpn.setBinaryWordsize((int)rpn.stack.pop_integer());
      return rpn::WordDefinition::Result::ok;
    }, nullptr });
  addDefinition("STWS", { rpn::StrictTypeValidator::d1_integer,
    [](rpn::Interp &rpn, rpn::WordContext *, std::string &) {
      rpn.setBinaryWordsize((int)rpn.stack.pop_integer());
      return rpn::WordDefinition::Result::ok;
    }, nullptr });
  addDefinition("WORDSIZE->", { rpn::StackSizeValidator::zero,
    [](rpn::Interp &rpn, rpn::WordContext *, std::string &) {
      rpn.stack.push_integer(rpn.binaryWordsize());
      return rpn::WordDefinition::Result::ok;
    }, nullptr });
  addDefinition("RCWS", { rpn::StackSizeValidator::zero,
    [](rpn::Interp &rpn, rpn::WordContext *, std::string &) {
      rpn.stack.push_integer(rpn.binaryWordsize());
      return rpn::WordDefinition::Result::ok;
    }, nullptr });

  addDefinition("<true>",  NATIVE_WORD_WDEF(logic, rpn::StackSizeValidator::zero, push_true,  nullptr));
  addDefinition("<false>", NATIVE_WORD_WDEF(logic, rpn::StackSizeValidator::zero, push_false, nullptr));

  addWordMetadata("IFTE",       "Inline conditional: `cond true-val false-val IFTE`. Leaves the chosen value on the stack.");
  addWordMetadata("==",         "Push true if TOS equals NOS.");
  addWordMetadata(">",          "Push true if NOS > TOS.");
  addWordMetadata(">=",         "Push true if NOS >= TOS.");
  addWordMetadata("<",          "Push true if NOS < TOS.");
  addWordMetadata("<=",         "Push true if NOS <= TOS.");
  addWordMetadata("!=",         "Push true if TOS does not equal NOS.");
  addWordMetadata("NOT",        "Boolean NOT.");
  addWordMetadata("AND",        "Boolean AND (two booleans) or bitwise AND (two integers), masked to current wordsize.");
  addWordMetadata("OR",         "Boolean OR (two booleans) or bitwise OR (two integers), masked to current wordsize.");
  addWordMetadata("NEG",        "Bitwise NOT of integer, masked to current wordsize.");
  addWordMetadata("XOR",        "Bitwise XOR of two integers, masked to current wordsize.");
  addWordMetadata("LSHIFT",     "Logical left shift. `value shift LSHIFT`. Result masked to wordsize.");
  addWordMetadata("RSHIFT",     "Logical right shift. `value shift RSHIFT`. Result masked to wordsize.");
  addWordMetadata("RLEFT",      "Circular rotate left. `value n RLEFT`. Rotates within current wordsize.");
  addWordMetadata("RRIGHT",     "Circular rotate right. `value n RRIGHT`. Rotates within current wordsize.");
  addWordMetadata("->WORDSIZE", "Set binary operation wordsize (1–64). Default 64 = no masking.");
  addWordMetadata("STWS",       "HP48 alias for ->WORDSIZE.");
  addWordMetadata("WORDSIZE->", "Push the current binary wordsize.");
  addWordMetadata("RCWS",       "HP48 alias for WORDSIZE->.");
  addWordMetadata("<true>",     "Push boolean true.");
  addWordMetadata("<false>",    "Push boolean false.");
}


/* end of qinc/rpn-lang/src/logic-dict.cpp */
