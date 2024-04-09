/***************************************************
 * file: qinc/rpn-lang/tests/runtime-test.cpp
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
#include "src/fraction.h"
#include "src/timecode.h"
#include <cmath>

rpn::Interp g_rpn;

TEST_CASE( "parse", "Stack Words" ) {

  /*
    "CLEAR"
   "OVER"
   "DROP"
   "DROPn"
  */

  std::string line;
  line = ("CLEAR 12.32 3 OVER DROP 1 2 3 4 5 6 2 DROPn");
  auto st = g_rpn.sync_eval(line);
  REQUIRE( (st == rpn::WordDefinition::Result::ok) );

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
  st = g_rpn.sync_eval(line);
  REQUIRE( (st == rpn::WordDefinition::Result::ok) );
  REQUIRE( 7 == g_rpn.stack.depth() );
  REQUIRE( ((4 == g_rpn.stack.peek_integer(1)) &&
	    (6 == g_rpn.stack.peek_integer(2))) );

  /* 
     "DUP"
  */
  line = ("9988 DUP DUP .S");
  st = g_rpn.sync_eval(line);
  REQUIRE( (st == rpn::WordDefinition::Result::ok) );
  REQUIRE( 10 == g_rpn.stack.depth() );
  REQUIRE( ((9988 == g_rpn.stack.peek_integer(1)) &&
	    (9988 == g_rpn.stack.peek_integer(2)) &&
	    (9988 == g_rpn.stack.peek_integer(3)) &&
	    (4 == g_rpn.stack.peek_integer(4))));

  line = ("3 DROPn");
  st = g_rpn.sync_eval(line);
  REQUIRE( (st == rpn::WordDefinition::Result::ok) );
  REQUIRE( 7 == g_rpn.stack.depth() );
  REQUIRE( ((4 == g_rpn.stack.peek_integer(1)) &&
	    (6 == g_rpn.stack.peek_integer(2))) );
  /*
    "DUPn"
  */
  line = ("3 DUPn");
  st = g_rpn.sync_eval(line);
  REQUIRE( (st == rpn::WordDefinition::Result::ok) );
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
  st = g_rpn.sync_eval(line);
  REQUIRE( (st == rpn::WordDefinition::Result::ok) );
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
  st = g_rpn.sync_eval(line);
  REQUIRE( (st == rpn::WordDefinition::Result::ok) );
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
  g_rpn.sync_eval(line);
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
  st = g_rpn.sync_eval(line);
  REQUIRE( (st == rpn::WordDefinition::Result::ok) );
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
  st = g_rpn.sync_eval(line);
  REQUIRE( (st == rpn::WordDefinition::Result::ok) );
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
  st = g_rpn.sync_eval(line);
  REQUIRE( (st == rpn::WordDefinition::Result::ok) );
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
  st = g_rpn.sync_eval(line);
  REQUIRE( (st == rpn::WordDefinition::Result::ok) );
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
  st = g_rpn.sync_eval(line);
  REQUIRE( (st == rpn::WordDefinition::Result::ok) );
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
  st = g_rpn.sync_eval(line);
  REQUIRE( (st == rpn::WordDefinition::Result::ok) );
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
    st = g_rpn.sync_eval(line);
    REQUIRE( (st == rpn::WordDefinition::Result::ok) );
    REQUIRE( 1 == g_rpn.stack.depth() );
    REQUIRE( ("abcdefg" == g_rpn.stack.peek_string(1)) );
    auto s2 = g_rpn.stack.pop_string();
    REQUIRE( ("abcdefg" == s2) );
  }

  /* REVERSE */
  {
    line = ("CLEAR 1 2 3 4 5 6 7 8 9 10 REVERSE");
    st = g_rpn.sync_eval(line);
    REQUIRE( (st == rpn::WordDefinition::Result::ok) );
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
    st = g_rpn.sync_eval(line);
    REQUIRE( (st == rpn::WordDefinition::Result::ok) );
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
  auto st = g_rpn.sync_eval(line);
  REQUIRE( (st == rpn::WordDefinition::Result::ok) );
  REQUIRE( (false == g_rpn.stack.peek_boolean(1) ) ); // integers equal fail

  line = ("CLEAR 123 456 !=");
  st = g_rpn.sync_eval(line);
  REQUIRE( (st == rpn::WordDefinition::Result::ok) );
  REQUIRE( (true == g_rpn.stack.peek_boolean(1) ) ); // integers not equal

  line = ("CLEAR 1.0 1 ==");
  st = g_rpn.sync_eval(line);
  REQUIRE( (st == rpn::WordDefinition::Result::ok) );
  REQUIRE( (false == g_rpn.stack.peek_boolean(1) ) ); // types don't match

  line = ("CLEAR .\" abc\" .\" xyz\" !=");
  st = g_rpn.sync_eval(line);
  REQUIRE( (st == rpn::WordDefinition::Result::ok) );
  REQUIRE( (true == g_rpn.stack.peek_boolean(1) ) );  // strings not equal

  line = ("CLEAR .\" abc\" .\" abc\" =="); // strings equal
  st = g_rpn.sync_eval(line);
  REQUIRE( (st == rpn::WordDefinition::Result::ok) );
  REQUIRE( (true == g_rpn.stack.peek_boolean(1) ) );

  line = ("CLEAR 3.14159 3.14159 =="); // doubles equal
  st = g_rpn.sync_eval(line);
  REQUIRE( (st == rpn::WordDefinition::Result::ok) );
  REQUIRE( (true == g_rpn.stack.peek_boolean(1) ) );

  line = ("CLEAR 3.14159 3.14159 == DUP NOT =="); // boolean equal fail
  st = g_rpn.sync_eval(line);
  REQUIRE( (st == rpn::WordDefinition::Result::ok) );
  REQUIRE( (false == g_rpn.stack.peek_boolean(1) ) );

  line = ("CLEAR 3.14159 3.14159 == DUP NOT !="); // boolean not-equal 
  st = g_rpn.sync_eval(line);
  REQUIRE( (st == rpn::WordDefinition::Result::ok) );
  REQUIRE( (true == g_rpn.stack.peek_boolean(1) ) );

}

TEST_CASE( "inequalities - < > <= >=", " runtime logic" ) {
  std::string line;

  // doubles 
  line = ("CLEAR 3.14159 3.14159 >");
  auto st = g_rpn.sync_eval(line);
  REQUIRE( (st == rpn::WordDefinition::Result::ok) );
  REQUIRE( (false == g_rpn.stack.peek_boolean(1) ) );

  line = ("CLEAR 3.14159 3.14159 >=");
  st = g_rpn.sync_eval(line);
  REQUIRE( (st == rpn::WordDefinition::Result::ok) );
  REQUIRE( (true == g_rpn.stack.peek_boolean(1) ) );

  line = ("CLEAR 3.14159 2.14159 >=");
  st = g_rpn.sync_eval(line);
  REQUIRE( (st == rpn::WordDefinition::Result::ok) );
  REQUIRE( (true == g_rpn.stack.peek_boolean(1) ) );

  line = ("CLEAR 3.14159 2.14159 .S > .S");
  st = g_rpn.sync_eval(line);
  REQUIRE( (st == rpn::WordDefinition::Result::ok) );
  REQUIRE( (true == g_rpn.stack.peek_boolean(1) ) );

  line = ("CLEAR 3.14159 3.14159 <");
  st = g_rpn.sync_eval(line);
  REQUIRE( (st == rpn::WordDefinition::Result::ok) );
  REQUIRE( (false == g_rpn.stack.peek_boolean(1) ) );

  line = ("CLEAR 3.14159 3.14159 <=");
  st = g_rpn.sync_eval(line);
  REQUIRE( (st == rpn::WordDefinition::Result::ok) );
  REQUIRE( (true == g_rpn.stack.peek_boolean(1) ) );

  line = ("CLEAR 2.14159 3.14159 <=");
  st = g_rpn.sync_eval(line);
  REQUIRE( (st == rpn::WordDefinition::Result::ok) );
  REQUIRE( (true == g_rpn.stack.peek_boolean(1) ) );

  line = ("CLEAR 2.13159 2.14159 <");
  st = g_rpn.sync_eval(line);
  REQUIRE( (st == rpn::WordDefinition::Result::ok) );
  REQUIRE( (true == g_rpn.stack.peek_boolean(1) ) );

  // integers
  line = ("CLEAR 5 7 >");
  st = g_rpn.sync_eval(line);
  REQUIRE( (st == rpn::WordDefinition::Result::ok) );
  REQUIRE( (false == g_rpn.stack.peek_boolean(1) ) );

  line = ("CLEAR 7 5 >=");
  st = g_rpn.sync_eval(line);
  REQUIRE( (st == rpn::WordDefinition::Result::ok) );
  REQUIRE( (true == g_rpn.stack.peek_boolean(1) ) );

  line = ("CLEAR 7 7 >=");
  st = g_rpn.sync_eval(line);
  REQUIRE( (st == rpn::WordDefinition::Result::ok) );
  REQUIRE( (true == g_rpn.stack.peek_boolean(1) ) );

  line = ("CLEAR 3 2 >=");
  st = g_rpn.sync_eval(line);
  REQUIRE( (st == rpn::WordDefinition::Result::ok) );
  REQUIRE( (true == g_rpn.stack.peek_boolean(1) ) );

  line = ("CLEAR 3 2 >");
  st = g_rpn.sync_eval(line);
  REQUIRE( (st == rpn::WordDefinition::Result::ok) );
  REQUIRE( (true == g_rpn.stack.peek_boolean(1) ) );

  line = ("CLEAR 3 3 <");
  st = g_rpn.sync_eval(line);
  REQUIRE( (st == rpn::WordDefinition::Result::ok) );
  REQUIRE( (false == g_rpn.stack.peek_boolean(1) ) );

  line = ("CLEAR 3 3 <=");
  st = g_rpn.sync_eval(line);
  REQUIRE( (st == rpn::WordDefinition::Result::ok) );
  REQUIRE( (true == g_rpn.stack.peek_boolean(1) ) );

  line = ("CLEAR 2 3 <=");
  st = g_rpn.sync_eval(line);
  REQUIRE( (st == rpn::WordDefinition::Result::ok) );
  REQUIRE( (true == g_rpn.stack.peek_boolean(1) ) );

  line = ("CLEAR 2 2 <");
  st = g_rpn.sync_eval(line);
  REQUIRE( (st == rpn::WordDefinition::Result::ok) );
  REQUIRE( (false == g_rpn.stack.peek_boolean(1) ) );

  line = ("CLEAR .\" abc\" .\" bcd\" <");
  st = g_rpn.sync_eval(line);
  REQUIRE( (st == rpn::WordDefinition::Result::ok) );
  REQUIRE( (true == g_rpn.stack.peek_boolean(1) ) );

  line = ("CLEAR .\" abc\" 123 <");
  {
    st = g_rpn.sync_eval(line);
    REQUIRE( (0 == g_rpn.stack.depth() ) );
    REQUIRE( (g_rpn.status() == "<: type error") );
    REQUIRE( (st == rpn::WordDefinition::Result::param_error) );
  }
  
}

TEST_CASE( "AND OR NOT" " boolean logic" ) {
  std::string line;

  line = ("CLEAR 1 1 == .S NOT .S");
  auto st = g_rpn.sync_eval(line);
  REQUIRE( (st == rpn::WordDefinition::Result::ok) );
  REQUIRE( (false == g_rpn.stack.peek_boolean(1) ) );

  line = ("1 1 == OR");
  st = g_rpn.sync_eval(line);
  REQUIRE( (st == rpn::WordDefinition::Result::ok) );
  REQUIRE( (true == g_rpn.stack.peek_boolean(1) ) );

  line = ("1 0 == AND");
  st = g_rpn.sync_eval(line);
  REQUIRE( (st == rpn::WordDefinition::Result::ok) );
  REQUIRE( (false == g_rpn.stack.peek_boolean(1) ) );

}

TEST_CASE( "AND OR NOT XOR" " binary logic" ) {
  std::string line;

  line = ("CLEAR 0x1234 0x4321 AND .S");
  auto st = g_rpn.sync_eval(line);
  REQUIRE( (st == rpn::WordDefinition::Result::ok) );
  REQUIRE( ((0x1234&0x4321) == g_rpn.stack.peek_integer(1) ) );

  line = ("0x9281 0xabcd OR .S");
  st = g_rpn.sync_eval(line);
  REQUIRE( (st == rpn::WordDefinition::Result::ok) );
  REQUIRE( ((0x9281 | 0xabcd) == g_rpn.stack.peek_integer(1) ) );

  line = ("0x55a8 0xaaaa XOR .S");
  st = g_rpn.sync_eval(line);
  REQUIRE( (st == rpn::WordDefinition::Result::ok) );
  REQUIRE( ((0x55a8 ^ 0xaaaa) == g_rpn.stack.peek_integer(1) ) );

  line = ("DUP NEG .S");
  st = g_rpn.sync_eval(line);
  REQUIRE( (st == rpn::WordDefinition::Result::ok) );
  REQUIRE( ((~(0x55a8 ^ 0xaaaa)) == g_rpn.stack.peek_integer(1) ) );

}

TEST_CASE( "file tests.rpn", "parsing" ) {
  std::string line;
  {
    line = ("( test bad comment");
    auto st = g_rpn.sync_eval(line);
    REQUIRE( (st == rpn::WordDefinition::Result::parse_error) );
  }

  {
    line = (".\" test bad string");
    auto st = g_rpn.sync_eval(line);
    REQUIRE( (st == rpn::WordDefinition::Result::parse_error) );
  }

  {
    line = (".\" inverabcdefg\" INV");
    auto st = g_rpn.sync_eval(line);
    REQUIRE( (st == rpn::WordDefinition::Result::param_error) );
  }

  {
    g_rpn.stack.clear();
    //    std::string file = "/Users/eric/work/qinc/rpn-lang/tests/tests.rpn";
    std::string file = "tests.rpn";
    auto st = g_rpn.sync_parseFile(file);
    g_rpn.stack.print("tests.rpn");

    REQUIRE( (st == rpn::WordDefinition::Result::ok) );
    REQUIRE( (18 == g_rpn.stack.depth() ) );

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
    auto st = g_rpn.sync_eval(line);

    REQUIRE( (st == rpn::WordDefinition::Result::ok) );
    REQUIRE( (2 == g_rpn.stack.depth() ) );

    REQUIRE( (3.  == g_rpn.stack.peek_double(2) ));
    REQUIRE( (4.  == g_rpn.stack.peek_double(1) ));
  }

  {
    g_rpn.stack.clear();
    line = ("k_PI k_E MIN k_PI k_E MAX");
    auto st = g_rpn.sync_eval(line);

    REQUIRE( (st == rpn::WordDefinition::Result::ok) );
    REQUIRE( (2 == g_rpn.stack.depth() ) );

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
    auto st = g_rpn.sync_eval(line);

    REQUIRE( (st == rpn::WordDefinition::Result::ok) );
    REQUIRE( (5 == g_rpn.stack.depth() ) );

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
    auto st = g_rpn.sync_eval(line);

    REQUIRE( (st == rpn::WordDefinition::Result::ok) );
    REQUIRE( (25 == g_rpn.stack.depth() ) );

    REQUIRE( (0. == g_rpn.stack.peek_double(25)) );
    REQUIRE( (1. == g_rpn.stack.peek_double(24)) );
    REQUIRE( (2. == g_rpn.stack.peek_double(23)) );
    REQUIRE( (3. == g_rpn.stack.peek_double(22)) );
    REQUIRE( (4. == g_rpn.stack.peek_double(21)) );
    REQUIRE( (10. == g_rpn.stack.peek_double(20)) );
    REQUIRE( (11. == g_rpn.stack.peek_double(19)) );
    REQUIRE( (12. == g_rpn.stack.peek_double(18)) );
    REQUIRE( (13. == g_rpn.stack.peek_double(17)) );
    REQUIRE( (14. == g_rpn.stack.peek_double(16)) );
    REQUIRE( (20. == g_rpn.stack.peek_double(15)) );
    REQUIRE( (21. == g_rpn.stack.peek_double(14)) );
    REQUIRE( (22. == g_rpn.stack.peek_double(13)) );
    REQUIRE( (23. == g_rpn.stack.peek_double(12)) );
    REQUIRE( (24. == g_rpn.stack.peek_double(11)) );
    REQUIRE( (30. == g_rpn.stack.peek_double(10)) );
    REQUIRE( (31. == g_rpn.stack.peek_double(9)) );
    REQUIRE( (32. == g_rpn.stack.peek_double(8)) );
    REQUIRE( (33. == g_rpn.stack.peek_double(7)) );
    REQUIRE( (34. == g_rpn.stack.peek_double(6)) );
    REQUIRE( (40. == g_rpn.stack.peek_double(5)) );
    REQUIRE( (41. == g_rpn.stack.peek_double(4)) );
    REQUIRE( (42. == g_rpn.stack.peek_double(3)) );
    REQUIRE( (43. == g_rpn.stack.peek_double(2)) );
    REQUIRE( (44. == g_rpn.stack.peek_double(1)) );
  }
  /*
    REQUIRE( (0. == g_rpn.stack.peek_double(9)) );
    REQUIRE( (0. == g_rpn.stack.peek_double(8)) );
    REQUIRE( (0. == g_rpn.stack.peek_double(7)) );
    REQUIRE( (0. == g_rpn.stack.peek_double(6)) );
    REQUIRE( (0. == g_rpn.stack.peek_double(5)) );
    REQUIRE( (0. == g_rpn.stack.peek_double(4)) );
    REQUIRE( (0. == g_rpn.stack.peek_double(3)) );
    REQUIRE( (0. == g_rpn.stack.peek_double(2)) );
    REQUIRE( (0. == g_rpn.stack.peek_double(1)) );
    REQUIRE( (0. == g_rpn.stack.peek_double(0)) );
  */
  {
    line = ("0 5 FOR i 0 5 FOR j  0 j FOR k i 100 * j 10 * + k + NEXT NEXT NEXT");
    g_rpn.stack.clear();
    auto st = g_rpn.sync_eval(line);
    g_rpn.stack.print("nested-for i,j,k");

    REQUIRE( (st == rpn::WordDefinition::Result::ok) );
    REQUIRE( (50 == g_rpn.stack.depth() ) );

    int index=0;
    for(int i=0; i<5; i++)
      for(int j=0; j<5; j++)
	for(int k=0; k<j; k++) {
	  double val = double((i*100)+(j*10)+k);
	  REQUIRE( (val == g_rpn.stack.peek_double(50-index)) );
      index++;
	}
    /*
    REQUIRE( (0. == g_rpn.stack.peek_double(48)) );
    REQUIRE( (0. == g_rpn.stack.peek_double(47)) );
    REQUIRE( (0. == g_rpn.stack.peek_double(46)) );
    REQUIRE( (0. == g_rpn.stack.peek_double(45)) );
    REQUIRE( (0. == g_rpn.stack.peek_double(44)) );
    REQUIRE( (0. == g_rpn.stack.peek_double(43)) );
    REQUIRE( (0. == g_rpn.stack.peek_double(42)) );
    REQUIRE( (0. == g_rpn.stack.peek_double(41)) );
    REQUIRE( (0. == g_rpn.stack.peek_double(40)) );

    REQUIRE( (0. == g_rpn.stack.peek_double(39)) );
    REQUIRE( (0. == g_rpn.stack.peek_double(38)) );
    REQUIRE( (0. == g_rpn.stack.peek_double(37)) );
    REQUIRE( (0. == g_rpn.stack.peek_double(36)) );
    REQUIRE( (0. == g_rpn.stack.peek_double(35)) );
    REQUIRE( (0. == g_rpn.stack.peek_double(34)) );
    REQUIRE( (0. == g_rpn.stack.peek_double(33)) );
    REQUIRE( (0. == g_rpn.stack.peek_double(32)) );
    REQUIRE( (0. == g_rpn.stack.peek_double(31)) );
    REQUIRE( (0. == g_rpn.stack.peek_double(30)) );

    REQUIRE( (0. == g_rpn.stack.peek_double(29)) );
    REQUIRE( (0. == g_rpn.stack.peek_double(28)) );
    REQUIRE( (0. == g_rpn.stack.peek_double(27)) );
    REQUIRE( (0. == g_rpn.stack.peek_double(26)) );
    REQUIRE( (0. == g_rpn.stack.peek_double(25)) );
    REQUIRE( (0. == g_rpn.stack.peek_double(24)) );
    REQUIRE( (0. == g_rpn.stack.peek_double(23)) );
    REQUIRE( (0. == g_rpn.stack.peek_double(22)) );
    REQUIRE( (0. == g_rpn.stack.peek_double(21)) );
    REQUIRE( (0. == g_rpn.stack.peek_double(20)) );

    REQUIRE( (0. == g_rpn.stack.peek_double(19)) );
    REQUIRE( (0. == g_rpn.stack.peek_double(18)) );
    REQUIRE( (0. == g_rpn.stack.peek_double(17)) );
    REQUIRE( (0. == g_rpn.stack.peek_double(16)) );
    REQUIRE( (0. == g_rpn.stack.peek_double(15)) );
    REQUIRE( (0. == g_rpn.stack.peek_double(14)) );
    REQUIRE( (0. == g_rpn.stack.peek_double(13)) );
    REQUIRE( (0. == g_rpn.stack.peek_double(12)) );
    REQUIRE( (0. == g_rpn.stack.peek_double(11)) );
    REQUIRE( (0. == g_rpn.stack.peek_double(10)) );

    REQUIRE( (0. == g_rpn.stack.peek_double(9)) );
    REQUIRE( (0. == g_rpn.stack.peek_double(8)) );
    REQUIRE( (0. == g_rpn.stack.peek_double(7)) );
    REQUIRE( (0. == g_rpn.stack.peek_double(6)) );
    REQUIRE( (0. == g_rpn.stack.peek_double(5)) );
    REQUIRE( (0. == g_rpn.stack.peek_double(4)) );
    REQUIRE( (0. == g_rpn.stack.peek_double(3)) );
    REQUIRE( (0. == g_rpn.stack.peek_double(2)) );
    REQUIRE( (0. == g_rpn.stack.peek_double(1)) );
    */
  }

  // loop in a define
  {
    line = (": abc-1 FOR i i 6 * NEXT ;  0 5 abc-1");
    g_rpn.stack.clear();
    auto st = g_rpn.sync_eval(line);
    g_rpn.stack.print("loop in defined word");

    REQUIRE( (st == rpn::WordDefinition::Result::ok) );
    REQUIRE( (5 == g_rpn.stack.depth() ) );

    REQUIRE( (0. == g_rpn.stack.peek_double(5)) );
    REQUIRE( (6. == g_rpn.stack.peek_double(4)) );
    REQUIRE( (12. == g_rpn.stack.peek_double(3)) );
    REQUIRE( (18. == g_rpn.stack.peek_double(2)) );
    REQUIRE( (24. == g_rpn.stack.peek_double(1)) );

  } 

  // nested loop in a define
  // this one is tricky... each time through the outer loop, the inner loop needs to
  // be properly set up (on the stack), but once the inner loop bounds have been popped
  // for the first run, there's no way to get them back for the subsequent iterations
  //
  // we will need progn local vars for some syntactic sugar
  {
    line = (": abc-2 FOR i DUP2 FOR j i 10 * j + 3 ROLLDn NEXT  NEXT DROP2 ;  0 5 0 4 abc-2");
    g_rpn.stack.clear();
    auto st = g_rpn.sync_eval(line);
    g_rpn.stack.print("nested-for (i,j) in defined word");

    REQUIRE( (st == rpn::WordDefinition::Result::ok) );
    REQUIRE( (20 == g_rpn.stack.depth() ) );

    REQUIRE( (0. == g_rpn.stack.peek_double(20)) );

    REQUIRE( (1. == g_rpn.stack.peek_double(19)) );
    REQUIRE( (2. == g_rpn.stack.peek_double(18)) );
    REQUIRE( (3. == g_rpn.stack.peek_double(17)) );
    REQUIRE( (4. == g_rpn.stack.peek_double(16)) );
    REQUIRE( (10. == g_rpn.stack.peek_double(15)) );
    REQUIRE( (11. == g_rpn.stack.peek_double(14)) );
    REQUIRE( (12. == g_rpn.stack.peek_double(13)) );
    REQUIRE( (13. == g_rpn.stack.peek_double(12)) );
    REQUIRE( (14. == g_rpn.stack.peek_double(11)) );
    REQUIRE( (20. == g_rpn.stack.peek_double(10)) );

    REQUIRE( (21. == g_rpn.stack.peek_double(9)) );
    REQUIRE( (22. == g_rpn.stack.peek_double(8)) );
    REQUIRE( (23. == g_rpn.stack.peek_double(7)) );
    REQUIRE( (24. == g_rpn.stack.peek_double(6)) );
    REQUIRE( (30. == g_rpn.stack.peek_double(5)) );
    REQUIRE( (31. == g_rpn.stack.peek_double(4)) );
    REQUIRE( (32. == g_rpn.stack.peek_double(3)) );
    REQUIRE( (33. == g_rpn.stack.peek_double(2)) );
    REQUIRE( (34. == g_rpn.stack.peek_double(1)) );

  }

  // recursion
  {
    g_rpn.stack.clear();
    line = (": sum-sq 0 SWAP 0 SWAP FOR i i i * + NEXT ;");
    auto st = g_rpn.sync_eval(line);
    REQUIRE( (st == rpn::WordDefinition::Result::ok) );

    line = ("<true> TRACE 10 sum-sq <false> TRACE .S");
    st = g_rpn.sync_eval(line);
    REQUIRE( (st == rpn::WordDefinition::Result::ok) );
    REQUIRE( (1 == g_rpn.stack.depth() ) );
    REQUIRE( (285 == g_rpn.stack.peek_double(1) ));

    line = (": word2 0 SWAP FOR i i 10 + sum-sq i + NEXT ;");
    /* 10 0 11 1 12 2 */
    st = g_rpn.sync_eval(line);
    REQUIRE( (st == rpn::WordDefinition::Result::ok) );

    line = ("3 word2");
    st = g_rpn.sync_eval(line);
    // 10 sum-sq 0 + (285)
    // 11 sum-sq 1 + (385)
    // 12 sum-sq 2 + (508)

    REQUIRE( (st == rpn::WordDefinition::Result::ok) );
    REQUIRE( (4 == g_rpn.stack.depth() ) );
    REQUIRE( (285 == g_rpn.stack.peek_double(4) ));
    REQUIRE( (285 == g_rpn.stack.peek_double(3) ));
    REQUIRE( (386 == g_rpn.stack.peek_double(2) ));
    REQUIRE( (508 == g_rpn.stack.peek_double(1) ));

    g_rpn.stack.print("recursive words with conflicting local names");
  }

#if 0
  // indefinite loop
  {
    line = ("5 DO DUP 1 - DUP DUP 0 == UNTIL");
    g_rpn.stack.clear();
    g_rpn.sync_eval(line);
  }

  // indefinite loop
  {
    line = ("5 3 DO DUP 1 - DUP DUP 0 == UNTIL");
    g_rpn.stack.clear();
    g_rpn.sync_eval(line);
  }
#endif

}

TEST_CASE( "bolt-circle", "control" ) {
  std::string line;

  g_rpn.stack.clear();
  /*
    std::string file = "/Users/eric/work/qinc/rpn-lang/tests/bolt-circle.rpn";
    auto st = g_rpn.parseFile(file);
  */
  line = R"(
: bolt-circle ( n diam phase -- < positions > )
0 4 PICK ( n diam phase 0 n )
FOR i ( n diam phase )
  360 i *  ( n diam phase angle2 )
  4 PICK / OVER + ( n diam phase angle2 )
  DUP COS 4 PICK 2 / *  ->VEC3x ( n diam phase angle2 x-loc )
  SWAP SIN 4 PICK 2 / * ->VEC3y + ( n diam phase xy-loc )
  4 ROLLDn ( xy-loc n diam phase )
NEXT
3 DROPn
;
)";
  auto st = g_rpn.sync_eval(line);
  REQUIRE( (st == rpn::WordDefinition::Result::ok) );
  REQUIRE( (0 == g_rpn.stack.depth() ) );

  line = ("5 5.5 0 bolt-circle");
  st = g_rpn.sync_eval(line);
  REQUIRE( (st == rpn::WordDefinition::Result::ok) );
  REQUIRE( (5 == g_rpn.stack.depth() ) );
  {
    const std::vector<std::pair<double,double>> positions = {
      { 0.849797, -2.615405 },
      { -2.224797, -1.616409 },
      { -2.224797, 1.616409 },
      { 0.849797, 2.615405 },
      { 2.750000, 0.000000 },
    };

    int i=0;
    for(const auto &p : positions) {
      auto &so = g_rpn.stack.peek(i+1);
      StVec3 &v3 = dynamic_cast<StVec3&>(so);
      REQUIRE_THAT(v3._x, Catch::Matchers::WithinAbs(p.first, 0.000001));
      REQUIRE_THAT(v3._y, Catch::Matchers::WithinAbs(p.second, 0.000001));
      i++;
    }
  }

  line = ("8 139.7 5 bolt-circle .S");
  st = g_rpn.sync_eval(line);
  REQUIRE( (st == rpn::WordDefinition::Result::ok) );
  REQUIRE( ((5 + 8) == g_rpn.stack.depth() ) );

  {
    const std::vector<std::pair<double,double>> positions = {
      { 53.508204, -44.898715 },
      { 6.087829, -69.584200 },
      { -44.898715, -53.508204 },
      { -69.584200, -6.087829 },
      { -53.508204, 44.898715 },
      { -6.087829, 69.584200 },
      { 44.898715, 53.508204 },
      { 69.584200, 6.087829 },
    };
    int i=0;
    for(const auto &p : positions) {
      auto &so = g_rpn.stack.peek(i+1);
      StVec3 &v3 = dynamic_cast<StVec3&>(so);
      REQUIRE_THAT(v3._x, Catch::Matchers::WithinAbs(p.first, 0.000001));
      REQUIRE_THAT(v3._y, Catch::Matchers::WithinAbs(p.second, 0.000001));
      i++;
    }
  }
}

#include "validator-tests.h"

static const std::map<std::size_t,std::string> sk_hashMap = {
  { typeid(StDouble).hash_code(), "Double" },
  { typeid(StInteger).hash_code(), "Integer" },
  { typeid(StBoolean).hash_code(), "Boolean" },
  { typeid(StString).hash_code(), "String" },
  { typeid(StObject).hash_code(), "Object" },
  { typeid(StArray).hash_code(), "Array" },
  { typeid(stack::Fraction).hash_code(), "Fraction" },
  { typeid(stack::Timecode).hash_code(), "Timecode" },
};

TEST_CASE("validators", "strict-type") {
  std::map<rpn::StrictTypeValidator,std::string> generators {
    { rpn::StrictTypeValidator::d1_double, "12.34" },
    { rpn::StrictTypeValidator::d1_integer, "1234" },
    { rpn::StrictTypeValidator::d1_boolean, "<true>" },
    { rpn::StrictTypeValidator::d1_object, "<true> .\" flag\" ->OBJ" },
    { rpn::StrictTypeValidator::d1_string, ".\" string 1\"" },
    { rpn::StrictTypeValidator::d1_array, "1. 2 .\" string\" 3 ->ARRAY" },
    { rpn::StrictTypeValidator::d1_vec3, "12.34 23.45 34.56 ->VEC3" },
    { rpn::StrictTypeValidator::d2_vec3_vec3, "1.2 2.3 3.4 ->VEC3 2.3 3.4 4.5 ->VEC3" },
    { rpn::StrictTypeValidator::d2_double_double, "3.1415 1." },
    { rpn::StrictTypeValidator::d2_double_integer, "2 3.1415" },
    { rpn::StrictTypeValidator::d2_integer_double, "12.345 6" },
    { rpn::StrictTypeValidator::d2_integer_integer, "2 3" },
    { rpn::StrictTypeValidator::d2_boolean_boolean, "<true> <false>" },
    { rpn::StrictTypeValidator::d2_vec3_double, "4.44 9.3 4.5 7.2 ->VEC3" },
    { rpn::StrictTypeValidator::d2_double_vec3, "9. 8. 7. ->VEC3 5.55" },
    { rpn::StrictTypeValidator::d2_vec3_integer, "7 4. 5. 6. ->VEC3" },
    { rpn::StrictTypeValidator::d2_integer_vec3, "8. 7. 6. ->VEC3 3" },
    { rpn::StrictTypeValidator::d2_array_any, "<true> <true> 1 ->ARRAY" },
    { rpn::StrictTypeValidator::d2_any_array, "1 2 3 3 ->ARRAY 4.5" },
    { rpn::StrictTypeValidator::d2_string_any, "<true> .\" flag\"" },
    { rpn::StrictTypeValidator::d2_any_string, ".\" abc\" <true> .\" flag\" ->OBJ" },
    { rpn::StrictTypeValidator::d2_object_any, "<true> .\" flag\" ->OBJ DUP" },
    { rpn::StrictTypeValidator::d2_any_object, "<true> .\" flag\" ->OBJ DUP" },
    { rpn::StrictTypeValidator::d3_double_double_double, "2.3 2.3 4." },
    { rpn::StrictTypeValidator::d3_integer_double_double, "3. 4. 2" },
    { rpn::StrictTypeValidator::d3_double_integer_double, "2. 3 4." },
    { rpn::StrictTypeValidator::d3_double_double_integer, "4 2. 3." },
    { rpn::StrictTypeValidator::d3_integer_integer_integer, "2 3 4" },
    { rpn::StrictTypeValidator::d3_double_integer_integer, "2 3 4." },
    { rpn::StrictTypeValidator::d3_integer_double_integer, "2 3. 4" },
    { rpn::StrictTypeValidator::d3_integer_integer_double, "2. 3 4" },
    { rpn::StrictTypeValidator::d3_any_any_boolean, "<true> 1 .\" string\"" }, 
    { rpn::StrictTypeValidator::d3_object_string_any, "99 .\" bottles\" 44 .\" xyz\" ->OBJ" },
    { rpn::StrictTypeValidator::d3_string_any_object, ".\" football\" .\" life\" ->OBJ 42 .\" meaning\"" },
    { rpn::StrictTypeValidator::d4_double_double_double_integer, "5 1.2 2.3 3.4" },
    { rpn::StrictTypeValidator::d4_integer_double_double_double, "2.2 3.3 4.4 5" },
    { timecode_validator::d1_tc, "60000 1001 ->FRAC 12345 ->TC" },
    { timecode_validator::d2_tc_tc, "60000 1001 ->FRAC 12345 ->TC 60000 1001 ->FRAC 145 ->TC" },
    { timecode_validator::d2_int_tc, "60000 1001 ->FRAC 4444 ->TC 17" },
    { timecode_validator::d2_tc_int, "120 60000 1001 ->FRAC 12345 ->TC" },
    { frac_validator::d1_frac, "2 3 ->FRAC" },
    { frac_validator::d2_frac_frac, "1 2 ->FRAC 0.75 ->FRAC" },
    { frac_validator::d2_frac_int, "7 1 9 ->FRAC" },
    { frac_validator::d2_frac_double, "3.6 0.9 ->FRAC" },
    { frac_validator::d2_int_frac, "0.1 ->FRAC 7" },
    { frac_validator::d2_double_frac, "0.2 ->FRAC 2.4" },
    { frac_validator::d5_int_int_int_int_frac, "60000 1001 ->FRAC 6 5 4 3" }
  };

  for(const auto &g : generators) {
    g_rpn.stack.clear();
    INFO("==> " << g.first.to_string() << " '" << g.second << "'");
    g_rpn.sync_eval(g.second);
    CHECK(g.first(g_rpn.stack.types(), g_rpn.stack) == true);
  }

  for(const auto &vt : sk_validatorTests) {
    const auto &g = generators.find(vt.first.second);
    REQUIRE (g != generators.end());
    g_rpn.stack.clear();
    g_rpn.sync_eval(g->second);
    INFO("==>(" << vt.first.first.to_string() << " / " << vt.first.second.to_string() << ") :=> '" << g->second <<"'");
    CHECK(vt.first.first(g_rpn.stack.types(), g_rpn.stack) == vt.second);
  }
  
}

TEST_CASE( "object", "types" ) {
  std::string line;
  {
    line = ("3.6 .\" abc\" ->OBJ 2.8 .\" def\" +");
    g_rpn.stack.clear();
    auto st = g_rpn.sync_eval(line);
    REQUIRE( (st == rpn::WordDefinition::Result::ok) );
  }
  
  {
    line = ("3.6");
    g_rpn.stack.clear();
    auto st = g_rpn.sync_eval(line);
    auto &so = g_rpn.stack.peek(1);
    REQUIRE_THROWS_AS( dynamic_cast<StObject&>(so),
		       std::bad_cast);
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

TEST_CASE( "Math", "operators") {
  std::string line;
  {
    line = ("3.6 4.3 +"); // float + float
    g_rpn.stack.clear();
    auto st = g_rpn.sync_eval(line);
    REQUIRE( (st == rpn::WordDefinition::Result::ok) );
    double dv = g_rpn.stack.peek_double(1);
    REQUIRE( (1 == g_rpn.stack.depth() ) );
    REQUIRE_THAT(dv, Catch::Matchers::WithinAbs((3.6 + 4.3), 0.000001));
  }
  
  {
    line = ("3 4.3 +"); // integer + float
    g_rpn.stack.clear();
    auto st = g_rpn.sync_eval(line);
    REQUIRE( (st == rpn::WordDefinition::Result::ok) );
    double dv = g_rpn.stack.peek_double(1);
    REQUIRE( (1 == g_rpn.stack.depth() ) );
    REQUIRE_THAT(dv, Catch::Matchers::WithinAbs((3 + 4.3), 0.000001));
  }

  {
    line = ("3.6 4 +"); // float + integer
    g_rpn.stack.clear();
    auto st = g_rpn.sync_eval(line);
    REQUIRE( (st == rpn::WordDefinition::Result::ok) );
    double dv = g_rpn.stack.peek_double(1);
    REQUIRE( (1 == g_rpn.stack.depth() ) );
    REQUIRE_THAT(dv, Catch::Matchers::WithinAbs((3.6 + 4), 0.000001));
  }

  {
    line = ("3 4 +"); // integer + integer
    g_rpn.stack.clear();
    auto st = g_rpn.sync_eval(line);
    REQUIRE( (st == rpn::WordDefinition::Result::ok) );
    int64_t iv = g_rpn.stack.peek_integer(1);
    REQUIRE( (1 == g_rpn.stack.depth() ) );
    REQUIRE_THAT(iv, Catch::Matchers::WithinAbs((3 + 4), 0.000001));
  }

}

/* end of qinc/rpn-lang/tests/runtime-test.cpp */
