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
rpn::Interp::addLogicDictionary() {
  if (_alreadyRegistered("logic")) return;
  setWordCategory("logic");
  //    IF
  //    IFTE
  //    EQ?
  addDefinition("IFTE", { rpn::StrictTypeValidator::d3_any_any_boolean, NATIVE_WORD_FN(logic, ifte), nullptr, "",
    rpn::StackEffect{{{"false-val", "any"}, {"true-val", "any"}, {"cond", "boolean"}}, {{"result", "any"}}} });
  addDefinition("==", { rpn::StackSizeValidator::two, NATIVE_WORD_FN(logic, equal), nullptr, "",
    rpn::StackEffect{{{"a", "any"}, {"b", "any"}}, {{"result", "boolean"}}} });
  addDefinition(">", { rpn::StackSizeValidator::two, NATIVE_WORD_FN(logic, greater), nullptr, "",
    rpn::StackEffect{{{"a", "any"}, {"b", "any"}}, {{"result", "boolean"}}} });
  addDefinition(">=", { rpn::StackSizeValidator::two, NATIVE_WORD_FN(logic, greater_eq), nullptr, "",
    rpn::StackEffect{{{"a", "any"}, {"b", "any"}}, {{"result", "boolean"}}} });
  addDefinition("<", { rpn::StackSizeValidator::two, NATIVE_WORD_FN(logic, less), nullptr, "",
    rpn::StackEffect{{{"a", "any"}, {"b", "any"}}, {{"result", "boolean"}}} });
  addDefinition("<=", { rpn::StackSizeValidator::two, NATIVE_WORD_FN(logic, less_eq), nullptr, "",
    rpn::StackEffect{{{"a", "any"}, {"b", "any"}}, {{"result", "boolean"}}} });
  addDefinition("!=", { rpn::StackSizeValidator::two, NATIVE_WORD_FN(logic, not_equal), nullptr, "",
    rpn::StackEffect{{{"a", "any"}, {"b", "any"}}, {{"result", "boolean"}}} });

  addDefinition("NOT", { rpn::StrictTypeValidator::d1_boolean, NATIVE_WORD_FN(logic, l_not), nullptr, "",
    rpn::StackEffect{{{"a", "boolean"}}, {{"result", "boolean"}}} });
  addDefinition("AND", { rpn::StrictTypeValidator::d2_boolean_boolean, NATIVE_WORD_FN(logic, l_and), nullptr, "",
    rpn::StackEffect{{{"a", "boolean"}, {"b", "boolean"}}, {{"result", "boolean"}}} });
  addDefinition("OR",  { rpn::StrictTypeValidator::d2_boolean_boolean, NATIVE_WORD_FN(logic, l_or), nullptr, "",
    rpn::StackEffect{{{"a", "boolean"}, {"b", "boolean"}}, {{"result", "boolean"}}} });

  addDefinition("NEG", { rpn::StrictTypeValidator::d1_integer, NATIVE_WORD_FN(logic, b_neg), nullptr, "",
    rpn::StackEffect{{{"a", "integer"}}, {{"result", "integer"}}} });
  addDefinition("AND", { rpn::StrictTypeValidator::d2_integer_integer, NATIVE_WORD_FN(logic, b_and), nullptr, "",
    rpn::StackEffect{{{"a", "integer"}, {"b", "integer"}}, {{"result", "integer"}}} });
  addDefinition("OR",  { rpn::StrictTypeValidator::d2_integer_integer, NATIVE_WORD_FN(logic, b_or),  nullptr, "",
    rpn::StackEffect{{{"a", "integer"}, {"b", "integer"}}, {{"result", "integer"}}} });
  addDefinition("XOR", { rpn::StrictTypeValidator::d2_integer_integer, NATIVE_WORD_FN(logic, b_xor), nullptr, "",
    rpn::StackEffect{{{"a", "integer"}, {"b", "integer"}}, {{"result", "integer"}}} });
  // LSHIFT/RSHIFT: value must be integer (TOS-1); shift count may be integer or double (TOS).
  addDefinition("LSHIFT", { rpn::StrictTypeValidator::d2_integer_integer, NATIVE_WORD_FN(logic, b_lshift), nullptr, "",
    rpn::StackEffect{{{"value", "integer"}, {"shift", "integer"}}, {{"result", "integer"}}} });
  addDefinition("LSHIFT", { rpn::StrictTypeValidator::d2_integer_double,  NATIVE_WORD_FN(logic, b_lshift), nullptr, "",
    rpn::StackEffect{{{"value", "integer"}, {"shift", "number"}},  {{"result", "integer"}}} });
  addDefinition("RSHIFT", { rpn::StrictTypeValidator::d2_integer_integer, NATIVE_WORD_FN(logic, b_rshift), nullptr, "",
    rpn::StackEffect{{{"value", "integer"}, {"shift", "integer"}}, {{"result", "integer"}}} });
  addDefinition("RSHIFT", { rpn::StrictTypeValidator::d2_integer_double,  NATIVE_WORD_FN(logic, b_rshift), nullptr, "",
    rpn::StackEffect{{{"value", "integer"}, {"shift", "number"}},  {{"result", "integer"}}} });
  addDefinition("RLEFT",  { rpn::StrictTypeValidator::d2_integer_integer, NATIVE_WORD_FN(logic, b_rleft),  nullptr, "",
    rpn::StackEffect{{{"value", "integer"}, {"n", "integer"}}, {{"result", "integer"}}} });
  addDefinition("RLEFT",  { rpn::StrictTypeValidator::d2_integer_double,  NATIVE_WORD_FN(logic, b_rleft),  nullptr, "",
    rpn::StackEffect{{{"value", "integer"}, {"n", "number"}},  {{"result", "integer"}}} });
  addDefinition("RRIGHT", { rpn::StrictTypeValidator::d2_integer_integer, NATIVE_WORD_FN(logic, b_rright), nullptr, "",
    rpn::StackEffect{{{"value", "integer"}, {"n", "integer"}}, {{"result", "integer"}}} });
  addDefinition("RRIGHT", { rpn::StrictTypeValidator::d2_integer_double,  NATIVE_WORD_FN(logic, b_rright), nullptr, "",
    rpn::StackEffect{{{"value", "integer"}, {"n", "number"}},  {{"result", "integer"}}} });

  // Binary wordsize: ->WORDSIZE / STWS sets (TOS integer, 1–64); WORDSIZE-> / RCWS queries.
  addDefinition("->WORDSIZE", { rpn::StrictTypeValidator::d1_integer,
    [](rpn::Interp &rpn, rpn::WordContext *, std::string &) {
      rpn.setBinaryWordsize((int)rpn.stack.pop_integer());
      return rpn::WordDefinition::Result::ok;
    }, nullptr, "",
    rpn::StackEffect{{{"wordsize", "integer"}}, {}} });
  addDefinition("STWS", { rpn::StrictTypeValidator::d1_integer,
    [](rpn::Interp &rpn, rpn::WordContext *, std::string &) {
      rpn.setBinaryWordsize((int)rpn.stack.pop_integer());
      return rpn::WordDefinition::Result::ok;
    }, nullptr, "",
    rpn::StackEffect{{{"wordsize", "integer"}}, {}} });
  addDefinition("WORDSIZE->", { rpn::StackSizeValidator::zero,
    [](rpn::Interp &rpn, rpn::WordContext *, std::string &) {
      rpn.stack.push_integer(rpn.binaryWordsize());
      return rpn::WordDefinition::Result::ok;
    }, nullptr, "",
    rpn::StackEffect{{}, {{"wordsize", "integer"}}} });
  addDefinition("RCWS", { rpn::StackSizeValidator::zero,
    [](rpn::Interp &rpn, rpn::WordContext *, std::string &) {
      rpn.stack.push_integer(rpn.binaryWordsize());
      return rpn::WordDefinition::Result::ok;
    }, nullptr, "",
    rpn::StackEffect{{}, {{"wordsize", "integer"}}} });

  addDefinition("<true>",  { rpn::StackSizeValidator::zero, NATIVE_WORD_FN(logic, push_true),  nullptr, "",
    rpn::StackEffect{{}, {{"value", "boolean"}}} });
  addDefinition("<false>", { rpn::StackSizeValidator::zero, NATIVE_WORD_FN(logic, push_false), nullptr, "",
    rpn::StackEffect{{}, {{"value", "boolean"}}} });

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
  addWordMetadata("NEG",        "Negate or bitwise-NOT — overloaded by type (integer: bitwise NOT masked to current wordsize; fraction: arithmetic negation).");
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
