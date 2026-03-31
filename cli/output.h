/***************************************************
 * file: QInc/Projects/RP42/rpn-lang/cli/output.h
 *
 * @brief   Stack and word output formatting for the rpn CLI.
 */

#pragma once
#include "rpn.h"

namespace rpn::cli {

// Print the stack.  json=false → human-readable, one item per line.
// full_stack=false → TOS only (or "(empty)" when stack is empty).
void printStack(rpn::Interp &interp, bool json, bool full_stack);

// Print all word names grouped by category as a JSON array of objects.
void printWords(rpn::Interp &interp);

// Print wordHelp for one word as JSON.
void printWordHelp(rpn::Interp &interp, const std::string &word);

} // namespace rpn::cli

/* end of QInc/Projects/RP42/rpn-lang/cli/output.h */
