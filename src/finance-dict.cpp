/***************************************************
 * file: rpn-lang/src/finance-dict.cpp
 *
 * @brief  Financial words: TVM (time value of money) plus the
 *         stack::Tvm constructors, solvers, setters, and AMORT.
 *         See docs/finance-plan.md.
 */
#include "../rpn.h"
#include "finance.h"

// ---------------------------------------------------------------------------
// Local validators
// ---------------------------------------------------------------------------
namespace finance_validator {
  const rpn::StrictTypeValidator d1_tvm(
    { typeid(::stack::Tvm).hash_code() }, "d1_tvm");

  // ->TVM ( n i pv pmt fv begin -- tvm ) : TOS=boolean, the five
  // below are numbers. _types[0] is TOS.
  const rpn::StrictTypeValidator d6_tvm_args(
    { typeid(::stack::Boolean).hash_code(),
      rpn::StrictTypeValidator::v_numbertype,
      rpn::StrictTypeValidator::v_numbertype,
      rpn::StrictTypeValidator::v_numbertype,
      rpn::StrictTypeValidator::v_numbertype,
      rpn::StrictTypeValidator::v_numbertype },
    "d6_number_number_number_number_number_boolean");
}

// ---------------------------------------------------------------------------
// TVM  ( -- tvm )   blank time-value-of-money object
// ---------------------------------------------------------------------------
NATIVE_WORD_DECL(finance, tvm_new) {
  rpn.stack.push(::stack::Tvm{});
  return rpn::WordDefinition::Result::ok;
}

// ---------------------------------------------------------------------------
// ->TVM  ( n i pv pmt fv begin -- tvm )
// ---------------------------------------------------------------------------
NATIVE_WORD_DECL(finance, to_tvm) {
  bool   begin = rpn.stack.pop_boolean();
  double fv    = rpn.stack.pop_as_double();
  double pmt   = rpn.stack.pop_as_double();
  double pv    = rpn.stack.pop_as_double();
  double i     = rpn.stack.pop_as_double();
  double n     = rpn.stack.pop_as_double();
  ::stack::Tvm t;
  t.n = n; t.i = i; t.pv = pv; t.pmt = pmt; t.fv = fv; t.begin = begin;
  rpn.stack.push(t);
  return rpn::WordDefinition::Result::ok;
}

// ---------------------------------------------------------------------------
// TVM->  ( tvm -- n i pv pmt fv begin )
// ---------------------------------------------------------------------------
NATIVE_WORD_DECL(finance, tvm_explode) {
  auto sv = rpn.stack.pop();
  const auto &t = POP_CAST(::stack::Tvm, sv);
  rpn.stack.push_double(t.n);
  rpn.stack.push_double(t.i);
  rpn.stack.push_double(t.pv);
  rpn.stack.push_double(t.pmt);
  rpn.stack.push_double(t.fv);
  rpn.stack.push_boolean(t.begin);
  return rpn::WordDefinition::Result::ok;
}

// ---------------------------------------------------------------------------
// addFinanceWords
// ---------------------------------------------------------------------------
void
rpn::Interp::addFinanceWords() {
  setWordCategory("finance");

  addDefinition("TVM", { rpn::StackSizeValidator::zero,
                         NATIVE_WORD_FN(finance, tvm_new), nullptr });

  addDefinition("->TVM", { finance_validator::d6_tvm_args,
                           NATIVE_WORD_FN(finance, to_tvm), nullptr });
  addDefinition("TVM->", { finance_validator::d1_tvm,
                           NATIVE_WORD_FN(finance, tvm_explode), nullptr });

  addWordMetadata("TVM",   "Push a blank time-value-of-money object.");
  addWordMetadata("->TVM", "Build a tvm. `n i pv pmt fv begin ->TVM`.");
  addWordMetadata("TVM->", "Decompose a tvm into `n i pv pmt fv begin`.");

  setWordCategory("");
}

/* end of rpn-lang/src/finance-dict.cpp */
