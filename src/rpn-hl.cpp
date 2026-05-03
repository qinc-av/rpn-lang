/***************************************************
 * file: QInc/Projects/RP42/rpn-lang/src/rpn-hl.cpp
 */
/**
 * @file    rpn-hl.cpp
 * @author  Eric L. Hernes
 * @version V1.0
 * @born_on   Thursday, January 16, 2025
 * @copyright (C) Copyright Portrait Displays, Inc 2025
 *
 * @brief   An Eric L. Hernes Signature Series C++ module
 *
 */

#include "../rpn-hl.h"
#define JSON_NO_IO
#include "nlohmann/json.hpp"
#include "../rpn.h"

RpnInterp::RpnInterp(bool async) : _interp(std::make_shared<rpn::Interp>(async)) {
}

// Defined out-of-line so the shared_ptr<rpn::Interp> destructor sees the
// complete type from rpn.h (rpn-hl.h only forward-declares rpn::Interp).
RpnInterp::~RpnInterp() = default;

void
RpnInterp::eval(std::string line, std::function<void(Result)> completionHandler) {
  _interp->eval(line, [completionHandler](rpn::WordDefinition::Result r1) {
    completionHandler((Result)r1);
  });
}

void
RpnInterp::parseFile(const std::string &path, std::function<void(Result)> completionHandler) {
  _interp->parseFile(path, [completionHandler](rpn::WordDefinition::Result r1) {
    completionHandler((Result)r1);
  });
}

bool
RpnInterp::validateWord(const std::string &word) {
  return _interp->validateWord(word);
}

bool
RpnInterp::wordExists(const std::string &word) {
  return _interp->wordExists(word);
}

std::string
RpnInterp::status() {
  return _interp->status();
}

std::vector<std::string>
RpnInterp::displayStack() const {
  std::vector<std::string> rv;
  size_t n = _interp->stack.depth();
  for (size_t i = 0; i < n; i++) {
    rv.push_back(_interp->stack.peek_for_display(i + 1));
  }
  return rv;
}

std::vector<StackItem>
RpnInterp::describeStack() {
  std::vector<StackItem> rv;
  size_t n = _interp->stack.depth();
  for (size_t i = 0; i < n; i++) {
    const auto &obj = _interp->stack.peek((int)(i + 1));
    rv.push_back({ obj.type_name(), (std::string)obj, obj.deparse(), obj.to_latex() });
  }
  return rv;
}

std::string
RpnInterp::stackJson(int n) const {
  return _interp->stack.peek(n).to_json().dump();
}

int RpnInterp::precision() const { return _interp->displayPrecision(); }
int RpnInterp::radix() const { return _interp->integerRadix(); }

std::string
RpnInterp::angleMode() const {
  switch (_interp->angleMode()) {
    case rpn::AngleMode::degrees:  return "DEG";
    case rpn::AngleMode::radians:  return "RAD";
    case rpn::AngleMode::gradians: return "GRAD";
  }
  return "DEG";
}

rpn::WordHelp
RpnInterp::wordHelp(const std::string &word) const {
  return _interp->wordHelp(word);
}

std::vector<std::string>
RpnInterp::wordList() const {
  return _interp->wordList();
}

void RpnInterp::cancel()                { _interp->cancel(); }
void RpnInterp::cancelAll()             { _interp->cancelAll(); }
bool RpnInterp::isCancelled() const     { return _interp->isCancelled(); }

void
RpnInterp::setProgressHandler(std::function<void(const std::string &, double)> handler) {
  _interp->setProgressHandler(std::move(handler));
}

void
RpnInterp::reportProgress(const std::string &message, double fraction) {
  _interp->reportProgress(message, fraction);
}

rpn::Interp &
RpnInterp::interp() {
  return *_interp;
}

void
RpnInterp::eval(const char *line, void (*callback)(int result, void *ctx), void *ctx) {
  _interp->eval(std::string(line), [callback, ctx](rpn::WordDefinition::Result r) {
    callback((int)r, ctx);
  });
}

void
RpnInterp::parseFile(const char *path, void (*callback)(int result, void *ctx), void *ctx) {
  _interp->parseFile(std::string(path), [callback, ctx](rpn::WordDefinition::Result r) {
    callback((int)r, ctx);
  });
}

/* end of QInc/Projects/RP42/rpn-lang/src/rpn-hl.cpp */
