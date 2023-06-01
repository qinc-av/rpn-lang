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
   "OVER"
   "DROP"
   "DROPN"
  */

  std::string line;
  line = ("CLEAR 12.32 3 OVER DROP 1 2 3 4 5 6 2 DROPN");
  g_rpn.parse(line);
  g_rpn.stack.print("test parse");
  // should be 12.32 3 1 2 3 4 "
  REQUIRE( ((4 == g_rpn.stack.peek_integer(1)) &&
	    (3 == g_rpn.stack.peek_integer(2)) &&
	    (2 == g_rpn.stack.peek_integer(3)) &&
	    (1 == g_rpn.stack.peek_integer(4)) &&
	    (3 == g_rpn.stack.peek_integer(5)) &&
	    (12.32 == g_rpn.stack.peek_double(6))) );
  REQUIRE( 6 == g_rpn.stack.depth() );

  /*
    "DEPTH"
    "SWAP"
  */
  line = ("DEPTH SWAP");
  g_rpn.parse(line);
  REQUIRE( 7 == g_rpn.stack.depth() );
  REQUIRE( ((4 == g_rpn.stack.peek_integer(1)) &&
	    (6 == g_rpn.stack.peek_integer(2))) );

  /* 
     "DUP"
  */
  line = ("9988 DUP DUP .S");
  g_rpn.parse(line);
  REQUIRE( 10 == g_rpn.stack.depth() );
  REQUIRE( ((9988 == g_rpn.stack.peek_integer(1)) &&
	    (9988 == g_rpn.stack.peek_integer(2)) &&
	    (9988 == g_rpn.stack.peek_integer(3)) &&
	    (4 == g_rpn.stack.peek_integer(4))));

  line = ("3 DROPN");
  g_rpn.parse(line);
  REQUIRE( 7 == g_rpn.stack.depth() );
  REQUIRE( ((4 == g_rpn.stack.peek_integer(1)) &&
	    (6 == g_rpn.stack.peek_integer(2))) );
  /*
    "DUPN"
  */
  line = ("3 DUPN");
  g_rpn.parse(line);
  REQUIRE( 10 == g_rpn.stack.depth() );
  REQUIRE( ((4 == g_rpn.stack.peek_integer(1)) &&
	    (6 == g_rpn.stack.peek_integer(2)) &&
	    (3 == g_rpn.stack.peek_integer(3))) );
  REQUIRE( ((4 == g_rpn.stack.peek_integer(4)) &&
	    (6 == g_rpn.stack.peek_integer(5)) &&
	    (3 == g_rpn.stack.peek_integer(6))) );
  /*
    "ROLLU"
  */
  line = ("CLEAR 10 9 8 7 6 5 4 3 2 1 ROLLU ROLLU");
  g_rpn.parse(line);
  REQUIRE( (8 == g_rpn.stack.peek_integer(10)) );
  REQUIRE( (7 == g_rpn.stack.peek_integer(9)) );
  REQUIRE( (6 == g_rpn.stack.peek_integer(8)) );
  REQUIRE( (5 == g_rpn.stack.peek_integer(7)) );
  REQUIRE( (4 == g_rpn.stack.peek_integer(6)) );
  REQUIRE( (3 == g_rpn.stack.peek_integer(5)) );
  REQUIRE( (2 == g_rpn.stack.peek_integer(4)) );
  REQUIRE( (1 == g_rpn.stack.peek_integer(3)) );
  REQUIRE( (10 == g_rpn.stack.peek_integer(2)) );
  REQUIRE( (9 == g_rpn.stack.peek_integer(1)) );
  
  /*
    "ROLLD"
  */
  line = ("CLEAR 10 9 8 7 6 5 4 3 2 1 ROLLD ROLLD");
  g_rpn.parse(line);
  REQUIRE( (2 == g_rpn.stack.peek_integer(10)) );
  REQUIRE( (1 == g_rpn.stack.peek_integer(9)) );
  REQUIRE( (10 == g_rpn.stack.peek_integer(8)) );
  REQUIRE( (9 == g_rpn.stack.peek_integer(7)) );
  REQUIRE( (8 == g_rpn.stack.peek_integer(6)) );
  REQUIRE( (7 == g_rpn.stack.peek_integer(5)) );
  REQUIRE( (6 == g_rpn.stack.peek_integer(4)) );
  REQUIRE( (5 == g_rpn.stack.peek_integer(3)) );
  REQUIRE( (4 == g_rpn.stack.peek_integer(2)) );
  REQUIRE( (3 == g_rpn.stack.peek_integer(1)) );

  /*
    "ROTU"
  */
  line = ("CLEAR 10 9 8 7 6 5 4 3 2 1 ROTU");
  g_rpn.parse(line);
  REQUIRE( (10 == g_rpn.stack.peek_integer(10)) );
  REQUIRE( (9 == g_rpn.stack.peek_integer(9)) );
  REQUIRE( (8 == g_rpn.stack.peek_integer(8)) );
  REQUIRE( (7 == g_rpn.stack.peek_integer(7)) );
  REQUIRE( (6 == g_rpn.stack.peek_integer(6)) );
  REQUIRE( (5 == g_rpn.stack.peek_integer(5)) );
  REQUIRE( (4 == g_rpn.stack.peek_integer(4)) );
  REQUIRE( (2 == g_rpn.stack.peek_integer(3)) );
  REQUIRE( (1 == g_rpn.stack.peek_integer(2)) );
  REQUIRE( (3 == g_rpn.stack.peek_integer(1)) );

  /*
    "ROTD"
  */
  line = ("CLEAR 10 9 8 7 6 5 4 3 2 1 ROTD");
  g_rpn.parse(line);
  REQUIRE( (10 == g_rpn.stack.peek_integer(10)) );
  REQUIRE( (9 == g_rpn.stack.peek_integer(9)) );
  REQUIRE( (8 == g_rpn.stack.peek_integer(8)) );
  REQUIRE( (7 == g_rpn.stack.peek_integer(7)) );
  REQUIRE( (6 == g_rpn.stack.peek_integer(6)) );
  REQUIRE( (5 == g_rpn.stack.peek_integer(5)) );
  REQUIRE( (4 == g_rpn.stack.peek_integer(4)) );
  REQUIRE( (1 == g_rpn.stack.peek_integer(3)) );
  REQUIRE( (3 == g_rpn.stack.peek_integer(2)) );
  REQUIRE( (2 == g_rpn.stack.peek_integer(1)) );

  /*
    "NIPN"
  */
  line = ("CLEAR 10 9 8 7 6 5 4 3 2 1 5 NIPN");
  g_rpn.parse(line);
  REQUIRE( 9 == g_rpn.stack.depth() );
  REQUIRE( (10 == g_rpn.stack.peek_integer(9)) );
  REQUIRE( (9 == g_rpn.stack.peek_integer(8)) );
  REQUIRE( (8 == g_rpn.stack.peek_integer(7)) );
  REQUIRE( (7 == g_rpn.stack.peek_integer(6)) );
  REQUIRE( (6 == g_rpn.stack.peek_integer(5)) );
  REQUIRE( (4 == g_rpn.stack.peek_integer(4)) );
  REQUIRE( (3 == g_rpn.stack.peek_integer(3)) );
  REQUIRE( (2 == g_rpn.stack.peek_integer(2)) );
  REQUIRE( (1 == g_rpn.stack.peek_integer(1)) );

  /*
    "PICK"
  */
  line = ("CLEAR 10 9 8 7 6 5 4 3 2 1 7 PICK");
  g_rpn.parse(line);
  REQUIRE( 11 == g_rpn.stack.depth() );
  REQUIRE( (10 == g_rpn.stack.peek_integer(11)) );
  REQUIRE( (9 == g_rpn.stack.peek_integer(10)) );
  REQUIRE( (8 == g_rpn.stack.peek_integer(9)) );
  REQUIRE( (7 == g_rpn.stack.peek_integer(8)) );
  REQUIRE( (6 == g_rpn.stack.peek_integer(7)) );
  REQUIRE( (5 == g_rpn.stack.peek_integer(6)) );
  REQUIRE( (4 == g_rpn.stack.peek_integer(5)) );
  REQUIRE( (3 == g_rpn.stack.peek_integer(4)) );
  REQUIRE( (2 == g_rpn.stack.peek_integer(3)) );
  REQUIRE( (1 == g_rpn.stack.peek_integer(2)) );
  REQUIRE( (7 == g_rpn.stack.peek_integer(1)) );

  /*
    "ROLLDN"
  */
  line = ("CLEAR 10 9 8 7 6 5 4 3 2 1 7 ROLLDN");
  g_rpn.parse(line);
  REQUIRE( 10 == g_rpn.stack.depth() );
  REQUIRE( (10 == g_rpn.stack.peek_integer(10)) );
  REQUIRE( (9 == g_rpn.stack.peek_integer(9)) );
  REQUIRE( (8 == g_rpn.stack.peek_integer(8)) );
  REQUIRE( (1 == g_rpn.stack.peek_integer(7)) );
  REQUIRE( (7 == g_rpn.stack.peek_integer(6)) );
  REQUIRE( (6 == g_rpn.stack.peek_integer(5)) );
  REQUIRE( (5 == g_rpn.stack.peek_integer(4)) );
  REQUIRE( (4 == g_rpn.stack.peek_integer(3)) );
  REQUIRE( (3 == g_rpn.stack.peek_integer(2)) );
  REQUIRE( (2 == g_rpn.stack.peek_integer(1)) );

  /*
    "ROLLUN"
  */
  line = ("CLEAR 10 9 8 7 6 5 4 3 2 1 5 ROLLUN");
  g_rpn.parse(line);
  REQUIRE( 10 == g_rpn.stack.depth() );
  REQUIRE( (10 == g_rpn.stack.peek_integer(10)) );
  REQUIRE( (9 == g_rpn.stack.peek_integer(9)) );
  REQUIRE( (8 == g_rpn.stack.peek_integer(8)) );
  REQUIRE( (7 == g_rpn.stack.peek_integer(7)) );
  REQUIRE( (6 == g_rpn.stack.peek_integer(6)) );
  REQUIRE( (4 == g_rpn.stack.peek_integer(5)) );
  REQUIRE( (3 == g_rpn.stack.peek_integer(4)) );
  REQUIRE( (2 == g_rpn.stack.peek_integer(3)) );
  REQUIRE( (1 == g_rpn.stack.peek_integer(2)) );
  REQUIRE( (5 == g_rpn.stack.peek_integer(1)) );

  /*
    "TUCKN"
  */
  line = ("CLEAR 10 9 8 7 6 5 4 3 2 1 5 TUCKN");
  g_rpn.parse(line);
  REQUIRE( 11 == g_rpn.stack.depth() );
  REQUIRE( (10 == g_rpn.stack.peek_integer(11)) );
  REQUIRE( (9 == g_rpn.stack.peek_integer(10)) );
  REQUIRE( (8 == g_rpn.stack.peek_integer(9)) );
  REQUIRE( (7 == g_rpn.stack.peek_integer(8)) );
  REQUIRE( (6 == g_rpn.stack.peek_integer(7)) );
  REQUIRE( (5 == g_rpn.stack.peek_integer(6)) );
  REQUIRE( (1 == g_rpn.stack.peek_integer(5)) );
  REQUIRE( (4 == g_rpn.stack.peek_integer(4)) );
  REQUIRE( (3 == g_rpn.stack.peek_integer(3)) );
  REQUIRE( (2 == g_rpn.stack.peek_integer(2)) );
  REQUIRE( (1 == g_rpn.stack.peek_integer(1)) );

  /*
    ".S"
  */
  
}

TEST_CASE( "== !=", " runtime logic" ) {
  std::string line("CLEAR 123 456 ==");
  g_rpn.parse(line);
  REQUIRE( (false == g_rpn.stack.peek_boolean(1) ) ); // integers equal fail

  line = ("CLEAR 123 456 !=");
  g_rpn.parse(line);
  REQUIRE( (true == g_rpn.stack.peek_boolean(1) ) ); // integers not equal

  line = ("CLEAR 1.0 1 ==");
  g_rpn.parse(line);
  REQUIRE( (false == g_rpn.stack.peek_boolean(1) ) ); // types don't match

  line = ("CLEAR .\" abc\" .\" xyz\" !=");
  g_rpn.parse(line);
  REQUIRE( (true == g_rpn.stack.peek_boolean(1) ) );  // strings not equal

  line = ("CLEAR .\" abc\" .\" abc\" =="); // strings equal
  g_rpn.parse(line);
  REQUIRE( (true == g_rpn.stack.peek_boolean(1) ) );

  line = ("CLEAR 3.14159 3.14159 =="); // doubles equal
  g_rpn.parse(line);
  REQUIRE( (true == g_rpn.stack.peek_boolean(1) ) );

  line = ("CLEAR 3.14159 3.14159 == DUP NOT =="); // boolean equal fail
  g_rpn.parse(line);
  REQUIRE( (false == g_rpn.stack.peek_boolean(1) ) );

  line = ("CLEAR 3.14159 3.14159 == DUP NOT !="); // boolean not-equal 
  g_rpn.parse(line);
  REQUIRE( (true == g_rpn.stack.peek_boolean(1) ) );

}

TEST_CASE( "inequalities - < > <= >=", " runtime logic" ) {
  std::string line;

  // doubles 
  line = ("CLEAR 3.14159 3.14159 >");
  g_rpn.parse(line);
  REQUIRE( (false == g_rpn.stack.peek_boolean(1) ) );

  line = ("CLEAR 3.14159 3.14159 >=");
  g_rpn.parse(line);
  REQUIRE( (true == g_rpn.stack.peek_boolean(1) ) );

  line = ("CLEAR 3.14159 2.14159 >=");
  g_rpn.parse(line);
  REQUIRE( (true == g_rpn.stack.peek_boolean(1) ) );

  line = ("CLEAR 3.14159 2.14159 .S > .S");
  g_rpn.parse(line);
  REQUIRE( (true == g_rpn.stack.peek_boolean(1) ) );

  line = ("CLEAR 3.14159 3.14159 <");
  g_rpn.parse(line);
  REQUIRE( (false == g_rpn.stack.peek_boolean(1) ) );

  line = ("CLEAR 3.14159 3.14159 <=");
  g_rpn.parse(line);
  REQUIRE( (true == g_rpn.stack.peek_boolean(1) ) );

  line = ("CLEAR 2.14159 3.14159 <=");
  g_rpn.parse(line);
  REQUIRE( (true == g_rpn.stack.peek_boolean(1) ) );

  line = ("CLEAR 2.13159 2.14159 <");
  g_rpn.parse(line);
  REQUIRE( (true == g_rpn.stack.peek_boolean(1) ) );

  // integers
  line = ("CLEAR 5 7 >");
  g_rpn.parse(line);
  REQUIRE( (false == g_rpn.stack.peek_boolean(1) ) );

  line = ("CLEAR 7 5 >=");
  g_rpn.parse(line);
  REQUIRE( (true == g_rpn.stack.peek_boolean(1) ) );

  line = ("CLEAR 7 7 >=");
  g_rpn.parse(line);
  REQUIRE( (true == g_rpn.stack.peek_boolean(1) ) );

  line = ("CLEAR 3 2 >=");
  g_rpn.parse(line);
  REQUIRE( (true == g_rpn.stack.peek_boolean(1) ) );

  line = ("CLEAR 3 2 >");
  g_rpn.parse(line);
  REQUIRE( (true == g_rpn.stack.peek_boolean(1) ) );

  line = ("CLEAR 3 3 <");
  g_rpn.parse(line);
  REQUIRE( (false == g_rpn.stack.peek_boolean(1) ) );

  line = ("CLEAR 3 3 <=");
  g_rpn.parse(line);
  REQUIRE( (true == g_rpn.stack.peek_boolean(1) ) );

  line = ("CLEAR 2 3 <=");
  g_rpn.parse(line);
  REQUIRE( (true == g_rpn.stack.peek_boolean(1) ) );

  line = ("CLEAR 2 2 <");
  g_rpn.parse(line);
  REQUIRE( (false == g_rpn.stack.peek_boolean(1) ) );

  line = ("CLEAR .\" abc\" .\" bcd\" <");
  g_rpn.parse(line);
  REQUIRE( (true == g_rpn.stack.peek_boolean(1) ) );

  line = ("CLEAR .\" abc\" 123 <");
  {
    auto st = g_rpn.parse(line);
    REQUIRE( (0 == g_rpn.stack.depth() ) );
    REQUIRE( (g_rpn.status() == "<: type error") );
    REQUIRE( (st == rpn::WordDefinition::Result::param_error) );
  }
  
}

TEST_CASE( "AND OR NOT" " boolean logic" ) {
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
