
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
      virtual bool operator==(const Object &rhs) =0;
      virtual operator std::string() const =0;
      virtual std::unique_ptr<Object> deep_copy() const =0;
      std::string to_string() const { return static_cast<std::string>(*this); }
    };

    Stack() {};
    ~Stack() {};

    void push(const Object &ob);
    void push_boolean(const bool &val);
    void push_string(const std::string &val);
    void push_integer(const int64_t &val);
    void push_double(const double &val);

    bool pop_boolean();
    std::string pop_string();
    int64_t pop_integer();
    double pop_double();
    double pop_as_double(); // auto-converts integers to double, returns NaN if it couldn't convert

    std::unique_ptr<Object> pop();
    const Object &peek(int n);

    // basic stack operations
    // XXX-ELH: this needs some cleanup. We should reduce to just the necessary
    // primitives - for example swap() is rolldn(2)
    void clear(); // [prim]
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

    std::vector<size_t> types() const;
  private:
    std::deque<std::unique_ptr<Object>> _stack;
  };

  class Runtime;

  class WordContext {
  public:
    WordContext() {}
    virtual ~WordContext() {}
  protected:
  };
  using type_t = uint32_t;

  struct WordDefinition {
    enum class Result {
      ok,
      parse_error, // parsing problem, definition, comment, string-literal, etc
      dict_error, // no such word
      param_error, // parameters not right for the word
      eval_error, // eval went awry
    };
    //    std::string description;
    std::vector<std::size_t> params;
    std::function<Result(Runtime &rpn, WordContext *ctx, std::string &rest)> eval;
    WordContext *context;
  };

  class Runtime {
  public:
    Runtime();
    ~Runtime();
    bool parse(std::string &line);
    bool parseFile(const std::string &path);

    bool addDefinition(const std::string &word, const WordDefinition &def);

    /*
     * XXX-ELH- should the stack be public or private?
     *
     * Making it public makes it easier for the native words to manipulate it.
     * It is not an implementation detail, but rather an integral part of what
     * it is and how it is intended to be used. 
     */

    Stack stack;

    struct Privates;
  private:
    void addInternalWords(WordContext *wc);
    Privates *m_p;
  };
}

#if 0
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

class StBoolean : public rpn::Stack::Object {
 public:
 StBoolean(const bool &v) : _v(v) {}
  ~StBoolean() {}
  virtual std::unique_ptr<rpn::Stack::Object> deep_copy() const { return std::make_unique<StBoolean>(*this); };
  virtual operator std::string() const { return _v ? "true" : "false"; };
  auto val() const { return _v; };
 private:
  bool _v;
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
#endif

#define OBJECT_CAST(obtype)  dynamic_cast<const obtype&>

template<typename T>
class TStackObject : public rpn::Stack::Object {
 public:
  TStackObject() = default; //: _v(v) {}
 TStackObject(const T &v) : _v(v) {}
  virtual bool operator==(const Object &orhs) override {
    auto &rhs = OBJECT_CAST(TStackObject<T>)(orhs);
    return (_v == rhs._v);
  }
  virtual ~TStackObject() {}
  virtual std::unique_ptr<rpn::Stack::Object> deep_copy() const override { return std::make_unique<TStackObject<T>>(*this); };
  virtual operator std::string() const override { return (std::string)_v; };
  auto val() const { return _v.val(); };
  auto &inner() { return _v; };
 private:
  T _v;
};

class XDouble {
 public:
 XDouble(const double &v) : _v(v) {}
  virtual operator std::string() const { return std::to_string(_v); };
  auto val() const { return _v; }
  bool operator==(const XDouble &rhs) {
    return _v == rhs._v;
  }
 private:
  double _v;
};

class XInteger {
 public:
 XInteger(const int64_t &v) : _v(v) {}
  virtual operator std::string() const { return std::to_string(_v); };
  auto val() const { return _v; }
  bool operator==(const XInteger &rhs) {
    return _v == rhs._v;
  }
 private:
  int64_t _v;
};

class XBoolean {
 public:
 XBoolean(const bool &v) : _v(v) {}
  virtual operator std::string() const { return _v ? "<true>" : "<false>"; };
  auto val() const { return _v; }
  bool operator==(const XBoolean &rhs) {
    return _v == rhs._v;
  }
 private:
  bool _v;
};

class XString {
 public:
 XString(const std::string &v) : _v(v) {}
  virtual operator std::string() const { return _v; };
  auto val() const { return _v; }
  bool operator==(const XString &rhs) {
    return _v == rhs._v;
  }
 private:
  std::string _v;
};

#include <map>
class XObject {
public:
  XObject() = default;
  XObject(const XObject &v)  {
    for(auto const &m : v._v) {
      _v.emplace(m.first, m.second->deep_copy());
    }
  }
  bool operator==(const XObject &rhs) {
    bool rv = _v.size() == rhs._v.size();
    for(auto i=_v.cbegin(), j=rhs._v.cbegin(); rv && i!= _v.cend(); i++,j++) {
      rv &= (i->first == j->first) && (*(i->second) == *(j->second));
    }
    return rv;
  }
  void add_value(const std::string &name, const rpn::Stack::Object &val) {
    _v.emplace(name, val.deep_copy());
  }
  virtual operator std::string() const {
    std::string rv = "{";
    for(auto const &m : _v) {
      rv += m.first;
      rv += ":";
      rv += m.second->to_string();
      rv += ", ";
    }
    rv += "}";
    return rv;
  };
protected:
  std::map<std::string,std::unique_ptr<rpn::Stack::Object>> _v;
};

class XArray {
public:
  XArray() = default;
  XArray(const XArray &a)  {
    for(auto const &e : a._v) {
      _v.push_back(e->deep_copy());
    }
  }
  bool operator==(const XArray &rhs) {
    bool rv = _v.size() == rhs._v.size();
    for(auto i=_v.cbegin(), j=rhs._v.cbegin(); rv && i!= _v.cend(); i++,j++) {
      rv &= (*i == *j);
    }
    return rv;
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
 protected:
  std::vector<std::unique_ptr<rpn::Stack::Object>> _v;
};

using StDouble = TStackObject<XDouble>;
using StInteger = TStackObject<XInteger>;
using StBoolean = TStackObject<XBoolean>;
using StString = TStackObject<XString>;
using StObject = TStackObject<XObject>;
using StArray = TStackObject<XArray>;


/* end of github/elh/rpn-cnc/Rpn.h */
