/***************************************************
 * file: github/elh/rpn-cnc/rpn-cnc.cpp
 *
 * @file    rpn-cnc.cpp
 * @author  Eric L. Hernes
 * @version V1.0
 * @born_on   Thursday, May 18, 2023
 * @copyright (C) Copyright Eric L. Hernes 2023
 * @copyright (C) Copyright Q, Inc. 2023
 *
 * @brief   An Eric L. Hernes Signature Series C++ module
 *
 */

#include <vector>
#include <map>
#include <fstream>
#include <functional>
#include <deque>
#include <variant>

#include "rpn-controller.h"

std::string::size_type
RpnController::nextWord(std::string &word, std::string &buffer, char delim) {
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

const std::string
to_string(const datatype_t &dt) {
  std::string rv;
  switch(dt) {
  case st_double:
    rv = "double";
    break;
  case st_integer:
    rv = "integer";
    break;
  case st_string:
    rv = "string";
    break;
  case st_vec3:
    rv = "vec3";
    break;
    //  case st_stacktype:
    //    rv = "stacktype";
    //    break;
  case st_number:
    rv = "number";
    break;
  case st_any:
    rv = "any";
    break;
  }
  return rv;
}

const std::string
to_string(const stacktype_t &e) {
  std::string rv;
  char tmp[64];
  snprintf(tmp, sizeof(tmp), "{%s}: ", to_string(datatype_t(e.index())).c_str());
  rv += tmp;
  switch(e.index()) {
  case st_double:
    snprintf(tmp, sizeof(tmp), "%f", std::get<st_double>(e));
    rv += tmp;
    break;
  case st_integer:
    snprintf(tmp, sizeof(tmp), "%ld", std::get<st_integer>(e));
    rv += tmp;
    break;
  case st_string:
    rv += std::get<st_string>(e);
    break;
  case st_vec3: {
    const Vec3 &v = std::get<st_vec3>(e);
    snprintf(tmp, sizeof(tmp), "{x:%f, y:%f z:%f}", v.x, v.y, v.z);
    rv += tmp;
  }
    break;
  default:
    break;
  }

  return rv;
}

const std::string
to_string(const std::pair<std::string,word_t> &wp) {
  std::string rv;
  rv += "((word " + wp.first + ") " + wp.second.description + " " + std::to_string(wp.second.params.size()) + "\n";
  for(const auto &pp : wp.second.params) {
    rv += "  "+std::to_string(pp.size())+"(";
    for(const auto &p : pp) {
      rv += "{ " + p.name + " " + to_string(p.type) + "} ";
    }
    rv += ")\n";
  }
  rv += ")\n";
  return rv;
}

const std::string
to_string(const Vec3 &v) {
  char tmp[64];
  snprintf(tmp, sizeof(tmp), "{x:%f, y:%f z:%f}", v.x, v.y, v.z);
  return tmp;
}

struct RpnController::Privates : public WordContext {
  Privates();
  ~Privates() {};

  std::string::size_type eval(RpnController &rpn, const std::string &word, std::string &rest);
  std::string::size_type runtime_eval(RpnController &rpn, const std::string &word, std::string &rest);
  std::string::size_type compiletime_eval(RpnController &rpn, const std::string &word, std::string &rest);

  std::vector<size_t> top_of_stack_types(size_t n);

  bool validate_stack_for_word(const std::pair<std::string,word_t> &de);

  /*
   *  back is considered top of stack
   */
  void stack_push(const stacktype_t &val) {
    _stack.push_back(val);
  }

  stacktype_t stack_pop() {
    stacktype_t rv = _stack.back();
    _stack.pop_back();
    return rv;
  }

  double stack_pop_as_double() {
    double d = std::nan("");
    auto s0 = stack_pop();
    switch(s0.index()) {
    case st_double:
      d = std::get<st_double>(s0);
      break;
    case st_integer:
      d = std::get<st_integer>(s0);
      break;

    case st_string:
    case st_vec3:
    default:
      // all return nan
      break;
    }
    return d;
  }

  long stack_pop_as_integer() {
    long val;
    auto s0 = stack_pop();
    switch(s0.index()) {
    case st_double:
     val = std::get<st_double>(s0);
      break;
    case st_integer:
      val = std::get<st_integer>(s0);
      break;

    case st_string:
    case st_vec3:
    default:
      // what do we return?
      break;
    }
    return val;
  }

  std::string stack_pop_as_string() {
    std::string val;
    auto s0 = stack_pop();
    switch(s0.index()) {
    case st_double:
      val = std::to_string(std::get<st_double>(s0));
      break;
    case st_integer:
      val = std::to_string(std::get<st_integer>(s0));
      break;

    case st_string:
      val = std::get<st_string>(s0);
      break;
    case st_vec3:
      val = to_string(std::get<st_vec3>(s0));
      break;
    default:
      // what do we return?
      break;
    }
    return val;
  }

  Vec3 stack_pop_as_vec3() {
    Vec3 val;
    auto s0 = stack_pop();
    switch(s0.index()) {
    case st_double:
    case st_integer:
    case st_string:
    case st_vec3:
      val = std::get<st_vec3>(s0);
      break;
    default:
      // what do we return?
      break;
    }
    return val;
  }

  void print_stack() {
    auto i=_stack.size();
    printf("--%20lu--\n", i);
    for(auto const &e: _stack) {
      printf("[%02lu] %s\n", i, to_string(e).c_str());
      i--;
    }
    printf("------------------------\n");
  }

  /*
   */
  std::map<std::string,word_t> _runtimeDictionary;
  std::map<std::string,word_t> _compiletimeDictionary;

  bool _isCompiling;
  std::string _newWord;
  CompiledWordContext _newDefinition;

  std::deque<stacktype_t> _stack;

  NATIVE_WORD_IMPL(COMPILED_EVAL);
};

std::vector<size_t>
RpnController::Privates::top_of_stack_types(size_t n) {
  //  printf("%s(%lu) (depth %lu)\n", __func__, n, _stack.size());
  //  print_stack();
  std::vector<size_t> rv;
  if (_stack.size() < n) {
    printf("size too small to validate (%lu < %lu)\n", _stack.size(), n);
  } else {
    auto ri = _stack.rbegin()+(n-1);
    for(size_t i = 0; i<n; i++, ri--) {
      //      printf("---  %lu : %s : %s (%lu)\n", (n-i), to_string(*ri).c_str(), to_string(datatype_t(ri->index())).c_str(), ri->index());
      rv.push_back(ri->index());
    }
  }
  return rv;
}

static bool
operator==(const datatype_t &dt, const size_t &index) {
  bool rv = false;
  switch(dt) {
    // match primitives
  case st_double:
  case st_integer:
  case st_string:
  case st_vec3:
    rv = (((int)dt) == ((int)index));
    break;

  case st_number:
    rv = ((((int)st_double) == ((int)index)) ||
	  (((int)st_integer) == ((int)index)));
    break;
  case st_any:
    rv = true;
    break;
  }
  return rv;
}

bool
RpnController::Privates::validate_stack_for_word(const std::pair<std::string,word_t> &de) {
  bool rv = (de.second.params.size()==0 ||
	     (de.second.params.size()==1 && de.second.params[0].size()==0));

  for(auto p = de.second.params.begin();
      p!=de.second.params.end() && rv==false;
      p++) {
    size_t n = p->size();

    const auto tv = top_of_stack_types(n);
    if (tv.size()!=0) {
      bool pred = true;
      for(unsigned i=0; i<n && pred==true; i++) {
	pred = ((*p)[i].type == tv[i]);
	//	printf("%lu: (%s ? %s) %s\n", (p-de.second.params.begin()), to_string((*p)[i].type).c_str(), to_string(datatype_t(tv[i])).c_str(), pred?"ok":"not-ok");
      }
      rv = pred;
    }
  }
  return rv;
}

std::string::size_type
RpnController::Privates::compiletime_eval(RpnController &rpn, const std::string &word, std::string &rest) {
  std::string::size_type rv = 0;

  if (_newWord == "") {
    _newWord = word;
  } else {
    const auto &cw = _compiletimeDictionary.find(word);
    if (cw != _compiletimeDictionary.end()) {
      // found something in the compiletime dict, evaluate it
      rv = cw->second.eval(rpn, cw->second.context, rest);

    } else {
      if (std::isdigit(word[0])) {
	// numbers just push
	_newDefinition.addWord(word);

      } else {
	// everything else, we check in the runtime dictionary
	const auto &rw = _runtimeDictionary.find(word);
	if (rw != _runtimeDictionary.end()) {
	  _newDefinition.addWord(word);
	} else {
	  rv = std::string::npos;
	  printf("unrecognized word at compile time: '%s'\n", word.c_str());
	}
      }
    }
  }
  return rv;
}

std::string::size_type
RpnController::Privates::runtime_eval(RpnController &rpn, const std::string &word, std::string &rest) {
  std::string::size_type rv = 0;
  // numbers just push
  if (std::isdigit(word[0])) {
    if (word.find('.') != std::string::npos) {
      double val = strtod(word.c_str(), nullptr);
      stack_push(val);
    } else {
      long val = strtol(word.c_str(), nullptr, 0);
      stack_push(val);
    }
  } else {
    const auto &we = _runtimeDictionary.find(word);
    if (we != _runtimeDictionary.end()) {
      if (validate_stack_for_word(*we)) {
	rv = we->second.eval(rpn,  we->second.context, rest);
      } else {
	printf("parameters not right for '%s'\n", word.c_str());
	print_stack();
	printf("%s", to_string(*we).c_str());
      }
    } else {
      printf("not found '%s' in dict\n", word.c_str());
    }
  }
  return rv;
}


std::string::size_type
RpnController::Privates::eval(RpnController &rpn, const std::string &word, std::string &rest) {
  std::string::size_type rv = 0; // std::string::npos;
  if (word.size()>0) {

    if (_isCompiling) {
      rv = compiletime_eval(rpn,word,rest);
    } else {
      rv = runtime_eval(rpn,word,rest);
    }
  }
  return rv;
}

RpnController::RpnController() {
  m_p = new Privates;
}

void
RpnController::addDictionary(const std::map<std::string,word_t> &newDict) {
  m_p->_runtimeDictionary.insert(newDict.begin(), newDict.end());
}

RpnController::~RpnController() {
  if (m_p) delete m_p;
}

bool
RpnController::parse(std::string &line) {
  bool rv=true;
  while (line.size()>0) {
    std::string word;
    auto p1 = nextWord(word,line);
    auto s = m_p->eval(*this,word, line);
  }

  return rv;
}

bool
RpnController::loadFile(const std::string &path) {
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

void RpnController::stack_push(const stacktype_t &v) {   m_p->stack_push(v); }
stacktype_t RpnController::stack_pop() { return m_p->stack_pop(); }
double RpnController::stack_pop_as_double() { return m_p->stack_pop_as_double(); }
long RpnController::stack_pop_as_integer() { return m_p->stack_pop_as_integer(); }
std::string RpnController::stack_pop_as_string() { return m_p->stack_pop_as_string(); }
Vec3 RpnController::stack_pop_as_vec3() { return m_p->stack_pop_as_vec3(); }

static double deg_to_rad(const double &deg) {
  return deg * (M_PI / 180.);
}
static double rad_to_deg(const double &rad) {
  return rad * 180. / M_PI;
}
#define type2_pred_v(v1,v2) ((v1.index()<<8)|(v2.index()<<0))
#define type2_pred(v1,v2) ((v1<<8)|(v2<<0))

static double nan_add(double a, double b) {
  double rv = std::nan("");
  switch((std::isnan(a)<<1)|(std::isnan(b))) {
  case 0:
    rv = a + b;
    break;
  case 1:
    rv = a;
    break;
  case 2:
    rv = b;
    break;
  case 3:
    // rv is already nan
    break;
  }
  return rv;
}

static double nan_sub(double a, double b) {
  double rv = std::nan("");
  switch((std::isnan(a)<<1)|(std::isnan(b))) {
  case 0:
    rv = a - b;
    break;
  case 1:
    rv = -b;
    break;
  case 2:
    rv = a;
    break;
  case 3:
    // rv is already nan
    break;
  }
  return rv;
}
static Vec3 operator+(const Vec3 &a, const Vec3 &b) {
  return Vec3(nan_add(a.x,b.x),
	      nan_add(a.y,b.y),
	      nan_add(a.z,b.z));
}
static Vec3 operator-(const Vec3 &a, const Vec3 &b) {
  return Vec3(nan_sub(a.x,b.x),
	      nan_sub(a.y,b.y),
	      nan_sub(a.z,b.z));
}

#if 0
std::map<std::string,std::string> skDescription;
struct nativedef_t {
  std::vector<param_t> params;
  std::function<std::string::size_type(RpnController &rpn, std::string &rest)> eval;
};
std::multimap<std::string,nativedef_t> skDictionary;

template<a,b,c,d>
skDescription.emplace(a,b);
static std::string::size_type d(RpnController &rpn, std::string &rest);
skDictionary.emplace(a,{c, d});
std::string::size_type d(RpnController &rpn, std::string &rest)
#endif
  
NATIVE_WORD_IMPL(ABS) {
  std::string::size_type rv = 0;
  double v = rpn.stack_pop_as_double();
  rpn.stack_push(abs(v));
  return rv;
}

NATIVE_WORD_IMPL(COS) {
  std::string::size_type rv = 0;
  double rad = deg_to_rad(rpn.stack_pop_as_double());
  rpn.stack_push(cos(rad));
  return rv;
}

NATIVE_WORD_IMPL(ACOS) {
  std::string::size_type rv = 0;
  double v = acos(rpn.stack_pop_as_double());
  rpn.stack_push(rad_to_deg(v));
  return rv;
}

NATIVE_WORD_IMPL(SIN) {
  std::string::size_type rv = 0;
  double rad = deg_to_rad(rpn.stack_pop_as_double());
  rpn.stack_push(sin(rad));
  return rv;
}

NATIVE_WORD_IMPL(ASIN) {
  std::string::size_type rv = 0;
  double v = asin(rpn.stack_pop_as_double());
  rpn.stack_push(rad_to_deg(v));
  return rv;
}

NATIVE_WORD_IMPL(TAN) {
  std::string::size_type rv = 0;
  double rad = deg_to_rad(rpn.stack_pop_as_double());
  rpn.stack_push(tan(rad));
  return rv;
}

NATIVE_WORD_IMPL(ATAN) {
  std::string::size_type rv = 0;
  double v = atan(rpn.stack_pop_as_double());
  rpn.stack_push(rad_to_deg(v));
  return rv;
}

NATIVE_WORD_IMPL(ATAN2) {
  std::string::size_type rv = 0;
  double x = rpn.stack_pop_as_double();
  double y = rpn.stack_pop_as_double();
  rpn.stack_push(rad_to_deg(atan2(y,x)));
  return rv;
}

NATIVE_WORD_IMPL(NEG) {
  std::string::size_type rv = 0;
  auto x = rpn.stack_pop();
  switch(x.index()) {
  case st_double:
    rpn.stack_push(std::get<st_double>(x)*-1.);
    break;
  case st_integer:
    rpn.stack_push(std::get<st_integer>(x)*-1);
    break;
  }
  return rv;
}

NATIVE_WORD_IMPL(SQRT) {
  std::string::size_type rv = 0;
  double x = rpn.stack_pop_as_double();
  rpn.stack_push(sqrt(x));
  return rv;
}

NATIVE_WORD_IMPL(SQ) {
  std::string::size_type rv = 0;
  double x = rpn.stack_pop_as_double();
  rpn.stack_push(x*x);
  return rv;
}

NATIVE_WORD_IMPL(POW) {
  std::string::size_type rv = 0;
  double x = rpn.stack_pop_as_double();
  double y = rpn.stack_pop_as_double();
  rpn.stack_push(pow(y,x));
  return rv;
}

NATIVE_WORD_IMPL(INV) {
  std::string::size_type rv = 0;
  double x = rpn.stack_pop_as_double();
  if (x!=0.) {
    rpn.stack_push(1.0/x);
  } else {
    rpn.stack_push(x);
  }
  return rv;
}

NATIVE_WORD_IMPL(PI) {
  std::string::size_type rv = 0;
  rpn.stack_push(M_PI);
  return rv;
}

NATIVE_WORD_IMPL(ADD) {
  std::string::size_type rv = 0;
  auto sx = rpn.stack_pop();
  auto sy = rpn.stack_pop();
  unsigned vv = type2_pred_v(sx,sy);
  switch(vv) {
  case type2_pred(st_integer,st_integer):
    rpn.stack_push(std::get<st_integer>(sx)+std::get<st_integer>(sy));
    break;
  case type2_pred(st_double,st_integer):
    rpn.stack_push(std::get<st_double>(sx)+std::get<st_integer>(sy));
    break;
  case type2_pred(st_integer,st_double):
    rpn.stack_push(std::get<st_integer>(sx)+std::get<st_double>(sy));
    break;
  case type2_pred(st_double,st_double):
    rpn.stack_push(std::get<st_double>(sx)+std::get<st_double>(sy));
    break;
  case type2_pred(st_vec3,st_vec3):
    rpn.stack_push(std::get<st_vec3>(sx)+std::get<st_vec3>(sy));
    break;
  }
  return rv;
}

NATIVE_WORD_IMPL(SUB) {
  std::string::size_type rv = 0;
  auto sx = rpn.stack_pop();
  auto sy = rpn.stack_pop();
  unsigned vv = type2_pred_v(sy,sx);
  switch(vv) {
  case type2_pred(st_integer,st_integer):
    rpn.stack_push(std::get<st_integer>(sy)-std::get<st_integer>(sx));
    break;
  case type2_pred(st_integer,st_double):
    rpn.stack_push(std::get<st_integer>(sy)-std::get<st_double>(sx));
    break;
  case type2_pred(st_double,st_integer):
    rpn.stack_push(std::get<st_double>(sy)-std::get<st_integer>(sx));
    break;
  case type2_pred(st_double,st_double):
    rpn.stack_push(std::get<st_double>(sy)-std::get<st_double>(sx));
    break;
  case type2_pred(st_vec3,st_vec3):
    rpn.stack_push(std::get<st_vec3>(sy)+std::get<st_vec3>(sx));
    break;
  }
  return rv;
}

NATIVE_WORD_IMPL(MULT) {
  std::string::size_type rv = 0;
    auto sx = rpn.stack_pop();
    auto sy = rpn.stack_pop();
    unsigned vv = type2_pred_v(sx,sy);
    switch(vv) {
    case type2_pred(st_integer,st_integer):
      rpn.stack_push(std::get<st_integer>(sx)*std::get<st_integer>(sy));
      break;
    case type2_pred(st_double,st_integer):
      rpn.stack_push(std::get<st_double>(sx)*std::get<st_integer>(sy));
      break;
    case type2_pred(st_integer,st_double):
      rpn.stack_push(std::get<st_integer>(sx)*std::get<st_double>(sy));
      break;
    case type2_pred(st_double,st_double):
      rpn.stack_push(std::get<st_double>(sx)*std::get<st_double>(sy));
      break;
    }
    return rv;
}

NATIVE_WORD_IMPL(DIV) {
  std::string::size_type rv = 0;
  auto sx = rpn.stack_pop();
  auto sy = rpn.stack_pop();
  unsigned vv = type2_pred_v(sx,sy);
  switch(vv) {
  case type2_pred(st_integer,st_integer):
    rpn.stack_push(std::get<st_integer>(sx)/std::get<st_integer>(sy));
    break;
  case type2_pred(st_double,st_integer):
    rpn.stack_push(std::get<st_double>(sx)/std::get<st_integer>(sy));
    break;
  case type2_pred(st_integer,st_double):
    rpn.stack_push(std::get<st_integer>(sx)/std::get<st_double>(sy));
    break;
  case type2_pred(st_double,st_double):
    rpn.stack_push(std::get<st_double>(sx)/std::get<st_double>(sy));
    break;
  }
  return rv;
}

#if 0
NATIVE_WORD_IMPL(MIN,st_integer,st_integer) {
}
NATIVE_WORD_IMPL(MIN,st_integer,st_double) {
}
NATIVE_WORD_IMPL(MIN,st_double,st_integer) {
}
NATIVE_WORD_IMPL(MIN,st_double,st_double) {
}

add_word("min", "Leaves the minimum of the top two stack items.  ( b a -- min(a,b) )", 2);
add_word_implementation(st_double,st_double,function,context);
#endif

NATIVE_WORD_IMPL(MIN) {
  auto s1 = rpn.stack_pop();
  auto s2 = rpn.stack_pop();
  unsigned vv = type2_pred_v(s1,s2);
  switch(vv) {
  case type2_pred(st_integer,st_integer):
    rpn.stack_push(std::min(std::get<st_integer>(s1),std::get<st_integer>(s2)));
    break;
  case type2_pred(st_double,st_integer):
    rpn.stack_push(std::min(std::get<st_double>(s1),double(std::get<st_integer>(s2))));
    break;
  case type2_pred(st_integer,st_double):
    rpn.stack_push(std::min(double(std::get<st_integer>(s1)),std::get<st_double>(s2)));
    break;
  case type2_pred(st_double,st_double):
    rpn.stack_push(std::min(std::get<st_double>(s1),std::get<st_double>(s2)));
    break;
  }
  return 0;
}

NATIVE_WORD_IMPL(MAX) {
  auto s1 = rpn.stack_pop();
  auto s2 = rpn.stack_pop();
  unsigned vv = type2_pred_v(s1,s2);
  switch(vv) {
  case type2_pred(st_integer,st_integer):
    rpn.stack_push(std::max(std::get<st_integer>(s1),std::get<st_integer>(s2)));
    break;
  case type2_pred(st_double,st_integer):
    rpn.stack_push(std::max(std::get<st_double>(s1),double(std::get<st_integer>(s2))));
    break;
  case type2_pred(st_integer,st_double):
    rpn.stack_push(std::max(double(std::get<st_integer>(s1)),std::get<st_double>(s2)));
    break;
  case type2_pred(st_double,st_double):
    rpn.stack_push(std::max(std::get<st_double>(s1),std::get<st_double>(s2)));
    break;
  }
  return 0;
}


NATIVE_WORD_IMPL(EVAL) {
  std::string::size_type rv = 0;
  printf("%s: not implemented\n", __func__);
  return rv;
}


NATIVE_WORD_IMPL(_S) {
  RpnController::Privates *p = dynamic_cast<RpnController::Privates*>(ctx);
  p->print_stack();
  return 0;
}

NATIVE_WORD_IMPL(_D) {
  RpnController::Privates *p = dynamic_cast<RpnController::Privates*>(ctx);
  for(auto const &de : p->_runtimeDictionary) {
    std::string s = to_string(de);
    printf("%s\n\n", s.c_str());
  }
  return 0;
}

NATIVE_WORD_IMPL(_Q) {
  std::string::size_type rv = 0;
  std::string literal;
  auto p = RpnController::nextWord(literal, rest, '"');
  if (p != std::string::npos) {
    rv = p;
    rpn.stack_push(literal);
  } else {
    printf("parse error in string literal: terminating '\"' not found [%s]\n", rest.c_str());
  }
  return rv;
}

NATIVE_WORD_IMPL(OPAREN) {
  std::string::size_type rv = 0;
  std::string comment;
  rv = RpnController::nextWord(comment, rest, ')');
  if (rv != std::string::npos) {
  } else {
    printf("parse error in comment: terminating ')' not found [%s]\n", rest.c_str());
  }
  return rv;
}

NATIVE_WORD_IMPL(COLON) {
  RpnController::Privates *p = dynamic_cast<RpnController::Privates*>(ctx);
  std::string::size_type rv = 0;
  return rv;
}

NATIVE_WORD_IMPL(CONCAT) {
  std::string::size_type rv = 0;
  std::string s1 = rpn.stack_pop_as_string();
  std::string s2 = rpn.stack_pop_as_string();
  rpn.stack_push(s2 + s1);
  return rv;
}

NATIVE_WORD_IMPL(DUP) {
  RpnController::Privates *p = dynamic_cast<RpnController::Privates*>(ctx);
  std::string::size_type rv = 0;
  rpn.stack_push(p->_stack.back());
  return rv;
}

NATIVE_WORD_IMPL(SWAP) {
  // there are definitely more efficient ways to do this.
  std::string::size_type rv = 0;
  auto s1 = rpn.stack_pop();
  auto s2 = rpn.stack_pop();
  rpn.stack_push(s1);
  rpn.stack_push(s2);
  return rv;
}

NATIVE_WORD_IMPL(DEPTH) {
  RpnController::Privates *p = dynamic_cast<RpnController::Privates*>(ctx);
  std::string::size_type rv = 0;
  rpn.stack_push(long(p->_stack.size()));
  return rv;
}

NATIVE_WORD_IMPL(DROP) {
  std::string::size_type rv = 0;
  rpn.stack_pop();
  return rv;
}

NATIVE_WORD_IMPL(DROPN) {
  RpnController::Privates *p = dynamic_cast<RpnController::Privates*>(ctx);
  std::string::size_type rv = 0;
  long n = rpn.stack_pop_as_integer();
  n = std::min((unsigned long)n,p->_stack.size());
  p->_stack.erase(p->_stack.end()-n, p->_stack.end());
  return rv;
}

NATIVE_WORD_IMPL(OVER) {
  RpnController::Privates *p = dynamic_cast<RpnController::Privates*>(ctx);
  std::string::size_type rv = 0;
  rpn.stack_push(*(p->_stack.end()-2));
  return rv;
}

NATIVE_WORD_IMPL(ROLLU) {
  RpnController::Privates *p = dynamic_cast<RpnController::Privates*>(ctx);
  std::string::size_type rv = 0;
  auto t = p->_stack.back();
  p->_stack.pop_back();
  p->_stack.push_front(t);
  return rv;
}

NATIVE_WORD_IMPL(ROLLD) {
  RpnController::Privates *p = dynamic_cast<RpnController::Privates*>(ctx);
  std::string::size_type rv = 0;
  auto b = p->_stack.front();
  p->_stack.pop_front();
  p->_stack.push_back(b);
  return rv;
}

NATIVE_WORD_IMPL(to_STR) {
  std::string::size_type rv = 0;
  rpn.stack_push(rpn.stack_pop_as_string());
  return rv;
}

NATIVE_WORD_IMPL(STR_to) {
  std::string::size_type rv = 0;
  printf("STR->: unimplemented\n");
  return rv;
}

NATIVE_WORD_IMPL(to_V3X) {
  std::string::size_type rv = 0;
  double x = rpn.stack_pop_as_double();
  rpn.stack_push(Vec3(x, std::nan(""), std::nan("")));
  return rv;
}

NATIVE_WORD_IMPL(to_V3Y) {
  std::string::size_type rv = 0;
  double y = rpn.stack_pop_as_double();
  rpn.stack_push(Vec3(std::nan(""), y, std::nan("")));
  return rv;
}

NATIVE_WORD_IMPL(to_V3Z) {
  std::string::size_type rv = 0;
  double z = rpn.stack_pop_as_double();
  rpn.stack_push(Vec3(std::nan(""), std::nan(""), z));
  return rv;
}

NATIVE_WORD_IMPL(V3_to) {
  std::string::size_type rv = 0;
  auto v = std::get<st_vec3>(rpn.stack_pop());
  rpn.stack_push(v.z);
  rpn.stack_push(v.y);
  rpn.stack_push(v.x);
  return rv;
}

SCOPED_NATIVE_WORD_IMPL(RpnController::Privates,COMPILED_EVAL) {
  CompiledWordContext *cw = dynamic_cast<CompiledWordContext*>(ctx);
  for(auto w : cw->wordlist()) {
    std::string rest;
    rpn.m_p->eval(rpn, w, rest);
  }
  return 0;
}

NATIVE_WORD_IMPL(SEMICOLON) {
  RpnController::Privates *p = dynamic_cast<RpnController::Privates*>(ctx);
  std::string::size_type rv = 0;
  p->_runtimeDictionary[p->_newWord] = {
    "compiled " + p->_newWord,
    {},
    SCOPED_NATIVE_WORD_FN(RpnController::Privates,COMPILED_EVAL),
    p->_newDefinition.clone()
  };
  p->_isCompiling = false;
  p->_newWord = "";
  p->_newDefinition.clear();
  return rv;
}
	       
RpnController::Privates::Privates() : _newWord(""), _isCompiling(false), _newDefinition({}) {

  _runtimeDictionary = {
    /*
     * Math words
     */
    { "ABS", {
	"Absolute Value (x -- |x|)", {
	  WORD_PARAMS_1(STACK_PARAM("x", st_number)),
	  //	  WORD_PARAMS_1(STACK_PARAM("x", st_vec3)),
	},
	NATIVE_WORD_FN(ABS), nullptr
      } },

    { "COS", {
	"Cosine (angle -- cos(angle))", {
	  WORD_PARAMS_1(STACK_PARAM("angle", st_number))
	},
	NATIVE_WORD_FN(COS), nullptr
      } },

    { "ACOS", {
	"Arc-Cosine (x -- acos(x))", {
	  WORD_PARAMS_1(STACK_PARAM("x", st_number)),
	},
	NATIVE_WORD_FN(ACOS), nullptr
      } },

    { "SIN", {
	"Sine (angle -- sin(angle))", {
	  WORD_PARAMS_1(STACK_PARAM("angle", st_number)),
	},
	NATIVE_WORD_FN(SIN), nullptr
      } },

    { "ASIN", {
	"Arc Sine (x -- asin(x))", {
	  WORD_PARAMS_1(STACK_PARAM("x", st_number)),
	},
	NATIVE_WORD_FN(ASIN), nullptr
      } },

    { "TAN", {
	"Tangent (angle -- tan(angle))", {
	  WORD_PARAMS_1(STACK_PARAM("angle", st_number)),
	},
	NATIVE_WORD_FN(TAN), nullptr
      } },

    { "ATAN", {
	"Arc-Tangent (x -- atan(x))", {
	  WORD_PARAMS_1(STACK_PARAM("x", st_number)),
	},
	NATIVE_WORD_FN(ATAN), nullptr
      } },

    { "ATAN2", {
	"Arc-Tangent of two variables (y x -- atan2(y,x))", {
	  WORD_PARAMS_2(STACK_PARAM("y", st_number), STACK_PARAM("x", st_number)),
	},
	NATIVE_WORD_FN(ATAN2), nullptr
      } },

    { "NEG", {
	"Negate (x -- -x)", {
	  WORD_PARAMS_1(STACK_PARAM("x", st_number)),
	},
	NATIVE_WORD_FN(NEG), nullptr
      } },

    { "SQRT", {
	"Square Root (x -- sqrt(x) )", {
	  WORD_PARAMS_1(STACK_PARAM("x", st_number)),
	},
	NATIVE_WORD_FN(SQRT), nullptr
      } },

    { "SQ", {
	"Square (x -- x^2)", {
	  WORD_PARAMS_1(STACK_PARAM("x", st_number)),
	},
	NATIVE_WORD_FN(SQ), nullptr
      } },

    { "POW", {
	"Exponentation (x y -- x^y)", {
	  WORD_PARAMS_2(STACK_PARAM("x", st_number), STACK_PARAM("y", st_number)),
	},
	NATIVE_WORD_FN(POW), nullptr
      } },

    { "INV", {
	"Invert (x -- 1/x))", {
	  WORD_PARAMS_1(STACK_PARAM("x", st_number)),
	},
	NATIVE_WORD_FN(INV), nullptr
      } },

    { "PI", {
	"The constant PI", {
	  WORD_PARAMS_0(),
	},
	NATIVE_WORD_FN(PI), nullptr
      } },

    { "+", {
	"Addition (y x -- x+y)", {
	  WORD_PARAMS_2(STACK_PARAM("x", st_number), STACK_PARAM("y", st_number)), // x + y
	  WORD_PARAMS_2(STACK_PARAM("vx", st_vec3), STACK_PARAM("vx", st_vec3)), // v1 + v2
	},
	NATIVE_WORD_FN(ADD), nullptr
      } },

    { "-", {
	"Subtract (y x -- y-x)", {
	  WORD_PARAMS_2(STACK_PARAM("x", st_number), STACK_PARAM("y", st_number)),
	  WORD_PARAMS_2(STACK_PARAM("vx", st_vec3), STACK_PARAM("vy", st_vec3))
	},
	NATIVE_WORD_FN(SUB), nullptr
      } },

    { "*", {
	"Multiply (x y -- x*y)", {
	  WORD_PARAMS_2(STACK_PARAM("x", st_number), STACK_PARAM("y", st_number))
	},
	NATIVE_WORD_FN(MULT), nullptr
      } },

    { "*", {
	"Divide (x y -- y/x)", {
	  WORD_PARAMS_2(STACK_PARAM("x", st_number),STACK_PARAM("y", st_number))
	},
	NATIVE_WORD_FN(DIV), nullptr
      } },

    { "EVAL", {
	"Evaluate string as an algebraic expression", {
	  WORD_PARAMS_1(STACK_PARAM("expr", st_string)),
	},
	NATIVE_WORD_FN(EVAL), nullptr
      } },

    /*
     * Stack and object words
     */
    { ".S", {
	"print stack", {
	  WORD_PARAMS_0(),
	},
	NATIVE_WORD_FN(_S), this
      } },
	      
    { ".D", {
	"print dictionary", {
	  WORD_PARAMS_0(),
	},
	NATIVE_WORD_FN(_D), this
      } },

    { ".\"", {
	"String literal", {
	  WORD_PARAMS_0(),
	},
	NATIVE_WORD_FN(_Q), nullptr
      } },

    { "(", {
	"Commment", {
	  WORD_PARAMS_0(),
	},
	NATIVE_WORD_FN(OPAREN), nullptr
      } },

    { ":", {
	"Define new word", {
	  WORD_PARAMS_0(),
	},
	NATIVE_WORD_FN(COLON), this
      } },

#if 0
    { "STO", {
	"store variable", {
	  WORD_PARAMS_1(STACK_PARAM("val", st_number  }, { "var", st_string } },
	},
	NATIVE_WORD_FN(STO)
	[](RpnController &rpn, std::string &rest) -> std::string::size_type {
	  std::string::size_type rv = 0;
	  printf("%s: unimplemented\n", word.c_str());
	  return rv;
	}
      } },
#endif

    { "CONCAT", {
	"String concatenation", {
	  WORD_PARAMS_2(STACK_PARAM("s2", st_string), STACK_PARAM("a1", st_any)), // s1 + a2 (convert a2 to string and concat)
	  WORD_PARAMS_2(STACK_PARAM("a2", st_any), STACK_PARAM("s1", st_string)), // a1 + s2 (convert a1 to string and concat)
	},
	NATIVE_WORD_FN(CONCAT), nullptr
      } },

    { "DUP", {
	"Duplicate top of stack", {
	  WORD_PARAMS_1(STACK_PARAM("s1", st_any))
	},
	NATIVE_WORD_FN(DUP), this
      } },

    { "SWAP", {
	"Swap top two stack items", {
	  WORD_PARAMS_2(STACK_PARAM("s2", st_any), STACK_PARAM("s1", st_any))
	},
	NATIVE_WORD_FN(SWAP), nullptr
      } },

    { "DEPTH",  {
	"Push stack depth", {
	  WORD_PARAMS_0()
	},
	NATIVE_WORD_FN(DEPTH), this
      } },

    { "DROP",  {
	"Drop top of stack", {
	  WORD_PARAMS_1(STACK_PARAM("s1", st_any))
	},
	NATIVE_WORD_FN(DROP), nullptr
      } },

    { "DROPN",  {
	"Drop n items top of stack", {
	  WORD_PARAMS_1(STACK_PARAM("s1", st_integer))
	},
	NATIVE_WORD_FN(DROPN), this
      } },

    { "OVER",  {
	"Copy second stack item to top", {
	  WORD_PARAMS_2(STACK_PARAM("s1", st_any), STACK_PARAM("s2", st_any))
	},
	NATIVE_WORD_FN(OVER), this
      } },

    { "ROLL+", {
	"Roll stack so that top goes to the bottom ( b .. t2 t1 -- t1 b ... t2 )", {
	  WORD_PARAMS_0(),
	},
	NATIVE_WORD_FN(ROLLU), this
      } },

    { "ROLL-", {
	"Roll stack so that bottom goes to the top ( b1 b2 ... t -- b2 ... b2 ... t b1 )", {
	  WORD_PARAMS_0(),
	},
	NATIVE_WORD_FN(ROLLD), this
      } },

    { "->STR", {
	"Convert top of stack to a string ( val -- str )", {
	  WORD_PARAMS_1(STACK_PARAM("v1", st_any))
	},
	NATIVE_WORD_FN(to_STR), nullptr
      } },

    { "STR->", {
	"Parse string at top of stack to another type ( str -- val )", {
	  WORD_PARAMS_1(STACK_PARAM("v1", st_string))
	},
	NATIVE_WORD_FN(STR_to), nullptr
      } },

    { "->{X}", {
	"Convert value on top of stack to X component of vec3 ( x -- {x,,} )", {
	  WORD_PARAMS_1(STACK_PARAM("X", st_number))
	},
	NATIVE_WORD_FN(to_V3X), nullptr
      } },

    { "->{Y}", {
	"Convert value on top of stack to Y component of vec3 ( y -- {,y,} )", {
	  WORD_PARAMS_1(STACK_PARAM("Y", st_number))
	},
	NATIVE_WORD_FN(to_V3Y), nullptr
      } },

    { "->{Z}", {
	"Convert value on top of stack to Z component of vec3 ( z -- {,,z} )", {
	  WORD_PARAMS_1(STACK_PARAM("Z", st_number))
	},
	NATIVE_WORD_FN(to_V3Z), nullptr
      } },

    { "{}->", {
	"Convert vector to components on stack ( v -- z y x )", {
	  WORD_PARAMS_1(STACK_PARAM("v", st_vec3))
	},
	NATIVE_WORD_FN(V3_to), nullptr
      } }

  };

  _compiletimeDictionary = {
    { ";", {
	"End Definition", {
	  WORD_PARAMS_0(),
	},
	NATIVE_WORD_FN(SEMICOLON), this
      } },

    { "(", {
	"Commment", {
	  WORD_PARAMS_0(),
	},
	NATIVE_WORD_FN(OPAREN), nullptr
      } },
  };

}


/* end of github/elh/rpn-cnc/rpn-cnc.cpp */
