/***************************************************
 * file: github/elh/rpn-cnc/rpn-cnc.h
 *
 * @file    rpn-cnc.h
 * @author  Eric L. Hernes
 * @born_on   Thursday, May 18, 2023
 * @copyright (C) Copyright Eric L. Hernes 2023
 * @copyright (C) Copyright Q, Inc. 2023
 *
 * @brief   An Eric L. Hernes Signature Series C/C++ header
 *
 * $Id$
 */

#pragma once

#include <string>
#include <map>
#include <vector>
#include <variant>

class RpnController;

/*
 * dataypes on the stack
 */
struct Vec3 {
  Vec3(double xx=std::nan(""), double yy=std::nan(""), double zz=std::nan("")) : x(xx), y(yy), z(zz) {}
  double x;
  double y;
  double z;
};

// the order here must match the enum below!
using stacktype_t = std::variant<double, long, std::string, Vec3>;

enum datatype_t {
  st_double,
  st_integer,
  st_string,
  st_vec3,

  // these are not real types, they are used for validating parameters
  st_number, // either double or integer
  st_any, // not a real type, just wildcard for don't care
};

/*
 * A pairing of name and type, used for documentation and checking parameters to words
 */
struct param_t {
  std::string name;
  datatype_t type;
};

// these two are used for a reference to the method (as in a native object 
// name of the function
// a plain old (mangled) function name
#define NATIVE_WORD_FN(nw) native_word_##nw
#define SCOPED_NATIVE_WORD_FN(clazz, nw) clazz::NATIVE_WORD_FN(nw)

// declares the native word
#define NATIVE_WORD_IMPL(nw) static std::string::size_type NATIVE_WORD_FN(nw)(RpnController &rpn, std::string &rest)

// declares a native word that is private to the Privates class - this lets it access rpn.m_p
#define SCOPED_NATIVE_WORD_IMPL(clazz,nw) std::string::size_type SCOPED_NATIVE_WORD_FN(clazz,nw) (RpnController &rpn, std::string &rest)

#define STACK_PARAM(n,t) { n, t }

// a word that requires more than 8 paramaters on the stack should probably be re-worked
#define WORD_PARAMS_0() {}
#define WORD_PARAMS_1(p1) { p1 }
#define WORD_PARAMS_2(p1,p2) { p1, p2 }
#define WORD_PARAMS_3(p1,p2,p3) { p1, p2, p3 }
#define WORD_PARAMS_4(p1,p2,p3,p4) { p1, p2, p3, p4 }
#define WORD_PARAMS_5(p1,p2,p3,p4,p5) { p1, p2, p3, p4, p5 }
#define WORD_PARAMS_6(p1,p2,p3,p4,p5,p6) { p1, p2, p3, p4, p5, p6 }
#define WORD_PARAMS_7(p1,p2,p3,p4,p5,p6,p7) { p1, p2, p3, p4, p5, p6, p7 }
#define WORD_PARAMS_8(p1,p2,p3,p4,p5,p6,p7,p8) { p1, p2, p3, p4, p5, p6, p7, p8 }

/*
 * Definition of a word within the dictionary.  This allows for checking against various
 * types of parameters
 */
//struct nativedef_t {
//  std::vector<param_t> params;
//  std::function<std::string::size_type(RpnController &rpn, std::string &rest)> eval;
//};

struct word_t {
  std::string description;
  std::vector<std::vector<param_t>> params;
  std::function<std::string::size_type(RpnController &rpn, std::string &rest)> eval;
};

class RpnController {
public:
  RpnController();
  ~RpnController();
  void addDictionary(const std::map<std::string,word_t> &applicationDictionary);

  bool parse(std::string &line);
  bool loadFile(const std::string &path);

  void stack_push(const stacktype_t &v);
  stacktype_t stack_pop();
  double stack_pop_as_double();
  long stack_pop_as_integer();
  std::string stack_pop_as_string();
  Vec3 stack_pop_as_vec3();

  static std::string::size_type nextWord(std::string &word, std::string &buffer, char delim=' ');

  struct Privates;
private:
  Privates *m_p;
};

const std::string to_string(const datatype_t &dt);
const std::string to_string(const stacktype_t &e);
const std::string to_string(const std::pair<std::string,word_t> &wp);
const std::string to_string(const Vec3 &v);

/* end of github/elh/rpn-cnc/rpn-cnc.h */
