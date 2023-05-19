/***************************************************
 * file: github/elh/rpn-cnc/trpn.cpp
 *
 * @file    trpn.cpp
 * @author  Eric L. Hernes
 * @version V1.0
 * @born_on   Thursday, May 18, 2023
 * @copyright (C) Copyright Eric L. Hernes 2023
 * @copyright (C) Copyright Q, Inc. 2023
 *
 * @brief   An Eric L. Hernes Signature Series C++ module
 *
 */

#include "rpn-controller.h"
#include "cnc-app.h"

int
main(int ac, char **av) {
  if (ac > 1) {
    RpnController control;
    MachineInterface mc(control);

    control.loadFile(av[1]);
  } else {
    printf("usage: %s <filename>\n", av[0]);
  }
  
  return 0;
}

/* end of github/elh/rpn-cnc/trpn.cpp */
