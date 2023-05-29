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

class CompiledWC : public rpn::WordContext {
public:
  CompiledWC() {};
  void addWord(const std::string &word) { _wordlist.push_back(word); };
  CompiledWC *deep_copy() const {
    CompiledWC *rv = new CompiledWC;
    rv->_wordlist = _wordlist;
    return rv;
  }
  const std::vector<std::string> &wordlist() const { return _wordlist; };
  void clear() { _wordlist.clear(); };
protected:
  std::vector<std::string> _wordlist;
};

struct rpn::Runtime::Privates : public rpn::WordContext {
  Privates() {};
  ~Privates() {};

  rpn::WordDefinition::Result eval(rpn::Runtime &rpn, const std::string &word, std::string &rest);
  rpn::WordDefinition::Result runtime_eval(rpn::Runtime &rpn, const std::string &word, std::string &rest);
  rpn::WordDefinition::Result compiletime_eval(rpn::Runtime &rpn, const std::string &word, std::string &rest);

  bool validate_stack_for_word(const rpn::WordDefinition &wd);

  // add words that require acces to the Privates struct.
  void addPrivateWords(rpn::Runtime &rpn);

  /*
   */
  std::multimap<std::string,WordDefinition> _rtDictionary;
  std::map<std::string,WordDefinition> _ctDictionary;

  bool _isCompiling;

  std::string _newWord;
  CompiledWC _newDefinition;
};

bool
rpn::Runtime::Privates::validate_stack_for_word(const rpn::WordDefinition &wd) {
  return true;
}

#define NATIVE_WORD_FN(name) private_def_##name

#define NATIVE_WORD_IMPL(name)						\
  static rpn::WordDefinition::Result NATIVE_WORD_FN(name)(rpn::Runtime &rpn, rpn::WordContext *ctx, std::string &rest)

NATIVE_WORD_IMPL(COMPILED_EVAL)  {
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  CompiledWC *cwc = dynamic_cast<CompiledWC*>(ctx);
  return rv;
}

NATIVE_WORD_IMPL(COLON) {
  // (rpn::Runtime &rpn, rpn::WordContext *ctx, std::string &rest)
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  rpn::Runtime::Privates *p = dynamic_cast<rpn::Runtime::Privates*>(ctx);
  p->_isCompiling = true;
  return rv;
}

NATIVE_WORD_IMPL(SEMICOLON) {
  // (rpn::Runtime &rpn, rpn::WordContext *ctx, std::string &rest)
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  rpn::Runtime::Privates *p = dynamic_cast<rpn::Runtime::Privates*>(ctx);
  
  p->_rtDictionary.emplace(p->_newWord, rpn::WordDefinition {
      {}, NATIVE_WORD_FN(COMPILED_EVAL), p->_newDefinition.deep_copy() });
  p->_isCompiling = false;
  p->_newWord = "";
  p->_newDefinition.clear();
  return rv;
}

NATIVE_WORD_IMPL(OPAREN) {
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

NATIVE_WORD_IMPL(DQUOTE) {
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
  rpn.addDefinition(":", { {}, NATIVE_WORD_FN(COLON), this });
  rpn.addDefinition("(", { {}, NATIVE_WORD_FN(OPAREN), this });
  rpn.addDefinition(".\"", { {}, NATIVE_WORD_FN(DQUOTE), this });
  _ctDictionary.emplace(";", rpn::WordDefinition { {}, NATIVE_WORD_FN(SEMICOLON), this });
  _ctDictionary.emplace("(", rpn::WordDefinition { {}, NATIVE_WORD_FN(OPAREN), this });
  //  _ctDictionary.emplace("\"", rpn::WordDefinition { {}, NATIVE_WORD_FN(DQUOTE), this });
}

rpn::WordDefinition::Result
rpn::Runtime::Privates::eval(rpn::Runtime &rpn, const std::string &word, std::string &rest) {
  rpn::WordDefinition::Result rv=rpn::WordDefinition::Result::eval_error;
  if (word.size()>0) {
    if (_isCompiling) {
      rv = compiletime_eval(rpn,word,rest);
    } else {
      rv = runtime_eval(rpn,word,rest);
    }
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
    for(auto we=beg; we!=end; we++) {
      if (validate_stack_for_word(we->second)) {
	rv = we->second.eval(rpn,  we->second.context, rest);
      } else {
	rv = rpn::WordDefinition::Result::param_error;
      }
    }
  }

  switch (rv) {
  case rpn::WordDefinition::Result::ok: {
    // crickes
  }
    break;

  case rpn::WordDefinition::Result::parse_error: {
    printf("error parsing word '%s' with '%s'\n", word.c_str(), rest.c_str());
    rest = ""; // discard the rest of the line;
  }
    break;

  case rpn::WordDefinition::Result::dict_error: {
    printf("'%s' not found in dict\n", word.c_str());
  }
    break;

  case rpn::WordDefinition::Result::param_error: {
    printf("parameters not right for '%s'\n", word.c_str());
    rpn.stack.print();
  }
    break;

  case rpn::WordDefinition::Result::eval_error: {
    printf("error '%s' evaluating\n", word.c_str());
  }
    break;
  }

  return rv;
}

rpn::WordDefinition::Result
rpn::Runtime::Privates::compiletime_eval(rpn::Runtime &rpn, const std::string &word, std::string &rest) {
  rpn::WordDefinition::Result rv=rpn::WordDefinition::Result::dict_error;
  if (_newWord == "") {
    _newWord = word;
  } else {
    const auto &cw = _ctDictionary.find(word);
    if (cw != _ctDictionary.end()) {
      // found something in the compiletime dict, evaluate it
      rv = cw->second.eval(rpn, cw->second.context, rest);

    } else {
      if (std::isdigit(word[0])) {
	// numbers just push
	_newDefinition.addWord(word);

      } else {
	// everything else, we check in the runtime dictionary
	const auto &rw = _rtDictionary.find(word);
	if (rw != _rtDictionary.end()) {
	  _newDefinition.addWord(word);
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

bool
rpn::Runtime::addDefinition(const std::string &word, const WordDefinition &def) {
  m_p->_rtDictionary.emplace(word, def);
  return false;
}

bool
rpn::Runtime::parse(std::string &line) {
  bool rv=true;
  while (line.size()>0) {
    std::string word;
    auto p1 = nextWord(word,line);
    auto s = m_p->eval(*this,word, line);
  }
  return rv;
}

bool
rpn::Runtime::parseFile(const std::string &path) {
  bool rv=false;
  std::ifstream ifs(path);

  std::string tmp;
  std::vector<std::string> lines;
  while(getline(ifs, tmp, '\n')) {
    lines.push_back(tmp);
  }

  rv=lines.size()>0;
  int lineNo=0;
  for(auto line = lines.begin(); line!=lines.end() && rv==true; line++, lineNo++) {
    rv = parse(*line);
    if (rv == false) {
      printf("parse error at %s:%d\n", path.c_str(), lineNo);
    }
  }

  return rv;
}

/* end of github/elh/rpn-cnc/rpn-runtime.cpp */
