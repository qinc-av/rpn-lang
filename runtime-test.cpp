/***************************************************
 * file: github/elh/rpn-cnc/runtime-test.cpp
 *
 * @file    runtime-test.cpp
 * @author  Eric L. Hernes
 * @version V1.0
 * @born_on   Saturday, May 27, 2023
 * @copyright (C) Copyright Eric L. Hernes 2023
 * @copyright (C) Copyright Q, Inc. 2023
 *
 * @brief   An Eric L. Hernes Signature Series C++ module
 *
 */

#include "rpn.h"

int
main(int ac, char **av) {
  rpn::Runtime rpn;

  //  std::string file = "/Users/eric/work/github/elh/rpn-cnc/xyz-probe.4nc";
  std::string file = "/Users/eric/work/github/elh/rpn-cnc/tests.4nc";

  if (ac > 1) {
    file = av[1];
  }

  rpn.parseFile(file);
  
  return 0;
  
}

/* end of github/elh/rpn-cnc/runtime-test.cpp */
