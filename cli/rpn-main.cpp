/***************************************************
 * file: QInc/Projects/RP42/rpn-lang/cli/rpn-main.cpp
 *
 * @brief   rpn CLI entry point.
 *
 * Usage:
 *   rpn [options] [expression]
 *
 * Options:
 *   -f, --file <script>     execute an rpn script file before expression
 *   -d, --dict <name>       load a named extension dictionary (repeatable)
 *   -s, --session <file>    load/save session state from file
 *   --json                  output stack as JSON
 *   --stack                 print full stack (default: TOS only)
 *   --words                 list all available words as JSON
 *   --help-word <word>      print wordHelp JSON for a word
 */

#include <iostream>
#include <string>
#include <vector>

#include "rpn.h"
#include "output.h"
// #include "session.h"       -- Task 2
// #include "dict-registry.h" -- Task 3

static void usage(const char *prog) {
  std::cerr <<
    "usage: " << prog << " [options] [expression]\n"
    "\n"
    "  -f, --file <script>    execute rpn script before expression\n"
    "  -s, --session <file>   load/save session state\n"
    "  -d, --dict <name>      load extension dictionary (repeatable)\n"
    "  --json                 output stack as JSON\n"
    "  --stack                print full stack (default: TOS only)\n"
    "  --words                list all words as JSON\n"
    "  --help-word <word>     print word help as JSON\n";
}

int main(int argc, char *argv[]) {
  bool opt_json       = false;
  bool opt_stack      = false;
  bool opt_words      = false;
  std::string opt_help_word;
  std::string opt_session;
  std::vector<std::string> opt_files;
  std::vector<std::string> opt_dicts;
  std::vector<std::string> expressions;

  for (int i = 1; i < argc; ++i) {
    std::string a = argv[i];
    if (a == "--json") {
      opt_json = true;
    } else if (a == "--stack") {
      opt_stack = true;
    } else if (a == "--words") {
      opt_words = true;
    } else if ((a == "--help-word") && i + 1 < argc) {
      opt_help_word = argv[++i];
    } else if ((a == "-f" || a == "--file") && i + 1 < argc) {
      opt_files.push_back(argv[++i]);
    } else if ((a == "-s" || a == "--session") && i + 1 < argc) {
      opt_session = argv[++i];
    } else if ((a == "-d" || a == "--dict") && i + 1 < argc) {
      opt_dicts.push_back(argv[++i]);
    } else if (a == "-h" || a == "--help") {
      usage(argv[0]);
      return 0;
    } else if (!a.empty() && a[0] == '-') {
      std::cerr << "unknown option: " << a << "\n";
      usage(argv[0]);
      return 1;
    } else {
      expressions.push_back(a);
    }
  }

  rpn::Interp interp(false);  // sync

  // Task 3: load extension dicts
  for (const auto &d : opt_dicts) {
    std::cerr << "rpn: extension dicts not yet implemented (-d " << d << ")\n";
    return 1;
  }

  // Task 2: load session
  if (!opt_session.empty()) {
    std::cerr << "rpn: session support not yet implemented\n";
    return 1;
  }

  // Execute script files
  for (const auto &f : opt_files) {
    auto r = interp.sync_parseFile(f);
    if (r != rpn::WordDefinition::Result::ok) {
      std::cerr << "rpn: error in file " << f << ": " << interp.status() << "\n";
      return 1;
    }
  }

  // Evaluate expression (all args joined with space)
  if (!expressions.empty()) {
    std::string expr;
    for (size_t i = 0; i < expressions.size(); ++i) {
      if (i) expr += ' ';
      expr += expressions[i];
    }
    auto r = interp.sync_eval(expr);
    if (r != rpn::WordDefinition::Result::ok) {
      std::cerr << "rpn: " << interp.status() << "\n";
      return 1;
    }
  }

  // Task 2: save session
  if (!opt_session.empty()) {
    std::cerr << "rpn: session support not yet implemented\n";
    return 1;
  }

  // Output
  if (opt_words) {
    rpn::cli::printWords(interp);
  } else if (!opt_help_word.empty()) {
    rpn::cli::printWordHelp(interp, opt_help_word);
  } else {
    rpn::cli::printStack(interp, opt_json, opt_stack);
  }

  return 0;
}

/* end of QInc/Projects/RP42/rpn-lang/cli/rpn-main.cpp */
