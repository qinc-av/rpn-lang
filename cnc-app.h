/***************************************************
 * file: qinc/rpn-lang/cnc-dictionary.h
 *
 * @file    cnc-dictionary.h
 * @author  Eric L. Hernes
 * @born_on   Friday, May 19, 2023
 * @copyright (C) Copyright Eric L. Hernes 2023
 * @copyright (C) Copyright Q, Inc. 2023
 *
 * @brief   An Eric L. Hernes Signature Series C/C++ header
 *
 * $Id$
 */

#pragma once

class MachineInterface : public WordContext {
public:
  MachineInterface(RpnController &rpn);
  ~MachineInterface();

  struct Privates;
private:
  Privates *m_p;
};

/* end of qinc/rpn-lang/cnc-dictionary.h */
