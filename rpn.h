/***************************************************
 * file: qinc/rpn-lang/rpn.h
 *
 * @file    rpn.h
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

#define _RPN_LANG_RPN_H_

#include <memory>
#include <vector>
#include <string>
#include <deque>
#include <map>
#include <cmath>
#include <complex>
#include <format>
#include <stdexcept>
#include <functional>
#include "nlohmann/json.hpp"
#include "rpn-hl.h"

namespace rpn {
  std::string to_string(const double &dv);
  std::string to_string(int64_t dv);
  int int_radix();

  class Stack {
  public:
    class Object {
    public:
      virtual ~Object() {};
      virtual bool operator==(const Object &rhs) const =0;
      virtual bool operator>(const Object &/*rhs*/) const {
        throw std::runtime_error("operator> invalid for type");
      };
      virtual bool operator<(const Object &/*rhs*/) const {
        throw std::runtime_error("operator> invalid for type");
      };
      virtual operator std::string() const =0;
      virtual std::unique_ptr<Object> deep_copy() const =0;
      virtual operator double() const { return std::nan(""); };
      //      virtual operator int64_t() const =0;
      virtual std::string deparse() const =0;

      virtual std::string to_text() const {
        return static_cast<std::string>(*this);
      }
      virtual std::string to_latex() const {
	std::string rv = "\\text{" + (std::string)(*this) + "}";
	return rv;
      }
      // Canonical type name string for this object.  Used by to_json() and the
      // type registry for Phase 2.3 compiled-word validation.
      virtual std::string type_name() const { return "any"; }

      // Returns a JSON descriptor: { "type": "...", "display": "...", "deparse": "...", "data": <json-value> }
      // Subclasses override to provide type-specific data encoding.
      virtual nlohmann::json to_json() const {
        return {{"type",type_name()},{"display",(std::string)(*this)},{"deparse",deparse()},{"data",nullptr}};
      }
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
    double pop_as_double(); // auto-converts integers to double, throws if not possible
    int64_t pop_as_integer(); // auto-converts doubles to integer, throws if not possible
    bool pop_as_boolean(); // auto-converts boolean, integer, double, and string, returns false if it couldn't convert

    std::unique_ptr<const Object> pop();

    const Object &peek(int n) const;
    bool peek_boolean(int n) const;
    std::string peek_string(int n) const;
    std::string peek_for_display(int n) const; // auto-converts to string if the type is not string
    int64_t peek_integer(int n) const;
    double peek_double(int n) const;
    std::optional<double> peek_as_double(int n) const; // auto-converts integers to double
    std::optional<int64_t> peek_as_integer(int n) const; // auto-converts doubles to integers

    // basic stack operations

    void clear(); // [prim]
    size_t depth() const; // [prim]
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

    void reverse();
    void reversen(int n);

    void print(const std::string &msg="");

    std::vector<size_t> types() const;
  private:
    std::deque<std::unique_ptr<const Object>> _stack;
  };

  class Interp;

  // base class to give a word definition context
  class WordContext {
  public:
    WordContext() {}
    virtual ~WordContext() {}
  protected:
  };

  // Class family for validating word definitions against stack type and depth
  class StackValidator {
  public:
    virtual bool operator()(const std::vector<size_t> &types, rpn::Stack &stack) const =0;
    const std::string to_string() const { return _name; }
    // Human-readable input types for word help display, e.g. "double integer".
    // Default returns the raw validator name; subclasses override for clean output.
    virtual std::string input_types() const { return _name; }
  protected:
  StackValidator(const std::string &name) : _name(name) {};
  std::string _name;
  };

  class StrictTypeValidator : public StackValidator {
  public:
    static const StrictTypeValidator d1_double;
    static const StrictTypeValidator d1_integer;
    static const StrictTypeValidator d1_boolean;
    static const StrictTypeValidator d1_object;
    static const StrictTypeValidator d1_string;
    static const StrictTypeValidator d1_array;
    static const StrictTypeValidator d1_vec3;
    static const StrictTypeValidator d1_mx3;

    static const StrictTypeValidator d2_vec3_vec3;
    static const StrictTypeValidator d2_mx3_vec3;
    static const StrictTypeValidator d2_double_double;
    static const StrictTypeValidator d2_integer_double;
    static const StrictTypeValidator d2_double_integer;
    static const StrictTypeValidator d2_integer_integer;
    static const StrictTypeValidator d2_boolean_boolean;

    static const StrictTypeValidator d2_double_vec3;
    static const StrictTypeValidator d2_vec3_double;
    static const StrictTypeValidator d2_integer_vec3;
    static const StrictTypeValidator d2_vec3_integer;

    static const StrictTypeValidator d2_any_array;
    static const StrictTypeValidator d2_array_any;

    static const StrictTypeValidator d2_any_string;
    static const StrictTypeValidator d2_string_any;

    static const StrictTypeValidator d2_any_object;
    static const StrictTypeValidator d2_object_any;

    static const StrictTypeValidator d3_double_double_double;
    static const StrictTypeValidator d3_double_double_integer;
    static const StrictTypeValidator d3_double_integer_double;
    static const StrictTypeValidator d3_integer_double_double;
    static const StrictTypeValidator d3_vec3_vec3_vec3;

    static const StrictTypeValidator d3_integer_integer_integer;
    static const StrictTypeValidator d3_integer_integer_double;
    static const StrictTypeValidator d3_integer_double_integer;
    static const StrictTypeValidator d3_double_integer_integer;

    static const StrictTypeValidator d3_boolean_any_any;
    static const StrictTypeValidator d3_any_any_boolean;
    static const StrictTypeValidator d3_any_string_object;
    static const StrictTypeValidator d3_object_any_string;

    static const StrictTypeValidator d4_integer_double_double_double;
    static const StrictTypeValidator d4_double_double_double_integer;
    static const StrictTypeValidator d4_double_double_double_double;

    // "number" validators — match either stack::Double or stack::Integer.
    // Use these for count/index parameters where the caller uses pop_as_integer()
    // or pop_as_double() to coerce.  Do NOT use for words that require strictly
    // integer semantics (binary ops, radix, wordsize — keep d1_integer there).
    static const StrictTypeValidator d1_number;
    static const StrictTypeValidator d2_number_number;

    static const size_t v_anytype;    // matches any type (base class hash, never instantiated directly)
    static const size_t v_numbertype; // matches stack::Double or stack::Integer (sentinel value 1)

  StrictTypeValidator(const std::vector<size_t> &types, const std::string name) : StackValidator(name),  _types(types) {}
    virtual bool operator()(const std::vector<size_t> &types, rpn::Stack &stack) const override;
    bool operator<(const StrictTypeValidator &rhs) const;
    // "d2_integer_double" → "integer double"  (stack-effect order: NOS first, TOS last)
    virtual std::string input_types() const override {
      auto p = _name.find('_');
      if (p == std::string::npos) return _name;
      std::string s = _name.substr(p + 1);
      for (char &c : s) if (c == '_') c = ' ';
      return s;
    }
    //    std::string to_string() const override;
  private:
    const std::vector<size_t> _types;
  };

  class StackSizeValidator : public StackValidator {
  public:
    static const StackSizeValidator zero;
    static const StackSizeValidator one;
    static const StackSizeValidator two;
    static const StackSizeValidator three;
    static const StackSizeValidator ntos; // n top of stack

  StackSizeValidator(size_t n) : StackValidator(std::string("StackSizeValidator") + ":" + std::to_string(n)), _n(n) {}
    virtual bool operator()(const std::vector<size_t> &types, rpn::Stack &stack) const override;
    // n=2 → "any any";  ntos sentinel (-1 cast to size_t) → "n*any"
    virtual std::string input_types() const override {
      if (_n == (size_t)-1) return "n*any";
      std::string s;
      for (size_t i = 0; i < _n; i++) { if (i) s += ' '; s += "any"; }
      return s;
    }
    //    std::string to_string() const override;
  private:
    size_t _n;
  };

  enum class AngleMode { degrees, radians, gradians };

  // rpn::WordHelp — see rpn-wordhelp.h (included above, outside namespace)

  struct WordDefinition {
    enum class Result {
      ok,
      parse_error, // parsing problem, definition, comment, string-literal, etc
      dict_error, // no such word
      param_error, // parameters not right for the word
      eval_error, // eval went awry
      compile_error, // error in compiling
      implementation_error, // not implmemented or similar
      cancelled, // execution was cancelled via Interp::cancel()
    };
    const StackValidator &validator;
    std::function<Result(Interp &rpn, WordContext *ctx, std::string &rest)> eval;
    WordContext *context;
    std::string return_types = ""; // output types, e.g. "double". Combined with
                                   // validator.input_types() to form the effect string.
  };

  class Interp {
  public:
    Interp(bool async);
    ~Interp();
    Interp (const Interp&) = delete;
    Interp& operator= (const Interp&) = delete;

    static void nullCompletionHandler(rpn::WordDefinition::Result) {};

    // mainly for test cases
    rpn::WordDefinition::Result sync_eval(std::string line);
    rpn::WordDefinition::Result sync_parseFile(const std::string &file);

    void eval(std::string line, std::function<void(rpn::WordDefinition::Result)>completionHandler=nullCompletionHandler);
    void parseFile(const std::string &path, std::function<void(rpn::WordDefinition::Result)>completionHandler=nullCompletionHandler);

    bool addDefinition(const std::string &word, const WordDefinition &def);
    bool removeDefinition(const std::string &word);
    bool addCompiledWord(const std::string &word, const std::string &def, const StackValidator &v = StackSizeValidator::zero);

    // Word introspection.  setWordCategory() sets the category stamped on all
    // subsequent addDefinition() calls (use at the top of each addXxxWords()).
    // addWordMetadata() sets the description for a word name.
    void setWordCategory(const std::string &category);
    void addWordMetadata(const std::string &word, const std::string &description);
    // Register a type name → typeid hash mapping for stack-effect comment parsing.
    // Called by each addXxxWords() for types not defined in rpn.h.
    void registerType(const std::string &name, size_t hash);

    WordHelp wordHelp(const std::string &word) const;
    std::vector<std::string> wordList() const;

    // Set a callback for debug/trace output.  Pass nullptr to disable.
    // Output is only produced when tracing is enabled (TRUE TRACE).
    void setDebugSink(std::function<void(const std::string &)> sink);

    // Cancel / progress.
    // cancel() requests cancellation of the currently running eval; the running
    // word returns Result::cancelled at the next word boundary or loop iteration.
    // cancelAll() additionally drains the pending request queue, calling each
    // completion handler with Result::cancelled.
    // isCancelled() may be polled by native words that implement long operations.
    // setProgressHandler() registers a sink called by reportProgress(); fraction
    // is in [0,1] or -1 for indeterminate.  The callback is invoked on whichever
    // thread the eval runs on — embedders should dispatch to the UI thread as needed.
    void cancel();
    void cancelAll();
    bool isCancelled() const;
    void setProgressHandler(std::function<void(const std::string &message, double fraction)> handler);
    void reportProgress(const std::string &message, double fraction = -1.0);

    bool validateWord(const std::string &word);
    bool wordExists(const std::string &word);

    AngleMode angleMode() const;
    void setAngleMode(AngleMode mode);

    int binaryWordsize() const;
    void setBinaryWordsize(int bits);  // valid range 1–64; clamped silently

    // Read-only display state.  Mutated only via ->PRECISION / ->RADIX words —
    // the engine owns this state and the UI must round-trip through eval().
    int displayPrecision() const;
    int integerRadix() const;

    /*
     * XXX-ELH- should the stack be public or private?
     *
     * Making it public makes it easier for the native words to manipulate it.
     * It is not an implementation detail, but rather an integral part of what
     * it is and how it is intended to be used.
     */

    Stack stack;
    const std::string &status() const;

    struct Privates;
    Privates *m_p; // it's opaque so it's still private, except that anything in rpn-interp.cpp can access it

  private:
    rpn::WordDefinition::Result parse(std::string &line);
    void addStackWords();
    void addMathWords();
    void addLogicWords();
    void addTypeWords();
    void addFractionWords();
    void addTimecodeWords();
    void addMatrixWords();
    void addStatsWords();
    void addFinanceWords();
    void addVec3Words();
    void addMx3Words();
    void addMarkerWords();
    void addStdlibWords();
  };


  class KeypadController : public WordContext {
  public:
    KeypadController();
    virtual void assignButton(unsigned column, unsigned row, const std::string &rpnword, const std::string &label="") =0;
    virtual void assignMenu(const std::string &menu, const std::string &rpnword, const std::string &label="") =0;
    virtual void clearAssignedButtons() =0;
    virtual void enable(bool pred) =0; // enables/disables the keypad buttons

  protected:
    void add_words(rpn::Interp &rpn);
    void remove_words(rpn::Interp &rpn);
  };
}

#define PEEK_CAST(obtype,ob)  dynamic_cast<const obtype&>(ob)
#define POP_CAST(obtype,ob)  dynamic_cast<const obtype&>(*ob.get())
#define OBJECTP_CAST(obtype)  dynamic_cast<obtype*>


namespace stack {
class Double : public rpn::Stack::Object {
 public:
 Double(const double &v) : _v(v) {}
  virtual std::unique_ptr<rpn::Stack::Object> deep_copy() const override { return std::make_unique<Double>(*this); };
  virtual operator std::string() const override { return rpn::to_string(_v); };
  operator double() const override { return _v; };
  virtual bool operator==(const Object &orhs) const override {
    const auto &rhs = PEEK_CAST(Double,orhs);
    return (_v == rhs._v);
  }
  virtual bool operator>(const Object &orhs) const override {
    auto &rhs = PEEK_CAST(Double,orhs);
    return (_v > rhs._v);
  }
  virtual bool operator<(const Object &orhs) const override {
    auto &rhs = PEEK_CAST(Double,orhs);
    return (_v < rhs._v);
  }
  virtual std::string deparse() const override {
    // 17 significant digits guarantees exact round-trip for IEEE 754 double.
    // Append '.' when the result looks like an integer so the parser treats it as a double.
    auto s = std::format("{:.17g}", _v);
    if (s.find_first_not_of("-0123456789") == std::string::npos) s += ".";
    return s;
  }
  // default to_text()
  virtual std::string to_latex() const override {
    return rpn::to_string(_v);
  }
  virtual std::string type_name() const override { return "double"; }
  virtual nlohmann::json to_json() const override {
    return {{"type",type_name()},{"display",(std::string)(*this)},{"deparse",deparse()},{"data",_v}};
  }

 private:
  double _v;
};

class Integer : public rpn::Stack::Object {
public:
Integer(const int64_t &v) : _v(v) {}
  virtual std::unique_ptr<rpn::Stack::Object> deep_copy() const override { return std::make_unique<Integer>(*this); };
  virtual operator std::string() const override { return rpn::to_string(_v); };
  virtual operator double() const override { return double(_v); };
  virtual bool operator==(const Object &orhs) const override {
    const auto &rhs = PEEK_CAST(Integer,orhs);
    return (_v == rhs._v);
  }
  operator int64_t() const { return _v; };
  operator uint64_t() const { return _v; };
  virtual bool operator>(const Object &orhs) const override {
    const auto &rhs = PEEK_CAST(Integer,orhs);
    return (_v > rhs._v);
  }
  virtual bool operator<(const Object &orhs) const override {
    const auto &rhs = PEEK_CAST(Integer,orhs);
    return (_v < rhs._v);
  }
  virtual std::string deparse() const override {
    return std::string("0d") + std::to_string(_v);
  }
  // default to_text()
  virtual std::string to_latex() const override {
    std::string digits = rpn::to_string(_v);
    if (rpn::int_radix() == 10) return digits;
    return digits + "_{" + std::to_string(rpn::int_radix()) + "}";
  }
  virtual std::string type_name() const override { return "integer"; }
  virtual nlohmann::json to_json() const override {
    return {{"type",type_name()},{"display",(std::string)(*this)},{"deparse",deparse()},{"data",_v}};
  }
 private:
  int64_t _v;
};

class Boolean : public rpn::Stack::Object {
 public:
  Boolean(const bool &v) : _v(v) {}
  virtual std::unique_ptr<rpn::Stack::Object> deep_copy() const override { return std::make_unique<Boolean>(_v); };
  virtual operator std::string() const override { return _v ? "<true>" : "<false>"; };
  operator bool() const { return _v; };
  virtual operator double() const override { return double(_v); };
  virtual bool operator==(const Object &orhs) const override {
    const auto &rhs = PEEK_CAST(Boolean,orhs);
    return (_v == rhs._v);
  }
  virtual bool operator>(const Object &orhs) const override {
    const auto &rhs = PEEK_CAST(Boolean,orhs);
    return (_v > rhs._v);
  }
  virtual bool operator<(const Object &orhs) const override {
    const auto &rhs = PEEK_CAST(Boolean,orhs);
    return (_v < rhs._v);
  }
  virtual std::string deparse() const override {
    return _v ? "TRUE" : "FALSE";
  }
  virtual std::string type_name() const override { return "boolean"; }
  virtual nlohmann::json to_json() const override {
    return {{"type",type_name()},{"display",(std::string)(*this)},{"deparse",deparse()},{"data",_v}};
  }
  // default to_latex()
 private:
  bool _v;
};

class String : public rpn::Stack::Object {
 public:
  String(const std::string &v) : _v(v) {}
  virtual operator std::string() const override { return _v; };
  virtual std::unique_ptr<rpn::Stack::Object> deep_copy() const override { return std::make_unique<String>(_v); };
  virtual bool operator==(const Object &orhs) const override {
    const auto &rhs = PEEK_CAST(String,orhs);
    return (_v == rhs._v);
  }
  virtual bool operator>(const Object &orhs) const override {
    auto &rhs = PEEK_CAST(String,orhs);
    return (_v > rhs._v);
  }
  virtual bool operator<(const Object &orhs) const override {
    auto &rhs = PEEK_CAST(String,orhs);
    return (_v < rhs._v);
  }
  virtual std::string deparse() const override {
    return "\"" + _v + "\"";
  }
  // default to_text()
  virtual std::string to_latex() const override {
    return std::string("\"") + _v + "\"";
  }
  virtual std::string type_name() const override { return "string"; }
  virtual nlohmann::json to_json() const override {
    return {{"type",type_name()},{"display",_v},{"deparse",deparse()},{"data",_v}};
  }

 private:
  std::string _v;
};

class Object : public rpn::Stack::Object {
public:
  Object() = default;
  Object(const Object &v)  {
    for(auto const &m : v._v) {
      _v.emplace(m.first, m.second->deep_copy());
    }
  }
  virtual std::unique_ptr<rpn::Stack::Object> deep_copy() const override {
    return std::make_unique<stack::Object>(*this);
  }
  virtual bool operator==(const rpn::Stack::Object &orhs) const override {
    const auto &rhs = PEEK_CAST(Object,orhs);
    bool rv = _v.size() == rhs._v.size();
    for(auto i=_v.cbegin(), j=rhs._v.cbegin(); rv && i!= _v.cend(); i++,j++) {
      rv &= (i->first == j->first) && (*(i->second) == *(j->second));
    }
    return rv;
  }
  virtual bool operator>(const rpn::Stack::Object &orhs) const override {
    auto &rhs = PEEK_CAST(stack::Object,orhs);
    // XXX-ELH: todo
    return false;
  }
  virtual bool operator<(const rpn::Stack::Object &orhs) const override {
    auto &rhs = PEEK_CAST(stack::Object,orhs);
    // XXX-ELH: todo
    return false;
  }
  void add_value(const std::string &name, const rpn::Stack::Object &val) {
    _v.emplace(name, val.deep_copy());
  }
  bool has_member(const std::string &name) const {
    return (_v.find(name) != _v.end());
  }
  const rpn::Stack::Object &member(const std::string &name) const {
    auto v = _v.find(name);
    if (v != _v.end()) {
      return *v->second;
    } else {
      std::string err = "XObject: no such member (";
      throw std::runtime_error(err + name + ")");
    }
  }
  virtual operator std::string() const override {
    std::string rv = "{";
    for(auto const &m : _v) {
      rv += m.first;
      rv += ":";
      rv += std::string(*m.second);
      rv += ", ";
    }
    rv += "}";
    return rv;
  };
  virtual std::string deparse() const override {
    if (_v.empty()) return "";
    std::string rv;
    bool first = true;
    for (const auto &m : _v) {
      if (first) {
        // ->OBJ takes d2_string_any: NOS=string-key, TOS=any-value.
        // Push key first, then value, then ->OBJ.
        rv += "\"" + m.first + "\" ";
        rv += m.second->deparse();
        rv += " ->OBJ ";
      } else {
        // + takes d3_object_any_string: NOS-most=object, NOS=any-value,
        // TOS=string-key.  Push value first, key second, then +.
        rv += m.second->deparse();
        rv += " \"" + m.first + "\"";
        rv += " + ";
      }
      first = false;
    }
    return rv;
  }
  const auto &val() const { return _v; };
  virtual std::string type_name() const override { return "object"; }
  virtual nlohmann::json to_json() const override {
    nlohmann::json data = nlohmann::json::object();
    for (const auto &m : _v) data[m.first] = m.second->to_json()["data"];
    return {{"type",type_name()},{"display",(std::string)(*this)},{"deparse",deparse()},{"data",data}};
  }
protected:
  std::map<std::string,std::unique_ptr<rpn::Stack::Object>> _v;
};

class Array : public rpn::Stack::Object {
public:
  Array() = default;
  Array(const Array &a)  {
    for(auto const &e : a._v) {
      _v.push_back(e->deep_copy());
    }
  }
  virtual std::unique_ptr<rpn::Stack::Object> deep_copy() const override {
    return std::make_unique<Array>(*this);
  }
  virtual bool operator==(const rpn::Stack::Object &orhs) const override {
    const auto &rhs = PEEK_CAST(Array,orhs);
    bool rv = _v.size() == rhs._v.size();
    for(auto i=_v.cbegin(), j=rhs._v.cbegin(); rv && i!= _v.cend(); i++,j++) {
      rv &= (*i == *j);
    }
    return rv;
  }
  virtual bool operator>(const rpn::Stack::Object &orhs) const override {
    const auto &rhs = PEEK_CAST(Array,orhs);
    // XXX-ELH: todo
    return false;
  }
  virtual bool operator<(const rpn::Stack::Object &orhs) const override {
    const auto &rhs = PEEK_CAST(Array,orhs);
    // XXX-ELH: todo
    return false;
  }
  virtual std::string deparse() const override {
    std::string rv;
    for(const auto &e : _v) {
      rv += e->deparse() + " ";
    }
    rv += std::to_string(_v.size()) + " ->ARRAY";
    return rv;
  }
  void add_value(const rpn::Stack::Object &val) {
    _v.push_back(val.deep_copy());
  }
  void reverse() {
    std::reverse(_v.begin(), _v.end());
  }
  virtual operator std::string() const override {
    std::string rv = "[";
    for(auto const &e : _v) {
      rv += std::string(*e);
      rv += ", ";
    }
    rv += "]";
    return rv;
  };
  const auto &val() const { return _v; };
  // default to_text()
  virtual std::string to_latex() const override {
    return (std::string)(*this);
  }
  virtual std::string type_name() const override { return "array"; }
  virtual nlohmann::json to_json() const override {
    nlohmann::json data = nlohmann::json::array();
    for (const auto &e : _v) data.push_back(e->to_json()["data"]);
    return {{"type",type_name()},{"display",(std::string)(*this)},{"deparse",deparse()},{"data",data}};
  }
 protected:
  std::vector<std::unique_ptr<rpn::Stack::Object>> _v;
};
// stack::Name — an HP48-style name/identifier object.
// Pushed by the 'name' literal syntax; used as the name argument to STO/RCL/PURGE.
// Names are more restricted than strings: no spaces, follow identifier rules.
class Name : public rpn::Stack::Object {
public:
  Name(const std::string &v) : _v(v) {}
  virtual operator std::string() const override { return _v; }
  virtual std::unique_ptr<rpn::Stack::Object> deep_copy() const override {
    return std::make_unique<Name>(_v);
  }
  virtual bool operator==(const Object &orhs) const override {
    const auto &rhs = PEEK_CAST(Name, orhs);
    return (_v == rhs._v);
  }
  virtual bool operator>(const Object &orhs) const override {
    const auto &rhs = PEEK_CAST(Name, orhs);
    return (_v > rhs._v);
  }
  virtual bool operator<(const Object &orhs) const override {
    const auto &rhs = PEEK_CAST(Name, orhs);
    return (_v < rhs._v);
  }
  virtual std::string deparse() const override { return "'" + _v + "'"; }
  virtual std::string to_latex() const override { return "'" + _v + "'"; }
  virtual std::string type_name() const override { return "name"; }
  virtual nlohmann::json to_json() const override {
    return {{"type",type_name()},{"display",_v},{"deparse",deparse()},{"data",_v}};
  }
private:
  std::string _v;
};

// Marker — a stack sentinel used to mark the base of a collection literal.
// Pushed by `[` and `{`; consumed by the matching close word (`]` or `}`).
class Marker : public rpn::Stack::Object {
public:
  explicit Marker(const std::string &label) : _label(label) {}
  const std::string &label() const { return _label; }
  virtual bool operator==(const rpn::Stack::Object &orhs) const override {
    const auto &rhs = PEEK_CAST(Marker, orhs);
    return _label == rhs._label;
  }
  virtual std::unique_ptr<rpn::Stack::Object> deep_copy() const override {
    return std::make_unique<Marker>(*this);
  }
  virtual operator std::string() const override { return _label + "..."; }
  virtual std::string deparse() const override { return _label; }
  virtual std::string to_latex() const override { return _label; }
  virtual std::string type_name() const override { return "marker"; }
  virtual nlohmann::json to_json() const override {
    return {{"type", type_name()}, {"display", (std::string)(*this)},
            {"deparse", deparse()}, {"data", _label}};
  }
private:
  std::string _label;
};

// Json — a JSON value on the stack. Uses MI: IS-A nlohmann::json and IS-A Stack::Object.
// Created by ->JSON (extracts "data" from any object's to_json() descriptor) or
// by JSON-> converting back to a native type.
class Json : public rpn::Stack::Object, public nlohmann::json {
public:
  Json() : nlohmann::json() {}
  explicit Json(const nlohmann::json &j) : nlohmann::json(j) {}
  virtual operator std::string() const override { return nlohmann::json::dump(); }
  virtual std::unique_ptr<rpn::Stack::Object> deep_copy() const override {
    return std::make_unique<Json>(*this);
  }
  virtual bool operator==(const rpn::Stack::Object &orhs) const override {
    const auto &rhs = PEEK_CAST(Json, orhs);
    return nlohmann::json::operator==(static_cast<const nlohmann::json &>(rhs));
  }
  virtual std::string deparse() const override {
    // JSON string itself is not valid RPN — must be pushed via JSON word or eval.
    // For round-trip we emit the raw dump; JSON-> reconstructs.
    return nlohmann::json::dump();
  }
  virtual std::string type_name() const override { return "json"; }
  virtual nlohmann::json to_json() const override {
    return {{"type",type_name()},{"display",dump()},{"deparse",deparse()},{"data",static_cast<const nlohmann::json &>(*this)}};
  }
};
class Complex : public rpn::Stack::Object, public std::complex<double> {
public:
  Complex() = delete;
  Complex(double re, double im) : std::complex<double>(re,im) {}
  Complex(const Complex &cx) : std::complex<double>(cx) {}
  Complex(const std::complex<double> &cx) : std::complex<double>(cx) {}
  virtual bool operator==(const rpn::Stack::Object &orhs) const override {
    auto &rhs = PEEK_CAST(Complex, orhs);
    return ((const std::complex<double> &)*this) == ((const std::complex<double> &)rhs);
  }
  virtual std::unique_ptr<rpn::Stack::Object> deep_copy() const override { return std::make_unique<Complex>(*this); };
  virtual operator std::string() const override {
    std::string rv = rpn::to_string(this->real());
    if (this->imag() > 0) rv += "+";
    rv += rpn::to_string(this->imag());
    rv += "i";
    return rv;
  }
  virtual std::string deparse() const override {
    auto dp = [](double v) {
      auto s = std::format("{:.17g}", v);
      if (s.find_first_not_of("-0123456789") == std::string::npos) s += ".";
      return s;
    };
    return dp(this->real()) + " " + dp(this->imag()) + " ->COMPLEX";
  }
  virtual std::string to_latex() const override { return (std::string)(*this); }
  virtual std::string type_name() const override { return "complex"; }
  virtual nlohmann::json to_json() const override {
    return {{"type",type_name()},{"display",(std::string)(*this)},{"deparse",deparse()},
            {"data",{{"re",this->real()},{"im",this->imag()}}}};
  }
};

} // namespace stack

// convenience macros for adding native methods
#define NATIVE_WORD_FN(mangler, op) mangler##_func_##op

#define NATIVE_WORD_DECL(mangler, fn) \
  static rpn::WordDefinition::Result NATIVE_WORD_FN(mangler, fn)(rpn::Interp &rpn, rpn::WordContext *ctx, std::string &rest)

#define NATIVE_WORD_FN_0_DOUBLE(mangler, fn, val) \
  NATIVE_WORD_DECL(mangler, fn) {					\
    rpn.stack.push_double(val);						\
    return rpn::WordDefinition::Result::ok;				\
  }

#if 0
#define NATIVE_WORD_FN_0_INTEGER(mangler, fn, val) \
  NATIVE_WORD_DECL(mangler, fn) {					\
    rpn.stack.push_integer(val);					\
    return rpn::WordDefinition::Result::ok;				\
  }
#endif
#define NATIVE_WORD_FN_1_NUMBER(mangler, fn)				\
  NATIVE_WORD_DECL(mangler, fn) {					\
    double s1 = rpn.stack.pop_as_double();				\
    rpn.stack.push_double(fn(s1));					\
    return rpn::WordDefinition::Result::ok;				\
  }

#define NATIVE_WORD_FN_1_INTEGER(mangler, fn) \
  NATIVE_WORD_DECL(mangler, fn) {					\
    auto s1 = rpn.stack.pop_integer();					\
    rpn.stack.push_integer(fn(s1));					\
    return rpn::WordDefinition::Result::ok;				\
  }

#define NATIVE_WORD_FN_2_NUMBER(mangler, fn)				\
  NATIVE_WORD_DECL(mangler, fn) {					\
    auto s1 = rpn.stack.pop_as_double();				\
    auto s2 = rpn.stack.pop_as_double();				\
    rpn.stack.push_double(fn(s2,s1));					\
    return rpn::WordDefinition::Result::ok;				\
  }

#define NATIVE_WORD_FN_2_INTEGER(mangler, fn)				\
  NATIVE_WORD_DECL(mangler, fn) {					\
    auto s1 = rpn.stack.pop_integer();					\
    auto s2 = rpn.stack.pop_integer();					\
    rpn.stack.push_integer(fn(s2,s1));					\
    return rpn::WordDefinition::Result::ok;				\
  }

#define NATIVE_WORD_WDEF(mangler, validator, w, ptr)			\
  { validator, NATIVE_WORD_FN(mangler, w), ptr }

// common case for binary function that converts to double except for
// when both parameters are integers
#define ADD_NATIVE_2_NUMBER_WDEF(mangler, r, symbol, double_func, integer_func, ptr) \
  r.addDefinition(symbol, NATIVE_WORD_WDEF(mangler, rpn::StrictTypeValidator::d2_double_double, double_func, ptr)); \
  r.addDefinition(symbol, NATIVE_WORD_WDEF(mangler, rpn::StrictTypeValidator::d2_integer_double, double_func, ptr)); \
  r.addDefinition(symbol, NATIVE_WORD_WDEF(mangler, rpn::StrictTypeValidator::d2_double_integer, double_func, ptr)); \
  r.addDefinition(symbol, NATIVE_WORD_WDEF(mangler, rpn::StrictTypeValidator::d2_integer_integer, integer_func, ptr))

#define ADD_NATIVE_1_NUMBER_WDEF(mangler, r, symbol, double_func, integer_func, ptr) \
  r.addDefinition(symbol, NATIVE_WORD_WDEF(mangler, rpn::StrictTypeValidator::d1_double, double_func, ptr)); \
  r.addDefinition(symbol, NATIVE_WORD_WDEF(mangler, rpn::StrictTypeValidator::d1_integer, integer_func, ptr))

// For words that accept integer or double and always produce a double result (single registration).
#define ADD_NATIVE_1_FLOAT_WDEF(mangler, r, symbol, func, ptr) \
  r.addDefinition(symbol, NATIVE_WORD_WDEF(mangler, rpn::StrictTypeValidator::d1_number, func, ptr))

#define ADD_NATIVE_2_FLOAT_WDEF(mangler, r, symbol, func, ptr) \
  r.addDefinition(symbol, NATIVE_WORD_WDEF(mangler, rpn::StrictTypeValidator::d2_number_number, func, ptr))

#define ADD_NATIVE_3_NUMBER_WDEF(mangler, r, symbol, double_func, integer_func, ptr) \
  r.addDefinition(symbol, NATIVE_WORD_WDEF(mangler, rpn::StrictTypeValidator::d3_double_double_double, double_func, ptr)); \
  r.addDefinition(symbol, NATIVE_WORD_WDEF(mangler, rpn::StrictTypeValidator::d3_integer_double_double, double_func, ptr)); \
  r.addDefinition(symbol, NATIVE_WORD_WDEF(mangler, rpn::StrictTypeValidator::d3_double_integer_double, double_func, ptr)); \
  r.addDefinition(symbol, NATIVE_WORD_WDEF(mangler, rpn::StrictTypeValidator::d3_integer_integer_double, integer_func, ptr)); \
  r.addDefinition(symbol, NATIVE_WORD_WDEF(mangler, rpn::StrictTypeValidator::d3_double_double_integer, double_func, ptr)); \
  r.addDefinition(symbol, NATIVE_WORD_WDEF(mangler, rpn::StrictTypeValidator::d3_integer_double_integer, double_func, ptr)); \
  r.addDefinition(symbol, NATIVE_WORD_WDEF(mangler, rpn::StrictTypeValidator::d3_double_integer_integer, double_func, ptr)); \
  r.addDefinition(symbol, NATIVE_WORD_WDEF(mangler, rpn::StrictTypeValidator::d3_integer_integer_integer, integer_func, ptr))

/* end of qinc/rpn-lang/rpn.h */
