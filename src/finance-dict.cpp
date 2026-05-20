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
}

// ---------------------------------------------------------------------------
// TVM  ( -- tvm )   blank time-value-of-money object
// ---------------------------------------------------------------------------
NATIVE_WORD_DECL(finance, tvm_new) {
  rpn.stack.push(::stack::Tvm{});
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

  addWordMetadata("TVM", "Push a blank time-value-of-money object.");

  setWordCategory("");
}

/* end of rpn-lang/src/finance-dict.cpp */
