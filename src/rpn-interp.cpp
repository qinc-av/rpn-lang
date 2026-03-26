/***************************************************
 * file: qinc/rpn-lang/src/rpn-interp.cpp
 *
 * @file    rpn-interp.cpp
 * @author  Eric L. Hernes
 * @version V1.0
 * @born_on   Saturday, May 27, 2023
 * @copyright (C) Copyright Eric L. Hernes 2023
 * @copyright (C) Copyright Q, Inc. 2023
 *
 * @brief   An Eric L. Hernes Signature Series C++ module
 *
 */

#include <fstream>
#include <iostream>
#include <queue>
#include <future>
#include <mutex>
#include <set>

#include <cmath>
#include <limits>
#include <algorithm>
#include <format>

#include "../rpn.h"
#include "geometry.h"

// These globals back the rpn::to_string() free functions used by stack type
// operator string() methods, which have no interpreter context.  The canonical
// per-instance state lives in Interp::Privates; these are kept in sync by the
// ->PRECISION and ->RADIX words.  TODO(Phase 2.2): remove once display is
// refactored to flow through the interpreter.
static int sk_double_decimals = 10;
static double sk_double_precision = 10000000000.0;
static int _sk_int_radix = 10;

std::string
rpn::to_string(const double &dv) {
  double dvr = std::round(sk_double_precision*(dv))/sk_double_precision;
  double intpart;

  std::string rv = std::format("{}", dvr);
  if (modf(dvr, &intpart) == 0.0) {
    rv += ".";
  }
  return rv;
}

std::string
rpn::to_string(int64_t iv) {

  int signbit = (iv<0 ? true : false);
  iv = std::abs(iv);

  const char digit[] = "0123456789ABCDEFGHIJKLMNOPRSTUVWXYZ";
  std::vector<char> stack;

  int64_t quot, rem;
  do {
    quot = iv / _sk_int_radix;
    rem = iv % _sk_int_radix;

    stack.push_back(digit[rem]);
    iv = quot;
  }  while( iv>0 );

  std::string result = (signbit ? "-" : "");
  return result + std::string(stack.rbegin(), stack.rend());
}

int
rpn::int_radix() {
  return _sk_int_radix;
}

static std::string::size_type
nextWord(std::string &word, std::string &buffer, const std::string &delim=" \n\t") {
  word = "";

  // Quoted string literal: "..." is one token even when it contains spaces.
  // Guard: only applies when " is not itself a delimiter (skip in ." handler context).
  if (!buffer.empty() && buffer[0] == '"' && delim.find('"') == std::string::npos) {
    auto p_close = buffer.find('"', 1);
    if (p_close != std::string::npos) {
      word = buffer.substr(0, p_close + 1);   // includes both quote chars
      buffer = buffer.substr(p_close + 1);
      auto p_start = buffer.find_first_not_of(" \n\t");
      buffer = (p_start != std::string::npos) ? buffer.substr(p_start) : "";
      return p_close;
    }
    // Unterminated quote — fall through to whitespace splitting
  }

  auto p1 = buffer.find_first_of(delim, 0);
  if (p1 == std::string::npos) { // not found
    word = buffer;
    buffer = "";
  } else {
    word = buffer.substr(0, p1);
    buffer = buffer.substr(p1+1, std::string::npos);
  }
  return p1;
}

using var_dict_t = std::map<std::string,std::unique_ptr<rpn::Stack::Object>>;

enum CompileType {
  ct_worddef,
  ct_forloop,
  ct_whileloop,
  ct_lambda,
  ct_mathexpr,
  ct_ifblock,      // IF ... THEN ... [ELSE ...] END
  ct_ifbranch,     // internal: collecting one branch of an if-block
};

struct Progn : public rpn::WordContext, public rpn::Stack::Object {
public:
  Progn(rpn::Interp::Privates &p, CompileType t) : _p(p), _type(t) { _locals = std::make_shared<var_dict_t>(); };
  Progn(const Progn &other) : _p(other._p), _wordlist(other._wordlist), _type(other._type), _ident(other._ident), _step(other._step) {
    _locals = std::make_shared<var_dict_t>();
    for(auto const &v : *other._locals) {
      _locals->emplace(v.first, v.second->deep_copy());
    }
  }

  virtual bool operator==(const Object &orhs) const override {
    auto &rhs = PEEK_CAST(const Progn,orhs);
    return ((_type == rhs._type) &&
	    (_wordlist == rhs._wordlist) &&
	    (_locals == rhs._locals));
  }

  virtual operator std::string() const override {
    std::string rv = "<<";
    for(auto const &w : _wordlist) {
      rv += " ";
      rv += w;
    }
    rv += " >>";
    return rv;
  };
  virtual std::string to_latex() const override {
    return (std::string)(*this);
  }
  virtual std::unique_ptr<rpn::Stack::Object> deep_copy() const override { return std::make_unique<Progn>(*this); };

  void addWord(const std::string &word) { _wordlist.push_back(word); };

  rpn::WordDefinition::Result eval(rpn::Interp &rpn);

  rpn::WordDefinition::Result eval_forloop(rpn::Interp &rpn);
  rpn::WordDefinition::Result eval_whileloop(rpn::Interp &rpn);
  rpn::WordDefinition::Result eval_lambda(rpn::Interp &rpn);
  rpn::WordDefinition::Result eval_mathexpr(rpn::Interp &rpn);
  rpn::WordDefinition::Result eval_ifblock(rpn::Interp &rpn);

  const std::vector<std::string> &wordlist() const { return _wordlist; };

  void clear() { _wordlist.clear(); };

  void print(std::ostream &out = std::cerr) {
    std::string str = (std::string)(*this);
    out << "Progn " << str << "\n";
    out << "  type: " << _type << "\n";
    out << "  ident: " << _ident << "\n";
    out << "  locals:";
    for(const auto &lv : *_locals) {
      out << " " << lv.first << ",";
    }
    out << "\n";
  }

  virtual std::string deparse() const override {
    // Note: wordlists containing address-key references to nested lambdas
    // (stored in _locals by ct_NEXT / ct_RSHIFT_LAMBDA) will not round-trip
    // correctly.  Flat lambda wordlists deparse cleanly.
    return (std::string)(*this);
  }

  // default to_latex()
  //  virtual std::string to_latex() const override {
  //	std::string rv = "\\text{" + (std::string)(*this) + "}";
  //	return rv;
  //  }

  rpn::Interp::Privates &_p;
  std::vector<std::string> _wordlist;
  std::shared_ptr<var_dict_t> _locals;
  CompileType _type;
  std::string _ident; // value and usage depends on type
  double _step = 1.0; // FOR loop step (default 1, set by STEP word)
};

#include <chrono>
using namespace std::chrono_literals;

struct rpn::Interp::Privates : public rpn::WordContext {
  // Per-instance display state.  Canonical values; globals above are kept in
  // sync for use by the rpn::to_string() free functions.
  int _double_decimals = 10;
  double _double_precision = 10000000000.0;
  int _int_radix = 10;

  // Optional debug/trace sink.  When set, _trace() messages are forwarded here
  // instead of being silently discarded.  Set via rpn::Interp::setDebugSink().
  std::function<void(const std::string &)> _debugSink;
  void _trace(const std::string &msg) const {
    if (_tracing && _debugSink) _debugSink(msg);
  }

  std::future<void> _arv;
  Privates(rpn::Interp &rpn, bool async) : _rpn(rpn), _tracing(false) {
    if (async) {
      _arv = std::async(std::launch::async, &rpn::Interp::Privates::main_loop, this);
    }
  };
  ~Privates() {
    if (_running && _arv.valid()) {
      _running = false;
      _qcv.notify_one();
      std::future_status status;
      do {
        status = _arv.wait_for(1s);
      } while (status != std::future_status::ready);
    }
  };

  rpn::WordDefinition::Result eval(const std::string &word, std::string &rest);
  rpn::WordDefinition::Result runtime_eval(const std::string &word, std::string &rest);
  rpn::WordDefinition::Result compiletime_eval(const std::string &word, std::string &rest);

  // add words that require acces to the Privates struct.
  void add_private_words();

  // validates a word in the dictionary and returns an iterator to it (or _rtDictionary.end() )
  std::multimap<std::string,WordDefinition>::iterator validate_word(const std::string &word, rpn::Stack &stack);
  bool word_exists(const std::string &word);

  rpn::WordDefinition::Result start_compile(CompileType t, bool needIdent);
  rpn::WordDefinition::Result end_compile(Progn *&progp, CompileType t);

  bool is_local_variable(const std::string &word);
  bool find_local_variable(var_dict_t::const_iterator &var, const std::string &word);

  rpn::WordDefinition::Result parse(std::string &line) {
    rpn::WordDefinition::Result rv=rpn::WordDefinition::Result::ok;
    for(; rv==rpn::WordDefinition::Result::ok && line.size()>0;) {
      std::string word;
      /*auto p1 = */ nextWord(word,line);
      rv = eval(word, line);
    }
    return rv;
  }

  rpn::WordDefinition::Result sync_parse_file(const std::string &path) {
    rpn::WordDefinition::Result rv=rpn::WordDefinition::Result::ok;
    std::ifstream ifs(path);

    // XXX-ELH: do we really need to stuff this into a vector before we parse it?
    std::string tmp;
    std::vector<std::string> lines;
    while(getline(ifs, tmp, '\n')) {
      lines.push_back(tmp);
    }

    int lineNo=0;
    for(auto line = lines.begin(); line!=lines.end() && rv==rpn::WordDefinition::Result::ok; line++, lineNo++) {
      rv = parse(*line);
      if (rv != rpn::WordDefinition::Result::ok) {
        _status = "parse error at " + path + ":" + std::to_string(lineNo) + " " + _status;
      }
    }

    return rv;
  }

  /*
   */
  std::multimap<std::string,WordDefinition> _rtDictionary;
  std::map<std::string,WordDefinition> _ctDictionary;

  rpn::Interp &_rpn;
  std::string _status;

  var_dict_t _globalVars;                              // STO/RCL global variables
  rpn::AngleMode _angleMode = rpn::AngleMode::degrees; // trig mode
  int _binaryWordsize = 64;                            // binary operation wordsize (1–64)

  std::vector<Progn> _ctVprogn;
  std::vector<std::shared_ptr<var_dict_t>> _vlocals;

  bool _needIdent;
  bool _tracing;

  std::mutex _qmx;
  std::condition_variable _qcv;

  struct Request {
    std::string cmd;
    std::string param;
    std::function<void(rpn::WordDefinition::Result res)> completionHandler;
  };
  void queue_request(const std::string &cmd, const std::string &param, const std::function<void(rpn::WordDefinition::Result res)> &completionHandler) {
    std::lock_guard lg(_qmx);
    _queue.push({cmd, param, completionHandler});
    _qcv.notify_one();
  }

  std::queue<Request> _queue;
  bool _running;
  void main_loop() {
    _running = true;
    for(;_running;) {

      std::unique_lock ul(_qmx);
      _qcv.wait(ul, [this]{return !_queue.empty() || !_running;});

      if (_running) {
	auto req = _queue.front();
	_queue.pop();

	if(req.cmd=="eval") {
	  req.completionHandler(parse(req.param));
	} else if (req.cmd == "parseFile") {
	  req.completionHandler(sync_parse_file(req.param));
	}
      }
    }
  }
};

rpn::WordDefinition::Result
Progn::eval_forloop(rpn::Interp &rpn) {
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  double end = rpn.stack.pop_as_double();
  double start = rpn.stack.pop_as_double();
  _p._vlocals.push_back(_locals);
  bool step_from_stack = std::isnan(_step);

  if (step_from_stack) {
    // FOR ... n STEP: body runs first, then pops step from TOS each iteration.
    // Loop exits when counter (after increment) would pass end.
    double current = start;
    double step = 1.0; // sentinel; overwritten after first body execution
    do {
      (*_locals)[_ident] = std::make_unique<StDouble>(current);
      rv = eval_lambda(rpn);
      if (rv != rpn::WordDefinition::Result::ok) break;
      step = rpn.stack.pop_as_double();
      current += step;
    } while (rv == rpn::WordDefinition::Result::ok &&
             (step > 0 ? current < end : current > end));
  } else {
    // FOR ... NEXT: check before body, fixed step (_step defaults to 1.0).
    double step = _step;
    auto loop_cond = [&]() { return step > 0 ? start < end : start > end; };
    for(; rv==rpn::WordDefinition::Result::ok && loop_cond(); start += step) {
      (*_locals)[_ident] = std::make_unique<StDouble>(start);
      rv = eval_lambda(rpn);
    }
  }
  _p._vlocals.pop_back();
  return rv;
}

rpn::WordDefinition::Result
Progn::eval_whileloop(rpn::Interp &rpn) {
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  bool until_style = (_locals->find("__until") != _locals->end());
  while (rv == rpn::WordDefinition::Result::ok) {
    rv = eval_lambda(rpn);
    if (rv != rpn::WordDefinition::Result::ok) break;
    bool cond = rpn.stack.pop_as_boolean();
    if (until_style ? cond : !cond) break;  // UNTIL: stop when true; WHILE: stop when false
  }
  return rv;
}

rpn::WordDefinition::Result
Progn::eval_ifblock(rpn::Interp &rpn) {
  bool cond = rpn.stack.pop_as_boolean();
  const char *key = cond ? "__true" : "__false";
  auto it = _locals->find(key);
  if (it != _locals->end()) {
    auto *branch = dynamic_cast<Progn *>(it->second.get());
    if (branch) return branch->eval(rpn);
  }
  return rpn::WordDefinition::Result::ok;
}

rpn::WordDefinition::Result
Progn::eval_lambda(rpn::Interp &rpn) {
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  std::string rest;

  _p._vlocals.push_back(_locals);

  for(auto wi= _wordlist.cbegin(); rv==rpn::WordDefinition::Result::ok && wi != _wordlist.cend(); wi++) {
    var_dict_t::const_iterator lv = _locals->find(*wi);
    bool lvp = (lv != _locals->end());
    if (!lvp) {
      lvp = _p.find_local_variable(lv, *wi);
    }

    if (lvp) {
      auto *pn = dynamic_cast<Progn*>(&(*lv->second));
      if (pn != nullptr) {
        if (_p._tracing) {
          rpn.stack.print("progn local");
          pn->print();
        }
        if (pn->_type == ct_lambda) {
          // A lambda found in locals is pushed onto the stack, not executed.
          // Use EXEC to execute it.
          rpn.stack.push(*pn);
        } else {
          pn->eval(rpn);
        }

      } else {
        if (_p._tracing) {
          std::string sv = (*lv->second);
          _p._trace("push local: " + lv->first + " => " + sv);
        }
        rpn.stack.push(*lv->second);

      }

    } else {
      if (*wi == ".\"") {
        // XXX-ELH: special treatment for the '."' word - we need 'rest' to contain the next word from the wordlist
        std::string word = *wi++;
        if (wi != _wordlist.cend()) {
          rest = *wi;
        }
        rv = _p.eval(word, rest);
        rest = ""; // and reset

      } else {
        rv = _p.eval(*wi, rest);
      }

    }
  }

  _p._vlocals.pop_back();

  return rv;
}

rpn::WordDefinition::Result
Progn::eval_mathexpr(rpn::Interp &rpn) {
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  return rv;
}

rpn::WordDefinition::Result
Progn::eval(rpn::Interp &rpn) {
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  switch (_type) {
  case ct_worddef:
    rv = eval_lambda(rpn);
    break;

  case ct_forloop:
    rv = eval_forloop(rpn);
    break;

  case ct_whileloop:
    rv = eval_whileloop(rpn);
    break;

  case ct_lambda:
    rv = eval_lambda(rpn);
    break;

  case ct_ifblock:
    rv = eval_ifblock(rpn);
    break;

  case ct_ifbranch:
    rv = eval_lambda(rpn);
    break;

  case ct_mathexpr:
    rv = eval_mathexpr(rpn);
    break;
  }
  return rv;
}

NATIVE_WORD_DECL(private, COMPILED_EVAL)  {
  Progn *progn = dynamic_cast<Progn*>(ctx);
  //  rpn::Interp::Privates *p = dynamic_cast<rpn::Interp::Privates*>(ctx);
  rpn::WordDefinition::Result rv = (progn) ? rpn::WordDefinition::Result::ok : rpn::WordDefinition::Result::eval_error;
  rv = progn->eval(rpn);
  return rv;
}

NATIVE_WORD_DECL(private, COLON) {
  // (rpn::Interp &rpn, rpn::WordContext *ctx, std::string &rest)
  rpn::Interp::Privates *p = dynamic_cast<rpn::Interp::Privates*>(ctx);
  return p->start_compile(ct_worddef, true);
}

NATIVE_WORD_DECL(private, ct_SEMICOLON) {
  // (rpn::Interp &rpn, rpn::WordContext *ctx, std::string &rest)
  rpn::Interp::Privates *p = dynamic_cast<rpn::Interp::Privates*>(ctx);

  Progn *progp=nullptr;
  rpn::WordDefinition::Result rv = p->end_compile(progp, ct_worddef);
  if (rv == rpn::WordDefinition::Result::ok) {

    p->_trace("adding '" + progp->_ident + "' to the dictionary");

    p->_rtDictionary.emplace(progp->_ident, rpn::WordDefinition {
	rpn::StackSizeValidator::zero, NATIVE_WORD_FN(private, COMPILED_EVAL), progp });

  } else {

    rv = rpn::WordDefinition::Result::compile_error;
  }
  return rv;
}

NATIVE_WORD_DECL(private, TRACE) {
  // (rpn::Interp &rpn, rpn::WordContext *ctx, std::string &rest)
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  rpn::Interp::Privates *p = dynamic_cast<rpn::Interp::Privates*>(ctx);
  bool pred = rpn.stack.pop_as_boolean();
  p->_tracing = pred;
  return rv;
}

NATIVE_WORD_DECL(private, WORDLIST) {
  // (rpn::Interp &rpn, rpn::WordContext *ctx, std::string &rest)
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  rpn::Interp::Privates *p = dynamic_cast<rpn::Interp::Privates*>(ctx);
  std::set<std::string> keys;
  for(const auto &dw : p->_rtDictionary) {
    keys.insert(dw.first);
  }
  StArray res;
  for(const auto &k : keys) {
    res.add_value(StString(k));
  }
  rpn.stack.push(res);
  return rv;
}

NATIVE_WORD_DECL(private, BOOL_TRUE) {
  // (rpn::Interp &rpn, rpn::WordContext *ctx, std::string &rest)
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  rpn.stack.push_boolean(true);
  return rv;
}

NATIVE_WORD_DECL(private, BOOL_FALSE) {
  // (rpn::Interp &rpn, rpn::WordContext *ctx, std::string &rest)
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  rpn.stack.push_boolean(false);
  return rv;
}

NATIVE_WORD_DECL(private, precision_to) {
  rpn::Interp::Privates *p = dynamic_cast<rpn::Interp::Privates*>(ctx);
  rpn.stack.push_double(p->_double_decimals);
  return rpn::WordDefinition::Result::ok;
}

NATIVE_WORD_DECL(private, to_precision) {
  rpn::Interp::Privates *p = dynamic_cast<rpn::Interp::Privates*>(ctx);
  auto new_dec = rpn.stack.pop_as_integer();
  p->_double_decimals = (int)std::clamp(new_dec, 0LL, 20LL);
  p->_double_precision = std::pow(10, p->_double_decimals);
  // Keep globals in sync for rpn::to_string() free functions (TODO Phase 2.2)
  sk_double_decimals = p->_double_decimals;
  sk_double_precision = p->_double_precision;
  return rpn::WordDefinition::Result::ok;
}

NATIVE_WORD_DECL(private, radix_to) {
  rpn::Interp::Privates *p = dynamic_cast<rpn::Interp::Privates*>(ctx);
  rpn.stack.push_double(p->_int_radix);
  return rpn::WordDefinition::Result::ok;
}

NATIVE_WORD_DECL(private, to_radix) {
  rpn::Interp::Privates *p = dynamic_cast<rpn::Interp::Privates*>(ctx);
  auto new_radix = rpn.stack.pop_as_integer();
  p->_int_radix = (int)new_radix;
  // Keep global in sync for rpn::to_string() free functions (TODO Phase 2.2)
  _sk_int_radix = p->_int_radix;
  return rpn::WordDefinition::Result::ok;
}

NATIVE_WORD_DECL(private, OPAREN) {
  // (rpn::Interp &rpn, rpn::WordContext *ctx, std::string &rest)
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  // rpn::Interp::Privates *p = dynamic_cast<rpn::Interp::Privates*>(ctx);
  std::string comment;
  auto cp = nextWord(comment, rest, ")");
  if (cp == std::string::npos) {
    rest = comment; // reset the buffer for future error message
    rv = rpn::WordDefinition::Result::parse_error;
  }
  return rv;
}

NATIVE_WORD_DECL(private, DQUOTE) {
  // (rpn::Interp &rpn, rpn::WordContext *ctx, std::string &rest)
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  // rpn::Interp::Privates *p = dynamic_cast<rpn::Interp::Privates*>(ctx);
  std::string literal;
  auto pos = nextWord(literal, rest, "\"");
  if (pos != std::string::npos) {
    rpn.stack.push_string(literal);
  } else {
    rv = rpn::WordDefinition::Result::parse_error;
    rest = literal; // reset buffer for error messages and diagnostics
  }
  return rv;
}

NATIVE_WORD_DECL(private, ct_DQUOTE) {
  // (rpn::Interp &rpn, rpn::WordContext *ctx, std::string &rest)
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  rpn::Interp::Privates *p = dynamic_cast<rpn::Interp::Privates*>(ctx);
  std::string literal;
  auto pos = nextWord(literal, rest, "\"");
  if (pos != std::string::npos) {
    p->_ctVprogn.back().addWord(".\"");
    p->_ctVprogn.back().addWord(literal + '"');
  } else {
    rv = rpn::WordDefinition::Result::parse_error;
    rest = literal; // reset buffer for error messages and diagnostics
  }
  return rv;
}

NATIVE_WORD_DECL(private, FOR) {
  rpn::Interp::Privates *p = dynamic_cast<rpn::Interp::Privates*>(ctx);
  return p->start_compile(ct_forloop, true);
}

NATIVE_WORD_DECL(private, ct_FOR) {
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  rpn::Interp::Privates *p = dynamic_cast<rpn::Interp::Privates*>(ctx);
  //  if (p->_ctVprogn.back()._type == ct_worddef) {
  //    p->_ctVprogn.back().addWord("FOR");
  //  } else {
    rv = p->start_compile(ct_forloop, true);
    //  }
  return rv;
}

NATIVE_WORD_DECL(private, deparse) { // not really private
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  std::string eval;
  auto obj = rpn.stack.pop();
  eval = obj->deparse();
  rpn.stack.push_string(eval);
  return rv;
}

NATIVE_WORD_DECL(private, eval) { // not really private
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  std::string eval = rpn.stack.pop_string();
  rpn.sync_eval(eval);
  return rv;
}

rpn::WordDefinition::Result
rpn::Interp::Privates::start_compile(CompileType t, bool needIdent) {
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  _needIdent = needIdent;
  _ctVprogn.push_back(Progn(*this, t));
  return rv;
}

rpn::WordDefinition::Result
rpn::Interp::Privates::end_compile(Progn *&progp, CompileType t) {
  rpn::WordDefinition::Result rv = ((_ctVprogn.size()>0) && _ctVprogn.back()._type == t)?
    rpn::WordDefinition::Result::ok : rpn::WordDefinition::Result::compile_error;

  progp=nullptr;
  if (rv == rpn::WordDefinition::Result::ok) {
    progp = new Progn(_ctVprogn.back());
    _ctVprogn.pop_back();
  }

  return rv;
}

NATIVE_WORD_DECL(private, ct_NEXT) {
  rpn::Interp::Privates *p = dynamic_cast<rpn::Interp::Privates*>(ctx);
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;

  /*  if (p->_ctVprogn.back()._type == ct_worddef) {
    p->_ctVprogn.back().addWord("NEXT");

    } else */ {
    Progn *progp=nullptr;

    rv = p->end_compile(progp, ct_forloop);

    if (rv == rpn::WordDefinition::Result::ok) {

      if (p->_ctVprogn.size() == 0) {
	// back to top level, evaluate here
	rv = progp->eval(rpn);

	delete progp;
	//	p->_locals.clear();

      } else {

	// in a definition or nested loops

	std::string word = std::to_string((uint64_t)progp);
	p->_ctVprogn.back()._locals->emplace(word, progp);
	//	delete progp; who owns progp???
	p->_ctVprogn.back().addWord(word);
      }

    } else {

      rv = rpn::WordDefinition::Result::compile_error;
    }
  }

  return rv;
}

// ct_STEP is the compile-time terminator for FOR...n STEP loops.
// It marks the Progn with NaN so eval_forloop knows to pop the step from TOS
// after each body execution (the body is responsible for leaving n on the stack).
// The actual stack usage is identical to ct_NEXT except for the NaN marker.
NATIVE_WORD_DECL(private, ct_STEP) {
  rpn::Interp::Privates *p = dynamic_cast<rpn::Interp::Privates*>(ctx);
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  p->_ctVprogn.back()._step = std::numeric_limits<double>::quiet_NaN();
  Progn *progp = nullptr;
  rv = p->end_compile(progp, ct_forloop);
  if (rv == rpn::WordDefinition::Result::ok) {
    if (p->_ctVprogn.size() == 0) {
      rv = progp->eval(rpn);
      delete progp;
    } else {
      std::string word = std::to_string((uint64_t)progp);
      p->_ctVprogn.back()._locals->emplace(word, progp);
      p->_ctVprogn.back().addWord(word);
    }
  } else {
    rv = rpn::WordDefinition::Result::compile_error;
  }
  return rv;
}

// ── BEGIN / WHILE / REPEAT / UNTIL ─────────────────────────────────────────
//
// Syntax (HP48-compatible):
//   BEGIN <body> <condition> WHILE REPEAT    ( loop while condition is true )
//   BEGIN <body> UNTIL                       ( loop until condition is true )
//
// All words between BEGIN and WHILE/UNTIL are compiled into one wordlist.
// At runtime, the wordlist is run; WHILE/UNTIL consume the top boolean:
//   WHILE: continue if true (stop if false)
//   UNTIL: stop if true (continue if false)
// "__until" key in _locals distinguishes the two flavours.

NATIVE_WORD_DECL(private, BEGIN) {
  rpn::Interp::Privates *p = dynamic_cast<rpn::Interp::Privates*>(ctx);
  return p->start_compile(ct_whileloop, false);
}

NATIVE_WORD_DECL(private, ct_WHILE) {
  // WHILE is a compile-time no-op: absence of "__until" marks WHILE semantics.
  (void)ctx; (void)rpn;
  return rpn::WordDefinition::Result::ok;
}

static rpn::WordDefinition::Result
finalize_whileloop(rpn::Interp &rpn, rpn::Interp::Privates *p) {
  Progn *progp = nullptr;
  auto rv = p->end_compile(progp, ct_whileloop);
  if (rv != rpn::WordDefinition::Result::ok) return rv;
  if (p->_ctVprogn.size() == 0) {
    rv = progp->eval(rpn);
    delete progp;
  } else {
    std::string word = std::to_string((uint64_t)progp);
    p->_ctVprogn.back()._locals->emplace(word, progp);
    p->_ctVprogn.back().addWord(word);
  }
  return rv;
}

NATIVE_WORD_DECL(private, ct_REPEAT) {
  rpn::Interp::Privates *p = dynamic_cast<rpn::Interp::Privates*>(ctx);
  return finalize_whileloop(rpn, p);
}

NATIVE_WORD_DECL(private, ct_UNTIL) {
  rpn::Interp::Privates *p = dynamic_cast<rpn::Interp::Privates*>(ctx);
  // Mark as UNTIL-style (exit when condition is true) before end_compile
  // so the flag is visible in the copied Progn's shared _locals.
  p->_ctVprogn.back()._locals->emplace("__until", std::make_unique<StBoolean>(StBoolean(true)));
  return finalize_whileloop(rpn, p);
}

// ── IF / THEN / ELSE / END ─────────────────────────────────────────────────
//
// Syntax (HP48-compatible):
//   IF <true-branch> THEN END
//   IF <true-branch> THEN <false-branch> ELSE END
//
// IF:   start ct_ifblock, start ct_ifbranch  (collecting true words)
// THEN: end ct_ifbranch → store as "__true" in if-block locals.
//       No new branch started — ELSE will start one if needed.
// ELSE: start ct_ifbranch  (collecting false words)
// END:  if a ct_ifbranch is open (ELSE was used), end it → "__false".
//       End ct_ifblock. Top-level: execute immediately.
//       Nested: store in parent locals with address key (ct_NEXT pattern).

NATIVE_WORD_DECL(private, IF) {
  rpn::Interp::Privates *p = dynamic_cast<rpn::Interp::Privates*>(ctx);
  auto rv = p->start_compile(ct_ifblock, false);
  if (rv == rpn::WordDefinition::Result::ok)
    rv = p->start_compile(ct_ifbranch, false);
  return rv;
}

NATIVE_WORD_DECL(private, ct_THEN) {
  rpn::Interp::Privates *p = dynamic_cast<rpn::Interp::Privates*>(ctx);
  Progn *branch = nullptr;
  auto rv = p->end_compile(branch, ct_ifbranch);
  if (rv != rpn::WordDefinition::Result::ok) return rv;
  p->_ctVprogn.back()._locals->emplace("__true", branch);
  return rv;
}

NATIVE_WORD_DECL(private, ct_ELSE) {
  rpn::Interp::Privates *p = dynamic_cast<rpn::Interp::Privates*>(ctx);
  // End the true branch (still open since IF), store it, start false branch.
  Progn *branch = nullptr;
  auto rv = p->end_compile(branch, ct_ifbranch);
  if (rv != rpn::WordDefinition::Result::ok) return rv;
  p->_ctVprogn.back()._locals->emplace("__true", branch);
  return p->start_compile(ct_ifbranch, false);
}

NATIVE_WORD_DECL(private, ct_END) {
  rpn::Interp::Privates *p = dynamic_cast<rpn::Interp::Privates*>(ctx);
  // If a false-branch is open (ELSE was used), close it.
  if (!p->_ctVprogn.empty() && p->_ctVprogn.back()._type == ct_ifbranch) {
    Progn *branch = nullptr;
    auto rv = p->end_compile(branch, ct_ifbranch);
    if (rv != rpn::WordDefinition::Result::ok) return rv;
    p->_ctVprogn.back()._locals->emplace("__false", branch);
  }
  // End the if-block.
  Progn *progp = nullptr;
  auto rv = p->end_compile(progp, ct_ifblock);
  if (rv != rpn::WordDefinition::Result::ok) return rv;
  if (p->_ctVprogn.size() == 0) {
    rv = progp->eval(rpn);
    delete progp;
  } else {
    std::string word = std::to_string((uint64_t)progp);
    p->_ctVprogn.back()._locals->emplace(word, progp);
    p->_ctVprogn.back().addWord(word);
  }
  return rv;
}

// << starts a lambda compile.  Valid at both runtime and compile time.
NATIVE_WORD_DECL(private, LSHIFT_LAMBDA) {
  rpn::Interp::Privates *p = dynamic_cast<rpn::Interp::Privates*>(ctx);
  return p->start_compile(ct_lambda, false);
}

// >> ends a lambda compile.
// At top level: push the compiled Progn onto the stack.
// Nested (inside a word def or another lambda): store in parent locals
// and add a reference that, when executed, pushes the lambda onto the stack.
NATIVE_WORD_DECL(private, ct_RSHIFT_LAMBDA) {
  rpn::Interp::Privates *p = dynamic_cast<rpn::Interp::Privates*>(ctx);
  Progn *progp = nullptr;
  rpn::WordDefinition::Result rv = p->end_compile(progp, ct_lambda);
  if (rv == rpn::WordDefinition::Result::ok) {
    if (p->_ctVprogn.size() == 0) {
      // Top level: push the lambda onto the stack
      rpn.stack.push(*progp);
      delete progp;
    } else {
      // Nested: store in parent locals; eval_lambda will push it (not execute)
      // because its _type is ct_lambda.
      std::string word = std::to_string((uint64_t)progp);
      p->_ctVprogn.back()._locals->emplace(word, progp);
      p->_ctVprogn.back().addWord(word);
    }
  }
  return rv;
}

// EXEC: pop a lambda (Progn) from TOS and evaluate it.
NATIVE_WORD_DECL(private, EXEC) {
  auto sob = rpn.stack.pop();
  Progn *progn = dynamic_cast<Progn*>(sob.get());
  if (progn == nullptr) return rpn::WordDefinition::Result::param_error;
  return progn->eval(rpn);
}

// ── STO / RCL / VARS / PURGE ────────────────────────────────────────────────

// Extract a variable name from TOS — accepts StName ('x') or StString ("x").
static std::string name_from_tos(rpn::Stack &stack) {
  auto obj = stack.pop();
  if (auto *n = dynamic_cast<StName*>(obj.get()))   return std::string(*n);
  if (auto *s = dynamic_cast<StString*>(obj.get())) return std::string(*s);
  throw std::runtime_error("expected name or string");
}

NATIVE_WORD_DECL(private, STO) {
  // ( val name -- )  Store TOS value into global variable.  Name is StName or StString.
  rpn::Interp::Privates *p = dynamic_cast<rpn::Interp::Privates*>(ctx);
  std::string name;
  try { name = name_from_tos(rpn.stack); }
  catch (...) { return rpn::WordDefinition::Result::param_error; }
  auto val = rpn.stack.pop();
  if (!val) return rpn::WordDefinition::Result::param_error;
  p->_globalVars[name] = std::move(val);
  return rpn::WordDefinition::Result::ok;
}

NATIVE_WORD_DECL(private, RCL) {
  // ( name -- val )  Recall a global variable.  Name is StName or StString.
  rpn::Interp::Privates *p = dynamic_cast<rpn::Interp::Privates*>(ctx);
  std::string name;
  try { name = name_from_tos(rpn.stack); }
  catch (...) { return rpn::WordDefinition::Result::param_error; }
  auto gv = p->_globalVars.find(name);
  if (gv == p->_globalVars.end()) return rpn::WordDefinition::Result::dict_error;
  rpn.stack.push(*gv->second);
  return rpn::WordDefinition::Result::ok;
}

NATIVE_WORD_DECL(private, VARS) {
  // ( -- array )  Push StName array of all global variable names.
  rpn::Interp::Privates *p = dynamic_cast<rpn::Interp::Privates*>(ctx);
  StArray arr;
  for (const auto &kv : p->_globalVars) {
    arr.add_value(StName(kv.first));
  }
  rpn.stack.push(arr);
  return rpn::WordDefinition::Result::ok;
}

NATIVE_WORD_DECL(private, PURGE) {
  // ( name -- )  Remove a global variable.  Name is StName or StString.
  rpn::Interp::Privates *p = dynamic_cast<rpn::Interp::Privates*>(ctx);
  std::string name;
  try { name = name_from_tos(rpn.stack); }
  catch (...) { return rpn::WordDefinition::Result::param_error; }
  p->_globalVars.erase(name);
  return rpn::WordDefinition::Result::ok;
}

void
rpn::Interp::Privates::add_private_words() {
  _rtDictionary.emplace(":", rpn::WordDefinition { rpn::StackSizeValidator::zero, NATIVE_WORD_FN(private, COLON), this });
  _rtDictionary.emplace("(", rpn::WordDefinition { rpn::StackSizeValidator::zero, NATIVE_WORD_FN(private, OPAREN), this });
  _rtDictionary.emplace(".\"", rpn::WordDefinition { rpn::StackSizeValidator::zero, NATIVE_WORD_FN(private, DQUOTE), this });
  _rtDictionary.emplace("FOR", rpn::WordDefinition { rpn::StrictTypeValidator::d2_double_double, NATIVE_WORD_FN(private, FOR), this });
  _rtDictionary.emplace("TRACE", rpn::WordDefinition { rpn::StrictTypeValidator::d1_boolean, NATIVE_WORD_FN(private, TRACE), this });
  _rtDictionary.emplace("WORDLIST", rpn::WordDefinition { rpn::StackSizeValidator::zero, NATIVE_WORD_FN(private, WORDLIST), this });
  _rtDictionary.emplace("DEPARSE", rpn::WordDefinition { rpn::StackSizeValidator::one, NATIVE_WORD_FN(private, deparse), this });
  _rtDictionary.emplace("EVAL", rpn::WordDefinition { rpn::StrictTypeValidator::d1_string, NATIVE_WORD_FN(private, eval), this });

  _rtDictionary.emplace("TRUE", rpn::WordDefinition { rpn::StackSizeValidator::zero, NATIVE_WORD_FN(private, BOOL_TRUE), this });
  _rtDictionary.emplace("FALSE", rpn::WordDefinition { rpn::StackSizeValidator::zero, NATIVE_WORD_FN(private, BOOL_FALSE), this });
  _rtDictionary.emplace("->PRECISION", rpn::WordDefinition { rpn::StrictTypeValidator::d1_double, NATIVE_WORD_FN(private, to_precision), this });
  _rtDictionary.emplace("PRECISION->", rpn::WordDefinition { rpn::StackSizeValidator::zero, NATIVE_WORD_FN(private, precision_to), this });

  _rtDictionary.emplace("->RADIX", rpn::WordDefinition { rpn::StrictTypeValidator::d1_double, NATIVE_WORD_FN(private, to_radix), this });
  _rtDictionary.emplace("RADIX->", rpn::WordDefinition { rpn::StackSizeValidator::zero, NATIVE_WORD_FN(private, radix_to), this });

  _rtDictionary.emplace("<<",    rpn::WordDefinition { rpn::StackSizeValidator::zero, NATIVE_WORD_FN(private, LSHIFT_LAMBDA), this });
  _rtDictionary.emplace("IF",    rpn::WordDefinition { rpn::StackSizeValidator::zero, NATIVE_WORD_FN(private, IF),             this });
  _rtDictionary.emplace("BEGIN", rpn::WordDefinition { rpn::StackSizeValidator::zero, NATIVE_WORD_FN(private, BEGIN),          this });
  _rtDictionary.emplace("EXEC",  rpn::WordDefinition { rpn::StackSizeValidator::one,  NATIVE_WORD_FN(private, EXEC),           this });
  _rtDictionary.emplace("STO",   rpn::WordDefinition { rpn::StackSizeValidator::two,  NATIVE_WORD_FN(private, STO),            this });
  _rtDictionary.emplace("RCL",   rpn::WordDefinition { rpn::StackSizeValidator::one,  NATIVE_WORD_FN(private, RCL),            this });
  _rtDictionary.emplace("VARS",  rpn::WordDefinition { rpn::StackSizeValidator::zero, NATIVE_WORD_FN(private, VARS),           this });
  _rtDictionary.emplace("PURGE", rpn::WordDefinition { rpn::StackSizeValidator::one,  NATIVE_WORD_FN(private, PURGE),          this });

  _ctDictionary.emplace(";",      rpn::WordDefinition { rpn::StackSizeValidator::zero, NATIVE_WORD_FN(private, ct_SEMICOLON),    this });
  _ctDictionary.emplace("(",      rpn::WordDefinition { rpn::StackSizeValidator::zero, NATIVE_WORD_FN(private, OPAREN),          this });
  _ctDictionary.emplace(".\"",    rpn::WordDefinition { rpn::StackSizeValidator::zero, NATIVE_WORD_FN(private, ct_DQUOTE),       this });
  _ctDictionary.emplace("FOR",    rpn::WordDefinition { rpn::StackSizeValidator::zero, NATIVE_WORD_FN(private, ct_FOR),          this });
  _ctDictionary.emplace("NEXT",   rpn::WordDefinition { rpn::StackSizeValidator::zero, NATIVE_WORD_FN(private, ct_NEXT),         this });
  _ctDictionary.emplace("BEGIN",  rpn::WordDefinition { rpn::StackSizeValidator::zero, NATIVE_WORD_FN(private, BEGIN),           this });
  _ctDictionary.emplace("WHILE",  rpn::WordDefinition { rpn::StackSizeValidator::zero, NATIVE_WORD_FN(private, ct_WHILE),        this });
  _ctDictionary.emplace("REPEAT", rpn::WordDefinition { rpn::StackSizeValidator::zero, NATIVE_WORD_FN(private, ct_REPEAT),       this });
  _ctDictionary.emplace("UNTIL",  rpn::WordDefinition { rpn::StackSizeValidator::zero, NATIVE_WORD_FN(private, ct_UNTIL),        this });
  _ctDictionary.emplace("<<",     rpn::WordDefinition { rpn::StackSizeValidator::zero, NATIVE_WORD_FN(private, LSHIFT_LAMBDA),   this });
  _ctDictionary.emplace(">>",     rpn::WordDefinition { rpn::StackSizeValidator::zero, NATIVE_WORD_FN(private, ct_RSHIFT_LAMBDA), this });
  _ctDictionary.emplace("IF",     rpn::WordDefinition { rpn::StackSizeValidator::zero, NATIVE_WORD_FN(private, IF),              this });
  _ctDictionary.emplace("THEN",   rpn::WordDefinition { rpn::StackSizeValidator::zero, NATIVE_WORD_FN(private, ct_THEN),         this });
  _ctDictionary.emplace("ELSE",   rpn::WordDefinition { rpn::StackSizeValidator::zero, NATIVE_WORD_FN(private, ct_ELSE),         this });
  _ctDictionary.emplace("END",    rpn::WordDefinition { rpn::StackSizeValidator::zero, NATIVE_WORD_FN(private, ct_END),          this });
  _ctDictionary.emplace("STEP",   rpn::WordDefinition { rpn::StackSizeValidator::zero,     NATIVE_WORD_FN(private, ct_STEP),         this });
}

rpn::WordDefinition::Result
rpn::Interp::Privates::eval(const std::string &word, std::string &rest) {
  _trace("evaluating: '" + word + "' '" + rest + "'");

  if (word.size()==0) {
    return rpn::WordDefinition::Result::ok;
  }

  rpn::WordDefinition::Result rv=rpn::WordDefinition::Result::eval_error;
  std::string wstatus  = word + ": ";

  std::string msg;
  if (_ctVprogn.size() != 0) {
    try {
      rv = compiletime_eval(word,rest);

    } catch (const std::bad_cast &/*bce*/) {
      msg = "type error compiling";
      if (rest.size()>0) msg += (std::string(" '") + rest + "'");
      rv = rpn::WordDefinition::Result::param_error;
      _ctVprogn.clear();

    } catch (const std::runtime_error &/*rte*/) {
      rv = rpn::WordDefinition::Result::eval_error;
      msg = "eval error compiling";
      if (rest.size()>0) msg += (std::string(" '") + rest + "'");
      _ctVprogn.clear();
    }

  } else {
    try {
      rv = runtime_eval(word,rest);

    } catch (const std::bad_cast &/*bce*/) {
      rv = rpn::WordDefinition::Result::param_error;
      msg = "type error";
      if (rest.size()>0) msg += (std::string(" '") + rest + "'");

    } catch (const std::runtime_error &/*rte*/) {
      rv = rpn::WordDefinition::Result::param_error;
      msg = "eval error";
      if (rest.size()>0) msg += (std::string(" '") + rest + "'");
    }
  }

  if (msg == "") {
    switch (rv) {
    case rpn::WordDefinition::Result::ok: {
      // crickets
      msg = "ok";
    }
      break;

    case rpn::WordDefinition::Result::parse_error: {
      msg = "parse error ";
      if (rest.size()>0) msg += (std::string(" '") + rest + "'");
      rest = ""; // discard the rest of the line;
    }
      break;

    case rpn::WordDefinition::Result::dict_error: {
      msg = "not found";
    }
      break;

    case rpn::WordDefinition::Result::param_error: {
      msg = "parameter error";
    }
      break;

    case rpn::WordDefinition::Result::implementation_error: {
      msg = "implementation error";
    }
      break;

    case rpn::WordDefinition::Result::eval_error: {
      msg = "eval error";
      if (rest.size()>0) msg += (std::string(" '") + rest + "'");
    }
      break;

    case rpn::WordDefinition::Result::compile_error: {
      msg = "compile error";
      if (rest.size()>0) msg += (std::string(" '") + rest + "'");
    }
      break;
    }
  }

  _status = wstatus + msg;

  _trace("returns: " + std::to_string((int)rv) + " ('" + rest + "')");

  return rv;
}

// A valid name starts with a letter or '_'; remaining chars are alnum, '_', or '-'.
// This prevents names that shadow operators ('+'), look like numbers ('42'), etc.
static bool is_valid_name(const std::string &s) {
  if (s.empty()) return false;
  if (!std::isalpha((unsigned char)s[0]) && s[0] != '_') return false;
  for (size_t i = 1; i < s.size(); ++i) {
    char c = s[i];
    if (!std::isalnum((unsigned char)c) && c != '_' && c != '-') return false;
  }
  return true;
}

rpn::WordDefinition::Result

rpn::Interp::Privates::runtime_eval(const std::string &word, std::string &rest) {
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::dict_error;

  // String literal: "content" → push StString (strips surrounding quotes)
  if (!word.empty() && word[0] == '"') {
    _rpn.stack.push_string(word.size() >= 2 ? word.substr(1, word.size() - 2) : "");
    return rpn::WordDefinition::Result::ok;
  }
  // Name literal: 'identifier' → push StName.  Content must be a valid name.
  // Invalid names (e.g. '+', '42') fall through to dict_error rather than
  // silently creating variables that shadow operators or number literals.
  if (word.size() >= 3 && word[0] == '\'' && word.back() == '\'') {
    std::string name = word.substr(1, word.size() - 2);
    if (is_valid_name(name)) {
      _rpn.stack.push(StName(name));
      return rpn::WordDefinition::Result::ok;
    }
    // Invalid name — fall through to dict_error
    _trace("invalid name literal: '" + name + "'");
    return rpn::WordDefinition::Result::dict_error;
  }

  // numbers just push
  if (std::isdigit(word[0])||(word[0]=='-'&&std::isdigit(word[1]))) {
    auto underscore = word.find("_");

    if (word.find("0x") != std::string::npos ||
	word.find("0d") != std::string::npos ||
	word.find("0o") != std::string::npos ||
	word.find("0b") != std::string::npos) {
      // XXX-ELH: parse as a base'd int
      int base = 10;
      auto zero = word.find("0");
      std::string::size_type pos = zero+2;
      switch(word[zero+1]) {
      case 'x':
	base = 16;
	break;

      case 'd':
	base = 10;
	break;

      case 'o':
	base = 8;
	break;

      case 'b':
	base = 2;
	break;

      default:
	rv = rpn::WordDefinition::Result::parse_error;
	break;
      }

      if (rv != rpn::WordDefinition::Result::parse_error) {
        long val = strtol(word.c_str()+pos, nullptr, base);
        _rpn.stack.push_integer(val);
        rv = rpn::WordDefinition::Result::ok;
      }

    } else if (underscore != std::string::npos) {

      int radix = (underscore == word.size()-1) ? 10 : strtol(word.c_str()+underscore+1, nullptr, 10);
      long val = strtol(word.c_str(), nullptr, radix);
      _rpn.stack.push_integer(val);
      rv = rpn::WordDefinition::Result::ok;

    } else {
      double val = strtod(word.c_str(), nullptr);
      _rpn.stack.push_double(val);
      rv = rpn::WordDefinition::Result::ok;

    }

  } else {
    // HP48 convention: global variables shadow dictionary words
    auto gv = _globalVars.find(word);
    if (gv != _globalVars.end()) {
      auto *pn = dynamic_cast<Progn*>(gv->second.get());
      if (pn && pn->_type != ct_lambda) {
        rv = pn->eval(_rpn);
      } else {
        _rpn.stack.push(*gv->second);
        rv = rpn::WordDefinition::Result::ok;
      }
    } else if (word_exists(word)) {
      auto we = validate_word(word, _rpn.stack);
      if (we != _rtDictionary.end()) {
	rv = we->second.eval(_rpn,  we->second.context, rest);
      } else {
	rv = rpn::WordDefinition::Result::param_error;
      }
    } else {
      _trace("runtime_eval: '" + word + "' does not exist");
    }
  }
  return rv;
}

bool
rpn::Interp::Privates::is_local_variable(const std::string &word) {
  bool rv = false;
  for(auto  pn =_ctVprogn.cbegin(); rv==false && pn != _ctVprogn.cend(); pn++) {
    rv = (rv || (word == pn->_ident) || (pn->_locals->find(word)!=pn->_locals->end()));
  }
  return rv;
}

bool
rpn::Interp::Privates::find_local_variable(var_dict_t::const_iterator &var, const std::string &word) {
  bool rv = false;
  for(auto  pn =_vlocals.crbegin(); rv==false && pn != _vlocals.crend(); pn++) {
    var = (*pn)->find(word);
    rv = (var != (*pn)->end());
  }
  return rv;
}

rpn::WordDefinition::Result
rpn::Interp::Privates::compiletime_eval(const std::string &word, std::string &rest) {
  rpn::WordDefinition::Result rv=rpn::WordDefinition::Result::dict_error;
  auto &progn = _ctVprogn.back();

  if (_needIdent && (progn._ident=="")) {
    progn._ident = word;
    _needIdent = false;

    rv=rpn::WordDefinition::Result::ok;

  } else {
    const auto &cw = _ctDictionary.find(word);
    if (cw != _ctDictionary.end()) {
      // found something in the compiletime dict, evaluate it
      rv = cw->second.eval(_rpn, cw->second.context, rest);

    } else if (!word.empty() && word[0] == '"') {
      // string literal: stored verbatim; runtime_eval detects and pushes StString
      progn.addWord(word);
      rv = rpn::WordDefinition::Result::ok;

    } else if (word.size() >= 3 && word[0] == '\'' && word.back() == '\'') {
      // name literal: validate then store verbatim; runtime_eval pushes StName
      std::string name = word.substr(1, word.size() - 2);
      if (is_valid_name(name)) {
        progn.addWord(word);
        rv = rpn::WordDefinition::Result::ok;
      } else {
        _trace("invalid name literal at compile time: '" + name + "'");
        rv = rpn::WordDefinition::Result::dict_error;
      }

    } else if (std::isdigit(word[0]) || (word[0]=='-' && word.size()>1 && std::isdigit(word[1]))) {
      // numbers just push (including negative literals like -1. or -42)
      progn.addWord(word);
      rv=rpn::WordDefinition::Result::ok;

    } else if (is_local_variable(word)) {
      // if we're in a loop compiling mode; and this is the loop variable,
      // push it
      progn.addWord(word);
      rv=rpn::WordDefinition::Result::ok;

    } else {
      // everything else, we check in the runtime dictionary
      const auto &rw = _rtDictionary.find(word);
      if (rw != _rtDictionary.end()) {
	progn.addWord(word);
	rv=rpn::WordDefinition::Result::ok;

	//      } else if ((progn._type == ct_worddef) && progn._wordlist.back()=="FOR") {
	// if it's a for-loop, we need the iteration var
	//	progn.addWord(word);

      } else {
	rv = rpn::WordDefinition::Result::dict_error;
        _trace("unrecognized word at compile time: '" + word + "'");

      }
    }
  }
  return rv;
}

rpn::Interp::Interp(bool async) {
  m_p = new Privates(*this, async);
  m_p->add_private_words();
  addStackWords();
  addLogicWords();
  addMathWords();
  addTypeWords();
  addFractionWords();
  addTimecodeWords();
  geometry::addWords(*this);
}

rpn::Interp::~Interp() {
  if (m_p) delete m_p;
}

rpn::AngleMode rpn::Interp::angleMode() const { return m_p->_angleMode; }
void rpn::Interp::setAngleMode(rpn::AngleMode mode) { m_p->_angleMode = mode; }

int rpn::Interp::binaryWordsize() const { return m_p->_binaryWordsize; }
void rpn::Interp::setBinaryWordsize(int bits) {
  m_p->_binaryWordsize = std::max(1, std::min(64, bits));
}

void
rpn::Interp::setDebugSink(std::function<void(const std::string &)> sink) {
  m_p->_debugSink = std::move(sink);
}

const std::string &
rpn::Interp::status() const {
  return m_p->_status;
}

bool
rpn::Interp::addDefinition(const std::string &word, const WordDefinition &def) {
  m_p->_rtDictionary.emplace(word, def);
  return true;
}

bool
rpn::Interp::removeDefinition(const std::string &word) {
  m_p->_rtDictionary.erase(word);
  return true;
}

bool
rpn::Interp::addCompiledWord(const std::string &word, const std::string &def, const StackValidator &/*v*/) {
  // Compile and register 'word' by evaluating a colon definition.
  // The custom validator 'v' is not yet applied; all compiled words currently
  // use StackSizeValidator::zero.  TODO(Phase 2.3): apply 'v' post-registration.
  auto rv = sync_eval(": " + word + " " + def + " ;");
  return rv == rpn::WordDefinition::Result::ok;
}

std::multimap<std::string,rpn::WordDefinition>::iterator
rpn::Interp::Privates::validate_word(const std::string &word, rpn::Stack &stack) {
  const auto &beg = _rtDictionary.lower_bound(word);
  const auto &end = _rtDictionary.upper_bound(word);
  if (beg != end) {
    auto stack_types = stack.types();
    for(auto we=beg; we!=end; we++) {
      if (we->second.validator(stack_types, stack)) {
	return we;
      }
    }
  }
  return _rtDictionary.end();
}

bool
rpn::Interp::Privates::word_exists(const std::string &word) {
  auto beg = _rtDictionary.lower_bound(word);
  const auto &end = _rtDictionary.upper_bound(word);
  return (beg != end);
}

bool
rpn::Interp::validateWord(const std::string &word) {
  return m_p->validate_word(word, this->stack) != m_p->_rtDictionary.end();
}

bool
rpn::Interp::wordExists(const std::string &word) {
  return m_p->word_exists(word);
}

void
rpn::Interp::eval(std::string line, std::function<void(rpn::WordDefinition::Result)>completionHandler) {
  //  rpn::WordDefinition::Result rv = m_p->parse(line);
  //  completionHandler(rv);
  m_p->queue_request("eval", line, completionHandler);
}

rpn::WordDefinition::Result
rpn::Interp::sync_eval(std::string line) {
  return m_p->parse(line);
}

rpn::WordDefinition::Result
rpn::Interp::sync_parseFile(const std::string &path) {
  return m_p->sync_parse_file(path);
}

void
rpn::Interp::parseFile(const std::string &path, std::function<void(rpn::WordDefinition::Result)>completionHandler) {
  //  rpn::WordDefinition::Result rv = m_p->sync_parse_file(path);
  //  completionHandler(rv);
  m_p->queue_request("parseFile", path, completionHandler);
}

/*
 */

bool
rpn::StrictTypeValidator::operator()(const std::vector<size_t> &types, rpn::Stack &stack) const {
  bool rv = types.size() >= _types.size();
  for(auto si=types.cbegin(), wi=_types.cbegin(); rv==true && wi!=_types.cend(); si++, wi++) {
    rv &= ((*wi==v_anytype) || (*si == *wi));
  }
  return rv;
}

bool
rpn::StrictTypeValidator::operator<(const rpn::StrictTypeValidator &rhs) const {
  return _types < rhs._types;
}

#if 0 //
std::string
rpn::StrictTypeValidator::to_string() const {
  std::string rv = "(StrictTypeValidator ";
  rv += std::to_string(_types.size());
  rv += ": ";
  for(size_t i=0; i<_types.size(); i++) {
    rv += "(" + std::to_string(i) + " " + std::to_string(_types[i]) + ") ";
  }
  rv += ")";
  return rv;
}

std::string
rpn::StackSizeValidator::to_string() const {
  std::string rv = "(StackSizeValidator ";
  rv += std::to_string(_n);
  rv += ")";
  return rv;
}
#endif

bool
rpn::StackSizeValidator::operator()(const std::vector<size_t> &types, rpn::Stack &stack) const {
  bool rv = false;
  if ((_n==(size_t)-1) &&
      types.size()>0 &&
      (types[0]==typeid(StInteger).hash_code() ||
       types[0]==typeid(StDouble).hash_code())) { // negative means to ntos - check top of stack as integer and make sure that the stack is >=
    auto nn = stack.peek_as_integer(1);
    rv = (nn && (types.size()-1) >= nn);
  } else {
    rv = (types.size() >=_n);
  }
  return rv;
}

/***************************************************
 * canned validators for common stack depth/types
 *
 */
const size_t rpn::StrictTypeValidator::v_anytype = typeid(rpn::Stack::Object).hash_code();
const rpn::StrictTypeValidator rpn::StrictTypeValidator::d1_double({typeid(StDouble).hash_code()},"d1_double");
const rpn::StrictTypeValidator rpn::StrictTypeValidator::d1_integer({typeid(StInteger).hash_code()},"d1_integer");
const rpn::StrictTypeValidator rpn::StrictTypeValidator::d1_boolean({typeid(StBoolean).hash_code()},"d1_boolean");
const rpn::StrictTypeValidator rpn::StrictTypeValidator::d1_string({typeid(StString).hash_code()},"d1_string");
const rpn::StrictTypeValidator rpn::StrictTypeValidator::d1_vec3({typeid(StVec3).hash_code()},"d1_vec3");
const rpn::StrictTypeValidator rpn::StrictTypeValidator::d1_object({typeid(StObject).hash_code()},"d1_object");
const rpn::StrictTypeValidator rpn::StrictTypeValidator::d1_array({typeid(StArray).hash_code()},"d1_array");

const rpn::StrictTypeValidator rpn::StrictTypeValidator::d2_boolean_boolean({typeid(StBoolean).hash_code(), typeid(StBoolean).hash_code()},"d2_boolean_boolean");
const rpn::StrictTypeValidator rpn::StrictTypeValidator::d2_double_double({typeid(StDouble).hash_code(), typeid(StDouble).hash_code()},"d2_double_double");
const rpn::StrictTypeValidator rpn::StrictTypeValidator::d2_double_integer({typeid(StDouble).hash_code(), typeid(StInteger).hash_code()},"d2_double_integer");
const rpn::StrictTypeValidator rpn::StrictTypeValidator::d2_integer_double({typeid(StInteger).hash_code(), typeid(StDouble).hash_code()},"d2_integer_double");
const rpn::StrictTypeValidator rpn::StrictTypeValidator::d2_integer_integer({typeid(StInteger).hash_code(), typeid(StInteger).hash_code()},"d2_integer_integer");

const rpn::StrictTypeValidator rpn::StrictTypeValidator::d2_double_vec3({typeid(StDouble).hash_code(), typeid(StVec3).hash_code()},"d2_double_vec3");
const rpn::StrictTypeValidator rpn::StrictTypeValidator::d2_vec3_double({typeid(StVec3).hash_code(), typeid(StDouble).hash_code()},"d2_vec3_double");

const rpn::StrictTypeValidator rpn::StrictTypeValidator::d2_integer_vec3({typeid(StInteger).hash_code(), typeid(StVec3).hash_code()},"d2_integer_vec3");
const rpn::StrictTypeValidator rpn::StrictTypeValidator::d2_vec3_integer({typeid(StVec3).hash_code(), typeid(StInteger).hash_code()},"d2_vec3_integer");

const rpn::StrictTypeValidator rpn::StrictTypeValidator::d2_vec3_vec3({typeid(StVec3).hash_code(), typeid(StVec3).hash_code()},"d2_vec3_vec3");

const rpn::StrictTypeValidator rpn::StrictTypeValidator::d2_string_any({typeid(StString).hash_code(),rpn::StrictTypeValidator::v_anytype},"d2_string_any");
const rpn::StrictTypeValidator rpn::StrictTypeValidator::d2_any_string({rpn::StrictTypeValidator::v_anytype,typeid(StString).hash_code()},"d2_any_string");

const rpn::StrictTypeValidator rpn::StrictTypeValidator::d2_array_any({typeid(StArray).hash_code(), rpn::StrictTypeValidator::v_anytype},"d2_array_any");
const rpn::StrictTypeValidator rpn::StrictTypeValidator::d2_any_array({rpn::StrictTypeValidator::v_anytype,typeid(StArray).hash_code()},"d2_any_array");
const rpn::StrictTypeValidator rpn::StrictTypeValidator::d2_object_any({typeid(StObject).hash_code(),rpn::StrictTypeValidator::v_anytype},"d2_object_any");
const rpn::StrictTypeValidator rpn::StrictTypeValidator::d2_any_object({rpn::StrictTypeValidator::v_anytype,typeid(StObject).hash_code()},"d2_any_object");

const rpn::StrictTypeValidator rpn::StrictTypeValidator::d3_double_double_double({typeid(StDouble).hash_code(),typeid(StDouble).hash_code(),typeid(StDouble).hash_code()},"d3_double_double_double");
const rpn::StrictTypeValidator rpn::StrictTypeValidator::d3_integer_double_double({typeid(StInteger).hash_code(),typeid(StDouble).hash_code(),typeid(StDouble).hash_code()},"d3_integer_double_double");
const rpn::StrictTypeValidator rpn::StrictTypeValidator::d3_double_integer_double({typeid(StDouble).hash_code(),typeid(StInteger).hash_code(),typeid(StDouble).hash_code()},"d3_double_integer_double");
const rpn::StrictTypeValidator rpn::StrictTypeValidator::d3_double_double_integer({typeid(StDouble).hash_code(),typeid(StDouble).hash_code(),typeid(StInteger).hash_code()},"d3_double_double_integer");

const rpn::StrictTypeValidator rpn::StrictTypeValidator::d3_integer_integer_integer({typeid(StInteger).hash_code(),typeid(StInteger).hash_code(),typeid(StInteger).hash_code()},"d3_integer_integer_integer");
const rpn::StrictTypeValidator rpn::StrictTypeValidator::d3_double_integer_integer({typeid(StDouble).hash_code(),typeid(StInteger).hash_code(),typeid(StInteger).hash_code()},"d3_double_integer_integer");
const rpn::StrictTypeValidator rpn::StrictTypeValidator::d3_integer_double_integer({typeid(StInteger).hash_code(),typeid(StDouble).hash_code(),typeid(StInteger).hash_code()},"d3_integer_double_integer");
const rpn::StrictTypeValidator rpn::StrictTypeValidator::d3_integer_integer_double({typeid(StInteger).hash_code(),typeid(StInteger).hash_code(),typeid(StDouble).hash_code()},"d3_integer_integer_double");

const rpn::StrictTypeValidator rpn::StrictTypeValidator::d3_object_string_any({typeid(StObject).hash_code(),typeid(StString).hash_code(),rpn::StrictTypeValidator::v_anytype},"d3_object_string_any");
const rpn::StrictTypeValidator rpn::StrictTypeValidator::d3_string_any_object({typeid(StString).hash_code(),rpn::StrictTypeValidator::v_anytype,typeid(StObject).hash_code()},"d3_string_any_object");
const rpn::StrictTypeValidator rpn::StrictTypeValidator::d3_boolean_any_any({typeid(StBoolean).hash_code(), rpn::StrictTypeValidator::v_anytype, rpn::StrictTypeValidator::v_anytype} ,"d3_boolean_any_any");
const rpn::StrictTypeValidator rpn::StrictTypeValidator::d3_any_any_boolean({rpn::StrictTypeValidator::v_anytype, rpn::StrictTypeValidator::v_anytype, typeid(StBoolean).hash_code()}, "d3_any_any_boolean");
const rpn::StrictTypeValidator rpn::StrictTypeValidator::d3_vec3_vec3_vec3({typeid(StVec3).hash_code(),typeid(StVec3).hash_code(),typeid(StVec3).hash_code()},"d3_vec3_vec3_vec3");

const rpn::StrictTypeValidator rpn::StrictTypeValidator::d4_double_double_double_integer({typeid(StDouble).hash_code(),typeid(StDouble).hash_code(),typeid(StDouble).hash_code(),typeid(StInteger).hash_code()},"d4_double_double_double_integer");
const rpn::StrictTypeValidator rpn::StrictTypeValidator::d4_integer_double_double_double({typeid(StInteger).hash_code(),typeid(StDouble).hash_code(),typeid(StDouble).hash_code(),typeid(StDouble).hash_code()},"d4_integer_double_double_double");

const rpn::StackSizeValidator rpn::StackSizeValidator::zero(0);
const rpn::StackSizeValidator rpn::StackSizeValidator::one(1);
const rpn::StackSizeValidator rpn::StackSizeValidator::two(2);
const rpn::StackSizeValidator rpn::StackSizeValidator::three(3);
const rpn::StackSizeValidator rpn::StackSizeValidator::ntos(-1); // n top of stack

/* end of qinc/rpn-lang/src/rpn-interp.cpp */
