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
#include <map>

namespace rpn {
  class Stack {
  public:
    class Object {
    public:
      virtual ~Object() {};
      virtual operator std::string() const =0;
      virtual std::unique_ptr<Object> deep_copy() const =0;
      std::string to_string() const { return static_cast<std::string>(*this); }
    };

    Stack() {};
    ~Stack() {};

    void push(const Object &ob);
    std::unique_ptr<Object> pop();
    const Object &peek(int n);

    // basic stack operations
    // XXX-ELH: this needs some cleanup. We should reduce to just the necessary
    // primitives - for example swap() is rolldn(2)
    size_t depth(); // [prim]
    void dropn(int n); // [prim]
    void dupn(int n); // [prim]
    void nipn(int n); // [prim] drop Nos
    void pick(int n); // [prim] copy Nos to top
    void rolldn(int n); // prim
    void rollun(int n); // prim
    void tuckn(int n); // [prim] copy Tos to n-pos
    void swap(); // [prim] for efficiency, not that it matters in 2023
    void drop(); // [prim] for efficiency, not that it matters in 2023
    void rollu(); // rollu(depth);
    void rolld(); // rollu(depth);

    void over(); // pick(2);
    void dup(); // pick(1);
    void rotu(); // rollu(3);
    void rotd(); // rolld(3);

    void print(const std::string &msg="");
  
  private:
    std::deque<std::unique_ptr<Object>> _stack;
  };

  class Controller;

  class WordContext {
  public:
    WordContext() {}
    virtual ~WordContext() {}
  protected:
  };
  using type_t = uint32_t;
  struct WordDefinition {
    //    std::string description;
    std::vector<type_t> params;
    std::function<bool(Controller &rpn, WordContext *ctx, std::string &rest)> eval;
    WordContext *context;
  };

  class Controller {
  public:
    Controller() {}
    ~Controller() {}
    bool parse(std::string &line);
    bool parseFile(const std::string &path);

    bool addDefinition(const std::string &word, const WordDefinition &def);
  private:
    std::multimap<std::string,WordDefinition> _rtDictionary;
    std::map<std::string,WordDefinition> _ctDictionary;
  };
}

/*
 * XXX-ELH: this doesn't work all that well
 */
#if 0
template<typename T>
class TStackObject : public rpn::Stack::Object {
 public:
 TStackObject(const T &v) : _v(v) {}
  virtual ~TStackObject() {}
  virtual std::unique_ptr<rpn::Stack::Object> deep_copy() const { return std::make_unique<TStackObject<T>>(*this); };
  virtual operator std::string() const { return (std::string)_v; };
  auto val() const { return _v.val(); }
 private:
  T _v;
};
#endif

class StDouble : public rpn::Stack::Object {
 public:
 StDouble(const double &v) : _v(v) {}
  ~StDouble() {}
  virtual std::unique_ptr<rpn::Stack::Object> deep_copy() const { return std::make_unique<StDouble>(*this); };
  virtual operator std::string() const { return std::to_string(_v); };
  auto val() const { return _v; };
 private:
  double _v;
};

class StInteger : public rpn::Stack::Object {
 public:
 StInteger(const int64_t &v) : _v(v) {}
  ~StInteger() {}
  virtual std::unique_ptr<rpn::Stack::Object> deep_copy() const { return std::make_unique<StInteger>(*this); };
  virtual operator std::string() const { return std::to_string(_v); };
  auto val() const { return _v; };
 private:
  int64_t _v;
};

class StString : public rpn::Stack::Object {
 public:
 StString(const std::string &v) : _v(v) {}
  ~StString() {}
  virtual std::unique_ptr<rpn::Stack::Object> deep_copy() const { return std::make_unique<StString>(*this); };
  virtual operator std::string() const { return (_v); };
  auto val() const { return _v; };
 private:
  std::string _v;
};

#include <map>
class StObject : public rpn::Stack::Object {
public:
  StObject() = default;
  StObject(const StObject &v)  {
    for(auto const &m : v._members) {
      _members.emplace(m.first, m.second->deep_copy());
    }
  }
  virtual std::unique_ptr<rpn::Stack::Object> deep_copy() const { return std::make_unique<StObject>(*this); };
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
  auto const &val() const { return _members; };
 private:
  std::map<std::string,std::unique_ptr<rpn::Stack::Object>> _members;
};

class StArray : public rpn::Stack::Object {
public:
  StArray() = default;
  StArray(const StArray &a)  {
    for(auto const &e : a._v) {
      _v.push_back(e->deep_copy());
    }
  }
  virtual std::unique_ptr<rpn::Stack::Object> deep_copy() const { return std::make_unique<StArray>(*this); };
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
  auto const &val() const { return _v; };
 private:
  std::vector<std::unique_ptr<rpn::Stack::Object>> _v;
};

/*
 * these XTypes were intended to work with the TStackObject<> template
 */
#if 0
class XDouble {
 public:
 XDouble(const double &v) : _v(v) {}
  virtual operator std::string() const { return std::to_string(_v); };
  auto val() const { return _v; }
 private:
  double _v;
};

class XInteger {
 public:
 XInteger(const int64_t &v) : _v(v) {}
  virtual operator std::string() const { return std::to_string(_v); };
  auto val() const { return _v; }
 private:
  int64_t _v;
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
#endif

/* end of github/elh/rpn-cnc/Rpn.h */
