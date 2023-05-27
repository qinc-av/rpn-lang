/***************************************************
 * file: github/elh/rpn-cnc/rpn-stack.cpp
 *
 * @file    rpn-stack.cpp
 * @author  Eric L. Hernes
 * @version V1.0
 * @born_on   Friday, May 26, 2023
 * @copyright (C) Copyright Eric L. Hernes 2023
 * @copyright (C) Copyright Q, Inc. 2023
 *
 * @brief   An Eric L. Hernes Signature Series C++ module
 *
 */

#include "rpn.h"

/*
 * primitives for stack operations
 */

void
rpn::Stack::push(const Object &ob) {
  std::unique_ptr<Object> ptr = ob.deep_copy();
  _stack.push_front(std::move(ptr));
}

void
rpn::Stack::push_boolean(const bool &val) {
  push(StBoolean(val));
}

void
rpn::Stack::push_string(const std::string &val) {
  push(StString(val));
}

void
rpn::Stack::push_integer(const int64_t &val) {
  push(StInteger(val));
}

void
rpn::Stack::push_double(const double &val) {
  push(StDouble(val));
}

std::unique_ptr<rpn::Stack::Object>
rpn::Stack::pop() {
  std::unique_ptr<Object> rv(nullptr);
  if (_stack.size()>0) {
    rv = std::move(_stack.front());
    _stack.pop_front();
  }
  return rv;
}

bool
rpn::Stack::pop_boolean() {
  std::unique_ptr<StBoolean> v(static_cast<StBoolean*>(pop().release()));
  return v->val();
}

std::string
rpn::Stack::pop_string() {
  std::unique_ptr<StString> v(static_cast<StString*>(pop().release()));
  return v->val();
}

int64_t
rpn::Stack::pop_integer() {
  std::unique_ptr<StInteger> v(static_cast<StInteger*>(pop().release()));
  return v->val();
}

double
rpn::Stack::pop_double() {
  std::unique_ptr<StDouble> v(static_cast<StDouble*>(pop().release()));
  return v->val();
}

const rpn::Stack::Object &
rpn::Stack::peek(int n) {
  if(n>0 && _stack.size()>=n) {
    return **(_stack.begin()+n-1);
  } else {
    std::string err = "peek: invalid paramaters (n ";
    err += std::to_string(n) + ") (depth " + std::to_string(_stack.size()) + ")";
    throw std::runtime_error(err);
  }
}

void
rpn::Stack::dropn(int n) {
  if (_stack.size()>=n) {
    _stack.erase(_stack.begin(), _stack.begin()+n);
  }
}

void
rpn::Stack::dupn(int n) {
  if (_stack.size()>=n) {
    for(int i = n; i; i--) {
      std::unique_ptr<Object> ptr = (*(_stack.begin()+(n-1)))->deep_copy();
      _stack.push_front(std::move(ptr));
    }
  } else {
    // handle error
    printf("%s: (size %lu) (n %d)\n", __func__, _stack.size(), n);
  }
}

void
rpn::Stack::nipn(int n) {
  if (_stack.size()>=n) {
    _stack.erase(_stack.begin()+(n-1));
  } else {
    // handle error
    printf("%s: (size %lu) (n %d)\n", __func__, _stack.size(), n);
  }
}

void
rpn::Stack::pick(int n) {
  if (n>0 && _stack.size()>=n) {
    std::unique_ptr<Object> ptr = (*(_stack.begin()+(n-1)))->deep_copy();
    _stack.push_front(std::move(ptr));
  } else {
    // throw error?
  }
}

void
rpn::Stack::rolldn(int n) {
  if (n>0 && n<=_stack.size()) {
    auto i = _stack.begin();
    auto ptr = std::move(*i);
    _stack.erase(i);
    _stack.insert(_stack.begin()+(n-1), std::move(ptr));
  } else {
    // handle error
  }
}

void
rpn::Stack::rollun(int n) {
  if (n>0 && n<=_stack.size()) {
    auto i = (_stack.begin()+(n-1));
    auto ptr = std::move(*i);
    _stack.erase(i);
    _stack.push_front(std::move(ptr));
  } else {
    // handle error
  }
}

void
rpn::Stack::tuckn(int n) {
  if (n>0 && n<=_stack.size()) {
    auto ptr = (*_stack.begin())->deep_copy();
    _stack.insert(_stack.begin()+(n-1), std::move(ptr));
  } else {
    // handle error
  }
}

void
rpn::Stack::swap() {
  if (_stack.size()>1) {
    std::swap(*_stack.begin(), *(_stack.begin()+1));
  }
}

void
rpn::Stack::drop() {
  if (_stack.size()>0) {
    _stack.pop_front();
  }
}

size_t
rpn::Stack::depth() {
  return _stack.size();
}

void
rpn::Stack::rollu() {
  rollun(_stack.size());
}

void
rpn::Stack::rolld() {
  rolldn(_stack.size());
}

void
rpn::Stack::over() {
  pick(2);
}

void
rpn::Stack::dup() {
  pick(1);
}

void
rpn::Stack::rotu() {
  rollun(3);
}

void
rpn::Stack::rotd() {
  rolldn(3);
}

void
rpn::Stack::print(const std::string &msg) {
  static const char *padding = "--------------------------------------------------------------------------------";
  int padlen = 66-msg.size();
  printf("+---- %02zu -- %s %*.*s+\n", _stack.size(), msg.c_str(), padlen, padlen, padding);
  size_t n = _stack.size();
  for(auto i=_stack.rbegin(); i!=_stack.rend(); i++, n--) {
    auto &r = **i; // https://stackoverflow.com/questions/46494928/clang-warning-on-expression-side-effects
    std::string type = typeid(r).name();
    if (type.size() > 30) {
      type.erase(30);
    }
    std::string strval = (*i)->to_string();
    if (strval.size() > 40) {
      strval.erase(37);
      strval += "...";
    }
    printf("|%30s | %40s | %02zu|\n", type.c_str(), strval.c_str(), n);
  }
  printf("+%*.*s+\n", 78,78,padding);
}

/* end of github/elh/rpn-cnc/rpn-stack.cpp */
