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

/*
 * Definition of a word within the dictionary.  This allows for checking against various
 * types of parameters
 */
struct word_t {
  std::string description;
  std::vector<std::vector<param_t>> params;
  std::function<std::string::size_type(const std::string &word,std::string &rest)> eval;
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
