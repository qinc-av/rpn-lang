/***************************************************
 * file: ../work/QInc/Projects/RP42/rpn-lang/ui/wasm/rpn-wasm.cpp
 */
/**
 * @file    rpn-wasm.cpp
 * @author  Eric L. Hernes
 * @version V1.0
 * @born_on   Thursday, January 16, 2025
 * @copyright (C) Copyright Portrait Displays, Inc 2025
 *
 * @brief   An Eric L. Hernes Signature Series C++ module
 *
 */

#include <iostream>

#include <emscripten/emscripten.h>
#include <emscripten/bind.h>
#include <emscripten/val.h>

#include "../../rpn.h"

class RpnInterp {
public:
  RpnInterp(bool async) : _interp(async) {}

  rpn::WordDefinition::Result eval(const std::string &line) {
    return _interp.sync_eval(line);
  }

  bool validateWord(const std::string &word) {
    return _interp.validateWord(word);
  }

  bool wordExists(const std::string &word) {
    return _interp.wordExists(word);
  }

  const std::string status() const {
    return _interp.status();
  }

  const emscripten::val displayStack() const {
    std::vector<std::string> ds;
    size_t n = _interp.stack.depth();
    for(size_t i=0; i<n; i++) {
      ds.push_back(_interp.stack.peek_as_string(i+1));
    }
    return emscripten::val::array(ds.begin(), ds.end());
  }

private:
  rpn::Interp _interp;
}; // class RpnInterp



EMSCRIPTEN_BINDINGS(rpn_lang) {
  emscripten::class_<RpnInterp>("RpnInterp")
    .constructor<bool>()
    .function("eval", &RpnInterp::eval)
    .function("validateWord", &RpnInterp::validateWord)
    .function("wordExists", &RpnInterp::wordExists)
    .property("status", &RpnInterp::status)
    .property("displayStack", &RpnInterp::displayStack)
    ;

  emscripten::enum_<rpn::WordDefinition::Result>("Result")
    .value("ok", rpn::WordDefinition::Result::ok)
    .value("parse_error", rpn::WordDefinition::Result::parse_error) // parsing problem, definition, comment, string-literal, etc
    .value("dict_error", rpn::WordDefinition::Result::dict_error) // no such word
    .value("param_error", rpn::WordDefinition::Result::param_error) // parameters not right for the word
    .value("eval_error", rpn::WordDefinition::Result::compile_error) // error in compiling
    .value("implementation_error", rpn::WordDefinition::Result::implementation_error) // not implmemented or similar
    ;
}

/* end of ../work/QInc/Projects/RP42/rpn-lang/ui/wasm/rpn-wasm.cpp */
