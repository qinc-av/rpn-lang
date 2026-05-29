/***************************************************
 * file: qinc/rpn-lang/src/stack-dict.cpp
 *
 * @file    stack-dict.cpp
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

/* */

#define STACK_OP(op) NATIVE_WORD_FN(stack,op)

#define STACK_OP_FUNC(op)							\
  static rpn::WordDefinition::Result STACK_OP(op)(rpn::Interp &rpn, rpn::WordContext *ctx, std::string &rest) { \
    rpn.stack.op();							\
    return rpn::WordDefinition::Result::ok;				\
  }

#define STACK_OPn_FUNC(op)						\
  static rpn::WordDefinition::Result STACK_OP(op)(rpn::Interp &rpn, rpn::WordContext *ctx, std::string &rest) { \
    int n = (int)rpn.stack.pop_as_double();					\
    rpn.stack.op(n);							\
    return rpn::WordDefinition::Result::ok;				\
  }

STACK_OP_FUNC(drop);
STACK_OP_FUNC(clear);
STACK_OP_FUNC(swap);
STACK_OP_FUNC(rollu);
STACK_OP_FUNC(rolld);
STACK_OP_FUNC(over);
STACK_OP_FUNC(dup);
STACK_OP_FUNC(rotu);
STACK_OP_FUNC(rotd);
STACK_OP_FUNC(print);
STACK_OP_FUNC(reverse);

STACK_OPn_FUNC(dropn);
STACK_OPn_FUNC(dupn);
STACK_OPn_FUNC(nipn);
STACK_OPn_FUNC(pick);
STACK_OPn_FUNC(rolldn);
STACK_OPn_FUNC(rollun);
STACK_OPn_FUNC(tuckn);
STACK_OPn_FUNC(reversen);

// depth is special because we push the value back on the stack
static rpn::WordDefinition::Result STACK_OP(depth)(rpn::Interp &rpn, rpn::WordContext *ctx, std::string &rest) {
  rpn.stack.push_double(rpn.stack.depth());
  return rpn::WordDefinition::Result::ok;
}

#define ADD_STACK_OP(r, symbol, vv, func)				\
  r.addDefinition(symbol, NATIVE_WORD_WDEF(stack, rpn::StackSizeValidator::vv, func, nullptr))

void
rpn::Interp::addStackDictionary() {
  if (_alreadyRegistered("stack")) return;
  rpn::Interp &rpn(*this);
  setWordCategory("stack");

  rpn.addDefinition("DROP",  { rpn::StackSizeValidator::one,  STACK_OP(drop),  nullptr, "",
    rpn::StackEffect{{{"a", "any"}}, {}} });
  rpn.addDefinition("CLEAR", { rpn::StackSizeValidator::zero, STACK_OP(clear), nullptr, "",
    rpn::StackEffect{{}, {}} });
  rpn.addDefinition("DEPTH", { rpn::StackSizeValidator::zero, STACK_OP(depth), nullptr, "",
    rpn::StackEffect{{}, {{"depth", "integer"}}} });
  rpn.addDefinition("SWAP",  { rpn::StackSizeValidator::two,  STACK_OP(swap),  nullptr, "",
    rpn::StackEffect{{{"a", "any"}, {"b", "any"}}, {{"b", "any"}, {"a", "any"}}} });
  rpn.addDefinition("ROLLU", { rpn::StackSizeValidator::zero, STACK_OP(rollu), nullptr, "",
    rpn::StackEffect{{}, {}} });
  rpn.addDefinition("ROLLD", { rpn::StackSizeValidator::zero, STACK_OP(rolld), nullptr, "",
    rpn::StackEffect{{}, {}} });
  rpn.addDefinition("OVER",  { rpn::StackSizeValidator::two,  STACK_OP(over),  nullptr, "",
    rpn::StackEffect{{{"a", "any"}, {"b", "any"}}, {{"a", "any"}, {"b", "any"}, {"a", "any"}}} });
  rpn.addDefinition("DUP",   { rpn::StackSizeValidator::one,  STACK_OP(dup),   nullptr, "",
    rpn::StackEffect{{{"a", "any"}}, {{"a", "any"}, {"a", "any"}}} });
  rpn.addDefinition("ROTU",  { rpn::StackSizeValidator::three, STACK_OP(rotu), nullptr, "",
    rpn::StackEffect{{{"a", "any"}, {"b", "any"}, {"c", "any"}}, {{"b", "any"}, {"c", "any"}, {"a", "any"}}} });
  rpn.addDefinition("ROTD",  { rpn::StackSizeValidator::three, STACK_OP(rotd), nullptr, "",
    rpn::StackEffect{{{"a", "any"}, {"b", "any"}, {"c", "any"}}, {{"c", "any"}, {"a", "any"}, {"b", "any"}}} });
  rpn.addDefinition("DROPn", { rpn::StackSizeValidator::ntos, STACK_OP(dropn), nullptr, "",
    rpn::StackEffect{{{"n", "integer"}}, {}} });
  rpn.addDefinition("DUPn",  { rpn::StackSizeValidator::ntos, STACK_OP(dupn),  nullptr, "",
    rpn::StackEffect{{{"n", "integer"}}, {}} });
  rpn.addDefinition("NIPn",  { rpn::StackSizeValidator::ntos, STACK_OP(nipn),  nullptr, "",
    rpn::StackEffect{{{"n", "integer"}}, {}} });
  rpn.addDefinition("PICK",  { rpn::StackSizeValidator::ntos, STACK_OP(pick),  nullptr, "",
    rpn::StackEffect{{{"n", "integer"}}, {{"item", "any"}}} });
  rpn.addDefinition("ROLLDn",{ rpn::StackSizeValidator::ntos, STACK_OP(rolldn),nullptr, "",
    rpn::StackEffect{{{"n", "integer"}}, {}} });
  rpn.addDefinition("ROLLUn",{ rpn::StackSizeValidator::ntos, STACK_OP(rollun),nullptr, "",
    rpn::StackEffect{{{"n", "integer"}}, {}} });
  rpn.addDefinition("TUCKn", { rpn::StackSizeValidator::ntos, STACK_OP(tuckn), nullptr, "",
    rpn::StackEffect{{{"n", "integer"}}, {}} });
  rpn.addDefinition(".S",    { rpn::StackSizeValidator::zero, STACK_OP(print), nullptr, "",
    rpn::StackEffect{{}, {}} });
  rpn.addDefinition("REVERSE", { rpn::StackSizeValidator::zero, STACK_OP(reverse), nullptr, "",
    rpn::StackEffect{{}, {}} });
  rpn.addDefinition("REVERSEn",{ rpn::StackSizeValidator::ntos, STACK_OP(reversen), nullptr, "",
    rpn::StackEffect{{{"n", "integer"}}, {}} });

  addWordMetadata("DROP",     "Discard TOS.");
  addWordMetadata("CLEAR",    "Clear the entire stack.");
  addWordMetadata("DEPTH",    "Push the current stack depth.");
  addWordMetadata("SWAP",     "Exchange the top two items.");
  addWordMetadata("ROLLU",    "Roll the whole stack up: TOS moves to the bottom.");
  addWordMetadata("ROLLD",    "Roll the whole stack down: bottom item becomes TOS.");
  addWordMetadata("OVER",     "Copy NOS (second item) to TOS.");
  addWordMetadata("DUP",      "Duplicate TOS.");
  addWordMetadata("ROTU",     "Rotate the top three items up.");
  addWordMetadata("ROTD",     "Rotate the top three items down.");
  addWordMetadata("DROPn",    "Drop the top n items. n is on TOS.");
  addWordMetadata("DUPn",     "Duplicate the top n items. n is on TOS.");
  addWordMetadata("NIPn",     "Remove the item at depth n. n is on TOS.");
  addWordMetadata("PICK",     "Copy the item at depth n to TOS. n is on TOS.");
  addWordMetadata("ROLLDn",   "Roll the top n items down. n is on TOS.");
  addWordMetadata("ROLLUn",   "Roll the top n items up. n is on TOS.");
  addWordMetadata("TUCKn",    "Tuck TOS under the n-th item. n is on TOS.");
  addWordMetadata(".S",       "Print the stack contents to the debug sink.");
  addWordMetadata("REVERSE",  "Reverse the entire stack.");
  addWordMetadata("REVERSEn", "Reverse the top n items. n is on TOS.");
  addWordMetadata("DUP2",     "Duplicate the top two items (equivalent to OVER OVER).");
  addWordMetadata("DROP2",    "Drop the top two items (equivalent to DROP DROP).");

}

/* end of qinc/rpn-lang/src/stack-dict.cpp */
