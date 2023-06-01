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

struct CompiledWC : public rpn::WordContext {
public:
  CompiledWC(rpn::Runtime::Privates &p) : _p(p) {};
  void addWord(const std::string &word) { _wordlist.push_back(word); };
  CompiledWC *deep_copy() const {
    CompiledWC *rv = new CompiledWC(_p);
    rv->_wordlist = _wordlist;
    return rv;
  }
  const std::vector<std::string> &wordlist() const { return _wordlist; };
  void clear() { _wordlist.clear(); };

  rpn::Runtime::Privates &_p;
  std::vector<std::string> _wordlist;
};

struct rpn::Runtime::Privates : public rpn::WordContext {
  Privates() : _newDefinition(*this), _tracing(false) {};
  ~Privates() {};

  rpn::WordDefinition::Result eval(rpn::Runtime &rpn, const std::string &word, std::string &rest);
  rpn::WordDefinition::Result runtime_eval(rpn::Runtime &rpn, const std::string &word, std::string &rest);
  rpn::WordDefinition::Result compiletime_eval(rpn::Runtime &rpn, const std::string &word, std::string &rest);

  bool validate_stack_for_word(const std::vector<size_t> &stack, const rpn::WordDefinition &wd);

  // add words that require acces to the Privates struct.
  void addPrivateWords(rpn::Runtime &rpn);

  /*
   */
  std::multimap<std::string,WordDefinition> _rtDictionary;
  std::map<std::string,WordDefinition> _ctDictionary;

  std::string _status;

  bool _isCompiling;
  std::string _newWord;
  CompiledWC _newDefinition;

  bool _tracing;
};

NATIVE_WORD_DECL(private, COMPILED_EVAL)  {
  CompiledWC *cwc = dynamic_cast<CompiledWC*>(ctx);
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
  p->_isCompiling = true;
  return rv;
}

NATIVE_WORD_DECL(private, SEMICOLON) {
  // (rpn::Runtime &rpn, rpn::WordContext *ctx, std::string &rest)
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  rpn::Runtime::Privates *p = dynamic_cast<rpn::Runtime::Privates*>(ctx);

  printf("adding '%s' to the dictionary\n", p->_newWord.c_str());

  p->_rtDictionary.emplace(p->_newWord, rpn::WordDefinition {
      rpn::StackSizeValidator::zero, NATIVE_WORD_FN(private, COMPILED_EVAL), p->_newDefinition.deep_copy() });
  p->_isCompiling = false;
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

void
rpn::Runtime::Privates::addPrivateWords(rpn::Runtime &rpn) {
  rpn.addDefinition(":", { rpn::StackSizeValidator::zero, NATIVE_WORD_FN(private, COLON), this });
  rpn.addDefinition("(", { rpn::StackSizeValidator::zero, NATIVE_WORD_FN(private, OPAREN), this });
  rpn.addDefinition(".\"", { rpn::StackSizeValidator::zero, NATIVE_WORD_FN(private, DQUOTE), this });
  _ctDictionary.emplace(";", rpn::WordDefinition { rpn::StackSizeValidator::zero, NATIVE_WORD_FN(private, SEMICOLON), this });
  _ctDictionary.emplace("(", rpn::WordDefinition { rpn::StackSizeValidator::zero, NATIVE_WORD_FN(private, OPAREN), this });
  //  _ctDictionary.emplace("\"", rpn::WordDefinition { {}, NATIVE_WORD_FN(private, DQUOTE), this });
}

rpn::WordDefinition::Result
rpn::Runtime::Privates::eval(rpn::Runtime &rpn, const std::string &word, std::string &rest) {
  if (word.size()==0) {
    return rpn::WordDefinition::Result::ok;
  }

  rpn::WordDefinition::Result rv=rpn::WordDefinition::Result::eval_error;
  _status = word + ": ";
  std::string msg;
  if (_isCompiling) {
    try {
      rv = compiletime_eval(rpn,word,rest);

    } catch (const std::bad_cast &bce) {
      msg = "type error compiling";
      if (rest.size()>0) msg += (std::string(" '") + rest + "'");
      rv = rpn::WordDefinition::Result::param_error;
      _isCompiling = false;
      _newWord = "";
      _newDefinition.clear();

    } catch (const std::runtime_error &rte) {
      rv = rpn::WordDefinition::Result::eval_error;
      msg = "eval error compiling";
      if (rest.size()>0) msg += (std::string(" '") + rest + "'");
      _isCompiling = false;
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
  if (std::isdigit(word[0])) {
    if (word.find('.') != std::string::npos) {
      double val = strtod(word.c_str(), nullptr);
      rpn.stack.push_double(val);
    } else {
      long val = strtol(word.c_str(), nullptr, 0);
      rpn.stack.push_integer(val);
    }
    rv = rpn::WordDefinition::Result::ok;
  } else {
    auto beg = _rtDictionary.lower_bound(word);
    const auto &end = _rtDictionary.upper_bound(word);
    if (beg != end) {
      auto stack_types = rpn.stack.types();
      for(auto we=beg; we!=end; we++) {
	if (we->second.validator(stack_types, rpn.stack)) {
          rv = we->second.eval(rpn,  we->second.context, rest);
	  break;
	} else {
	  rv = rpn::WordDefinition::Result::param_error;
	}
      }
    }
  }
  return rv;
}

rpn::WordDefinition::Result
rpn::Runtime::Privates::compiletime_eval(rpn::Runtime &rpn, const std::string &word, std::string &rest) {
  rpn::WordDefinition::Result rv=rpn::WordDefinition::Result::dict_error;
  if (_newWord == "") {
    _newWord = word;
    rv=rpn::WordDefinition::Result::ok;

  } else {
    const auto &cw = _ctDictionary.find(word);
    if (cw != _ctDictionary.end()) {
      // found something in the compiletime dict, evaluate it
      rv = cw->second.eval(rpn, cw->second.context, rest);

    } else {
      if (std::isdigit(word[0])) {
	// numbers just push
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
  }
  return rv;
}

rpn::Runtime::Runtime() {
  m_p = new Privates;
  m_p->addPrivateWords(*this);
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
