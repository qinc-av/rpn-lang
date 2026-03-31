/***************************************************
 * file: QInc/Projects/RP42/rpn-lang/cli/output.cpp
 *
 * @brief   Stack and word output formatting for the rpn CLI.
 */

#include <iostream>
#include <map>
#include <vector>
#define JSON_NO_IO
#include "nlohmann/json.hpp"
#include "output.h"

using json = nlohmann::json;

namespace rpn::cli {

void printStack(rpn::Interp &interp, bool as_json, bool full_stack) {
  size_t depth = interp.stack.depth();

  if (depth == 0) {
    if (as_json) {
      std::cout << "[]" << std::endl;
    } else {
      std::cout << "(empty)" << std::endl;
    }
    return;
  }

  if (as_json) {
    // peek is 1-based: peek(1)=TOS, peek(depth)=bottom
    json arr = json::array();
    for (int i = (int)depth; i >= 1; --i) {
      arr.push_back(interp.stack.peek(i).to_json());
      if (!full_stack) break;
    }
    if (!full_stack) {
      std::cout << arr[0].dump() << std::endl;
    } else {
      std::cout << arr.dump(2) << std::endl;
    }
  } else {
    if (full_stack) {
      for (int i = (int)depth; i >= 1; --i) {
        std::cout << (std::string)interp.stack.peek(i) << "\n";
      }
    } else {
      std::cout << (std::string)interp.stack.peek(1) << std::endl;
    }
  }
}

void printWords(rpn::Interp &interp) {
  // Group words by category using wordHelp metadata.
  std::map<std::string, std::vector<json>> byCategory;
  for (const auto &name : interp.wordList()) {
    auto h = interp.wordHelp(name);
    std::string cat = h.category.empty() ? "other" : h.category;
    json w;
    w["name"] = name;
    if (!h.description.empty()) w["description"] = h.description;
    if (!h.effects.empty())     w["effects"]     = h.effects;
    byCategory[cat].push_back(std::move(w));
  }

  json result = json::array();
  for (auto &[cat, words] : byCategory) {
    json entry;
    entry["category"] = cat;
    entry["words"] = std::move(words);
    result.push_back(std::move(entry));
  }
  std::cout << result.dump(2) << std::endl;
}

void printWordHelp(rpn::Interp &interp, const std::string &word) {
  auto h = interp.wordHelp(word);
  json j;
  j["name"]        = h.name;
  j["description"] = h.description;
  j["category"]    = h.category;
  j["effects"]     = h.effects;
  std::cout << j.dump(2) << std::endl;
}

} // namespace rpn::cli

/* end of QInc/Projects/RP42/rpn-lang/cli/output.cpp */
