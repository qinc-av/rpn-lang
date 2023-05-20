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

struct RpnController::Privates {
  Privates();
  ~Privates() {};

  std::string::size_type eval(const std::string &word, std::string &rest);
  std::string::size_type runtime_eval(const std::string &word, std::string &rest);
  std::string::size_type compiletime_eval(const std::string &word, std::string &rest);

  std::string::size_type parse_comment(const std::string &word, std::string &rest);

  std::string::size_type nodef(const std::string &word, std::string &rest) {
    return 0;
  }

  std::string::size_type user_eval(const std::string &word, std::string &rest, const std::vector<std::string> &wordlist) {
    //    printf("%s: (%s)\n", __func__, word.c_str());
    for(auto w : wordlist) {
      std::string rest;
      eval(w, rest);
      //      printf("  %s\n", w.c_str());
    }
    return 0;
  }

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
  std::vector<std::string> _newDefinition;

  std::deque<stacktype_t> _stack;
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
RpnController::Privates::compiletime_eval(const std::string &word, std::string &rest) {
  std::string::size_type rv = 0;

  if (_newWord == "") {
    _newWord = word;
  } else {
    const auto &cw = _compiletimeDictionary.find(word);
    if (cw != _compiletimeDictionary.end()) {
      // found something in the compiletime dict, evaluate it
      rv = cw->second.eval(word,rest);

    } else {
      if (std::isdigit(word[0])) {
	// numbers just push
	_newDefinition.push_back(word);

      } else {
	// everything else, we check in the runtime dictionary
	const auto &rw = _runtimeDictionary.find(word);
	if (rw != _runtimeDictionary.end()) {
	  _newDefinition.push_back(word);
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
RpnController::Privates::runtime_eval(const std::string &word, std::string &rest) {
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
	rv = we->second.eval(word,rest);
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
RpnController::Privates::eval(const std::string &word, std::string &rest) {
  std::string::size_type rv = 0; // std::string::npos;
  if (word.size()>0) {

    if (_isCompiling) {
      rv = compiletime_eval(word,rest);
    } else {
      rv = runtime_eval(word,rest);
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
    auto s = m_p->eval(word, line);
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

std::string::size_type
RpnController::Privates::parse_comment(const std::string &word, std::string &rest) {
  std::string::size_type rv = 0;
  std::string comment;
  rv = nextWord(comment, rest, ')');
  if (rv != std::string::npos) {
    //    printf("comment: [%s]\n", comment.c_str());
  } else {
    printf("parse error in comment: terminating ')' not found [%s %s]\n", word.c_str(), rest.c_str());
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
  switch((isnan(a)<<1)|(isnan(b))) {
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
  switch((isnan(a)<<1)|(isnan(b))) {
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

RpnController::Privates::Privates() : _newWord(""), _isCompiling(false), _newDefinition({}) {

  _runtimeDictionary = {
    /*
     * Math words
     */
    { "ABS", { "Absolute Value (x -- |x|)", {
	  { { "x", st_number } },
	  //	  { { "v", st_vec3 } },
	},
	       [this](const std::string &word, std::string &rest) -> std::string::size_type {
		 std::string::size_type rv = 0;
		 double v = stack_pop_as_double();
		 stack_push(v*v);
		 return rv;
	       }
      } },

    { "COS", { "Cosine (angle -- cos(angle))", {
	  { { "angle", st_number } }
	},
	       [this](const std::string &word, std::string &rest) -> std::string::size_type {
		 std::string::size_type rv = 0;
		 double rad = deg_to_rad(stack_pop_as_double());
		 stack_push(cos(rad));
		 return rv;
	       }
      } },

    { "ACOS", { "Arc-Cosine (x -- acos(x))", {
	  { { "x", st_number } }
	},
		[this](const std::string &word, std::string &rest) -> std::string::size_type {
		  std::string::size_type rv = 0;
		  double v = acos(stack_pop_as_double());
		  stack_push(rad_to_deg(v));
		  return rv;
		}
      } },

    { "SIN", { "Sine (angle -- sin(angle))", {
	  { { "angle", st_number } }
	},
	       [this](const std::string &word, std::string &rest) -> std::string::size_type {
		 std::string::size_type rv = 0;
		 double rad = deg_to_rad(stack_pop_as_double());
		 stack_push(sin(rad));
		 return rv;
	       }
      } },

    { "ASIN", { "Arc Sine (x -- asin(x))", {
	  { { "x", st_number } }
	},
		[this](const std::string &word, std::string &rest) -> std::string::size_type {
		  std::string::size_type rv = 0;
		  double v = asin(stack_pop_as_double());
		  stack_push(rad_to_deg(v));
		  return rv;
		}
      } },

    { "TAN", { "Tangent (angle -- tan(angle))", {
	  { { "angle", st_number } }
	},
	       [this](const std::string &word, std::string &rest) -> std::string::size_type {
		 std::string::size_type rv = 0;
		 double rad = deg_to_rad(stack_pop_as_double());
		 stack_push(tan(rad));
		 return rv;
	       }
      } },

    { "ATAN", { "Arc-Tangent (x -- atan(x))", {
	  { { "x", st_number } }
	},
		[this](const std::string &word, std::string &rest) -> std::string::size_type {
		  std::string::size_type rv = 0;
		  double v = atan(stack_pop_as_double());
		  stack_push(rad_to_deg(v));
		  return rv;
		}
      } },

    { "ATAN2", { "Arc-Tangent of two variables (y x -- atan2(y,x))", {
	  { { "y", st_number }, { "x", st_number } }
	},
		 [this](const std::string &word, std::string &rest) -> std::string::size_type {
		   std::string::size_type rv = 0;
		   double x = stack_pop_as_double();
		   double y = stack_pop_as_double();
		   stack_push(rad_to_deg(atan2(y,x)));
		   return rv;
		 }
      } },

    { "NEG", { "Negate (x -- -x)", {
	  { { "x", st_number } }
	},
	       [this](const std::string &word, std::string &rest) -> std::string::size_type {
		 std::string::size_type rv = 0;
		 auto x = stack_pop();
		 switch(x.index()) {
		 case st_double:
		   stack_push(std::get<st_double>(x)*-1.);
		   break;
		 case st_integer:
		   stack_push(std::get<st_integer>(x)*-1);
		   break;
		 }
		 return rv;
	       }
      } },

    { "SQRT", { "Square Root (x -- sqrt(x) )", {
	  { { "x", st_number } }
	},
		[this](const std::string &word, std::string &rest) -> std::string::size_type {
		  std::string::size_type rv = 0;
		  double x = stack_pop_as_double();
		  stack_push(sqrt(x));
		  return rv;
		}
      } },

    { "SQ", { "Square (x -- x^2)", {
	  { { "x", st_number } }
	},
	      [this](const std::string &word, std::string &rest) -> std::string::size_type {
		std::string::size_type rv = 0;
		double x = stack_pop_as_double();
		stack_push(x*x);
		return rv;
	      }
      } },

    { "POW", { "Exponentation (x y -- x^y)", {
	  { { "x", st_number }, { "y", st_number } }
	},
	       [this](const std::string &word, std::string &rest) -> std::string::size_type {
		 std::string::size_type rv = 0;
		 double x = stack_pop_as_double();
		 double y = stack_pop_as_double();
		 stack_push(pow(x,y));
		 return rv;
	       }
      } },

    { "INV", { "Invert (x -- 1/x))", {
	  { { "x", st_number } },
	},
	       [this](const std::string &word, std::string &rest) -> std::string::size_type {
		 std::string::size_type rv = 0;
		 double x = stack_pop_as_double();
		 if (x!=0.) {
		   stack_push(1.0/x);
		 } else {
		   stack_push(x);
		 }
		 return rv;
	       }
      } },

    { "PI", { "The constant PI", {
	  {},
	},
	      [this](const std::string &word, std::string &rest) -> std::string::size_type {
		std::string::size_type rv = 0;
		stack_push(M_PI);
		return rv;
	      }
      } },

    { "+", { "Addition (x y -- x+y)", {
	  { { "x", st_number }, { "y", st_number } }, // x + y
	  { { "vx", st_vec3 }, { "vx", st_vec3 } }, // v1 + v2
	},
	     [this](const std::string &word, std::string &rest) -> std::string::size_type {
	       std::string::size_type rv = 0;
	       auto sx = stack_pop();
	       auto sy = stack_pop();
	       unsigned vv = type2_pred_v(sx,sy);
	       switch(vv) {
	       case type2_pred(st_integer,st_integer):
		 stack_push(std::get<st_integer>(sx)+std::get<st_integer>(sy));
		 break;
	       case type2_pred(st_double,st_integer):
		 stack_push(std::get<st_double>(sx)+std::get<st_integer>(sy));
		 break;
	       case type2_pred(st_integer,st_double):
		 stack_push(std::get<st_integer>(sx)+std::get<st_double>(sy));
		 break;
	       case type2_pred(st_double,st_double):
		 stack_push(std::get<st_double>(sx)+std::get<st_double>(sy));
		 break;
	       case type2_pred(st_vec3,st_vec3):
		 stack_push(std::get<st_vec3>(sx)+std::get<st_vec3>(sy));
		 break;
	       }
	       return rv;
	     }
      } },

    { "-", { "Subtract (x y -- y-x)", {
	  { { "x", st_number }, { "y", st_number } },
	  { { "vx", st_vec3 }, { "vy", st_vec3 } }, // v1 + v2
	},
	     [this](const std::string &word, std::string &rest) -> std::string::size_type {
	       std::string::size_type rv = 0;
	       auto sx = stack_pop();
	       auto sy = stack_pop();
	       unsigned vv = type2_pred_v(sx,sy);
	       switch(vv) {
	       case type2_pred(st_integer,st_integer):
		 stack_push(std::get<st_integer>(sx)-std::get<st_integer>(sy));
		 break;
	       case type2_pred(st_double,st_integer):
		 stack_push(std::get<st_double>(sx)-std::get<st_integer>(sy));
		 break;
	       case type2_pred(st_integer,st_double):
		 stack_push(std::get<st_integer>(sx)-std::get<st_double>(sy));
		 break;
	       case type2_pred(st_double,st_double):
		 stack_push(std::get<st_double>(sx)-std::get<st_double>(sy));
		 break;
	       case type2_pred(st_vec3,st_vec3):
		 stack_push(std::get<st_vec3>(sx)+std::get<st_vec3>(sy));
		 break;
	       }
	       return rv;
	     }
      } },

    { "*", { "Multiply (x y -- x*y)", {
	  { { "x", st_number }, { "y", st_number } }
	},
	     [this](const std::string &word, std::string &rest) -> std::string::size_type {
	       std::string::size_type rv = 0;
	       auto sx = stack_pop();
	       auto sy = stack_pop();
	       unsigned vv = type2_pred_v(sx,sy);
	       switch(vv) {
	       case type2_pred(st_integer,st_integer):
		 stack_push(std::get<st_integer>(sx)*std::get<st_integer>(sy));
		 break;
	       case type2_pred(st_double,st_integer):
		 stack_push(std::get<st_double>(sx)*std::get<st_integer>(sy));
		 break;
	       case type2_pred(st_integer,st_double):
		 stack_push(std::get<st_integer>(sx)*std::get<st_double>(sy));
		 break;
	       case type2_pred(st_double,st_double):
		 stack_push(std::get<st_double>(sx)*std::get<st_double>(sy));
		 break;
	       }
	       return rv;
	     }
      } },

    { "*", { "Divide (x y -- y/x)", {
	  { { "x", st_number }, { "y", st_number } }
	},
	     [this](const std::string &word, std::string &rest) -> std::string::size_type {
	       std::string::size_type rv = 0;
	       auto sx = stack_pop();
	       auto sy = stack_pop();
	       unsigned vv = type2_pred_v(sx,sy);
	       switch(vv) {
	       case type2_pred(st_integer,st_integer):
		 stack_push(std::get<st_integer>(sx)/std::get<st_integer>(sy));
		 break;
	       case type2_pred(st_double,st_integer):
		 stack_push(std::get<st_double>(sx)/std::get<st_integer>(sy));
		 break;
	       case type2_pred(st_integer,st_double):
		 stack_push(std::get<st_integer>(sx)/std::get<st_double>(sy));
		 break;
	       case type2_pred(st_double,st_double):
		 stack_push(std::get<st_double>(sx)/std::get<st_double>(sy));
		 break;
	       }
	       return rv;
	     }
      } },

    { "EVAL", { "Evaluate string as an algebraic expression", {
	  { { "expr", st_string } },
	},
		[this](const std::string &word, std::string &rest) -> std::string::size_type {
		  std::string::size_type rv = 0;
		  return rv;
		}
      } },

    /*
     * Stack and object words
     */
    { ".S", { "print stack", {
	  {},
	},
	      [this](const std::string &word, std::string &rest) -> std::string::size_type {
		print_stack();
		return 0;
	      }
      } },
	      
    { ".D", { "print dictionary", {
	  {},
	},
	      [this](const std::string &word, std::string &rest) -> std::string::size_type {
		for(auto const &de : _runtimeDictionary) {
		  std::string s = to_string(de);
		  printf("%s\n\n", s.c_str());
		}
		return 0;
	      }
      } },

    { ".\"", { "String literal", {
	  {},
	},
	       [this](const std::string &word, std::string &rest) -> std::string::size_type {
		 std::string::size_type rv = 0;
		 std::string literal;
		 auto p = nextWord(literal, rest, '"');
		 if (p != std::string::npos) {
		   rv = p;
		   stack_push(literal);
		 } else {
		   printf("parse error in string literal: terminating '\"' not found [%s]\n", rest.c_str());
		 }
		 return rv;
	       }
      } },

    { "(", { "Commment", {
	  {},
	},
	     [this](const std::string &word, std::string &rest) -> std::string::size_type {
	       return parse_comment(word,rest);
	     }
      } },

    { ":", { "Define new word", {
	  {},
	},
	     [this](const std::string &word, std::string &rest) -> std::string::size_type {
	       std::string::size_type rv = 0;
	       _isCompiling = true;
	       return rv;
	     }
      } },

#if 0
    { "STO", { "store variable", {
	  { { "val", st_number  }, { "var", st_string } },
	},
	       [this](const std::string &word, std::string &rest) -> std::string::size_type {
		 std::string::size_type rv = 0;
		 printf("%s: unimplemented\n", word.c_str());
		 return rv;
	       }
      } },
#endif

    { "CONCAT", { "String concatenation", {
	  { { "s2", st_string }, { "a1", st_any } }, // s1 + a2 (convert a2 to string and concat)
	  { { "a2", st_any }, { "s1", st_string } }, // a1 + s2 (convert a1 to string and concat)
	},
		  [this](const std::string &word, std::string &rest) -> std::string::size_type {
		    std::string::size_type rv = 0;
		    std::string s1 = stack_pop_as_string();
		    std::string s2 = stack_pop_as_string();
		    stack_push(s2 + s1);
		    return rv;
		  }
      } },

    { "DUP", { "Duplicate top of stack", {
	  { { "s1", st_any } }
	},
	       [this](const std::string &word, std::string &rest) -> std::string::size_type {
		 std::string::size_type rv = 0;
		 stack_push(_stack.back());
		 return rv;
	       }
      } },

    { "SWAP", { "Swap top two stack items", {
	  { { "s2", st_any }, { "s1", st_any } }
	},
	       [this](const std::string &word, std::string &rest) -> std::string::size_type {
		 // there are definitely more efficient ways to do this.
		 std::string::size_type rv = 0;
		 auto s1 = stack_pop();
		 auto s2 = stack_pop();
		 stack_push(s1);
		 stack_push(s2);
		 return rv;
	       }
      } },

    { "DEPTH",  { "Push stack depth", {
	  {}
	},
		 [this](const std::string &word, std::string &rest) -> std::string::size_type {
		   std::string::size_type rv = 0;
		   stack_push(_stack.size());
		   return rv;
		 }
      } },

    { "DROP",  { "Drop top of stack", {
	  { { "s1", st_any } }
	},
		 [this](const std::string &word, std::string &rest) -> std::string::size_type {
		   std::string::size_type rv = 0;
		   stack_pop();
		   return rv;
		 }
      } },

    { "DROPN",  { "Drop n items top of stack", {
	  { { "s1", st_integer } }
	},
		  [this](const std::string &word, std::string &rest) -> std::string::size_type {
		    std::string::size_type rv = 0;
		    long n = stack_pop_as_integer();
		    n = std::min((unsigned long)n,_stack.size());
		    _stack.erase(_stack.end()-n, _stack.end());
		    return rv;
		  }
      } },

    { "OVER",  { "Copy second stack item to top", {
	  { { "s1", st_any }, { "s2", st_any } }
	},
		 [this](const std::string &word, std::string &rest) -> std::string::size_type {
		   std::string::size_type rv = 0;
		   stack_push(*(_stack.end()-2));
		   return rv;
		 }
      } },

    { "ROLL+", { "Roll stack so that top goes to the bottom ( t1 t2 ... b -- t2 ... b t1 )", {
	  {},
	},
		 [this](const std::string &word, std::string &rest) -> std::string::size_type {
		   std::string::size_type rv = 0;
		   printf("%s: unimplemented\n", word.c_str());
		   return rv;
		 }
      } },

    { "ROLL-", { "Roll stack so that bottom goes to the top ( t ... b2 b1 -- b1 t ... b2 )", {
	  {},
	},
		 [this](const std::string &word, std::string &rest) -> std::string::size_type {
		   std::string::size_type rv = 0;
		   printf("%s: unimplemented\n", word.c_str());
		   return rv;
		 }
      } },

    { "->STR", { "Convert top of stack to a string ( val -- str )", {
	  { { "v1", st_any } }
	},
		 [this](const std::string &word, std::string &rest) -> std::string::size_type {
		   std::string::size_type rv = 0;
		   stack_push(stack_pop_as_string());
		   return rv;
		 }
      } },

    { "STR->", { "Parse string at top of stack to another type ( str -- val )", {
	  { { "v1", st_string } }
	},
		 [this](const std::string &word, std::string &rest) -> std::string::size_type {
		   std::string::size_type rv = 0;
		   printf("%s: unimplemented\n", word.c_str());
		   return rv;
		 }
      } },

    { "->{X}", { "Convert value on top of stack to X component of vec3 ( x -- {x,,} )", {
	  { { "X", st_number } }
	},
		 [this](const std::string &word, std::string &rest) -> std::string::size_type {
		   std::string::size_type rv = 0;
		   double x = stack_pop_as_double();
		   stack_push(Vec3(x, std::nan(""), std::nan("")));
		   return rv;
		 }
      } },

    { "->{Y}", { "Convert value on top of stack to Y component of vec3 ( y -- {,y,} )", {
	  { { "Y", st_number } }
	},
		 [this](const std::string &word, std::string &rest) -> std::string::size_type {
		   std::string::size_type rv = 0;
		   double y = stack_pop_as_double();
		   stack_push(Vec3(std::nan(""), y, std::nan("")));
		   return rv;
		 }
      } },

    { "->{Z}", { "Convert value on top of stack to Z component of vec3 ( z -- {,,z} )", {
	  { { "Z", st_number } }
	},
		 [this](const std::string &word, std::string &rest) -> std::string::size_type {
		   std::string::size_type rv = 0;
		   double z = stack_pop_as_double();
		   stack_push(Vec3(std::nan(""), std::nan(""), z));
		   return rv;
		 }
      } },

    { "{}->", { "Convert vector to components on stack ( v -- z y x )", {
	  { { "v", st_vec3 } }
	},
		[this](const std::string &word, std::string &rest) -> std::string::size_type {
		  std::string::size_type rv = 0;
		  auto v = std::get<st_vec3>(stack_pop());
		  stack_push(v.z);
		  stack_push(v.y);
		  stack_push(v.x);
		  return rv;
		}
      } }

  };

  _compiletimeDictionary = {
    { ";", { "End Definition", {
	  {},
	},
	     [this](const std::string &word, std::string &rest) -> std::string::size_type {
	       std::string::size_type rv = 0;
	       auto wl = std::vector<std::string>(_newDefinition);
	       _runtimeDictionary[_newWord] = {
		 "user " + _newWord,
		 {},
		 [this,wl](const std::string &word_, std::string &rest_) -> std::string::size_type {
		   user_eval(word_,rest_, wl);
		   return 0;
		 }
	       };
	       _isCompiling = false;
	       _newWord = "";
	       _newDefinition.clear();
	       return rv;
	     }
      } },

    { "(", { "Commment", {
	  {},
	},
	     [this](const std::string &word, std::string &rest) -> std::string::size_type {
	       return parse_comment(word,rest);
	     }
      } },

  };

}


/* end of github/elh/rpn-cnc/rpn-cnc.cpp */
