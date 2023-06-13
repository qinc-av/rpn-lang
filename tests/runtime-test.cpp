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
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "rpn.h"

#include <cmath>

rpn::Runtime g_rpn;

TEST_CASE( "parse", "Stack Words" ) {

  /*
    "CLEAR"
   "OVER"
   "DROP"
   "DROPn"
  */

  std::string line;
  line = ("CLEAR 12.32 3 OVER DROP 1 2 3 4 5 6 2 DROPn");
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

  line = ("3 DROPn");
  g_rpn.parse(line);
  REQUIRE( 7 == g_rpn.stack.depth() );
  REQUIRE( ((4 == g_rpn.stack.peek_integer(1)) &&
	    (6 == g_rpn.stack.peek_integer(2))) );
  /*
    "DUPn"
  */
  line = ("3 DUPn");
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
    "NIPn"
  */
  line = ("CLEAR 10 9 8 7 6 5 4 3 2 1 5 NIPn");
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
    "ROLLDn"
  */
  line = ("CLEAR 10 9 8 7 6 5 4 3 2 1 7 ROLLDn");
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
    "ROLLUn"
  */
  line = ("CLEAR 10 9 8 7 6 5 4 3 2 1 5 ROLLUn");
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
    "TUCKn"
  */
  line = ("CLEAR 10 9 8 7 6 5 4 3 2 1 5 TUCKn");
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
  {
    line = ("CLEAR .\" abcdefg\"");
    g_rpn.parse(line);
    REQUIRE( 1 == g_rpn.stack.depth() );
    REQUIRE( ("abcdefg" == g_rpn.stack.peek_string(1)) );
    auto s2 = g_rpn.stack.pop_string();
    REQUIRE( ("abcdefg" == s2) );
  }

  /* REVERSE */
  {
    line = ("CLEAR 1 2 3 4 5 6 7 8 9 10 REVERSE");
    g_rpn.parse(line);
    REQUIRE( 10 == g_rpn.stack.depth() );
    REQUIRE( (10 == g_rpn.stack.peek_integer(10)) );
    REQUIRE( (9 == g_rpn.stack.peek_integer(9)) );
    REQUIRE( (8 == g_rpn.stack.peek_integer(8)) );
    REQUIRE( (7 == g_rpn.stack.peek_integer(7)) );
    REQUIRE( (6 == g_rpn.stack.peek_integer(6)) );
    REQUIRE( (5 == g_rpn.stack.peek_integer(5)) );
    REQUIRE( (4 == g_rpn.stack.peek_integer(4)) );
    REQUIRE( (3 == g_rpn.stack.peek_integer(3)) );
    REQUIRE( (2 == g_rpn.stack.peek_integer(2)) );
    REQUIRE( (1 == g_rpn.stack.peek_integer(1)) );
  }

  /* REVERSEn */
  {
    line = ("CLEAR 1 2 3 4 5 6 7 8 9 10 7 REVERSEn .S");
    g_rpn.parse(line);
    REQUIRE( 10 == g_rpn.stack.depth() );
    REQUIRE( (1 == g_rpn.stack.peek_integer(10)) );
    REQUIRE( (2 == g_rpn.stack.peek_integer(9)) );
    REQUIRE( (3 == g_rpn.stack.peek_integer(8)) );
    REQUIRE( (10 == g_rpn.stack.peek_integer(7)) );
    REQUIRE( (9 == g_rpn.stack.peek_integer(6)) );
    REQUIRE( (8 == g_rpn.stack.peek_integer(5)) );
    REQUIRE( (7 == g_rpn.stack.peek_integer(4)) );
    REQUIRE( (6 == g_rpn.stack.peek_integer(3)) );
    REQUIRE( (5 == g_rpn.stack.peek_integer(2)) );
    REQUIRE( (4 == g_rpn.stack.peek_integer(1)) );
  }
  
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

TEST_CASE( "file tests.4nc", "parsing" ) {
  std::string line;
  {
    line = ("( test bad comment");
    auto st = g_rpn.parse(line);
    REQUIRE( (st == rpn::WordDefinition::Result::parse_error) );
  }

  {
    line = (".\" test bad string");
    auto st = g_rpn.parse(line);
    REQUIRE( (st == rpn::WordDefinition::Result::parse_error) );
  }

  {
    line = (".\" inverabcdefg\" INV");
    auto st = g_rpn.parse(line);
    REQUIRE( (st == rpn::WordDefinition::Result::param_error) );
  }

  {
    g_rpn.stack.clear();
    std::string file = "/Users/eric/work/github/elh/rpn-cnc/tests.4nc";
    auto st = g_rpn.parseFile(file);
    g_rpn.stack.print("tests.4nc");
    REQUIRE( (st == rpn::WordDefinition::Result::ok) );

    REQUIRE_THAT(g_rpn.stack.peek_double(18), Catch::Matchers::WithinAbs(2463.008640, 0.000001));
    REQUIRE( (false == g_rpn.stack.peek_boolean(17) ));
    REQUIRE( (64 == g_rpn.stack.peek_integer(16) ));
    REQUIRE( (5  == g_rpn.stack.peek_integer(15) ));
    REQUIRE( (6  == g_rpn.stack.peek_integer(14) ));
    REQUIRE( (5  == g_rpn.stack.peek_integer(13) ));
    REQUIRE( (8  == g_rpn.stack.peek_integer(12) ));
    REQUIRE( (10.000000 == g_rpn.stack.peek_double(11) ));
    REQUIRE( (1.000000  == g_rpn.stack.peek_double(10) ));
    REQUIRE_THAT(g_rpn.stack.peek_double(9), Catch::Matchers::WithinAbs(0.046083, 0.000001));
    REQUIRE( ("test addition"  == g_rpn.stack.peek_string(8) ));
    REQUIRE( (6  == g_rpn.stack.peek_integer(7) ));
    REQUIRE( (6.500000  == g_rpn.stack.peek_double(6) ));
    REQUIRE( (5.200000  == g_rpn.stack.peek_double(5) ));
    REQUIRE( (9.700000  == g_rpn.stack.peek_double(4) ));
    REQUIRE( ("test subtraction" == g_rpn.stack.peek_string(3) ));
    REQUIRE( (-2  == g_rpn.stack.peek_integer(2) ));
    REQUIRE( (-9.000000  == g_rpn.stack.peek_double(1) ));
  }
}

TEST_CASE( "other tests", "math" ) {
   std::string line;
  {
    g_rpn.stack.clear();
    line = ("k_PI FLOOR k_PI CEIL");
    auto st = g_rpn.parse(line);
    REQUIRE( (st == rpn::WordDefinition::Result::ok) );
    REQUIRE( (3.  == g_rpn.stack.peek_double(2) ));
    REQUIRE( (4.  == g_rpn.stack.peek_double(1) ));
  }

  {
    g_rpn.stack.clear();
    line = ("k_PI k_E MIN k_PI k_E MAX");
    auto st = g_rpn.parse(line);
    REQUIRE( (st == rpn::WordDefinition::Result::ok) );
    REQUIRE_THAT(g_rpn.stack.peek_double(2), Catch::Matchers::WithinAbs(M_E, 0.000001));
    REQUIRE_THAT(g_rpn.stack.peek_double(1), Catch::Matchers::WithinAbs(M_PI, 0.000001));
  }
 
}

TEST_CASE( "loop tests", "control" ) {
  std::string line;
  // simple single for loop
  {
    line = ("0 5 FOR i i 10 * NEXT .S");
    g_rpn.stack.clear();
    g_rpn.parse(line);
    REQUIRE( (0. == g_rpn.stack.peek_double(5)) );
    REQUIRE( (10 == g_rpn.stack.peek_double(4)) );
    REQUIRE( (20. == g_rpn.stack.peek_double(3)) );
    REQUIRE( (30. == g_rpn.stack.peek_double(2)) );
    REQUIRE( (40. == g_rpn.stack.peek_double(1)) );
  }

  // nested for loops
  {
    line = ("0 5 FOR i 0 5 FOR j i 10 * j + NEXT NEXT .S");
    g_rpn.stack.clear();
    g_rpn.parse(line);
  }

  {
    line = ("0 5 FOR i 0 5 FOR j  0 j FOR k i 100 * j 10 * + k + NEXT NEXT NEXT");
    g_rpn.stack.clear();
    g_rpn.parse(line);
    g_rpn.stack.print("nested-for i,j,k");
  }

  // loop in a define
  {
    line = (": abc-1 FOR i i 6 * NEXT ;  0 5 abc-1");
    g_rpn.stack.clear();
    g_rpn.parse(line);
  }

  // nested loop in a define
  {
    line = (": abc-2 FOR i FOR j i 10 *  j + NEXT NEXT ;  0 5 0 4 abc-2");
    g_rpn.stack.clear();
    g_rpn.parse(line);
  }

  // indefinite loop
  {
    line = ("5 DO DUP 1 - DUP DUP 0 == UNTIL");
    g_rpn.stack.clear();
    g_rpn.parse(line);
  }

  // indefinite loop
  {
    line = ("5 3 DO DUP 1 - DUP DUP 0 == UNTIL");
    g_rpn.stack.clear();
    g_rpn.parse(line);
  }
  
}

TEST_CASE( "bolt-circle", "control" ) {
  std::string line;

  {
    g_rpn.stack.clear();
    std::string file = "/Users/eric/work/github/elh/rpn-cnc/bolt-circle.rpn";
    auto st = g_rpn.parseFile(file);

    line = ("5 5.5 0 bolt-circle");
    g_rpn.parse(line);

    line = ("5 139.7 0 bolt-circle");
    g_rpn.parse(line);

    g_rpn.stack.print("bolt-circle.rpn");
    REQUIRE( (st == rpn::WordDefinition::Result::ok) );
  }
}

TEST_CASE( "object", "types" ) {
  std::string line;
  {
    line = ("3.6 .\" abc\" ->OBJECT 2.8 \" def\" +");
    g_rpn.stack.clear();
    g_rpn.parse(line);
  }
}

TEST_CASE( "array", "types" ) {
}
TEST_CASE( "vec3", "types" ) {
}
TEST_CASE( "double", "types" ) {
}
TEST_CASE( "string", "types" ) {
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
