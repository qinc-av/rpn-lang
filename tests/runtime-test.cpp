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

TEST_CASE( "parse", "Stack Words" ) {
  /*
  "CLEAR"
  "DEPTH"
  "SWAP"
  "ROLLU"
  "ROLLD"
  "DUP"
  "ROTU"
  "ROTD"
  "DUPN"
  "NIPN"
  "PICK"
  "ROLLDN"
  "ROLLUN"
  "TUCKN"
  ".S"
  */

  /*
   "OVER"
   "DROP"
   "DROPN"
  */

  std::string line("12.32 3 OVER DROP 1 2 3 4 5 6 2 DROPN");
  g_rpn.parse(line);
  g_rpn.stack.print("test parse");
  // should be 12.32 3 1 2 3 4 "
  REQUIRE( ((4 == g_rpn.stack.peek_integer(1)) &&
	    (3 == g_rpn.stack.peek_integer(2)) &&
	    (2 == g_rpn.stack.peek_integer(3)) &&
	    (1 == g_rpn.stack.peek_integer(4)) &&
	    (3 == g_rpn.stack.peek_integer(5)) &&
	    (12.32 == g_rpn.stack.peek_double(6))) );
}

TEST_CASE( "== !=", " runtime logic" ) {
  std::string line("CLEAR 123 456 ==");
  g_rpn.parse(line);
  REQUIRE( (false == g_rpn.stack.peek_boolean(1) ) );

  line = ("CLEAR 123 456 !=");
  g_rpn.parse(line);
  REQUIRE( (true == g_rpn.stack.peek_boolean(1) ) );

  line = ("CLEAR 1.0 1 ==");
  g_rpn.parse(line);
  REQUIRE( (false == g_rpn.stack.peek_boolean(1) ) );

  line = ("CLEAR .\" abc\" .\" xyz\" !=");
  g_rpn.parse(line);
  REQUIRE( (true == g_rpn.stack.peek_boolean(1) ) );

  line = ("CLEAR .\" abc\" .\" abc\" ==");
  g_rpn.parse(line);
  REQUIRE( (true == g_rpn.stack.peek_boolean(1) ) );

  line = ("CLEAR 3.14159 3.14159 ==");
  g_rpn.parse(line);
  REQUIRE( (true == g_rpn.stack.peek_boolean(1) ) );
}

TEST_CASE( "AND OR NOT" " runtime logic" ) {
  std::string line;

  line = ("CLEAR 1 1 == .S NOT .S");
  g_rpn.parse(line);
  REQUIRE( (false == g_rpn.stack.peek_boolean(1) ) );

  line = ("1 1 == OR");
  g_rpn.parse(line);
  REQUIRE( (true == g_rpn.stack.peek_boolean(1) ) );

  line = ("1 0 == AND");
  g_rpn.parse(line);
  REQUIRE( (false == g_rpn.stack.peek_boolean(1) ) );

}

TEST_CASE( "AND OR NOT XOR" " binary logic" ) {
  std::string line;

  line = ("CLEAR 0x1234 0x4321 AND .S");
  g_rpn.parse(line);
  REQUIRE( ((0x1234&0x4321) == g_rpn.stack.peek_integer(1) ) );

  line = ("0x9281 0xabcd OR .S");
  g_rpn.parse(line);
  REQUIRE( ((0x9281 | 0xabcd) == g_rpn.stack.peek_integer(1) ) );

  line = ("0x55a8 0xaaaa XOR .S");
  g_rpn.parse(line);
  REQUIRE( ((0x55a8 ^ 0xaaaa) == g_rpn.stack.peek_integer(1) ) );

  line = ("DUP NEG .S");
  g_rpn.parse(line);
  REQUIRE( ((~(0x55a8 ^ 0xaaaa)) == g_rpn.stack.peek_integer(1) ) );

}

/*
TEST_CASE( "parse file", "" ) {
  std::string file = "/Users/eric/work/github/elh/rpn-cnc/tests.4nc";
  g_rpn.parseFile(file);
}
*/

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
