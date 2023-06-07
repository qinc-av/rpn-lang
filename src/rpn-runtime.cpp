/***************************************************
 * file: github/elh/rpn-cnc/rpn-runtime.cpp
 *
 * @file    rpn-runtime.cpp
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

#include "rpn.h"

static std::string::size_type
nextWord(std::string &word, std::string &buffer, char delim=' ') {
  word = "";
  auto p1 = buffer.find(delim, 0);
  if (p1 == std::string::npos) { // not found
    word = buffer;
    buffer = "";
  } else {
    word = buffer.substr(0, p1);
    buffer = buffer.substr(p1+1, std::string::npos);
  }
  return p1;
}

struct Progn : public rpn::WordContext, public rpn::Stack::Object {
public:
  Progn(rpn::Runtime::Privates &p) : _p(p) {};
  Progn(const Progn &other) : _p(other._p), _wordlist(other._wordlist) {
    for(auto const &v : other._locals) {
      _locals.emplace(v.first, v.second->deep_copy());
    }
  }

  virtual bool operator==(const Object &orhs) const override {
    auto &rhs = OBJECT_CAST(Progn)(orhs);
    return ((_wordlist == rhs._wordlist) &&
	    (_locals == rhs._locals));
  }

  virtual operator std::string() const override {
    std::string rv = "<<";
    for(auto const &w : _wordlist) {
      rv += " ";
      rv += w;
    }
    rv += ">>";
    return rv;
  };
  virtual std::unique_ptr<rpn::Stack::Object> deep_copy() const override { return std::make_unique<Progn>(*this); };

  void addWord(const std::string &word) { _wordlist.push_back(word); };

  rpn::WordDefinition::Result eval();
  const std::vector<std::string> &wordlist() const { return _wordlist; };

  void clear() { _wordlist.clear(); };

  rpn::Runtime::Privates &_p;
  std::vector<std::string> _wordlist;
  std::map<std::string,std::unique_ptr<rpn::Stack::Object>> _locals;
};

struct LoopContext {
  std::string loopVar;
  double start;
  double end;
};

enum runtime_state {
  st_run,
  st_define,
  st_loop
};

struct rpn::Runtime::Privates : public rpn::WordContext {
  Privates() : _newDefinition(*this), _tracing(true), _loopVars({}), _loopCount(0) {};
  ~Privates() {};

  rpn::WordDefinition::Result eval(rpn::Runtime &rpn, const std::string &word, std::string &rest);
  rpn::WordDefinition::Result runtime_eval(rpn::Runtime &rpn, const std::string &word, std::string &rest);
  rpn::WordDefinition::Result compiletime_eval(rpn::Runtime &rpn, const std::string &word, std::string &rest);

  // add words that require acces to the Privates struct.
  void add_private_words(rpn::Runtime &rpn);

  rpn::WordDefinition::Result for_loop(rpn::Runtime &rpn, double increment);

  // validates a word in the dictionary and returns an iterator to it (or _rtDictionary.end() )
  std::multimap<std::string,WordDefinition>::iterator validate_word(const std::string &word, rpn::Stack &stack);
  bool word_exists(const std::string &word);

  bool is_local_variable(const std::string &word);

  /*
   */
  std::multimap<std::string,WordDefinition> _rtDictionary;
  std::map<std::string,WordDefinition> _ctDictionary;

  std::string _status;

  runtime_state _state;
  std::string _newWord;
  Progn _newDefinition;

  LoopContext _loopContext;
  std::vector<std::string> _loopVars;
  size_t _loopCount;

  bool _tracing;
};

NATIVE_WORD_DECL(private, COMPILED_EVAL)  {
  Progn *cwc = dynamic_cast<Progn*>(ctx);
  //  rpn::Runtime::Privates *p = dynamic_cast<rpn::Runtime::Privates*>(ctx);
  rpn::WordDefinition::Result rv = (cwc) ? rpn::WordDefinition::Result::ok : rpn::WordDefinition::Result::eval_error;
  if (rv == rpn::WordDefinition::Result::ok) {
    std::string rest;
    for(auto w : cwc->wordlist()) {
      cwc->_p.eval(rpn, w, rest);
    }
  }
  return rv;
}

NATIVE_WORD_DECL(private, COLON) {
  // (rpn::Runtime &rpn, rpn::WordContext *ctx, std::string &rest)
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  rpn::Runtime::Privates *p = dynamic_cast<rpn::Runtime::Privates*>(ctx);
  p->_state = st_define;
  return rv;
}

NATIVE_WORD_DECL(private, SEMICOLON) {
  // (rpn::Runtime &rpn, rpn::WordContext *ctx, std::string &rest)
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  rpn::Runtime::Privates *p = dynamic_cast<rpn::Runtime::Privates*>(ctx);

  printf("adding '%s' to the dictionary\n", p->_newWord.c_str());

  p->_rtDictionary.emplace(p->_newWord, rpn::WordDefinition {
      rpn::StackSizeValidator::zero, NATIVE_WORD_FN(private, COMPILED_EVAL), new Progn(p->_newDefinition) });
  p->_state = st_run;
  p->_newWord = "";
  p->_newDefinition.clear();
  return rv;
}

NATIVE_WORD_DECL(private, OPAREN) {
  // (rpn::Runtime &rpn, rpn::WordContext *ctx, std::string &rest)
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  rpn::Runtime::Privates *p = dynamic_cast<rpn::Runtime::Privates*>(ctx);
  std::string comment;
  auto cp = nextWord(comment, rest, ')');
  if (cp == std::string::npos) {
    rest = comment; // reset the buffer for future error message
    rv = rpn::WordDefinition::Result::parse_error;
  }
  return rv;
}

NATIVE_WORD_DECL(private, DQUOTE) {
  // (rpn::Runtime &rpn, rpn::WordContext *ctx, std::string &rest)
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  rpn::Runtime::Privates *p = dynamic_cast<rpn::Runtime::Privates*>(ctx);
  std::string literal;
  auto pos = nextWord(literal, rest, '"');
  if (pos != std::string::npos) {
    rpn.stack.push_string(literal);
  } else {
    rv = rpn::WordDefinition::Result::parse_error;
    rest = literal; // reset buffer for error messages and diagnostics
  }
  return rv;
}

NATIVE_WORD_DECL(private, ct_DQUOTE) {
  // (rpn::Runtime &rpn, rpn::WordContext *ctx, std::string &rest)
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  rpn::Runtime::Privates *p = dynamic_cast<rpn::Runtime::Privates*>(ctx);
  std::string literal;
  auto pos = nextWord(literal, rest, '"');
  if (pos != std::string::npos) {
    p->_newDefinition.addWord(".\" " + literal + '"');
  } else {
    rv = rpn::WordDefinition::Result::parse_error;
    rest = literal; // reset buffer for error messages and diagnostics
  }
  return rv;
}

NATIVE_WORD_DECL(private, FOR) {
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  rpn::Runtime::Privates *p = dynamic_cast<rpn::Runtime::Privates*>(ctx);
  p->_loopContext.end = rpn.stack.pop_as_double();
  p->_loopContext.start = rpn.stack.pop_as_double();
  p->_state = st_loop;
  p->_loopCount++;
  return rv;
}

NATIVE_WORD_DECL(private, ct_FOR) {
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  rpn::Runtime::Privates *p = dynamic_cast<rpn::Runtime::Privates*>(ctx);
  std::string var;
  auto pos = nextWord(var, rest);
  if (pos != std::string::npos) {
    p->_loopVars.push_back(var);
    p->_loopCount++;
    p->_newDefinition.addWord("FOR");
    p->_newDefinition.addWord(var);
  } else {
    rv = rpn::WordDefinition::Result::compile_error;
  }
  return rv;
}

rpn::WordDefinition::Result
rpn::Runtime::Privates::for_loop(rpn::Runtime &rpn, double increment) {
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  auto loopVar = _loopVars.back();
  _loopVars.pop_back();
  _loopCount--;

  if (loopVar != _loopContext.loopVar) {
    printf("compile_error %s %s\n", loopVar.c_str(), _loopContext.loopVar.c_str());
    rv = rpn::WordDefinition::Result::compile_error;

  } else {
    _newWord = "";
    Progn wdef = _newDefinition;
    _newDefinition.clear();
    LoopContext lctx = _loopContext;
    _loopContext.loopVar = "";
    _loopContext.start = 0.;
    _loopContext.end = 0.;

    _state = st_run;
    printf("%s: (var %s) (begin %f) (end %f) (incr %f)\n",
	   __func__, loopVar.c_str(), lctx.start, lctx.end, increment);
    printf("  << ");
    for(const auto &w : wdef._wordlist) {
      printf("%s ", w.c_str());
    }
    printf(" >>\n");

    std::string rest;
    for(double cv = lctx.start; rv==rpn::WordDefinition::Result::ok && cv<lctx.end; cv+=increment) {
      for(auto wi = wdef._wordlist.cbegin(); rv==rpn::WordDefinition::Result::ok && wi != wdef._wordlist.cend(); wi++) {
	printf("for_loop-eval(lv %s %f): %s\n", loopVar.c_str(), cv, wi->c_str());
	//	if (*wi == loopVar) {
	//	  rpn.stack.push_double(cv);
	//	} else {
	  rv = eval(rpn, *wi, rest);
	  //	}
      }
    }
  }
  return rv;
}


NATIVE_WORD_DECL(private, ct_NEXT) {
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  rpn::Runtime::Privates *p = dynamic_cast<rpn::Runtime::Privates*>(ctx);

  if (p->_state != st_loop || p->_loopVars.size()==0) {
    // not in a loop or something went wrong
    rv = rpn::WordDefinition::Result::compile_error;

  } else if (p->_loopVars.size()>1) {
    // nested level loop, keep pushing

    p->_loopVars.pop_back();
    p->_loopCount--;
    p->_newDefinition.addWord("NEXT");

  } else {
    // current top-level loop, let's run this one.

    rv = p->for_loop(rpn, 1);
  }

  return rv;
}

NATIVE_WORD_DECL(private, ct_STEP) {
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  rpn::Runtime::Privates *p = dynamic_cast<rpn::Runtime::Privates*>(ctx);
  auto step = rpn.stack.pop_integer();
  p->_state = st_run;
  return rv;
}

void
rpn::Runtime::Privates::add_private_words(rpn::Runtime &rpn) {
  rpn.addDefinition(":", { rpn::StackSizeValidator::zero, NATIVE_WORD_FN(private, COLON), this });
  rpn.addDefinition("(", { rpn::StackSizeValidator::zero, NATIVE_WORD_FN(private, OPAREN), this });
  rpn.addDefinition(".\"", { rpn::StackSizeValidator::zero, NATIVE_WORD_FN(private, DQUOTE), this });
  rpn.addDefinition("FOR", { rpn::StrictTypeValidator::d2_integer_integer, NATIVE_WORD_FN(private, FOR), this });

  _ctDictionary.emplace(";", rpn::WordDefinition { rpn::StackSizeValidator::zero, NATIVE_WORD_FN(private, SEMICOLON), this });
  _ctDictionary.emplace("(", rpn::WordDefinition { rpn::StackSizeValidator::zero, NATIVE_WORD_FN(private, OPAREN), this });
  _ctDictionary.emplace(".\"", rpn::WordDefinition { rpn::StackSizeValidator::zero, NATIVE_WORD_FN(private, ct_DQUOTE), this });
  _ctDictionary.emplace("FOR", rpn::WordDefinition { rpn::StackSizeValidator::zero, NATIVE_WORD_FN(private, ct_FOR), this });
  _ctDictionary.emplace("NEXT", rpn::WordDefinition { rpn::StackSizeValidator::zero, NATIVE_WORD_FN(private, ct_NEXT), this });
  _ctDictionary.emplace("STEP", rpn::WordDefinition { rpn::StrictTypeValidator::d1_double, NATIVE_WORD_FN(private, ct_STEP), this });
}

rpn::WordDefinition::Result
rpn::Runtime::Privates::eval(rpn::Runtime &rpn, const std::string &word, std::string &rest) {
  if (word.size()==0) {
    return rpn::WordDefinition::Result::ok;
  }

  rpn::WordDefinition::Result rv=rpn::WordDefinition::Result::eval_error;
  _status = word + ": ";
  std::string msg;
  if (_state != st_run) {
    try {
      rv = compiletime_eval(rpn,word,rest);

    } catch (const std::bad_cast &bce) {
      msg = "type error compiling";
      if (rest.size()>0) msg += (std::string(" '") + rest + "'");
      rv = rpn::WordDefinition::Result::param_error;
      _state = st_run;
      _newWord = "";
      _newDefinition.clear();

    } catch (const std::runtime_error &rte) {
      rv = rpn::WordDefinition::Result::eval_error;
      msg = "eval error compiling";
      if (rest.size()>0) msg += (std::string(" '") + rest + "'");
      _state = st_run;
      _newWord = "";
      _newDefinition.clear();
    }

  } else {
    try {
      rv = runtime_eval(rpn,word,rest);

    } catch (const std::bad_cast &bce) {
      rv = rpn::WordDefinition::Result::param_error;
      msg = "type error";
      if (rest.size()>0) msg += (std::string(" '") + rest + "'");

    } catch (const std::runtime_error &rte) {
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
  _status += msg;

  if (rv != rpn::WordDefinition::Result::ok) {
    printf("%s: %s\n", __func__, _status.c_str());
  }
    
  return rv;
}

rpn::WordDefinition::Result
rpn::Runtime::Privates::runtime_eval(rpn::Runtime &rpn, const std::string &word, std::string &rest) {
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::dict_error;
  // numbers just push
  if (std::isdigit(word[0])||(word[0]=='-'&&std::isdigit(word[1]))) {
    if (word.find('.') != std::string::npos) {
      double val = strtod(word.c_str(), nullptr);
      rpn.stack.push_double(val);
    } else {
      long val = strtol(word.c_str(), nullptr, 0);
      rpn.stack.push_integer(val);
    }
    rv = rpn::WordDefinition::Result::ok;
  } else {
    if (word_exists(word)) {
      auto we = validate_word(word, rpn.stack);
      if (we != _rtDictionary.end()) {
	rv = we->second.eval(rpn,  we->second.context, rest);
      } else {
	rv = rpn::WordDefinition::Result::param_error;
      }
    } else {
      // default to dictionary error
    }
  }
  return rv;
}

bool
rpn::Runtime::Privates::is_local_variable(const std::string &word) {
  bool rv = (_state==st_loop && (std::find(_loopVars.begin(), _loopVars.end(), word) != _loopVars.end()));
  printf("%s: '%s' ? [",  __func__, word.c_str());
  for(const auto &v : _loopVars) {
    printf("%s ", v.c_str());
  }
  printf("] %d\n", rv);
  return rv;
}

rpn::WordDefinition::Result
rpn::Runtime::Privates::compiletime_eval(rpn::Runtime &rpn, const std::string &word, std::string &rest) {
  rpn::WordDefinition::Result rv=rpn::WordDefinition::Result::dict_error;
  if (_newWord == "") {
    _newWord = word;
    if (_state == st_loop && (_loopCount != _loopVars.size())) {
      _loopContext.loopVar = _newWord;
      _loopVars.push_back(_newWord);
    }
    rv=rpn::WordDefinition::Result::ok;

  } else {
    const auto &cw = _ctDictionary.find(word);
    if (cw != _ctDictionary.end()) {
      // found something in the compiletime dict, evaluate it
      rv = cw->second.eval(rpn, cw->second.context, rest);

    } else if (std::isdigit(word[0])) {
      // numbers just push
      _newDefinition.addWord(word);
      rv=rpn::WordDefinition::Result::ok;

    } else if (is_local_variable(word)) {
      // if we're in a loop compiling mode; and this is the loop variable,
      // push it
      _newDefinition.addWord(word);
      rv=rpn::WordDefinition::Result::ok;
      
    } else {
      // everything else, we check in the runtime dictionary
      const auto &rw = _rtDictionary.find(word);
      if (rw != _rtDictionary.end()) {
	_newDefinition.addWord(word);
	rv=rpn::WordDefinition::Result::ok;

      } else {
	rv = rpn::WordDefinition::Result::dict_error;
	printf("unrecognized word at compile time: '%s'\n", word.c_str());

      }
    }
  }
  return rv;
}

rpn::Runtime::Runtime() {
  m_p = new Privates;
  m_p->add_private_words(*this);
  addInternalWords(m_p);
}

rpn::Runtime::~Runtime() {
  if (m_p) delete m_p;
}

//void
//rpn::Runtime::addDictionary(const std::map<std::string,word_t> &newDict) {
//  m_p->_runtimeDictionary.insert(newDict.begin(), newDict.end());
//}

const std::string &
rpn::Runtime::status() {
  return m_p->_status;
}

bool
rpn::Runtime::addDefinition(const std::string &word, const WordDefinition &def) {
  m_p->_rtDictionary.emplace(word, def);
  return false;
}

std::multimap<std::string,rpn::WordDefinition>::iterator
rpn::Runtime::Privates::validate_word(const std::string &word, rpn::Stack &stack) {
  auto beg = _rtDictionary.lower_bound(word);
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
rpn::Runtime::Privates::word_exists(const std::string &word) {
  auto beg = _rtDictionary.lower_bound(word);
  const auto &end = _rtDictionary.upper_bound(word);
  return (beg != end);
}

bool
rpn::Runtime::validateWord(const std::string &word) {
  return m_p->validate_word(word, this->stack) != m_p->_rtDictionary.end();
}

bool
rpn::Runtime::wordExists(const std::string &word) {
  return m_p->word_exists(word);
}

rpn::WordDefinition::Result
rpn::Runtime::parse(std::string &line) {
  rpn::WordDefinition::Result rv=rpn::WordDefinition::Result::ok;
  for(; rv==rpn::WordDefinition::Result::ok && line.size()>0;) {
    std::string word;
    auto p1 = nextWord(word,line);
    if (m_p->_tracing)
      printf("evaluating: '%s' '%s'\n", word.c_str(), line.c_str());
    rv = m_p->eval(*this, word, line);
    if (m_p->_tracing)
      printf("returns: %d (%s)\n", rv, line.c_str());
  }
  return rv;
}

rpn::WordDefinition::Result
rpn::Runtime::parseFile(const std::string &path) {
  rpn::WordDefinition::Result rv=rpn::WordDefinition::Result::ok;
  std::ifstream ifs(path);

  std::string tmp;
  std::vector<std::string> lines;
  while(getline(ifs, tmp, '\n')) {
    lines.push_back(tmp);
  }

  int lineNo=0;
  for(auto line = lines.begin(); line!=lines.end() && rv==rpn::WordDefinition::Result::ok; line++, lineNo++) {
    rv = parse(*line);
    if (rv != rpn::WordDefinition::Result::ok) {
      printf("parse error at %s:%d\n", path.c_str(), lineNo);
    }
  }

  return rv;
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
rpn::StackSizeValidator::operator()(const std::vector<size_t> &types, rpn::Stack &stack) const {
  bool rv = false;
  if ((_n==(size_t)-1) && types.size()>0 && types[0]==typeid(StInteger).hash_code()) { // negative means to ntos - check top of stack as integer and make sure that the stack is >=
    auto &nn = dynamic_cast<const StInteger&>(stack.peek(1));
    rv = (types.size()-1) >= nn.val();
  } else {
    rv = (types.size() >=_n);
  }
  return rv;
}
const size_t rpn::StrictTypeValidator::v_anytype = typeid(rpn::Stack::Object).hash_code();
const rpn::StrictTypeValidator rpn::StrictTypeValidator::d1_double({typeid(StDouble).hash_code()});
const rpn::StrictTypeValidator rpn::StrictTypeValidator::d1_integer({typeid(StInteger).hash_code()});
const rpn::StrictTypeValidator rpn::StrictTypeValidator::d1_boolean({typeid(StBoolean).hash_code()});
const rpn::StrictTypeValidator rpn::StrictTypeValidator::d2_boolean_boolean({typeid(StBoolean).hash_code(), typeid(StBoolean).hash_code()});
const rpn::StrictTypeValidator rpn::StrictTypeValidator::d2_double_double({typeid(StDouble).hash_code(), typeid(StDouble).hash_code()});
const rpn::StrictTypeValidator rpn::StrictTypeValidator::d2_double_integer({typeid(StDouble).hash_code(), typeid(StInteger).hash_code()});
const rpn::StrictTypeValidator rpn::StrictTypeValidator::d2_integer_double({typeid(StInteger).hash_code(), typeid(StDouble).hash_code()});
const rpn::StrictTypeValidator rpn::StrictTypeValidator::d2_integer_integer({typeid(StInteger).hash_code(), typeid(StInteger).hash_code()});
const rpn::StrictTypeValidator rpn::StrictTypeValidator::d3_any_any_boolean({rpn::StrictTypeValidator::v_anytype, rpn::StrictTypeValidator::v_anytype, typeid(StBoolean).hash_code()} );

const rpn::StackSizeValidator rpn::StackSizeValidator::zero(0);
const rpn::StackSizeValidator rpn::StackSizeValidator::one(1);
const rpn::StackSizeValidator rpn::StackSizeValidator::two(2);
const rpn::StackSizeValidator rpn::StackSizeValidator::three(3);
const rpn::StackSizeValidator rpn::StackSizeValidator::ntos(-1); // n top of stack

/* end of github/elh/rpn-cnc/rpn-runtime.cpp */
