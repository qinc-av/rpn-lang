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
#include "../rpn.h"

/***********************************************************************
 * C++ version
 */
RpnInterp::RpnInterp(bool async) : _interp(new rpn::Interp(async)) {
}
RpnInterp::~RpnInterp() {
  delete _interp;
}

void
RpnInterp::eval(std::string line, std::function<void(Result)>completionHandler) {
  _interp->eval(line, [&](rpn::WordDefinition::Result r1) {
      completionHandler((Result)r1);
    });
}

void
RpnInterp::parseFile(const std::string &path, std::function<void(Result)>completionHandler) {
  _interp->parseFile(path, [&](rpn::WordDefinition::Result r1) {
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
RpnInterp::displayStack() {
  std::vector<std::string> rv;
  size_t n = _interp->stack.depth();
  for(size_t i=0; i<n; i++) {
    rv.push_back(_interp->stack.peek_for_display(i));
  }
  return rv;
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

/* end of QInc/Projects/RP42/rpn-lang/src/rpn-hl.cpp */
