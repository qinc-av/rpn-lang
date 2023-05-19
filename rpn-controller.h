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

class RpnCncController {
public:
  RpnCncController();
  ~RpnCncController();
  bool parse(std::string &line);
  bool loadFile(const std::string &path);

  struct Privates;
private:
  Privates *m_p;
};

/* end of github/elh/rpn-cnc/rpn-cnc.h */
