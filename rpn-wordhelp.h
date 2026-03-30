#pragma once
#include <string>
#include <vector>

namespace rpn {

// Assembled word documentation returned by Interp::wordHelp() and RpnInterp::wordHelp().
// Defined here so rpn-hl.h can expose it without including rpn.h.
struct WordHelp {
  std::string name;
  std::string description;
  std::string category;
  std::vector<std::string> effects;  // one entry per overload
};

} // namespace rpn
