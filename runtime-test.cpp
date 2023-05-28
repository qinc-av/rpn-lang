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

#include <catch2/catch_test_macros.hpp>
#include "rpn.h"

rpn::Runtime g_rpn;


TEST_CASE( "parse", "[single-file]" ) {
  fprintf(stderr, "parse\n");
  std::string line("12.32 DROP 1 2 3 4 5 6 2 DROPN");
  g_rpn.parse(line);
  g_rpn.stack.print("test parse");
  REQUIRE(1 == 0);
}

TEST_CASE( "parse file", "[single-file]" ) {
  std::string file = "/Users/eric/work/github/elh/rpn-cnc/tests.4nc";
  g_rpn.parseFile(file);
}

#if 0
int
main(int ac, char **av) {

  //  std::string file = "/Users/eric/work/github/elh/rpn-cnc/xyz-probe.4nc";
  std::string file = "/Users/eric/work/github/elh/rpn-cnc/tests.4nc";

  if (ac > 1) {
    file = av[1];
  }

  rpn.parseFile(file);
  
  return 0;
  
}
#endif

/* end of github/elh/rpn-cnc/runtime-test.cpp */
