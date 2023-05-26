/***************************************************
 * file: github/elh/rpn-cnc/Rpn.h
 *
 * @file    Rpn.h
 * @author  Eric L. Hernes
 * @born_on   Thursday, May 25, 2023
 * @copyright (C) Copyright Eric L. Hernes 2023
 * @copyright (C) Copyright Q, Inc. 2023
 *
 * @brief   An Eric L. Hernes Signature Series C/C++ header
 *
 * $Id$
 */

#pragma once

#include <string>
#include <deque>

namespace rpn {
  class Stack {
  public:
    class Object {
    public:
      virtual ~Object() {};
      virtual operator std::string() const =0;
      virtual std::unique_ptr<Object> deep_copy() const =0;
      //    std::unique_ptr<RpnStack::Object> deep_copy() const { return std::make_unique<RpnStack::Object>(this->deep_copy()); };
      std::string to_string() const { return static_cast<std::string>(*this); }
    };

    Stack() {};
    ~Stack() {};

    void push(const Object &ob) {
      std::unique_ptr<Object> ptr = ob.deep_copy();
      _stack.push_front(std::move(ptr));
    }

    std::unique_ptr<Object> pop() {
      auto rv = std::move(_stack.front());
      _stack.pop_front();
      return rv;
    }

    const Object &peek() {
      return *_stack.front();
    }

    // basic stack operations
    // XXX-ELH: this needs some cleanup. We should reduce to just the necessary
    // primitives - for example swap() is rolldn(2)
    size_t depth(); // [prim]
    void dropn(int n); // [prim]
    void dupn(int n); // [prim]
    void nipn(); // [prim] drop Nos
    void pick(int n); // [prim] copy Nos to top
    void rolldn(int n); // prim
    void rollun(int n); // prim
    void tuck(); // [prim] copy Tos to 2os
    void tuckn(int n); // [prim] copy Tos to n-pos

    void swap(); // rolld(2);
    void drop(); // dropn(1);
    void nip(); // nipn(2); -- drop 2os
    void over(); // pick(2);
    void dup(); // pick(1);
    void rotu(); // rollu(3);
    void rotd(); // rolld(3);
    void rollu(); // rollu(depth);
    void rolld(); // rollu(depth);

    void print(const std::string &msg="");
  
  private:
    std::deque<std::unique_ptr<Object>> _stack;
  };
}

template<typename T>
class TStackObject : public rpn::Stack::Object {
 public:
 TStackObject(const T &v) : _v(v) {}
  virtual ~TStackObject() {}
  virtual std::unique_ptr<rpn::Stack::Object> deep_copy() const { return std::make_unique<TStackObject<T>>(*this); };
  virtual operator std::string() const { return (std::string)_v; };
 private:
  T _v;
};

class StDouble : public rpn::Stack::Object {
 public:
 StDouble(const double &v) : _v(v) {}
  ~StDouble() {}
  virtual std::unique_ptr<rpn::Stack::Object> deep_copy() const { return std::make_unique<StDouble>(*this); };
  virtual operator std::string() const { return std::to_string(_v); };
 private:
  double _v;
};

class XDouble {
 public:
 XDouble(const double &d) : _d(d) {}
  virtual operator std::string() const { return std::to_string(_d); };
 private:
  double _d;
};

class StString : public rpn::Stack::Object {
 public:
 StString(const std::string &v) : _v(v) {}
  ~StString() {}
  virtual std::unique_ptr<rpn::Stack::Object> deep_copy() const { return std::make_unique<StString>(*this); };
  virtual operator std::string() const { return (_v); };
 private:
  std::string _v;
};

#include <map>
class XObject {
public:
  XObject() = default;
  XObject(const XObject &v)  {
    for(auto const &m : v._members) {
      _members.emplace(m.first, m.second->deep_copy());
    }
  }
  void add_value(const std::string &name, const rpn::Stack::Object &val) {
    _members.emplace(name, val.deep_copy());
  }
  virtual operator std::string() const {
    std::string rv = "{";
    for(auto const &m : _members) {
      rv += m.first;
      rv += ":";
      rv += m.second->to_string();
      rv += ", ";
    }
    rv += "}";
    return rv;
  };
private:
  std::map<std::string,std::unique_ptr<rpn::Stack::Object>> _members;
};

class XArray {
public:
  XArray() = default;
  XArray(const XArray &a)  {
    for(auto const &e : a._v) {
      _v.push_back(e->deep_copy());
    }
  }
  void add_value(const rpn::Stack::Object &val) {
    _v.push_back(val.deep_copy());
  }
  virtual operator std::string() const {
    std::string rv = "[";
    for(auto const &e : _v) {
      rv += e->to_string();
      rv += ", ";
    }
    rv += "]";
    return rv;
  };
 private:
  std::vector<std::unique_ptr<rpn::Stack::Object>> _v;
};


/* end of github/elh/rpn-cnc/Rpn.h */
