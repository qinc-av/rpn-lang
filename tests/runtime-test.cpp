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

#include <atomic>
#include <chrono>
#include <future>
#include <mutex>
#include <thread>

#include "../rpn.h"
#include "../rpn-matrix.h"
#include "../src/fraction.h"
#include "../src/timecode.h"
#include "../src/finance.h"
#include <cmath>

static rpn::Interp& g_rpn() { static rpn::Interp i(false); return i; }

TEST_CASE( "parse", "Stack Words" ) {

  /*
    "CLEAR"
   "OVER"
   "DROP"
   "DROPn"
  */

  std::string line;
  line = ("CLEAR 12.32 3 OVER DROP 1 2 3 4 5 6 2 DROPn");
  auto st = g_rpn().sync_eval(line);
  REQUIRE( (st == rpn::WordDefinition::Result::ok) );

  g_rpn().stack.print("test parse");
  // should be 12.32 3 1 2 3 4 "
  REQUIRE( ((4 == g_rpn().stack.peek_as_integer(1)) &&
	    (3 == g_rpn().stack.peek_as_integer(2)) &&
	    (2 == g_rpn().stack.peek_as_integer(3)) &&
	    (1 == g_rpn().stack.peek_as_integer(4)) &&
	    (3 == g_rpn().stack.peek_as_integer(5)) &&
	    (12.32 == g_rpn().stack.peek_double(6))) );
  REQUIRE( 6 == g_rpn().stack.depth() );

  /*
    "DEPTH"
    "SWAP"
  */
  line = ("DEPTH SWAP");
  st = g_rpn().sync_eval(line);
  REQUIRE( (st == rpn::WordDefinition::Result::ok) );
  REQUIRE( 7 == g_rpn().stack.depth() );
  REQUIRE( ((4 == g_rpn().stack.peek_as_integer(1)) &&
	    (6 == g_rpn().stack.peek_as_integer(2))) );

  /*
     "DUP"
  */
  line = ("9988 DUP DUP .S");
  st = g_rpn().sync_eval(line);
  REQUIRE( (st == rpn::WordDefinition::Result::ok) );
  REQUIRE( 10 == g_rpn().stack.depth() );
  REQUIRE( ((9988 == g_rpn().stack.peek_as_integer(1)) &&
	    (9988 == g_rpn().stack.peek_as_integer(2)) &&
	    (9988 == g_rpn().stack.peek_as_integer(3)) &&
	    (4 == g_rpn().stack.peek_as_integer(4))));

  line = ("3 DROPn");
  st = g_rpn().sync_eval(line);
  REQUIRE( (st == rpn::WordDefinition::Result::ok) );
  REQUIRE( 7 == g_rpn().stack.depth() );
  REQUIRE( ((4 == g_rpn().stack.peek_as_integer(1)) &&
	    (6 == g_rpn().stack.peek_as_integer(2))) );
  /*
    "DUPn"
  */
  line = ("3 DUPn");
  st = g_rpn().sync_eval(line);
  REQUIRE( (st == rpn::WordDefinition::Result::ok) );
  REQUIRE( 10 == g_rpn().stack.depth() );
  REQUIRE( ((4 == g_rpn().stack.peek_as_integer(1)) &&
	    (6 == g_rpn().stack.peek_as_integer(2)) &&
	    (3 == g_rpn().stack.peek_as_integer(3))) );
  REQUIRE( ((4 == g_rpn().stack.peek_as_integer(4)) &&
	    (6 == g_rpn().stack.peek_as_integer(5)) &&
	    (3 == g_rpn().stack.peek_as_integer(6))) );
  /*
    "ROLLU"
  */
  line = ("CLEAR 10 9 8 7 6 5 4 3 2 1 ROLLU ROLLU");
  st = g_rpn().sync_eval(line);
  REQUIRE( (st == rpn::WordDefinition::Result::ok) );
  REQUIRE( (8 == g_rpn().stack.peek_as_integer(10)) );
  REQUIRE( (7 == g_rpn().stack.peek_as_integer(9)) );
  REQUIRE( (6 == g_rpn().stack.peek_as_integer(8)) );
  REQUIRE( (5 == g_rpn().stack.peek_as_integer(7)) );
  REQUIRE( (4 == g_rpn().stack.peek_as_integer(6)) );
  REQUIRE( (3 == g_rpn().stack.peek_as_integer(5)) );
  REQUIRE( (2 == g_rpn().stack.peek_as_integer(4)) );
  REQUIRE( (1 == g_rpn().stack.peek_as_integer(3)) );
  REQUIRE( (10 == g_rpn().stack.peek_as_integer(2)) );
  REQUIRE( (9 == g_rpn().stack.peek_as_integer(1)) );

  /*
    "ROLLD"
  */
  line = ("CLEAR 10 9 8 7 6 5 4 3 2 1 ROLLD ROLLD");
  st = g_rpn().sync_eval(line);
  REQUIRE( (st == rpn::WordDefinition::Result::ok) );
  REQUIRE( (2 == g_rpn().stack.peek_as_integer(10)) );
  REQUIRE( (1 == g_rpn().stack.peek_as_integer(9)) );
  REQUIRE( (10 == g_rpn().stack.peek_as_integer(8)) );
  REQUIRE( (9 == g_rpn().stack.peek_as_integer(7)) );
  REQUIRE( (8 == g_rpn().stack.peek_as_integer(6)) );
  REQUIRE( (7 == g_rpn().stack.peek_as_integer(5)) );
  REQUIRE( (6 == g_rpn().stack.peek_as_integer(4)) );
  REQUIRE( (5 == g_rpn().stack.peek_as_integer(3)) );
  REQUIRE( (4 == g_rpn().stack.peek_as_integer(2)) );
  REQUIRE( (3 == g_rpn().stack.peek_as_integer(1)) );

  /*
    "ROTU"
  */
  line = ("CLEAR 10 9 8 7 6 5 4 3 2 1 ROTU");
  g_rpn().sync_eval(line);
  REQUIRE( (10 == g_rpn().stack.peek_as_integer(10)) );
  REQUIRE( (9 == g_rpn().stack.peek_as_integer(9)) );
  REQUIRE( (8 == g_rpn().stack.peek_as_integer(8)) );
  REQUIRE( (7 == g_rpn().stack.peek_as_integer(7)) );
  REQUIRE( (6 == g_rpn().stack.peek_as_integer(6)) );
  REQUIRE( (5 == g_rpn().stack.peek_as_integer(5)) );
  REQUIRE( (4 == g_rpn().stack.peek_as_integer(4)) );
  REQUIRE( (2 == g_rpn().stack.peek_as_integer(3)) );
  REQUIRE( (1 == g_rpn().stack.peek_as_integer(2)) );
  REQUIRE( (3 == g_rpn().stack.peek_as_integer(1)) );

  /*
    "ROTD"
  */
  line = ("CLEAR 10 9 8 7 6 5 4 3 2 1 ROTD");
  st = g_rpn().sync_eval(line);
  REQUIRE( (st == rpn::WordDefinition::Result::ok) );
  REQUIRE( (10 == g_rpn().stack.peek_as_integer(10)) );
  REQUIRE( (9 == g_rpn().stack.peek_as_integer(9)) );
  REQUIRE( (8 == g_rpn().stack.peek_as_integer(8)) );
  REQUIRE( (7 == g_rpn().stack.peek_as_integer(7)) );
  REQUIRE( (6 == g_rpn().stack.peek_as_integer(6)) );
  REQUIRE( (5 == g_rpn().stack.peek_as_integer(5)) );
  REQUIRE( (4 == g_rpn().stack.peek_as_integer(4)) );
  REQUIRE( (1 == g_rpn().stack.peek_as_integer(3)) );
  REQUIRE( (3 == g_rpn().stack.peek_as_integer(2)) );
  REQUIRE( (2 == g_rpn().stack.peek_as_integer(1)) );

  /*
    "NIPn"
  */
  line = ("CLEAR 10 9 8 7 6 5 4 3 2 1 5 NIPn");
  st = g_rpn().sync_eval(line);
  REQUIRE( (st == rpn::WordDefinition::Result::ok) );
  REQUIRE( 9 == g_rpn().stack.depth() );
  REQUIRE( (10 == g_rpn().stack.peek_as_integer(9)) );
  REQUIRE( (9 == g_rpn().stack.peek_as_integer(8)) );
  REQUIRE( (8 == g_rpn().stack.peek_as_integer(7)) );
  REQUIRE( (7 == g_rpn().stack.peek_as_integer(6)) );
  REQUIRE( (6 == g_rpn().stack.peek_as_integer(5)) );
  REQUIRE( (4 == g_rpn().stack.peek_as_integer(4)) );
  REQUIRE( (3 == g_rpn().stack.peek_as_integer(3)) );
  REQUIRE( (2 == g_rpn().stack.peek_as_integer(2)) );
  REQUIRE( (1 == g_rpn().stack.peek_as_integer(1)) );

  /*
    "PICK"
  */
  line = ("CLEAR 10 9 8 7 6 5 4 3 2 1 7 PICK");
  st = g_rpn().sync_eval(line);
  REQUIRE( (st == rpn::WordDefinition::Result::ok) );
  REQUIRE( 11 == g_rpn().stack.depth() );
  REQUIRE( (10 == g_rpn().stack.peek_as_integer(11)) );
  REQUIRE( (9 == g_rpn().stack.peek_as_integer(10)) );
  REQUIRE( (8 == g_rpn().stack.peek_as_integer(9)) );
  REQUIRE( (7 == g_rpn().stack.peek_as_integer(8)) );
  REQUIRE( (6 == g_rpn().stack.peek_as_integer(7)) );
  REQUIRE( (5 == g_rpn().stack.peek_as_integer(6)) );
  REQUIRE( (4 == g_rpn().stack.peek_as_integer(5)) );
  REQUIRE( (3 == g_rpn().stack.peek_as_integer(4)) );
  REQUIRE( (2 == g_rpn().stack.peek_as_integer(3)) );
  REQUIRE( (1 == g_rpn().stack.peek_as_integer(2)) );
  REQUIRE( (7 == g_rpn().stack.peek_as_integer(1)) );

  /*
    "ROLLDn"
  */
  line = ("CLEAR 10 9 8 7 6 5 4 3 2 1 7 ROLLDn");
  st = g_rpn().sync_eval(line);
  REQUIRE( (st == rpn::WordDefinition::Result::ok) );
  REQUIRE( 10 == g_rpn().stack.depth() );
  REQUIRE( (10 == g_rpn().stack.peek_as_integer(10)) );
  REQUIRE( (9 == g_rpn().stack.peek_as_integer(9)) );
  REQUIRE( (8 == g_rpn().stack.peek_as_integer(8)) );
  REQUIRE( (1 == g_rpn().stack.peek_as_integer(7)) );
  REQUIRE( (7 == g_rpn().stack.peek_as_integer(6)) );
  REQUIRE( (6 == g_rpn().stack.peek_as_integer(5)) );
  REQUIRE( (5 == g_rpn().stack.peek_as_integer(4)) );
  REQUIRE( (4 == g_rpn().stack.peek_as_integer(3)) );
  REQUIRE( (3 == g_rpn().stack.peek_as_integer(2)) );
  REQUIRE( (2 == g_rpn().stack.peek_as_integer(1)) );

  /*
    "ROLLUn"
  */
  line = ("CLEAR 10 9 8 7 6 5 4 3 2 1 5 ROLLUn");
  st = g_rpn().sync_eval(line);
  REQUIRE( (st == rpn::WordDefinition::Result::ok) );
  REQUIRE( 10 == g_rpn().stack.depth() );
  REQUIRE( (10 == g_rpn().stack.peek_as_integer(10)) );
  REQUIRE( (9 == g_rpn().stack.peek_as_integer(9)) );
  REQUIRE( (8 == g_rpn().stack.peek_as_integer(8)) );
  REQUIRE( (7 == g_rpn().stack.peek_as_integer(7)) );
  REQUIRE( (6 == g_rpn().stack.peek_as_integer(6)) );
  REQUIRE( (4 == g_rpn().stack.peek_as_integer(5)) );
  REQUIRE( (3 == g_rpn().stack.peek_as_integer(4)) );
  REQUIRE( (2 == g_rpn().stack.peek_as_integer(3)) );
  REQUIRE( (1 == g_rpn().stack.peek_as_integer(2)) );
  REQUIRE( (5 == g_rpn().stack.peek_as_integer(1)) );

  /*
    "TUCKn"
  */
  line = ("CLEAR 10 9 8 7 6 5 4 3 2 1 5 TUCKn");
  st = g_rpn().sync_eval(line);
  REQUIRE( (st == rpn::WordDefinition::Result::ok) );
  REQUIRE( 11 == g_rpn().stack.depth() );
  REQUIRE( (10 == g_rpn().stack.peek_as_integer(11)) );
  REQUIRE( (9 == g_rpn().stack.peek_as_integer(10)) );
  REQUIRE( (8 == g_rpn().stack.peek_as_integer(9)) );
  REQUIRE( (7 == g_rpn().stack.peek_as_integer(8)) );
  REQUIRE( (6 == g_rpn().stack.peek_as_integer(7)) );
  REQUIRE( (5 == g_rpn().stack.peek_as_integer(6)) );
  REQUIRE( (1 == g_rpn().stack.peek_as_integer(5)) );
  REQUIRE( (4 == g_rpn().stack.peek_as_integer(4)) );
  REQUIRE( (3 == g_rpn().stack.peek_as_integer(3)) );
  REQUIRE( (2 == g_rpn().stack.peek_as_integer(2)) );
  REQUIRE( (1 == g_rpn().stack.peek_as_integer(1)) );

  /*
    ".S"
  */
  {
    line = ("CLEAR .\" abcdefg\"");
    st = g_rpn().sync_eval(line);
    REQUIRE( (st == rpn::WordDefinition::Result::ok) );
    REQUIRE( 1 == g_rpn().stack.depth() );
    REQUIRE( ("abcdefg" == g_rpn().stack.peek_string(1)) );
    auto s2 = g_rpn().stack.pop_string();
    REQUIRE( ("abcdefg" == s2) );
  }

  /* REVERSE */
  {
    line = ("CLEAR 1 2 3 4 5 6 7 8 9 10 REVERSE");
    st = g_rpn().sync_eval(line);
    REQUIRE( (st == rpn::WordDefinition::Result::ok) );
    REQUIRE( 10 == g_rpn().stack.depth() );
    REQUIRE( (10 == g_rpn().stack.peek_as_integer(10)) );
    REQUIRE( (9 == g_rpn().stack.peek_as_integer(9)) );
    REQUIRE( (8 == g_rpn().stack.peek_as_integer(8)) );
    REQUIRE( (7 == g_rpn().stack.peek_as_integer(7)) );
    REQUIRE( (6 == g_rpn().stack.peek_as_integer(6)) );
    REQUIRE( (5 == g_rpn().stack.peek_as_integer(5)) );
    REQUIRE( (4 == g_rpn().stack.peek_as_integer(4)) );
    REQUIRE( (3 == g_rpn().stack.peek_as_integer(3)) );
    REQUIRE( (2 == g_rpn().stack.peek_as_integer(2)) );
    REQUIRE( (1 == g_rpn().stack.peek_as_integer(1)) );
  }

  /* REVERSEn */
  {
    line = ("CLEAR 1 2 3 4 5 6 7 8 9 10 7 REVERSEn .S");
    st = g_rpn().sync_eval(line);
    REQUIRE( (st == rpn::WordDefinition::Result::ok) );
    REQUIRE( 10 == g_rpn().stack.depth() );
    REQUIRE( (1 == g_rpn().stack.peek_as_integer(10)) );
    REQUIRE( (2 == g_rpn().stack.peek_as_integer(9)) );
    REQUIRE( (3 == g_rpn().stack.peek_as_integer(8)) );
    REQUIRE( (10 == g_rpn().stack.peek_as_integer(7)) );
    REQUIRE( (9 == g_rpn().stack.peek_as_integer(6)) );
    REQUIRE( (8 == g_rpn().stack.peek_as_integer(5)) );
    REQUIRE( (7 == g_rpn().stack.peek_as_integer(4)) );
    REQUIRE( (6 == g_rpn().stack.peek_as_integer(3)) );
    REQUIRE( (5 == g_rpn().stack.peek_as_integer(2)) );
    REQUIRE( (4 == g_rpn().stack.peek_as_integer(1)) );
  }

}

TEST_CASE( "numeric literal shapes", "parse" ) {
  // Leading-decimal literals (no integer part).  Previously the
  // tokenizer required an isdigit first character; tokens like
  // ".8" or "-.8" fell through to the dictionary and errored.
  std::string line = ("CLEAR .8 -.8 0.8 -0.8");
  auto st = g_rpn().sync_eval(line);
  REQUIRE( (st == rpn::WordDefinition::Result::ok) );
  REQUIRE( 4 == g_rpn().stack.depth() );
  REQUIRE_THAT( g_rpn().stack.peek_double(4), Catch::Matchers::WithinAbs( 0.8, 1e-12) );
  REQUIRE_THAT( g_rpn().stack.peek_double(3), Catch::Matchers::WithinAbs(-0.8, 1e-12) );
  REQUIRE_THAT( g_rpn().stack.peek_double(2), Catch::Matchers::WithinAbs( 0.8, 1e-12) );
  REQUIRE_THAT( g_rpn().stack.peek_double(1), Catch::Matchers::WithinAbs(-0.8, 1e-12) );

  // Same shapes inside a word definition (compile-time path).
  line = ("CLEAR : leading-decimal .8 -.8 ; leading-decimal");
  st = g_rpn().sync_eval(line);
  REQUIRE( (st == rpn::WordDefinition::Result::ok) );
  REQUIRE( 2 == g_rpn().stack.depth() );
  REQUIRE_THAT( g_rpn().stack.peek_double(2), Catch::Matchers::WithinAbs( 0.8, 1e-12) );
  REQUIRE_THAT( g_rpn().stack.peek_double(1), Catch::Matchers::WithinAbs(-0.8, 1e-12) );
}

TEST_CASE( "== !=", " runtime logic" ) {
  std::string line("CLEAR 123 456 ==");
  auto st = g_rpn().sync_eval(line);
  REQUIRE( (st == rpn::WordDefinition::Result::ok) );
  REQUIRE( (false == g_rpn().stack.peek_boolean(1) ) ); // integers equal fail

  line = ("CLEAR 123 456 !=");
  st = g_rpn().sync_eval(line);
  REQUIRE( (st == rpn::WordDefinition::Result::ok) );
  REQUIRE( (true == g_rpn().stack.peek_boolean(1) ) ); // integers not equal

  line = ("CLEAR 1.0 0x1 ==");
  st = g_rpn().sync_eval(line);
  REQUIRE( (st == rpn::WordDefinition::Result::ok) );
  REQUIRE( (false == g_rpn().stack.peek_boolean(1) ) ); // types don't match

  line = ("CLEAR .\" abc\" .\" xyz\" !=");
  st = g_rpn().sync_eval(line);
  REQUIRE( (st == rpn::WordDefinition::Result::ok) );
  REQUIRE( (true == g_rpn().stack.peek_boolean(1) ) );  // strings not equal

  line = ("CLEAR .\" abc\" .\" abc\" =="); // strings equal
  st = g_rpn().sync_eval(line);
  REQUIRE( (st == rpn::WordDefinition::Result::ok) );
  REQUIRE( (true == g_rpn().stack.peek_boolean(1) ) );

  line = ("CLEAR 3.14159 3.14159 =="); // doubles equal
  st = g_rpn().sync_eval(line);
  REQUIRE( (st == rpn::WordDefinition::Result::ok) );
  REQUIRE( (true == g_rpn().stack.peek_boolean(1) ) );

  line = ("CLEAR 3.14159 3.14159 == DUP NOT =="); // boolean equal fail
  st = g_rpn().sync_eval(line);
  REQUIRE( (st == rpn::WordDefinition::Result::ok) );
  REQUIRE( (false == g_rpn().stack.peek_boolean(1) ) );

  line = ("CLEAR 3.14159 3.14159 == DUP NOT !="); // boolean not-equal
  st = g_rpn().sync_eval(line);
  REQUIRE( (st == rpn::WordDefinition::Result::ok) );
  REQUIRE( (true == g_rpn().stack.peek_boolean(1) ) );

}

TEST_CASE( "inequalities - < > <= >=", " runtime logic" ) {
  std::string line;

  // doubles
  line = ("CLEAR 3.14159 3.14159 >");
  auto st = g_rpn().sync_eval(line);
  REQUIRE( (st == rpn::WordDefinition::Result::ok) );
  REQUIRE( (false == g_rpn().stack.peek_boolean(1) ) );

  line = ("CLEAR 3.14159 3.14159 >=");
  st = g_rpn().sync_eval(line);
  REQUIRE( (st == rpn::WordDefinition::Result::ok) );
  REQUIRE( (true == g_rpn().stack.peek_boolean(1) ) );

  line = ("CLEAR 3.14159 2.14159 >=");
  st = g_rpn().sync_eval(line);
  REQUIRE( (st == rpn::WordDefinition::Result::ok) );
  REQUIRE( (true == g_rpn().stack.peek_boolean(1) ) );

  line = ("CLEAR 3.14159 2.14159 .S > .S");
  st = g_rpn().sync_eval(line);
  REQUIRE( (st == rpn::WordDefinition::Result::ok) );
  REQUIRE( (true == g_rpn().stack.peek_boolean(1) ) );

  line = ("CLEAR 3.14159 3.14159 <");
  st = g_rpn().sync_eval(line);
  REQUIRE( (st == rpn::WordDefinition::Result::ok) );
  REQUIRE( (false == g_rpn().stack.peek_boolean(1) ) );

  line = ("CLEAR 3.14159 3.14159 <=");
  st = g_rpn().sync_eval(line);
  REQUIRE( (st == rpn::WordDefinition::Result::ok) );
  REQUIRE( (true == g_rpn().stack.peek_boolean(1) ) );

  line = ("CLEAR 2.14159 3.14159 <=");
  st = g_rpn().sync_eval(line);
  REQUIRE( (st == rpn::WordDefinition::Result::ok) );
  REQUIRE( (true == g_rpn().stack.peek_boolean(1) ) );

  line = ("CLEAR 2.13159 2.14159 <");
  st = g_rpn().sync_eval(line);
  REQUIRE( (st == rpn::WordDefinition::Result::ok) );
  REQUIRE( (true == g_rpn().stack.peek_boolean(1) ) );

  // integers
  line = ("CLEAR 5 7 >");
  st = g_rpn().sync_eval(line);
  REQUIRE( (st == rpn::WordDefinition::Result::ok) );
  REQUIRE( (false == g_rpn().stack.peek_boolean(1) ) );

  line = ("CLEAR 7 5 >=");
  st = g_rpn().sync_eval(line);
  REQUIRE( (st == rpn::WordDefinition::Result::ok) );
  REQUIRE( (true == g_rpn().stack.peek_boolean(1) ) );

  line = ("CLEAR 7 7 >=");
  st = g_rpn().sync_eval(line);
  REQUIRE( (st == rpn::WordDefinition::Result::ok) );
  REQUIRE( (true == g_rpn().stack.peek_boolean(1) ) );

  line = ("CLEAR 3 2 >=");
  st = g_rpn().sync_eval(line);
  REQUIRE( (st == rpn::WordDefinition::Result::ok) );
  REQUIRE( (true == g_rpn().stack.peek_boolean(1) ) );

  line = ("CLEAR 3 2 >");
  st = g_rpn().sync_eval(line);
  REQUIRE( (st == rpn::WordDefinition::Result::ok) );
  REQUIRE( (true == g_rpn().stack.peek_boolean(1) ) );

  line = ("CLEAR 3 3 <");
  st = g_rpn().sync_eval(line);
  REQUIRE( (st == rpn::WordDefinition::Result::ok) );
  REQUIRE( (false == g_rpn().stack.peek_boolean(1) ) );

  line = ("CLEAR 3 3 <=");
  st = g_rpn().sync_eval(line);
  REQUIRE( (st == rpn::WordDefinition::Result::ok) );
  REQUIRE( (true == g_rpn().stack.peek_boolean(1) ) );

  line = ("CLEAR 2 3 <=");
  st = g_rpn().sync_eval(line);
  REQUIRE( (st == rpn::WordDefinition::Result::ok) );
  REQUIRE( (true == g_rpn().stack.peek_boolean(1) ) );

  line = ("CLEAR 2 2 <");
  st = g_rpn().sync_eval(line);
  REQUIRE( (st == rpn::WordDefinition::Result::ok) );
  REQUIRE( (false == g_rpn().stack.peek_boolean(1) ) );

  line = ("CLEAR .\" abc\" .\" bcd\" <");
  st = g_rpn().sync_eval(line);
  REQUIRE( (st == rpn::WordDefinition::Result::ok) );
  REQUIRE( (true == g_rpn().stack.peek_boolean(1) ) );

  line = ("CLEAR .\" abc\" 123 <");
  {
    st = g_rpn().sync_eval(line);
    REQUIRE( (0 == g_rpn().stack.depth() ) );
    REQUIRE( (g_rpn().status() == "<: type error") );
    REQUIRE( (st == rpn::WordDefinition::Result::param_error) );
  }

}

TEST_CASE( "AND OR NOT" " boolean logic" ) {
  std::string line;

  line = ("CLEAR 1 1 == .S NOT .S");
  auto st = g_rpn().sync_eval(line);
  REQUIRE( (st == rpn::WordDefinition::Result::ok) );
  REQUIRE( (false == g_rpn().stack.peek_boolean(1) ) );

  line = ("1 1 == OR");
  st = g_rpn().sync_eval(line);
  REQUIRE( (st == rpn::WordDefinition::Result::ok) );
  REQUIRE( (true == g_rpn().stack.peek_boolean(1) ) );

  line = ("1 0 == AND");
  st = g_rpn().sync_eval(line);
  REQUIRE( (st == rpn::WordDefinition::Result::ok) );
  REQUIRE( (false == g_rpn().stack.peek_boolean(1) ) );

}

TEST_CASE( "AND OR NOT XOR" " binary logic" ) {
  std::string line;

  line = ("CLEAR 0x1234 0x4321 AND .S");
  auto st = g_rpn().sync_eval(line);
  REQUIRE( (st == rpn::WordDefinition::Result::ok) );
  REQUIRE( ((0x1234&0x4321) == g_rpn().stack.peek_as_integer(1) ) );

  line = ("0x9281 0xabcd OR .S");
  st = g_rpn().sync_eval(line);
  REQUIRE( (st == rpn::WordDefinition::Result::ok) );
  REQUIRE( ((0x9281 | 0xabcd) == g_rpn().stack.peek_as_integer(1) ) );

  line = ("0x55a8 0xaaaa XOR .S");
  st = g_rpn().sync_eval(line);
  REQUIRE( (st == rpn::WordDefinition::Result::ok) );
  REQUIRE( ((0x55a8 ^ 0xaaaa) == g_rpn().stack.peek_as_integer(1) ) );

  line = ("DUP NEG .S");
  st = g_rpn().sync_eval(line);
  REQUIRE( (st == rpn::WordDefinition::Result::ok) );
  REQUIRE( ((~(0x55a8 ^ 0xaaaa)) == g_rpn().stack.peek_as_integer(1) ) );

}

TEST_CASE("binary wordsize", "binary logic") {
  // Reset state at start of test case
  g_rpn().stack.clear();
  g_rpn().setBinaryWordsize(64);

  // WORDSIZE-> query
  {
    g_rpn().stack.clear();
    g_rpn().sync_eval("WORDSIZE->");
    REQUIRE( g_rpn().stack.peek_as_integer(1) == 64 );
  }

  // ->WORDSIZE set via stack word
  {
    g_rpn().stack.clear();
    g_rpn().sync_eval("8 ->WORDSIZE WORDSIZE->");
    REQUIRE( g_rpn().stack.peek_as_integer(1) == 8 );
    g_rpn().setBinaryWordsize(64);
  }

  // NEG (bitwise NOT) masked to 8 bits: ~0x05 & 0xFF = 0xFA
  {
    g_rpn().setBinaryWordsize(8);
    g_rpn().stack.clear();
    g_rpn().sync_eval("0x05 NEG");
    REQUIRE( g_rpn().stack.peek_as_integer(1) == 0xFA );
    g_rpn().setBinaryWordsize(64);
  }

  // AND masked to 8 bits
  {
    g_rpn().setBinaryWordsize(8);
    g_rpn().stack.clear();
    g_rpn().sync_eval("0x1FF 0xFF AND");
    REQUIRE( g_rpn().stack.peek_as_integer(1) == 0xFF );
    g_rpn().setBinaryWordsize(64);
  }

  // LSHIFT: 1 shifted left 4 bits = 16, with wordsize 8
  {
    g_rpn().setBinaryWordsize(8);
    g_rpn().stack.clear();
    g_rpn().sync_eval("0x01 4 LSHIFT");
    REQUIRE( g_rpn().stack.peek_as_integer(1) == 0x10 );
  }

  // LSHIFT overflow masked: 0x01 << 8 = 0x100, masked to 8 bits = 0x00
  {
    g_rpn().stack.clear();
    g_rpn().sync_eval("0x01 8 LSHIFT");
    REQUIRE( g_rpn().stack.peek_as_integer(1) == 0x00 );
    g_rpn().setBinaryWordsize(64);
  }

  // RSHIFT: 0x80 >> 3 = 0x10 (logical, unsigned)
  {
    g_rpn().setBinaryWordsize(8);
    g_rpn().stack.clear();
    g_rpn().sync_eval("0x80 3 RSHIFT");
    REQUIRE( g_rpn().stack.peek_as_integer(1) == 0x10 );
    g_rpn().setBinaryWordsize(64);
  }

  // RLEFT: 0x01 rotated left 1 in 8-bit = 0x02
  {
    g_rpn().setBinaryWordsize(8);
    g_rpn().stack.clear();
    g_rpn().sync_eval("0x01 1 RLEFT");
    REQUIRE( g_rpn().stack.peek_as_integer(1) == 0x02 );
  }

  // RLEFT: MSB wraps to LSB: 0x80 rotated left 1 in 8-bit = 0x01
  {
    g_rpn().stack.clear();
    g_rpn().sync_eval("0x80 1 RLEFT");
    REQUIRE( g_rpn().stack.peek_as_integer(1) == 0x01 );
    g_rpn().setBinaryWordsize(64);
  }

  // RRIGHT: 0x80 rotated right 1 in 8-bit = 0x40
  {
    g_rpn().setBinaryWordsize(8);
    g_rpn().stack.clear();
    g_rpn().sync_eval("0x80 1 RRIGHT");
    REQUIRE( g_rpn().stack.peek_as_integer(1) == 0x40 );
  }

  // RRIGHT: LSB wraps to MSB: 0x01 rotated right 1 in 8-bit = 0x80
  {
    g_rpn().stack.clear();
    g_rpn().sync_eval("0x01 1 RRIGHT");
    REQUIRE( g_rpn().stack.peek_as_integer(1) == 0x80 );
    g_rpn().setBinaryWordsize(64);
  }

  // RLEFT by 0 is identity
  {
    g_rpn().setBinaryWordsize(8);
    g_rpn().stack.clear();
    g_rpn().sync_eval("0xA5 0 RLEFT");
    REQUIRE( g_rpn().stack.peek_as_integer(1) == 0xA5 );
    g_rpn().setBinaryWordsize(64);
  }

  // STWS / RCWS HP48 aliases
  {
    g_rpn().stack.clear();
    g_rpn().sync_eval("16 STWS RCWS");
    REQUIRE( g_rpn().stack.peek_as_integer(1) == 16 );
    g_rpn().setBinaryWordsize(64);
  }

  // Clamping: wordsize < 1 → clamped to 1; > 64 → clamped to 64
  {
    g_rpn().setBinaryWordsize(0);
    REQUIRE( g_rpn().binaryWordsize() == 1 );
    g_rpn().setBinaryWordsize(100);
    REQUIRE( g_rpn().binaryWordsize() == 64 );
    g_rpn().setBinaryWordsize(64);
  }
}

TEST_CASE( "file tests.rpn", "parsing" ) {
  std::string line;
  {
    line = ("( test bad comment");
    auto st = g_rpn().sync_eval(line);
    REQUIRE( (st == rpn::WordDefinition::Result::parse_error) );
  }

  {
    line = (".\" test bad string");
    auto st = g_rpn().sync_eval(line);
    REQUIRE( (st == rpn::WordDefinition::Result::parse_error) );
  }

  {
    line = (".\" inverabcdefg\" INV");
    auto st = g_rpn().sync_eval(line);
    REQUIRE( (st == rpn::WordDefinition::Result::param_error) );
  }

  {
    g_rpn().stack.clear();
    std::string file = TESTS_DIR "/tests.rpn";
    auto st = g_rpn().sync_parseFile(file);
    g_rpn().stack.print("tests.rpn");

    REQUIRE( (st == rpn::WordDefinition::Result::ok) );
    REQUIRE( (18 == g_rpn().stack.depth() ) );

    REQUIRE_THAT(g_rpn().stack.peek_double(18), Catch::Matchers::WithinAbs(2463.008640, 0.000001));
    REQUIRE( (false == g_rpn().stack.peek_boolean(17) ));
    REQUIRE( (64 == g_rpn().stack.peek_as_integer(16) ));
    REQUIRE( (5  == g_rpn().stack.peek_as_integer(15) ));
    REQUIRE( (6  == g_rpn().stack.peek_as_integer(14) ));
    REQUIRE( (5  == g_rpn().stack.peek_as_integer(13) ));
    REQUIRE( (8  == g_rpn().stack.peek_as_integer(12) ));
    REQUIRE( (10.000000 == g_rpn().stack.peek_double(11) ));
    REQUIRE( (1.000000  == g_rpn().stack.peek_double(10) ));
    REQUIRE_THAT(g_rpn().stack.peek_as_double(9).value(), Catch::Matchers::WithinAbs(0.046083, 0.000001));
    REQUIRE( ("test addition"  == g_rpn().stack.peek_string(8) ));
    REQUIRE( (6  == g_rpn().stack.peek_as_integer(7) ));
    REQUIRE( (6.500000  == g_rpn().stack.peek_double(6) ));
    REQUIRE( (5.200000  == g_rpn().stack.peek_double(5) ));
    REQUIRE( (9.700000  == g_rpn().stack.peek_double(4) ));
    REQUIRE( ("test subtraction" == g_rpn().stack.peek_string(3) ));
    REQUIRE( (-2  == g_rpn().stack.peek_as_integer(2) ));
    REQUIRE( (-9.000000  == g_rpn().stack.peek_double(1) ));
  }
}

TEST_CASE( "other tests", "math" ) {
   std::string line;
  {
    g_rpn().stack.clear();
    line = ("k_PI FLOOR k_PI CEIL");
    auto st = g_rpn().sync_eval(line);

    REQUIRE( (st == rpn::WordDefinition::Result::ok) );
    REQUIRE( (2 == g_rpn().stack.depth() ) );

    REQUIRE( (3.  == g_rpn().stack.peek_double(2) ));
    REQUIRE( (4.  == g_rpn().stack.peek_double(1) ));
  }

  {
    g_rpn().stack.clear();
    line = ("k_PI k_E MIN k_PI k_E MAX");
    auto st = g_rpn().sync_eval(line);

    REQUIRE( (st == rpn::WordDefinition::Result::ok) );
    REQUIRE( (2 == g_rpn().stack.depth() ) );

    REQUIRE_THAT(g_rpn().stack.peek_double(2), Catch::Matchers::WithinAbs(M_E, 0.000001));
    REQUIRE_THAT(g_rpn().stack.peek_double(1), Catch::Matchers::WithinAbs(M_PI, 0.000001));
  }

}

TEST_CASE( "loop tests", "control" ) {
  std::string line;
  // simple single for loop
  {
    line = ("0 5 FOR i i 10 * NEXT .S");
    g_rpn().stack.clear();
    auto st = g_rpn().sync_eval(line);

    REQUIRE( (st == rpn::WordDefinition::Result::ok) );
    REQUIRE( (5 == g_rpn().stack.depth() ) );

    REQUIRE( (0. == g_rpn().stack.peek_double(5)) );
    REQUIRE( (10 == g_rpn().stack.peek_double(4)) );
    REQUIRE( (20. == g_rpn().stack.peek_double(3)) );
    REQUIRE( (30. == g_rpn().stack.peek_double(2)) );
    REQUIRE( (40. == g_rpn().stack.peek_double(1)) );
  }

  // nested for loops
  {
    line = ("0 5 FOR i 0 5 FOR j i 10 * j + NEXT NEXT .S");
    g_rpn().stack.clear();
    auto st = g_rpn().sync_eval(line);

    REQUIRE( (st == rpn::WordDefinition::Result::ok) );
    REQUIRE( (25 == g_rpn().stack.depth() ) );

    REQUIRE( (0. == g_rpn().stack.peek_double(25)) );
    REQUIRE( (1. == g_rpn().stack.peek_double(24)) );
    REQUIRE( (2. == g_rpn().stack.peek_double(23)) );
    REQUIRE( (3. == g_rpn().stack.peek_double(22)) );
    REQUIRE( (4. == g_rpn().stack.peek_double(21)) );
    REQUIRE( (10. == g_rpn().stack.peek_double(20)) );
    REQUIRE( (11. == g_rpn().stack.peek_double(19)) );
    REQUIRE( (12. == g_rpn().stack.peek_double(18)) );
    REQUIRE( (13. == g_rpn().stack.peek_double(17)) );
    REQUIRE( (14. == g_rpn().stack.peek_double(16)) );
    REQUIRE( (20. == g_rpn().stack.peek_double(15)) );
    REQUIRE( (21. == g_rpn().stack.peek_double(14)) );
    REQUIRE( (22. == g_rpn().stack.peek_double(13)) );
    REQUIRE( (23. == g_rpn().stack.peek_double(12)) );
    REQUIRE( (24. == g_rpn().stack.peek_double(11)) );
    REQUIRE( (30. == g_rpn().stack.peek_double(10)) );
    REQUIRE( (31. == g_rpn().stack.peek_double(9)) );
    REQUIRE( (32. == g_rpn().stack.peek_double(8)) );
    REQUIRE( (33. == g_rpn().stack.peek_double(7)) );
    REQUIRE( (34. == g_rpn().stack.peek_double(6)) );
    REQUIRE( (40. == g_rpn().stack.peek_double(5)) );
    REQUIRE( (41. == g_rpn().stack.peek_double(4)) );
    REQUIRE( (42. == g_rpn().stack.peek_double(3)) );
    REQUIRE( (43. == g_rpn().stack.peek_double(2)) );
    REQUIRE( (44. == g_rpn().stack.peek_double(1)) );
  }
  /*
    REQUIRE( (0. == g_rpn().stack.peek_double(9)) );
    REQUIRE( (0. == g_rpn().stack.peek_double(8)) );
    REQUIRE( (0. == g_rpn().stack.peek_double(7)) );
    REQUIRE( (0. == g_rpn().stack.peek_double(6)) );
    REQUIRE( (0. == g_rpn().stack.peek_double(5)) );
    REQUIRE( (0. == g_rpn().stack.peek_double(4)) );
    REQUIRE( (0. == g_rpn().stack.peek_double(3)) );
    REQUIRE( (0. == g_rpn().stack.peek_double(2)) );
    REQUIRE( (0. == g_rpn().stack.peek_double(1)) );
    REQUIRE( (0. == g_rpn().stack.peek_double(0)) );
  */
  {
    line = ("0 5 FOR i 0 5 FOR j  0 j FOR k i 100 * j 10 * + k + NEXT NEXT NEXT");
    g_rpn().stack.clear();
    auto st = g_rpn().sync_eval(line);
    g_rpn().stack.print("nested-for i,j,k");

    REQUIRE( (st == rpn::WordDefinition::Result::ok) );
    REQUIRE( (50 == g_rpn().stack.depth() ) );

    int index=0;
    for(int i=0; i<5; i++)
      for(int j=0; j<5; j++)
	for(int k=0; k<j; k++) {
	  double val = double((i*100)+(j*10)+k);
	  REQUIRE( (val == g_rpn().stack.peek_double(50-index)) );
      index++;
	}
    /*
    REQUIRE( (0. == g_rpn().stack.peek_double(48)) );
    REQUIRE( (0. == g_rpn().stack.peek_double(47)) );
    REQUIRE( (0. == g_rpn().stack.peek_double(46)) );
    REQUIRE( (0. == g_rpn().stack.peek_double(45)) );
    REQUIRE( (0. == g_rpn().stack.peek_double(44)) );
    REQUIRE( (0. == g_rpn().stack.peek_double(43)) );
    REQUIRE( (0. == g_rpn().stack.peek_double(42)) );
    REQUIRE( (0. == g_rpn().stack.peek_double(41)) );
    REQUIRE( (0. == g_rpn().stack.peek_double(40)) );

    REQUIRE( (0. == g_rpn().stack.peek_double(39)) );
    REQUIRE( (0. == g_rpn().stack.peek_double(38)) );
    REQUIRE( (0. == g_rpn().stack.peek_double(37)) );
    REQUIRE( (0. == g_rpn().stack.peek_double(36)) );
    REQUIRE( (0. == g_rpn().stack.peek_double(35)) );
    REQUIRE( (0. == g_rpn().stack.peek_double(34)) );
    REQUIRE( (0. == g_rpn().stack.peek_double(33)) );
    REQUIRE( (0. == g_rpn().stack.peek_double(32)) );
    REQUIRE( (0. == g_rpn().stack.peek_double(31)) );
    REQUIRE( (0. == g_rpn().stack.peek_double(30)) );

    REQUIRE( (0. == g_rpn().stack.peek_double(29)) );
    REQUIRE( (0. == g_rpn().stack.peek_double(28)) );
    REQUIRE( (0. == g_rpn().stack.peek_double(27)) );
    REQUIRE( (0. == g_rpn().stack.peek_double(26)) );
    REQUIRE( (0. == g_rpn().stack.peek_double(25)) );
    REQUIRE( (0. == g_rpn().stack.peek_double(24)) );
    REQUIRE( (0. == g_rpn().stack.peek_double(23)) );
    REQUIRE( (0. == g_rpn().stack.peek_double(22)) );
    REQUIRE( (0. == g_rpn().stack.peek_double(21)) );
    REQUIRE( (0. == g_rpn().stack.peek_double(20)) );

    REQUIRE( (0. == g_rpn().stack.peek_double(19)) );
    REQUIRE( (0. == g_rpn().stack.peek_double(18)) );
    REQUIRE( (0. == g_rpn().stack.peek_double(17)) );
    REQUIRE( (0. == g_rpn().stack.peek_double(16)) );
    REQUIRE( (0. == g_rpn().stack.peek_double(15)) );
    REQUIRE( (0. == g_rpn().stack.peek_double(14)) );
    REQUIRE( (0. == g_rpn().stack.peek_double(13)) );
    REQUIRE( (0. == g_rpn().stack.peek_double(12)) );
    REQUIRE( (0. == g_rpn().stack.peek_double(11)) );
    REQUIRE( (0. == g_rpn().stack.peek_double(10)) );

    REQUIRE( (0. == g_rpn().stack.peek_double(9)) );
    REQUIRE( (0. == g_rpn().stack.peek_double(8)) );
    REQUIRE( (0. == g_rpn().stack.peek_double(7)) );
    REQUIRE( (0. == g_rpn().stack.peek_double(6)) );
    REQUIRE( (0. == g_rpn().stack.peek_double(5)) );
    REQUIRE( (0. == g_rpn().stack.peek_double(4)) );
    REQUIRE( (0. == g_rpn().stack.peek_double(3)) );
    REQUIRE( (0. == g_rpn().stack.peek_double(2)) );
    REQUIRE( (0. == g_rpn().stack.peek_double(1)) );
    */
  }

  // loop in a define
  {
    line = (": abc-1 FOR i i 6 * NEXT ;  0 5 abc-1");
    g_rpn().stack.clear();
    auto st = g_rpn().sync_eval(line);
    g_rpn().stack.print("loop in defined word");

    REQUIRE( (st == rpn::WordDefinition::Result::ok) );
    REQUIRE( (5 == g_rpn().stack.depth() ) );

    REQUIRE( (0. == g_rpn().stack.peek_double(5)) );
    REQUIRE( (6. == g_rpn().stack.peek_double(4)) );
    REQUIRE( (12. == g_rpn().stack.peek_double(3)) );
    REQUIRE( (18. == g_rpn().stack.peek_double(2)) );
    REQUIRE( (24. == g_rpn().stack.peek_double(1)) );

  }

  // nested loop in a define
  // this one is tricky... each time through the outer loop, the inner loop needs to
  // be properly set up (on the stack), but once the inner loop bounds have been popped
  // for the first run, there's no way to get them back for the subsequent iterations
  //
  // we will need progn local vars for some syntactic sugar
  {
    line = (": abc-2 FOR i DUP2 FOR j i 10 * j + 3 ROLLDn NEXT  NEXT DROP2 ;  0 5 0 4 abc-2");
    g_rpn().stack.clear();
    auto st = g_rpn().sync_eval(line);
    g_rpn().stack.print("nested-for (i,j) in defined word");

    REQUIRE( (st == rpn::WordDefinition::Result::ok) );
    REQUIRE( (20 == g_rpn().stack.depth() ) );

    REQUIRE( (0. == g_rpn().stack.peek_double(20)) );

    REQUIRE( (1. == g_rpn().stack.peek_double(19)) );
    REQUIRE( (2. == g_rpn().stack.peek_double(18)) );
    REQUIRE( (3. == g_rpn().stack.peek_double(17)) );
    REQUIRE( (4. == g_rpn().stack.peek_double(16)) );
    REQUIRE( (10. == g_rpn().stack.peek_double(15)) );
    REQUIRE( (11. == g_rpn().stack.peek_double(14)) );
    REQUIRE( (12. == g_rpn().stack.peek_double(13)) );
    REQUIRE( (13. == g_rpn().stack.peek_double(12)) );
    REQUIRE( (14. == g_rpn().stack.peek_double(11)) );
    REQUIRE( (20. == g_rpn().stack.peek_double(10)) );

    REQUIRE( (21. == g_rpn().stack.peek_double(9)) );
    REQUIRE( (22. == g_rpn().stack.peek_double(8)) );
    REQUIRE( (23. == g_rpn().stack.peek_double(7)) );
    REQUIRE( (24. == g_rpn().stack.peek_double(6)) );
    REQUIRE( (30. == g_rpn().stack.peek_double(5)) );
    REQUIRE( (31. == g_rpn().stack.peek_double(4)) );
    REQUIRE( (32. == g_rpn().stack.peek_double(3)) );
    REQUIRE( (33. == g_rpn().stack.peek_double(2)) );
    REQUIRE( (34. == g_rpn().stack.peek_double(1)) );

  }

  // recursion
  {
    g_rpn().stack.clear();
    line = (": sum-sq 0 SWAP 0 SWAP FOR i i i * + NEXT ;");
    auto st = g_rpn().sync_eval(line);
    REQUIRE( (st == rpn::WordDefinition::Result::ok) );

    line = ("<true> TRACE 10 sum-sq <false> TRACE .S");
    st = g_rpn().sync_eval(line);
    REQUIRE( (st == rpn::WordDefinition::Result::ok) );
    REQUIRE( (1 == g_rpn().stack.depth() ) );
    REQUIRE( (285 == g_rpn().stack.peek_double(1) ));

    line = (": word2 0 SWAP FOR i i 10 + sum-sq i + NEXT ;");
    /* 10 0 11 1 12 2 */
    st = g_rpn().sync_eval(line);
    REQUIRE( (st == rpn::WordDefinition::Result::ok) );

    line = ("3 word2");
    st = g_rpn().sync_eval(line);
    // 10 sum-sq 0 + (285)
    // 11 sum-sq 1 + (385)
    // 12 sum-sq 2 + (508)

    REQUIRE( (st == rpn::WordDefinition::Result::ok) );
    REQUIRE( (4 == g_rpn().stack.depth() ) );
    REQUIRE( (285 == g_rpn().stack.peek_double(4) ));
    REQUIRE( (285 == g_rpn().stack.peek_double(3) ));
    REQUIRE( (386 == g_rpn().stack.peek_double(2) ));
    REQUIRE( (508 == g_rpn().stack.peek_double(1) ));

    g_rpn().stack.print("recursive words with conflicting local names");
  }

#if 0
  // indefinite loop
  {
    line = ("5 DO DUP 1 - DUP DUP 0 == UNTIL");
    g_rpn().stack.clear();
    g_rpn().sync_eval(line);
  }

  // indefinite loop
  {
    line = ("5 3 DO DUP 1 - DUP DUP 0 == UNTIL");
    g_rpn().stack.clear();
    g_rpn().sync_eval(line);
  }
#endif

  // FOR ... n STEP: body leaves n on TOS each iteration; STEP pops it as step.
  // "0. 6. FOR i i 2. STEP" → i=0,2,4 (body pushes i then 2.; STEP pops 2.)
  {
    g_rpn().stack.clear();
    auto st = g_rpn().sync_eval("0. 6. FOR i i 2. STEP");
    REQUIRE( st == rpn::WordDefinition::Result::ok );
    REQUIRE( g_rpn().stack.depth() == 3 );
    REQUIRE( g_rpn().stack.peek_double(3) == 0. );
    REQUIRE( g_rpn().stack.peek_double(2) == 2. );
    REQUIRE( g_rpn().stack.peek_double(1) == 4. );
  }

  // Countdown: "5. 0. FOR i i -1. STEP" → i=5,4,3,2,1
  {
    g_rpn().stack.clear();
    auto st = g_rpn().sync_eval("5. 0. FOR i i -1. STEP");
    REQUIRE( st == rpn::WordDefinition::Result::ok );
    REQUIRE( g_rpn().stack.depth() == 5 );
    REQUIRE( g_rpn().stack.peek_double(5) == 5. );
    REQUIRE( g_rpn().stack.peek_double(4) == 4. );
    REQUIRE( g_rpn().stack.peek_double(3) == 3. );
    REQUIRE( g_rpn().stack.peek_double(2) == 2. );
    REQUIRE( g_rpn().stack.peek_double(1) == 1. );
  }

}

TEST_CASE( "bolt-circle", "control" ) {
  std::string line;

  g_rpn().stack.clear();
  /*
    std::string file = "/Users/eric/work/qinc/rpn-lang/tests/bolt-circle.rpn";
    auto st = g_rpn().parseFile(file);
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
  auto st = g_rpn().sync_eval(line);
  REQUIRE( (st == rpn::WordDefinition::Result::ok) );
  REQUIRE( (0 == g_rpn().stack.depth() ) );

  line = ("5 5.5 0 bolt-circle");
  st = g_rpn().sync_eval(line);
  REQUIRE( (st == rpn::WordDefinition::Result::ok) );
  REQUIRE( (5 == g_rpn().stack.depth() ) );
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
      auto &so = g_rpn().stack.peek(i+1);
      const stack::Vec3 &v3 = dynamic_cast<const stack::Vec3&>(so);
      REQUIRE_THAT(v3[0], Catch::Matchers::WithinAbs(p.first, 0.000001));
      REQUIRE_THAT(v3[1], Catch::Matchers::WithinAbs(p.second, 0.000001));
      i++;
    }
  }

  line = ("8 139.7 5 bolt-circle .S");
  st = g_rpn().sync_eval(line);
  REQUIRE( (st == rpn::WordDefinition::Result::ok) );
  REQUIRE( ((5 + 8) == g_rpn().stack.depth() ) );

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
      auto &so = g_rpn().stack.peek(i+1);
      const stack::Vec3 &v3 = dynamic_cast<const stack::Vec3&>(so);
      REQUIRE_THAT(v3[0], Catch::Matchers::WithinAbs(p.first, 0.000001));
      REQUIRE_THAT(v3[1], Catch::Matchers::WithinAbs(p.second, 0.000001));
      i++;
    }
  }
}

#include "validator-tests.h"

static const std::map<std::size_t,std::string> sk_hashMap = {
  { typeid(stack::Double).hash_code(), "Double" },
  { typeid(stack::Integer).hash_code(), "Integer" },
  { typeid(stack::Boolean).hash_code(), "Boolean" },
  { typeid(stack::String).hash_code(), "String" },
  { typeid(stack::Object).hash_code(), "Object" },
  { typeid(stack::Array).hash_code(), "Array" },
  { typeid(stack::Fraction).hash_code(), "Fraction" },
  { typeid(stack::Timecode).hash_code(), "Timecode" },
};

TEST_CASE("validators", "strict-type") {
  std::map<rpn::StrictTypeValidator,std::string> generators {
    { rpn::StrictTypeValidator::d1_double, "12.34" },
    { rpn::StrictTypeValidator::d1_integer, "0x1234" },
    { rpn::StrictTypeValidator::d1_boolean, "<true>" },
    { rpn::StrictTypeValidator::d1_object, ".\" flag\" <true> ->OBJ" },
    { rpn::StrictTypeValidator::d1_string, ".\" string 1\"" },
    { rpn::StrictTypeValidator::d1_array, "1. 2 .\" string\" 3 ->ARRAY" },
    { rpn::StrictTypeValidator::d1_vec3, "12.34 23.45 34.56 ->VEC3" },
    { rpn::StrictTypeValidator::d2_vec3_vec3, "1.2 2.3 3.4 ->VEC3 2.3 3.4 4.5 ->VEC3" },
    { rpn::StrictTypeValidator::d2_double_double, "3.1415 1." },
    { rpn::StrictTypeValidator::d2_integer_double, "2_ 3.1415" },
    { rpn::StrictTypeValidator::d2_double_integer, "12.345 6_" },
    { rpn::StrictTypeValidator::d2_integer_integer, "2_ 3_" },
    { rpn::StrictTypeValidator::d2_boolean_boolean, "<true> <false>" },
    { rpn::StrictTypeValidator::d2_double_vec3, "4.44 9.3 4.5 7.2 ->VEC3" },
    { rpn::StrictTypeValidator::d2_vec3_double, "9. 8. 7. ->VEC3 5.55" },
    { rpn::StrictTypeValidator::d2_integer_vec3, "7_ 4. 5. 6. ->VEC3" },
    { rpn::StrictTypeValidator::d2_vec3_integer, "8. 7. 6. ->VEC3 3_" },
    { rpn::StrictTypeValidator::d2_any_array, "<true> <true> 1 ->ARRAY" },
    { rpn::StrictTypeValidator::d2_array_any, "1 2 3 3 ->ARRAY 4.5" },
    { rpn::StrictTypeValidator::d2_any_string, "<true> .\" flag\"" },
    { rpn::StrictTypeValidator::d2_string_any, ".\" abc\" .\" flag\" <true> ->OBJ" },
    { rpn::StrictTypeValidator::d2_any_object, ".\" flag\" <true> ->OBJ DUP" },
    { rpn::StrictTypeValidator::d2_object_any, ".\" flag\" <true> ->OBJ DUP" },

    { rpn::StrictTypeValidator::d3_double_double_double, "2.3 2.3 4" },
    { rpn::StrictTypeValidator::d3_integer_double_double, "0x2 3 4." },
    { rpn::StrictTypeValidator::d3_double_integer_double, "2. 0x3 4." },
    { rpn::StrictTypeValidator::d3_integer_integer_double, "2_ 0b10 4." },

    { rpn::StrictTypeValidator::d3_double_double_integer, "3. 4. 2_" },
    { rpn::StrictTypeValidator::d3_integer_double_integer, "4_10 2. 3_" },
    { rpn::StrictTypeValidator::d3_double_integer_integer, "2. 3_ 4_" },
    { rpn::StrictTypeValidator::d3_integer_integer_integer, "2_10 0x3 4_16" },

    { rpn::StrictTypeValidator::d3_boolean_any_any, "<true> 1 .\" string\"" },
    { rpn::StrictTypeValidator::d3_any_string_object, "99 .\" bottles\" .\" xyz\" 44 ->OBJ" },
    { rpn::StrictTypeValidator::d3_object_any_string, ".\" life\" .\" football\" ->OBJ 42 .\" meaning\"" },
    { rpn::StrictTypeValidator::d4_integer_double_double_double, "0x5 1.2 2.3 3.4" },
    { rpn::StrictTypeValidator::d4_double_double_double_integer, "2.2 3.3 4.4 0x5" },
    { timecode_validator::d1_tc, "60000_ 1001_ ->FRAC 12345 ->TC" },
    { timecode_validator::d2_tc_tc, "60000_10 1001_10 ->FRAC 12345 ->TC 60000_10 1001_10 ->FRAC 145 ->TC" },
    { timecode_validator::d2_tc_double, "60000_10 1001_10 ->FRAC 4444 ->TC 17" },
    { timecode_validator::d2_double_tc, "120 60000_ 1001_ ->FRAC 12345 ->TC" },
    { frac_validator::d1_frac, "2_10 3_10 ->FRAC" },
    { frac_validator::d2_frac_frac, "1_ 2_10 ->FRAC 0.75 ->FRAC" },
    { frac_validator::d2_int_frac, "7_ 1_ 9_ ->FRAC" },
    { frac_validator::d2_double_frac, "3.6 0.9 ->FRAC" },
    { frac_validator::d2_frac_int, "0.1 ->FRAC 7_" },
    { frac_validator::d2_frac_double, "0.2 ->FRAC 2.4" },
    { frac_validator::d5_frac_double_double_double_double, "60000 1001 ->FRAC 6 5 4 3" }

  };

  for(const auto &g : generators) {
    g_rpn().stack.clear();
    INFO("==> " << g.first.to_string() << " '" << g.second << "'");
    g_rpn().sync_eval(g.second);
    CHECK(g.first(g_rpn().stack.types(), g_rpn().stack) == true);
  }

  for(const auto &vt : sk_validatorTests()) {
    const auto &g = generators.find(vt.first.second);
    INFO("validator test: " + vt.first.second.to_string());
    REQUIRE (g != generators.end());
    g_rpn().stack.clear();
    g_rpn().sync_eval(g->second);
    INFO("==>(" << vt.first.first.to_string() << " / " << vt.first.second.to_string() << ") :=> '" << g->second <<"'");
    CHECK(vt.first.first(g_rpn().stack.types(), g_rpn().stack) == vt.second);
  }

}

TEST_CASE( "object", "types" ) {
  std::string line;
  {
    line = (".\" abc\" 3.6 ->OBJ 2.8 .\" def\" +");
    g_rpn().stack.clear();
    auto st = g_rpn().sync_eval(line);
    REQUIRE( (st == rpn::WordDefinition::Result::ok) );
  }

  {
    line = ("3.6");
    g_rpn().stack.clear();
    auto st = g_rpn().sync_eval(line);
    auto &so = g_rpn().stack.peek(1);
    REQUIRE_THROWS_AS( dynamic_cast<const stack::Object&>(so),
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
    g_rpn().stack.clear();
    auto st = g_rpn().sync_eval(line);
    REQUIRE( (st == rpn::WordDefinition::Result::ok) );
    double dv = g_rpn().stack.peek_double(1);
    REQUIRE( (1 == g_rpn().stack.depth() ) );
    REQUIRE_THAT(dv, Catch::Matchers::WithinAbs((3.6 + 4.3), 0.000001));
  }

  {
    line = ("0d3 4.3 +"); // integer + float
    g_rpn().stack.clear();
    auto st = g_rpn().sync_eval(line);
    REQUIRE( (st == rpn::WordDefinition::Result::ok) );
    double dv = g_rpn().stack.peek_double(1);
    REQUIRE( (1 == g_rpn().stack.depth() ) );
    REQUIRE_THAT(dv, Catch::Matchers::WithinAbs((3 + 4.3), 0.000001));
  }

  {
    line = ("3.6 4_ +"); // float + integer
    g_rpn().stack.clear();
    auto st = g_rpn().sync_eval(line);
    REQUIRE( (st == rpn::WordDefinition::Result::ok) );
    double dv = g_rpn().stack.peek_double(1);
    REQUIRE( (1 == g_rpn().stack.depth() ) );
    REQUIRE_THAT(dv, Catch::Matchers::WithinAbs((3.6 + 4), 0.000001));
  }

  {
    line = ("0d3 4_10 +"); // integer + integer
    g_rpn().stack.clear();
    auto st = g_rpn().sync_eval(line);
    REQUIRE( (st == rpn::WordDefinition::Result::ok) );
    int64_t iv = g_rpn().stack.peek_as_integer(1).value();
    REQUIRE( (1 == g_rpn().stack.depth() ) );
    REQUIRE_THAT(iv, Catch::Matchers::WithinAbs((3 + 4), 0.000001));
  }

}


TEST_CASE( "geometry tests", "geometry" ) {
  std::string line;
  {
    g_rpn().stack.clear();
    line = ("0.25  0.125 ->{xy} 2.875 1.9 ->{xy} -3.0 0. ->{xy} CIRCLE");
    auto st = g_rpn().sync_eval(line);

    REQUIRE( (st == rpn::WordDefinition::Result::ok) );
    REQUIRE( (2 == g_rpn().stack.depth() ) );

    g_rpn().stack.print("CIRCLE vvv");

    line = ("0.25  0.125 ->{xy} 2.875 1.9 ->{xy} -3.0 0. ->{xy} 3 ->ARRAY CIRCLE");
    st = g_rpn().sync_eval(line);
    REQUIRE( (st == rpn::WordDefinition::Result::ok) );
    REQUIRE( (4 == g_rpn().stack.depth() ) );

    g_rpn().stack.print("CIRCLE a");
  }

}


TEST_CASE( "if/then/else", "control" ) {
  g_rpn().stack.clear();

  // basic IF THEN (true branch)
  {
    g_rpn().stack.clear();
    auto st = g_rpn().sync_eval("TRUE IF 42 THEN END");
    REQUIRE( st == rpn::WordDefinition::Result::ok );
    REQUIRE( 1 == g_rpn().stack.depth() );
    REQUIRE( 42.0 == g_rpn().stack.peek_double(1) );
  }

  // basic IF THEN (false branch — stack unchanged)
  {
    g_rpn().stack.clear();
    auto st = g_rpn().sync_eval("FALSE IF 42 THEN END");
    REQUIRE( st == rpn::WordDefinition::Result::ok );
    REQUIRE( 0 == g_rpn().stack.depth() );
  }

  // IF THEN ELSE END (true)
  {
    g_rpn().stack.clear();
    auto st = g_rpn().sync_eval("TRUE IF 1 ELSE 2 END");
    REQUIRE( st == rpn::WordDefinition::Result::ok );
    REQUIRE( 1 == g_rpn().stack.depth() );
    REQUIRE( 1.0 == g_rpn().stack.peek_double(1) );
  }

  // IF THEN ELSE END (false)
  {
    g_rpn().stack.clear();
    auto st = g_rpn().sync_eval("FALSE IF 1 ELSE 2 END");
    REQUIRE( st == rpn::WordDefinition::Result::ok );
    REQUIRE( 1 == g_rpn().stack.depth() );
    REQUIRE( 2.0 == g_rpn().stack.peek_double(1) );
  }

  // IF inside a word definition
  {
    g_rpn().stack.clear();
    auto st = g_rpn().sync_eval(": abs-val DUP 0 < IF CHS THEN END ;");
    REQUIRE( st == rpn::WordDefinition::Result::ok );

    g_rpn().stack.clear();
    g_rpn().sync_eval("-5 abs-val");
    REQUIRE( 1 == g_rpn().stack.depth() );
    REQUIRE( 5.0 == g_rpn().stack.peek_double(1) );

    g_rpn().stack.clear();
    g_rpn().sync_eval("5 abs-val");
    REQUIRE( 1 == g_rpn().stack.depth() );
    REQUIRE( 5.0 == g_rpn().stack.peek_double(1) );
  }

  // IF THEN ELSE inside a word definition
  {
    g_rpn().stack.clear();
    auto st = g_rpn().sync_eval(": sign DUP 0 < IF DROP -1. ELSE DROP 1. END ;");
    REQUIRE( st == rpn::WordDefinition::Result::ok );

    g_rpn().stack.clear();
    g_rpn().sync_eval("-99 sign");
    REQUIRE( 1 == g_rpn().stack.depth() );
    REQUIRE( -1.0 == g_rpn().stack.peek_double(1) );

    g_rpn().stack.clear();
    g_rpn().sync_eval("99 sign");
    REQUIRE( 1 == g_rpn().stack.depth() );
    REQUIRE( 1.0 == g_rpn().stack.peek_double(1) );
  }

  // nested IF inside FOR loop
  {
    g_rpn().stack.clear();
    // push values less than 3 from 0..4
    auto st = g_rpn().sync_eval("0 5 FOR i i 3. < IF i THEN END NEXT");
    REQUIRE( st == rpn::WordDefinition::Result::ok );
    REQUIRE( 3 == g_rpn().stack.depth() );
    REQUIRE( 0.0 == g_rpn().stack.peek_double(3) );
    REQUIRE( 1.0 == g_rpn().stack.peek_double(2) );
    REQUIRE( 2.0 == g_rpn().stack.peek_double(1) );
  }
}

TEST_CASE("while/until loops", "control") {

  // BEGIN...UNTIL: count up until reaching 3
  {
    g_rpn().stack.clear();
    auto st = g_rpn().sync_eval("0 BEGIN 1 + DUP 3. == UNTIL");
    REQUIRE( st == rpn::WordDefinition::Result::ok );
    REQUIRE( 1 == g_rpn().stack.depth() );
    REQUIRE( 3.0 == g_rpn().stack.peek_double(1) );
  }

  // BEGIN...WHILE...REPEAT: count down from 3 to 0
  {
    g_rpn().stack.clear();
    auto st = g_rpn().sync_eval("3 BEGIN 1 - DUP 0. > WHILE REPEAT");
    REQUIRE( st == rpn::WordDefinition::Result::ok );
    REQUIRE( 1 == g_rpn().stack.depth() );
    REQUIRE( 0.0 == g_rpn().stack.peek_double(1) );
  }

  // UNTIL inside a word definition
  {
    g_rpn().stack.clear();
    auto st = g_rpn().sync_eval(": count-up 0 BEGIN 1 + DUP 5. == UNTIL ;");
    REQUIRE( st == rpn::WordDefinition::Result::ok );
    st = g_rpn().sync_eval("count-up");
    REQUIRE( st == rpn::WordDefinition::Result::ok );
    REQUIRE( 1 == g_rpn().stack.depth() );
    REQUIRE( 5.0 == g_rpn().stack.peek_double(1) );
  }

  // WHILE inside a word definition: count down from 5, leave 0
  {
    g_rpn().stack.clear();
    auto st = g_rpn().sync_eval(": countdown5 5 BEGIN 1 - DUP 0. > WHILE REPEAT ;");
    REQUIRE( st == rpn::WordDefinition::Result::ok );
    st = g_rpn().sync_eval("countdown5");
    REQUIRE( st == rpn::WordDefinition::Result::ok );
    REQUIRE( 1 == g_rpn().stack.depth() );
    REQUIRE( 0.0 == g_rpn().stack.peek_double(1) );
  }
}

TEST_CASE("literal syntax", "types") {

  // String literal "..." pushes stack::String
  {
    g_rpn().stack.clear();
    auto st = g_rpn().sync_eval("\"hello\"");
    REQUIRE( st == rpn::WordDefinition::Result::ok );
    REQUIRE( 1 == g_rpn().stack.depth() );
    REQUIRE( "hello" == g_rpn().stack.peek_string(1) );
  }

  // String literal with embedded spaces (requires parser grouping)
  {
    g_rpn().stack.clear();
    auto st = g_rpn().sync_eval("\"hello world\"");
    REQUIRE( st == rpn::WordDefinition::Result::ok );
    REQUIRE( 1 == g_rpn().stack.depth() );
    REQUIRE( "hello world" == g_rpn().stack.peek_string(1) );
  }

  // Backward compat: ." still works
  {
    g_rpn().stack.clear();
    auto st = g_rpn().sync_eval(".\" abcdefg\"");
    REQUIRE( st == rpn::WordDefinition::Result::ok );
    REQUIRE( 1 == g_rpn().stack.depth() );
    REQUIRE( "abcdefg" == g_rpn().stack.peek_string(1) );
  }

  // Name literal 'x' pushes stack::Name
  {
    g_rpn().stack.clear();
    auto st = g_rpn().sync_eval("'myvar'");
    REQUIRE( st == rpn::WordDefinition::Result::ok );
    REQUIRE( 1 == g_rpn().stack.depth() );
    // stack::Name operator string() returns the name
    REQUIRE( "myvar" == std::string(g_rpn().stack.peek(1)) );
  }

  // String literal inside a compiled word
  {
    g_rpn().stack.clear();
    auto st = g_rpn().sync_eval(": push-greeting \"hi there\" ;");
    REQUIRE( st == rpn::WordDefinition::Result::ok );
    st = g_rpn().sync_eval("push-greeting");
    REQUIRE( st == rpn::WordDefinition::Result::ok );
    REQUIRE( 1 == g_rpn().stack.depth() );
    REQUIRE( "hi there" == g_rpn().stack.peek_string(1) );
  }

  // Name literal inside a compiled word
  {
    g_rpn().stack.clear();
    auto st = g_rpn().sync_eval(": push-name 'x' ;");
    REQUIRE( st == rpn::WordDefinition::Result::ok );
    st = g_rpn().sync_eval("push-name");
    REQUIRE( st == rpn::WordDefinition::Result::ok );
    REQUIRE( 1 == g_rpn().stack.depth() );
    REQUIRE( "x" == std::string(g_rpn().stack.peek(1)) );
  }

  // Invalid name literals are rejected (fall through to dict_error)
  {
    g_rpn().stack.clear();
    auto st = g_rpn().sync_eval("'42'");        // starts with digit
    REQUIRE( st != rpn::WordDefinition::Result::ok );
    st = g_rpn().sync_eval("'+'");              // operator char
    REQUIRE( st != rpn::WordDefinition::Result::ok );
    st = g_rpn().sync_eval("'hello world'");    // space splits token; outer ' missing from each half
    REQUIRE( st != rpn::WordDefinition::Result::ok );
    REQUIRE( 0 == g_rpn().stack.depth() );
  }

  // deparse produces new "..." format
  {
    g_rpn().stack.clear();
    g_rpn().sync_eval("\"hello\"");
    auto st = g_rpn().sync_eval("DEPARSE");
    REQUIRE( st == rpn::WordDefinition::Result::ok );
    REQUIRE( "\"hello\"" == g_rpn().stack.peek_string(1) );
  }
}

TEST_CASE("global variables STO/RCL", "variables") {

  // STO and RCL with stack::Name ('x' syntax)
  {
    g_rpn().stack.clear();
    auto st = g_rpn().sync_eval("42. 'x' STO");
    REQUIRE( st == rpn::WordDefinition::Result::ok );
    REQUIRE( 0 == g_rpn().stack.depth() );

    st = g_rpn().sync_eval("'x' RCL");
    REQUIRE( st == rpn::WordDefinition::Result::ok );
    REQUIRE( 1 == g_rpn().stack.depth() );
    REQUIRE( 42.0 == g_rpn().stack.peek_double(1) );
  }

  // STO and RCL also accept stack::String ("x" syntax — backward compat for embedders)
  {
    g_rpn().stack.clear();
    auto st = g_rpn().sync_eval("99. \"mystr\" STO");
    REQUIRE( st == rpn::WordDefinition::Result::ok );
    st = g_rpn().sync_eval("\"mystr\" RCL");
    REQUIRE( st == rpn::WordDefinition::Result::ok );
    REQUIRE( 99.0 == g_rpn().stack.peek_double(1) );
    g_rpn().sync_eval("'mystr' PURGE");
  }

  // Auto-recall by name (HP48 convention: variables shadow words)
  {
    g_rpn().stack.clear();
    auto st = g_rpn().sync_eval("77. 'myvar' STO");
    REQUIRE( st == rpn::WordDefinition::Result::ok );
    st = g_rpn().sync_eval("myvar");
    REQUIRE( st == rpn::WordDefinition::Result::ok );
    REQUIRE( 1 == g_rpn().stack.depth() );
    REQUIRE( 77.0 == g_rpn().stack.peek_double(1) );
    g_rpn().sync_eval("'myvar' PURGE");
  }

  // VARS returns stack::Name array of all global variable names
  {
    g_rpn().stack.clear();
    g_rpn().sync_eval("'x' PURGE");
    auto st = g_rpn().sync_eval("1. 'a' STO  2. 'b' STO");
    REQUIRE( st == rpn::WordDefinition::Result::ok );
    st = g_rpn().sync_eval("VARS");
    REQUIRE( st == rpn::WordDefinition::Result::ok );
    REQUIRE( 1 == g_rpn().stack.depth() );
    auto arr_str = std::string(g_rpn().stack.peek(1));
    REQUIRE( arr_str.find("a") != std::string::npos );
    REQUIRE( arr_str.find("b") != std::string::npos );
  }

  // PURGE removes a variable
  {
    g_rpn().stack.clear();
    g_rpn().sync_eval("'a' PURGE  'b' PURGE");
    auto st = g_rpn().sync_eval("VARS");
    REQUIRE( st == rpn::WordDefinition::Result::ok );
    auto arr_str = std::string(g_rpn().stack.peek(1));
    REQUIRE( arr_str.find("a") == std::string::npos );
    REQUIRE( arr_str.find("b") == std::string::npos );
  }

  // Variable overwrite
  {
    g_rpn().stack.clear();
    g_rpn().sync_eval("10. 'z' STO");
    g_rpn().sync_eval("20. 'z' STO");
    auto st = g_rpn().sync_eval("'z' RCL");
    REQUIRE( st == rpn::WordDefinition::Result::ok );
    REQUIRE( 20.0 == g_rpn().stack.peek_double(1) );
    g_rpn().sync_eval("'z' PURGE");
  }

  // Word definition callable by name (existing dict behavior)
  {
    g_rpn().stack.clear();
    auto st = g_rpn().sync_eval(": double2 2 * ;");
    REQUIRE( st == rpn::WordDefinition::Result::ok );
    st = g_rpn().sync_eval("7. double2");
    REQUIRE( st == rpn::WordDefinition::Result::ok );
    REQUIRE( 14.0 == g_rpn().stack.peek_double(1) );
  }
}

TEST_CASE("angle mode DEG/RAD/GRAD", "trig") {
  static constexpr double kPi = M_PI;
  // helper: abs difference
  auto near = [](double a, double b, double eps=1e-10) { return std::abs(a-b) < eps; };

  // Default is degrees
  {
    g_rpn().setAngleMode(rpn::AngleMode::degrees);
    g_rpn().stack.clear();
    g_rpn().sync_eval("90. SIN");
    REQUIRE( near(g_rpn().stack.peek_double(1), 1.0) );

    g_rpn().stack.clear();
    g_rpn().sync_eval("0. COS");
    REQUIRE( near(g_rpn().stack.peek_double(1), 1.0) );

    g_rpn().stack.clear();
    g_rpn().sync_eval("45. TAN");
    REQUIRE( near(g_rpn().stack.peek_double(1), 1.0) );
  }

  // ASIN/ACOS/ATAN in degrees mode
  {
    g_rpn().setAngleMode(rpn::AngleMode::degrees);
    g_rpn().stack.clear();
    g_rpn().sync_eval("1. ASIN");
    REQUIRE( near(g_rpn().stack.peek_double(1), 90.0) );

    g_rpn().stack.clear();
    g_rpn().sync_eval("1. ACOS");
    REQUIRE( near(g_rpn().stack.peek_double(1), 0.0) );

    g_rpn().stack.clear();
    g_rpn().sync_eval("1. ATAN");
    REQUIRE( near(g_rpn().stack.peek_double(1), 45.0) );
  }

  // Radians mode
  {
    g_rpn().stack.clear();
    g_rpn().sync_eval("->RAD");
    REQUIRE( g_rpn().angleMode() == rpn::AngleMode::radians );

    g_rpn().stack.clear();
    g_rpn().sync_eval("1. ASIN");
    REQUIRE( near(g_rpn().stack.peek_double(1), kPi/2.) );

    g_rpn().stack.clear();
    g_rpn().sync_eval("0. SIN");
    REQUIRE( near(g_rpn().stack.peek_double(1), 0.0) );

    // ATAN2: stack is ( x y -- angle ); y=TOS, x=TOS-1
    g_rpn().stack.clear();
    g_rpn().sync_eval("0. 1. ATAN2");   // atan2(1, 0) = π/2
    REQUIRE( near(g_rpn().stack.peek_double(1), kPi/2.) );
  }

  // Gradians mode: 100 grad = 90 deg = π/2 rad
  {
    g_rpn().stack.clear();
    g_rpn().sync_eval("->GRAD");
    REQUIRE( g_rpn().angleMode() == rpn::AngleMode::gradians );

    g_rpn().stack.clear();
    g_rpn().sync_eval("100. SIN");
    REQUIRE( near(g_rpn().stack.peek_double(1), 1.0) );

    g_rpn().stack.clear();
    g_rpn().sync_eval("1. ASIN");
    REQUIRE( near(g_rpn().stack.peek_double(1), 100.0) );
  }

  // ANGLEMODE query word
  {
    g_rpn().stack.clear();
    g_rpn().sync_eval("->DEG ANGLEMODE");
    REQUIRE( g_rpn().stack.peek_string(1) == "DEG" );

    g_rpn().stack.clear();
    g_rpn().sync_eval("->RAD ANGLEMODE");
    REQUIRE( g_rpn().stack.peek_string(1) == "RAD" );

    g_rpn().stack.clear();
    g_rpn().sync_eval("->GRAD ANGLEMODE");
    REQUIRE( g_rpn().stack.peek_string(1) == "GRAD" );
  }

  // Leave in degrees for subsequent tests
  g_rpn().setAngleMode(rpn::AngleMode::degrees);
}

TEST_CASE("deparse round-trips", "display") {
  // For each type: push a value, DEPARSE to get the RPN string, EVAL to re-push,
  // verify the reconstructed value matches the original.

  // Double — full precision preserved
  {
    static const double kPi = 3.14159265358979323846;
    g_rpn().stack.clear();
    g_rpn().sync_eval("3.14159265358979323846. DEPARSE EVAL");
    REQUIRE( g_rpn().stack.depth() == 1 );
    REQUIRE( g_rpn().stack.peek_double(1) == kPi );
  }
  // Double — integer-valued double retains double type
  {
    g_rpn().stack.clear();
    g_rpn().sync_eval("42. DEPARSE EVAL");
    REQUIRE( g_rpn().stack.depth() == 1 );
    REQUIRE( g_rpn().stack.peek_double(1) == 42.0 );
  }

  // Integer
  {
    g_rpn().stack.clear();
    g_rpn().sync_eval("0d12345 DEPARSE EVAL");
    REQUIRE( g_rpn().stack.depth() == 1 );
    REQUIRE( g_rpn().stack.peek_integer(1) == 12345 );
  }
  {
    g_rpn().stack.clear();
    g_rpn().sync_eval("0d-99 DEPARSE EVAL");
    REQUIRE( g_rpn().stack.depth() == 1 );
    REQUIRE( g_rpn().stack.peek_integer(1) == -99 );
  }

  // String
  {
    g_rpn().stack.clear();
    g_rpn().sync_eval("\"hello world\" DEPARSE EVAL");
    REQUIRE( g_rpn().stack.depth() == 1 );
    REQUIRE( g_rpn().stack.peek_string(1) == "hello world" );
  }

  // Boolean — TRUE and FALSE words; deparse produces TRUE / FALSE
  {
    g_rpn().stack.clear();
    g_rpn().sync_eval("TRUE DEPARSE EVAL");
    REQUIRE( g_rpn().stack.depth() == 1 );
    REQUIRE( g_rpn().stack.peek_boolean(1) == true );

    g_rpn().stack.clear();
    g_rpn().sync_eval("FALSE DEPARSE EVAL");
    REQUIRE( g_rpn().stack.depth() == 1 );
    REQUIRE( g_rpn().stack.peek_boolean(1) == false );
  }
  // Comparison-produced boolean also round-trips via TRUE/FALSE
  {
    g_rpn().stack.clear();
    g_rpn().sync_eval("2. 3. < DEPARSE EVAL");
    REQUIRE( g_rpn().stack.peek_boolean(1) == true );
  }

  // Name
  {
    g_rpn().stack.clear();
    g_rpn().sync_eval("'myvar' DEPARSE EVAL");
    REQUIRE( g_rpn().stack.depth() == 1 );
    auto obj = g_rpn().stack.pop();
    auto *n = dynamic_cast<const stack::Name*>(obj.get());
    REQUIRE( n != nullptr );
    REQUIRE( std::string(*n) == "myvar" );
  }

  // Vec3
  {
    g_rpn().stack.clear();
    g_rpn().sync_eval("1.5 2.25 -3.125 ->VEC3 DEPARSE EVAL");
    REQUIRE( g_rpn().stack.depth() == 1 );
    auto obj = g_rpn().stack.pop();
    auto *v = dynamic_cast<const stack::Vec3*>(obj.get());
    REQUIRE( v != nullptr );
    REQUIRE( (*v)[0] == 1.5 );
    REQUIRE( (*v)[1] == 2.25 );
    REQUIRE( (*v)[2] == -3.125 );
  }

  // Fraction (use 0d prefix to create integers — ->FRAC expects d2_integer_integer)
  {
    g_rpn().stack.clear();
    g_rpn().sync_eval("0d3 0d4 ->FRAC DEPARSE EVAL");
    REQUIRE( g_rpn().stack.depth() == 1 );
    REQUIRE_THAT( g_rpn().stack.peek_as_double(1).value(), Catch::Matchers::WithinAbs(0.75, 1e-10) );
  }

  // Timecode — ->TC takes d2_frac_double (NOS:Fraction, TOS:Double).
  // Round-trip exercises that deparse emits tokens in the right order.
  {
    g_rpn().stack.clear();
    g_rpn().sync_eval("0d30000 0d1001 ->FRAC 89915. ->TC DEPARSE EVAL");
    REQUIRE( g_rpn().stack.depth() == 1 );
    auto obj = g_rpn().stack.pop();
    auto *tc = dynamic_cast<const stack::Timecode*>(obj.get());
    REQUIRE( tc != nullptr );
    REQUIRE( tc->to_frames() == 89915 );
  }

  // Array of doubles
  {
    g_rpn().stack.clear();
    g_rpn().sync_eval("1. 2. 3. 3 ->ARRAY DEPARSE EVAL");
    REQUIRE( g_rpn().stack.depth() == 1 );
    auto obj = g_rpn().stack.pop();
    auto *arr = dynamic_cast<const stack::Array*>(obj.get());
    REQUIRE( arr != nullptr );
    REQUIRE( arr->val().size() == 3 );
  }

  // Object — single field via ->OBJ (d2_string_any: NOS=string, TOS=any).
  // Round-trip exercises that deparse emits the key BEFORE the value for
  // the ->OBJ token.
  {
    g_rpn().stack.clear();
    g_rpn().sync_eval("\"a\" 42. ->OBJ DEPARSE EVAL");
    REQUIRE( g_rpn().stack.depth() == 1 );
    auto obj = g_rpn().stack.pop();
    auto *o = dynamic_cast<const stack::Object*>(obj.get());
    REQUIRE( o != nullptr );
    REQUIRE( o->val().size() == 1 );
  }

  // Object — multi-field, exercises both ->OBJ (first pair) and + (each
  // subsequent pair: d3_object_any_string with value pushed before key).
  {
    g_rpn().stack.clear();
    g_rpn().sync_eval("\"a\" 1. ->OBJ 2. \"b\" + 3. \"c\" + DEPARSE EVAL");
    REQUIRE( g_rpn().stack.depth() == 1 );
    auto obj = g_rpn().stack.pop();
    auto *o = dynamic_cast<const stack::Object*>(obj.get());
    REQUIRE( o != nullptr );
    REQUIRE( o->val().size() == 3 );
  }

  // Lambda / Progn (flat case round-trips cleanly)
  {
    g_rpn().stack.clear();
    g_rpn().sync_eval("<< 2. * >> DEPARSE EVAL 5. SWAP EXEC");
    REQUIRE( g_rpn().stack.depth() == 1 );
    REQUIRE( g_rpn().stack.peek_double(1) == 10.0 );
  }

  // Verify TRUE/FALSE deparse strings
  {
    g_rpn().stack.clear();
    g_rpn().sync_eval("TRUE DEPARSE");
    REQUIRE( g_rpn().stack.peek_string(1) == "TRUE" );

    g_rpn().stack.clear();
    g_rpn().sync_eval("FALSE DEPARSE");
    REQUIRE( g_rpn().stack.peek_string(1) == "FALSE" );
  }

  // Verify double deparse preserves precision
  {
    g_rpn().stack.clear();
    g_rpn().sync_eval("3.14159265358979. DEPARSE");
    // deparse should not be the display-precision "3.14159265358979."
    // it should be full 17-digit form
    std::string dep = g_rpn().stack.peek_string(1);
    REQUIRE( !dep.empty() );
    // Can be evaluated back to a double
    g_rpn().stack.clear();
    g_rpn().sync_eval("3.14159265358979. DEPARSE EVAL");
    REQUIRE( g_rpn().stack.peek_double(1) == 3.14159265358979 );
  }

  // Tvm
  {
    g_rpn().stack.clear();
    g_rpn().sync_eval("36. 0.5 200000. -1199.10 0. FALSE ->TVM");
    auto before = g_rpn().stack.peek(1).deep_copy();
    g_rpn().sync_eval("DEPARSE EVAL");
    REQUIRE( *before == g_rpn().stack.peek(1) );
  }
}

TEST_CASE("cancel and progress", "threading") {
  // cancel() from another thread interrupts a long FOR loop running in sync_eval.
  // Uses a local interpreter so g_rpn state is unaffected.
  {
    rpn::Interp interp(false);
    std::thread canceller([&]{
      std::this_thread::sleep_for(std::chrono::milliseconds(5));
      interp.cancel();
    });
    auto rv = interp.sync_eval("0. 10000000. FOR i << >> NEXT");
    canceller.join();
    REQUIRE( rv == rpn::WordDefinition::Result::cancelled );
    // isCancelled() is false after sync_eval clears the flag at start
    // of subsequent call
    rv = interp.sync_eval("2. 3. +");
    REQUIRE( rv == rpn::WordDefinition::Result::ok );
    REQUIRE( interp.stack.peek_double(1) == 5.0 );
  }

  // cancel() also stops an infinite BEGIN...WHILE...REPEAT loop
  {
    rpn::Interp interp(false);
    std::thread canceller([&]{
      std::this_thread::sleep_for(std::chrono::milliseconds(5));
      interp.cancel();
    });
    // BEGIN TRUE WHILE REPEAT: loops forever (condition always true)
    auto rv = interp.sync_eval("BEGIN TRUE WHILE REPEAT");
    canceller.join();
    REQUIRE( rv == rpn::WordDefinition::Result::cancelled );
  }

  // cancelAll() ensures all pending completion handlers are called.
  // Queue several requests then immediately cancelAll; every handler must fire.
  {
    rpn::Interp interp(true);  // async
    const int N = 20;
    std::atomic<int> count{0};
    std::mutex mx;
    std::condition_variable cv;
    for (int i = 0; i < N; i++) {
      interp.eval("1. 2. +", [&](rpn::WordDefinition::Result) {
        count.fetch_add(1);
        cv.notify_one();
      });
    }
    interp.cancelAll();
    // All N handlers must fire within a reasonable window
    std::unique_lock ul(mx);
    cv.wait_for(ul, std::chrono::milliseconds(500), [&]{ return count.load() == N; });
    REQUIRE( count.load() == N );
  }

  // Progress callback is invoked by reportProgress(); nullptr clears it
  {
    std::vector<std::pair<std::string,double>> events;
    g_rpn().setProgressHandler([&](const std::string &msg, double frac) {
      events.push_back({msg, frac});
    });
    g_rpn().reportProgress("step1", 0.0);
    g_rpn().reportProgress("step2", 0.5);
    g_rpn().reportProgress("done",  1.0);
    g_rpn().setProgressHandler(nullptr);
    REQUIRE( events.size() == 3 );
    REQUIRE( events[0].first == "step1" );
    REQUIRE( events[1].second == 0.5 );
    REQUIRE( events[2].second == 1.0 );
    g_rpn().reportProgress("ignored", 0.0);  // no-op after clear
    REQUIRE( events.size() == 3 );
  }

  // isCancelled() is false on a freshly used interpreter
  {
    rpn::Interp interp(false);
    interp.sync_eval("1. 1. +");
    REQUIRE( !interp.isCancelled() );
  }
}

TEST_CASE("word introspection", "API") {
  // wordList returns a non-empty list without duplicates
  {
    auto wl = g_rpn().wordList();
    REQUIRE( !wl.empty() );
    // spot-check expected words are present
    auto has = [&](const std::string &w) {
      return std::find(wl.begin(), wl.end(), w) != wl.end();
    };
    REQUIRE( has("+") );
    REQUIRE( has("STO") );
    REQUIRE( has("CIRCLE") );
    // no duplicates
    std::vector<std::string> sorted = wl;
    std::sort(sorted.begin(), sorted.end());
    auto it = std::adjacent_find(sorted.begin(), sorted.end());
    REQUIRE( it == sorted.end() );
  }

  // wordHelp: known word has name, category, and effects
  {
    auto h = g_rpn().wordHelp("+");
    REQUIRE( h.name == "+" );
    REQUIRE( !h.description.empty() );
    REQUIRE( h.category == "math" );
    REQUIRE( !h.effects.empty() );
  }

  // wordHelp: overloaded word has multiple effects entries
  {
    auto h = g_rpn().wordHelp("CIRCLE");
    REQUIRE( h.name == "CIRCLE" );
    REQUIRE( h.category == "geometry" );
    REQUIRE( h.effects.size() == 2 );
  }

  // wordHelp: unknown word returns name only, empty fields
  {
    auto h = g_rpn().wordHelp("__NO_SUCH_WORD__");
    REQUIRE( h.name == "__NO_SUCH_WORD__" );
    REQUIRE( h.description.empty() );
    REQUIRE( h.effects.empty() );
  }

  // Embedder-added word gets empty category (after constructor reset)
  {
    rpn::Interp local(false);
    local.addDefinition("TEST-WORD", rpn::WordDefinition {
      rpn::StackSizeValidator::zero,
      [](rpn::Interp&, rpn::WordContext*, std::string&) { return rpn::WordDefinition::Result::ok; },
      nullptr, ""
    });
    local.addWordMetadata("TEST-WORD", "A test word");
    auto h = local.wordHelp("TEST-WORD");
    REQUIRE( h.description == "A test word" );
    REQUIRE( h.category.empty() );  // no setWordCategory was called
  }
}

TEST_CASE("json type and words", "types") {
  rpn::Interp rpn(false);

  // ->JSON on a double produces a stack::Json containing the number
  {
    rpn.sync_eval("3.14 ->JSON");
    REQUIRE( rpn.stack.depth() == 1 );
    auto jobj = rpn.stack.pop();
    auto *j = dynamic_cast<const stack::Json*>(jobj.get());
    REQUIRE( j != nullptr );
    REQUIRE( j->is_number_float() );
    REQUIRE_THAT( j->get<double>(), Catch::Matchers::WithinAbs(3.14, 1e-10) );
  }

  // ->JSON on an integer produces a stack::Json containing the integer
  {
    rpn.sync_eval("0d42 ->JSON");
    REQUIRE( rpn.stack.depth() == 1 );
    auto jobj = rpn.stack.pop();
    auto *j = dynamic_cast<const stack::Json*>(jobj.get());
    REQUIRE( j != nullptr );
    REQUIRE( j->is_number_integer() );
    REQUIRE( j->get<int64_t>() == 42 );
  }

  // ->JSON on a boolean produces a stack::Json boolean
  {
    rpn.sync_eval("TRUE ->JSON");
    REQUIRE( rpn.stack.depth() == 1 );
    auto jobj = rpn.stack.pop();
    auto *j = dynamic_cast<const stack::Json*>(jobj.get());
    REQUIRE( j != nullptr );
    REQUIRE( j->is_boolean() );
    REQUIRE( j->get<bool>() == true );
  }

  // ->JSON on a string produces a stack::Json string
  {
    rpn.stack.push_string("hello");
    rpn.sync_eval("->JSON");
    REQUIRE( rpn.stack.depth() == 1 );
    auto jobj = rpn.stack.pop();
    auto *j = dynamic_cast<const stack::Json*>(jobj.get());
    REQUIRE( j != nullptr );
    REQUIRE( j->is_string() );
    REQUIRE( j->get<std::string>() == "hello" );
  }

  // JSON-> on a scalar float unpacks to native double
  {
    rpn.stack.push(stack::Json(nlohmann::json(2.718)));
    rpn.sync_eval("JSON->");
    REQUIRE( rpn.stack.depth() == 1 );
    REQUIRE_THAT( rpn.stack.peek_as_double(1).value(), Catch::Matchers::WithinAbs(2.718, 1e-10) );
    rpn.stack.drop();
  }

  // JSON-> on a scalar integer unpacks to native integer
  {
    rpn.stack.push(stack::Json(nlohmann::json(int64_t(99))));
    rpn.sync_eval("JSON->");
    REQUIRE( rpn.stack.depth() == 1 );
    REQUIRE( rpn.stack.peek_integer(1) == 99 );
    rpn.stack.drop();
  }

  // JSON-> on a scalar bool unpacks to native boolean
  {
    rpn.stack.push(stack::Json(nlohmann::json(false)));
    rpn.sync_eval("JSON->");
    REQUIRE( rpn.stack.depth() == 1 );
    REQUIRE( rpn.stack.peek_boolean(1) == false );
    rpn.stack.drop();
  }

  // JSON-> on a scalar string unpacks to native string
  {
    rpn.stack.push(stack::Json(nlohmann::json(std::string("world"))));
    rpn.sync_eval("JSON->");
    REQUIRE( rpn.stack.depth() == 1 );
    REQUIRE( rpn.stack.peek_string(1) == "world" );
    rpn.stack.drop();
  }

  // JSON-> on a JSON array unpacks elements (as stack::Json) plus integer count.
  // [10, 20, 30] → stack (bottom→top): 30, 20, 10, count=3
  // TOS = count; first element of original array is just below count.
  {
    nlohmann::json arr = nlohmann::json::array({10, 20, 30});
    rpn.stack.push(stack::Json(arr));
    rpn.sync_eval("JSON->");
    REQUIRE( rpn.stack.depth() == 4 );
    REQUIRE( rpn.stack.peek_integer(1) == 3 );  // TOS = count
    rpn.stack.drop();  // drop count
    // Now TOS = first element (10), then 20, then 30 (bottom)
    auto jobj = rpn.stack.pop();
    REQUIRE( dynamic_cast<const stack::Json*>(jobj.get())->get<int>() == 10 );  // first element on TOS
    rpn.stack.clear();
  }

  // JSON-> on a JSON object unpacks (value, key) pairs plus count.
  // {"a":42} → stack (bottom→top): value(stack::Json=42), key("a"), count=1
  {
    nlohmann::json obj = {{"a", 42}};
    rpn.stack.push(stack::Json(obj));
    rpn.sync_eval("JSON->");
    // depth = 3: value(stack::Json), key(stack::String), count(stack::Integer)
    REQUIRE( rpn.stack.depth() == 3 );
    REQUIRE( rpn.stack.peek_integer(1) == 1 );   // TOS = count
    REQUIRE( rpn.stack.peek_string(2) == "a" );  // NOS = key
    rpn.stack.drop();  // drop count
    rpn.stack.drop();  // drop key
    auto vobj = rpn.stack.pop();
    REQUIRE( dynamic_cast<const stack::Json*>(vobj.get())->get<int>() == 42 );
    rpn.stack.clear();
  }

  // to_json() descriptor: type, display, deparse, data fields present for double
  {
    rpn.stack.push_double(1.5);
    auto j = rpn.stack.peek(1).to_json();
    REQUIRE( j["type"] == "double" );
    REQUIRE( j.contains("display") );
    REQUIRE( j.contains("deparse") );
    REQUIRE( j.contains("data") );
    REQUIRE_THAT( j["data"].get<double>(), Catch::Matchers::WithinAbs(1.5, 1e-10) );
    rpn.stack.drop();
  }

  // to_json() for integer
  {
    rpn.stack.push_integer(7);
    auto j = rpn.stack.peek(1).to_json();
    REQUIRE( j["type"] == "integer" );
    REQUIRE( j["data"].get<int64_t>() == 7 );
    rpn.stack.drop();
  }

  // to_json() for string
  {
    rpn.stack.push_string("abc");
    auto j = rpn.stack.peek(1).to_json();
    REQUIRE( j["type"] == "string" );
    REQUIRE( j["data"].get<std::string>() == "abc" );
    rpn.stack.drop();
  }

  // to_json() for boolean
  {
    rpn.stack.push_boolean(true);
    auto j = rpn.stack.peek(1).to_json();
    REQUIRE( j["type"] == "boolean" );
    REQUIRE( j["data"].get<bool>() == true );
    rpn.stack.drop();
  }

  // to_json() for vec3
  {
    rpn.sync_eval("1. 2. 3. ->VEC3");
    auto j = rpn.stack.peek(1).to_json();
    REQUIRE( j["type"] == "vec3" );
    REQUIRE_THAT( j["data"]["x"].get<double>(), Catch::Matchers::WithinAbs(1.0, 1e-10) );
    REQUIRE_THAT( j["data"]["y"].get<double>(), Catch::Matchers::WithinAbs(2.0, 1e-10) );
    REQUIRE_THAT( j["data"]["z"].get<double>(), Catch::Matchers::WithinAbs(3.0, 1e-10) );
    rpn.stack.drop();
  }

  // to_json() for fraction
  {
    rpn.sync_eval("0d3 0d4 ->FRAC");
    auto j = rpn.stack.peek(1).to_json();
    REQUIRE( j["type"] == "fraction" );
    REQUIRE( j["data"]["numerator"].get<int64_t>() == 3 );
    REQUIRE( j["data"]["denominator"].get<int64_t>() == 4 );
    rpn.stack.drop();
  }
}

TEST_CASE("compiled word validation (Phase 2.3)", "types") {
  rpn::Interp rpn(false);

  // Word with ( double -- double ) effect: only accepts a double on TOS.
  rpn.sync_eval(": DOUBLE2 ( double -- double ) 2. * ;");

  // Correct type — should succeed.
  rpn.sync_eval("3.");
  REQUIRE( rpn.stack.depth() == 1 );
  auto rv = rpn.sync_eval("DOUBLE2");
  REQUIRE( rv == rpn::WordDefinition::Result::ok );
  REQUIRE( rpn.stack.depth() == 1 );
  REQUIRE_THAT( rpn.stack.peek_double(1), Catch::Matchers::WithinAbs(6.0, 1e-10) );
  rpn.stack.drop();

  // Wrong type (integer) — validator should reject with param_error (word exists, type mismatch).
  rpn.sync_eval("0d3");
  REQUIRE( rpn.stack.depth() == 1 );
  rv = rpn.sync_eval("DOUBLE2");
  REQUIRE( rv == rpn::WordDefinition::Result::param_error );
  rpn.stack.drop();

  // Word with two inputs: ( double double -- double ).
  rpn.sync_eval(": MYAVG ( double double -- double ) + 2. / ;");
  rpn.sync_eval("4. 6.");
  rv = rpn.sync_eval("MYAVG");
  REQUIRE( rv == rpn::WordDefinition::Result::ok );
  REQUIRE( rpn.stack.depth() == 1 );
  REQUIRE_THAT( rpn.stack.peek_double(1), Catch::Matchers::WithinAbs(5.0, 1e-10) );
  rpn.stack.drop();

  // Word with ( any -- any ): accepts any type.
  rpn.sync_eval(": MYDUP ( any -- any ) DUP ;");
  rpn.sync_eval("7.");
  rv = rpn.sync_eval("MYDUP");
  REQUIRE( rv == rpn::WordDefinition::Result::ok );
  REQUIRE( rpn.stack.depth() == 2 );
  rpn.stack.drop(); rpn.stack.drop();

  rpn.sync_eval("\"hello\"");
  rv = rpn.sync_eval("MYDUP");
  REQUIRE( rv == rpn::WordDefinition::Result::ok );
  REQUIRE( rpn.stack.depth() == 2 );
  rpn.stack.drop(); rpn.stack.drop();

  // Word without effect comment: behaves as StackSizeValidator::zero (always matches).
  rpn.sync_eval(": NOOP ;");
  rv = rpn.sync_eval("NOOP");
  REQUIRE( rv == rpn::WordDefinition::Result::ok );
  REQUIRE( rpn.stack.depth() == 0 );

  // HP48-style variable names in effect → arity validation (size only, no type check).
  // ( n diam -- result ) doesn't know types but knows 2 inputs are required.
  rpn.sync_eval(": MYADD2 ( n diam -- result ) + ;");
  REQUIRE( rpn.wordExists("MYADD2") );

  rpn.sync_eval("3. 4.");
  rv = rpn.sync_eval("MYADD2");
  REQUIRE( rv == rpn::WordDefinition::Result::ok );
  REQUIRE_THAT( rpn.stack.peek_double(1), Catch::Matchers::WithinAbs(7.0, 1e-10) );
  rpn.stack.drop();

  // Stack too small → param_error (arity check fails).
  rpn.sync_eval("3.");
  rv = rpn.sync_eval("MYADD2");
  REQUIRE( rv == rpn::WordDefinition::Result::param_error );
  rpn.stack.drop();

  // Valid zero-input effect ( -- double ) → StackSizeValidator::zero, always matches.
  rpn.sync_eval(": PUSHPI ( -- double ) 3.14159 ;");
  rv = rpn.sync_eval("PUSHPI");
  REQUIRE( rv == rpn::WordDefinition::Result::ok );
  REQUIRE( rpn.stack.depth() == 1 );
  REQUIRE_THAT( rpn.stack.peek_double(1), Catch::Matchers::WithinAbs(3.14159, 1e-5) );
  rpn.stack.drop();
}

TEST_CASE("stdlib migrated words", "stdlib") {
  rpn::Interp rpn(false);
  using Catch::Matchers::WithinAbs;
  auto ev = [&](const std::string &s) { return rpn.sync_eval(s); };
  constexpr double eps = 1e-10;

  // NaN
  REQUIRE( ev("NaN") == rpn::WordDefinition::Result::ok );
  REQUIRE( std::isnan(rpn.stack.peek_double(1)) );
  rpn.stack.drop();

  // DUP2 / DROP2
  REQUIRE( ev("1. 2. DUP2") == rpn::WordDefinition::Result::ok );
  REQUIRE( rpn.stack.depth() == 4 );
  REQUIRE_THAT( rpn.stack.peek_double(1), Catch::Matchers::WithinAbs(2., eps) );
  REQUIRE_THAT( rpn.stack.peek_double(2), Catch::Matchers::WithinAbs(1., eps) );
  REQUIRE( ev("DROP2") == rpn::WordDefinition::Result::ok );
  REQUIRE( rpn.stack.depth() == 2 );
  REQUIRE( ev("DROP2") == rpn::WordDefinition::Result::ok );
  REQUIRE( rpn.stack.depth() == 0 );

  // SQ — double
  REQUIRE( ev("3. SQ") == rpn::WordDefinition::Result::ok );
  REQUIRE_THAT( rpn.stack.peek_double(1), Catch::Matchers::WithinAbs(9., eps) );
  rpn.stack.drop();

  // HYPOT
  REQUIRE( ev("3. 4. HYPOT") == rpn::WordDefinition::Result::ok );
  REQUIRE_THAT( rpn.stack.peek_double(1), Catch::Matchers::WithinAbs(5., eps) );
  rpn.stack.drop();
}

TEST_CASE("RPL stdlib", "stdlib") {
  rpn::Interp rpn(false);
  using Catch::Matchers::WithinAbs;
  auto ev = [&](const std::string &s) { return rpn.sync_eval(s); };
  constexpr double eps = 1e-10;

  // GAMMA: n! = GAMMA(n+1)
  REQUIRE( ev("5. GAMMA") == rpn::WordDefinition::Result::ok );
  REQUIRE_THAT( rpn.stack.peek_double(1), Catch::Matchers::WithinAbs(std::tgamma(5.), eps) ); // 24
  rpn.stack.drop();

  // LGAMMA
  REQUIRE( ev("5. LGAMMA") == rpn::WordDefinition::Result::ok );
  REQUIRE_THAT( rpn.stack.peek_double(1), Catch::Matchers::WithinAbs(std::lgamma(5.), eps) );
  rpn.stack.drop();

  // Hyperbolic trig against std:: functions
  for (double x : {-1.5, 0., 0.5, 1.5}) {
    REQUIRE( ev(std::to_string(x) + " SINH") == rpn::WordDefinition::Result::ok );
    REQUIRE_THAT( rpn.stack.peek_double(1), Catch::Matchers::WithinAbs(std::sinh(x), 1e-9) );
    rpn.stack.drop();

    REQUIRE( ev(std::to_string(x) + " COSH") == rpn::WordDefinition::Result::ok );
    REQUIRE_THAT( rpn.stack.peek_double(1), Catch::Matchers::WithinAbs(std::cosh(x), 1e-9) );
    rpn.stack.drop();

    REQUIRE( ev(std::to_string(x) + " TANH") == rpn::WordDefinition::Result::ok );
    REQUIRE_THAT( rpn.stack.peek_double(1), Catch::Matchers::WithinAbs(std::tanh(x), 1e-9) );
    rpn.stack.drop();
  }

  // Inverse hyperbolic trig
  for (double x : {0., 0.5, 1.5}) {
    REQUIRE( ev(std::to_string(x) + " ASINH") == rpn::WordDefinition::Result::ok );
    REQUIRE_THAT( rpn.stack.peek_double(1), Catch::Matchers::WithinAbs(std::asinh(x), 1e-9) );
    rpn.stack.drop();
  }
  for (double x : {1.0, 2.0, 3.5}) {
    REQUIRE( ev(std::to_string(x) + " ACOSH") == rpn::WordDefinition::Result::ok );
    REQUIRE_THAT( rpn.stack.peek_double(1), Catch::Matchers::WithinAbs(std::acosh(x), 1e-9) );
    rpn.stack.drop();
  }
  for (double x : {-0.9, 0., 0.9}) {
    REQUIRE( ev(std::to_string(x) + " ATANH") == rpn::WordDefinition::Result::ok );
    REQUIRE_THAT( rpn.stack.peek_double(1), Catch::Matchers::WithinAbs(std::atanh(x), 1e-9) );
    rpn.stack.drop();
  }

  // Factorial: n! = GAMMA(n+1)
  for (double n : {0., 1., 2., 3., 5., 10.}) {
    REQUIRE( ev(std::to_string(n) + " !") == rpn::WordDefinition::Result::ok );
    REQUIRE_THAT( rpn.stack.peek_double(1), Catch::Matchers::WithinAbs(std::tgamma(n + 1.), eps) );
    rpn.stack.drop();
  }

  // nCr: C(5,2) = 10, C(6,3) = 20
  REQUIRE( ev("5. 2. nCr") == rpn::WordDefinition::Result::ok );
  REQUIRE_THAT( rpn.stack.peek_double(1), Catch::Matchers::WithinAbs(10., eps) );
  rpn.stack.drop();

  REQUIRE( ev("6. 3. nCr") == rpn::WordDefinition::Result::ok );
  REQUIRE_THAT( rpn.stack.peek_double(1), Catch::Matchers::WithinAbs(20., eps) );
  rpn.stack.drop();

  // nPr: P(5,2) = 20, P(6,3) = 120
  REQUIRE( ev("5. 2. nPr") == rpn::WordDefinition::Result::ok );
  REQUIRE_THAT( rpn.stack.peek_double(1), Catch::Matchers::WithinAbs(20., eps) );
  rpn.stack.drop();

  REQUIRE( ev("6. 3. nPr") == rpn::WordDefinition::Result::ok );
  REQUIRE_THAT( rpn.stack.peek_double(1), Catch::Matchers::WithinAbs(120., eps) );
  rpn.stack.drop();

  // GCD
  REQUIRE( ev("12. 8. GCD") == rpn::WordDefinition::Result::ok );
  REQUIRE_THAT( rpn.stack.peek_double(1), Catch::Matchers::WithinAbs(4., eps) );
  rpn.stack.drop();

  REQUIRE( ev("35. 14. GCD") == rpn::WordDefinition::Result::ok );
  REQUIRE_THAT( rpn.stack.peek_double(1), Catch::Matchers::WithinAbs(7., eps) );
  rpn.stack.drop();

  // GCD with zero
  REQUIRE( ev("8. 0. GCD") == rpn::WordDefinition::Result::ok );
  REQUIRE_THAT( rpn.stack.peek_double(1), Catch::Matchers::WithinAbs(8., eps) );
  rpn.stack.drop();

  // LCM
  REQUIRE( ev("4. 6. LCM") == rpn::WordDefinition::Result::ok );
  REQUIRE_THAT( rpn.stack.peek_double(1), Catch::Matchers::WithinAbs(12., eps) );
  rpn.stack.drop();

  REQUIRE( ev("5. 7. LCM") == rpn::WordDefinition::Result::ok );
  REQUIRE_THAT( rpn.stack.peek_double(1), Catch::Matchers::WithinAbs(35., eps) );
  rpn.stack.drop();
}

TEST_CASE("vector type", "matrix") {
  rpn::Interp rpn(false);
  using Catch::Matchers::WithinAbs;
  auto ev = [&](const std::string &s) { return rpn.sync_eval(s); };
  constexpr double eps = 1e-10;
  auto ok = rpn::WordDefinition::Result::ok;

  // ------- construction / decomposition -------
  REQUIRE( ev("1. 2. 3. 3 ->VEC") == ok );
  REQUIRE( rpn.stack.depth() == 1 );
  REQUIRE( ev("VEC->") == ok );
  REQUIRE( rpn.stack.depth() == 4 );
  REQUIRE( rpn.stack.peek_as_integer(1) == 3 );          // count
  REQUIRE_THAT( rpn.stack.peek_double(2), Catch::Matchers::WithinAbs(3., eps) ); // v[2]
  REQUIRE_THAT( rpn.stack.peek_double(3), Catch::Matchers::WithinAbs(2., eps) ); // v[1]
  REQUIRE_THAT( rpn.stack.peek_double(4), Catch::Matchers::WithinAbs(1., eps) ); // v[0]
  rpn.stack.clear();

  // SIZE
  REQUIRE( ev("4. 5. 6. 3 ->VEC SIZE") == ok );
  REQUIRE( rpn.stack.peek_as_integer(1) == 3 );
  rpn.stack.clear();

  // ------- operations -------
  // VDOT
  REQUIRE( ev("1. 2. 3. 3 ->VEC  4. 5. 6. 3 ->VEC VDOT") == ok );
  REQUIRE_THAT( rpn.stack.peek_double(1), Catch::Matchers::WithinAbs(32., eps) ); // 1*4+2*5+3*6
  rpn.stack.clear();

  // VNORM: [3, 4] → 5
  REQUIRE( ev("3. 4. 2 ->VEC VNORM") == ok );
  REQUIRE_THAT( rpn.stack.peek_double(1), Catch::Matchers::WithinAbs(5., eps) );
  rpn.stack.clear();

  // + element-wise
  REQUIRE( ev("1. 2. 3. 3 ->VEC  10. 20. 30. 3 ->VEC + VEC->") == ok );
  REQUIRE( rpn.stack.peek_as_integer(1) == 3 );
  REQUIRE_THAT( rpn.stack.peek_double(2), Catch::Matchers::WithinAbs(33., eps) );
  REQUIRE_THAT( rpn.stack.peek_double(3), Catch::Matchers::WithinAbs(22., eps) );
  REQUIRE_THAT( rpn.stack.peek_double(4), Catch::Matchers::WithinAbs(11., eps) );
  rpn.stack.clear();

  // - element-wise
  REQUIRE( ev("10. 20. 30. 3 ->VEC  1. 2. 3. 3 ->VEC - VEC->") == ok );
  REQUIRE_THAT( rpn.stack.peek_double(2), Catch::Matchers::WithinAbs(27., eps) );
  REQUIRE_THAT( rpn.stack.peek_double(3), Catch::Matchers::WithinAbs(18., eps) );
  REQUIRE_THAT( rpn.stack.peek_double(4), Catch::Matchers::WithinAbs(9., eps) );
  rpn.stack.clear();

  // * scalar (vec * scalar)
  REQUIRE( ev("1. 2. 3. 3 ->VEC 2. * VEC->") == ok );
  REQUIRE_THAT( rpn.stack.peek_double(2), Catch::Matchers::WithinAbs(6., eps) );
  REQUIRE_THAT( rpn.stack.peek_double(3), Catch::Matchers::WithinAbs(4., eps) );
  REQUIRE_THAT( rpn.stack.peek_double(4), Catch::Matchers::WithinAbs(2., eps) );
  rpn.stack.clear();

  // ------- interop -------
  // VEC->COLVEC, COLVEC->VEC
  REQUIRE( ev("1. 2. 3. 3 ->VEC VEC->COLVEC COLVEC->VEC VEC->") == ok );
  REQUIRE( rpn.stack.peek_as_integer(1) == 3 );
  REQUIRE_THAT( rpn.stack.peek_double(2), Catch::Matchers::WithinAbs(3., eps) );
  REQUIRE_THAT( rpn.stack.peek_double(3), Catch::Matchers::WithinAbs(2., eps) );
  REQUIRE_THAT( rpn.stack.peek_double(4), Catch::Matchers::WithinAbs(1., eps) );
  rpn.stack.clear();

  // VEC3->VEC, VEC->VEC3
  REQUIRE( ev("1. 2. 3. ->VEC3 VEC3->VEC VEC->") == ok );
  REQUIRE( rpn.stack.peek_as_integer(1) == 3 );
  REQUIRE_THAT( rpn.stack.peek_double(2), Catch::Matchers::WithinAbs(3., eps) );
  REQUIRE_THAT( rpn.stack.peek_double(3), Catch::Matchers::WithinAbs(2., eps) );
  REQUIRE_THAT( rpn.stack.peek_double(4), Catch::Matchers::WithinAbs(1., eps) );
  rpn.stack.clear();
}

TEST_CASE("matrix type", "matrix") {
  rpn::Interp rpn(false);
  using Catch::Matchers::WithinAbs;
  auto ev = [&](const std::string &s) { return rpn.sync_eval(s); };
  constexpr double eps = 1e-10;
  auto ok = rpn::WordDefinition::Result::ok;

  // ------- construction / decomposition -------
  // [[1 2][3 4]], row-major: 1 2 3 4 then rows=2 cols=2
  REQUIRE( ev("1. 2. 3. 4. 2 2 ->MATRIX") == ok );
  REQUIRE( rpn.stack.depth() == 1 );
  REQUIRE( ev("MATRIX->") == ok );
  REQUIRE( rpn.stack.depth() == 6 );
  REQUIRE( rpn.stack.peek_as_integer(1) == 2 );                   // cols
  REQUIRE( rpn.stack.peek_as_integer(2) == 2 );                   // rows
  REQUIRE_THAT( rpn.stack.peek_double(3), Catch::Matchers::WithinAbs(4., eps) );   // [1][1]
  REQUIRE_THAT( rpn.stack.peek_double(4), Catch::Matchers::WithinAbs(3., eps) );   // [1][0]
  REQUIRE_THAT( rpn.stack.peek_double(5), Catch::Matchers::WithinAbs(2., eps) );   // [0][1]
  REQUIRE_THAT( rpn.stack.peek_double(6), Catch::Matchers::WithinAbs(1., eps) );   // [0][0]
  rpn.stack.clear();

  // ROWS / COLS
  REQUIRE( ev("1. 2. 3. 4. 5. 6. 2 3 ->MATRIX ROWS") == ok );
  REQUIRE( rpn.stack.peek_as_integer(1) == 2 );
  rpn.stack.clear();

  REQUIRE( ev("1. 2. 3. 4. 5. 6. 2 3 ->MATRIX COLS") == ok );
  REQUIRE( rpn.stack.peek_as_integer(1) == 3 );
  rpn.stack.clear();

  // ------- operations -------
  // DET: det([[1,2],[3,4]]) = 4 - 6 = -2
  REQUIRE( ev("1. 2. 3. 4. 2 2 ->MATRIX DET") == ok );
  REQUIRE_THAT( rpn.stack.peek_double(1), Catch::Matchers::WithinAbs(-2., eps) );
  rpn.stack.clear();

  // TRANS: [[1,2],[3,4]]^T = [[1,3],[2,4]]
  REQUIRE( ev("1. 2. 3. 4. 2 2 ->MATRIX TRANS MATRIX->") == ok );
  REQUIRE_THAT( rpn.stack.peek_double(3), Catch::Matchers::WithinAbs(4., eps) );   // [1][1]=4
  REQUIRE_THAT( rpn.stack.peek_double(4), Catch::Matchers::WithinAbs(2., eps) );   // [1][0]=2
  REQUIRE_THAT( rpn.stack.peek_double(5), Catch::Matchers::WithinAbs(3., eps) );   // [0][1]=3
  REQUIRE_THAT( rpn.stack.peek_double(6), Catch::Matchers::WithinAbs(1., eps) );   // [0][0]=1
  rpn.stack.clear();

  // INV: inv([[1,2],[3,4]]) = [[-2,1],[1.5,-0.5]]
  REQUIRE( ev("1. 2. 3. 4. 2 2 ->MATRIX INV MATRIX->") == ok );
  REQUIRE_THAT( rpn.stack.peek_double(3), Catch::Matchers::WithinAbs(-0.5, eps) ); // [1][1]
  REQUIRE_THAT( rpn.stack.peek_double(4), Catch::Matchers::WithinAbs(1.5,  eps) ); // [1][0]
  REQUIRE_THAT( rpn.stack.peek_double(5), Catch::Matchers::WithinAbs(1.,   eps) ); // [0][1]
  REQUIRE_THAT( rpn.stack.peek_double(6), Catch::Matchers::WithinAbs(-2.,  eps) ); // [0][0]
  rpn.stack.clear();

  // IDENTITY: 3×3
  REQUIRE( ev("3 IDENTITY MATRIX->") == ok );
  REQUIRE( rpn.stack.peek_as_integer(1) == 3 );  // cols
  REQUIRE( rpn.stack.peek_as_integer(2) == 3 );  // rows
  REQUIRE_THAT( rpn.stack.peek_double(3), Catch::Matchers::WithinAbs(1., eps) );   // [2][2]
  REQUIRE_THAT( rpn.stack.peek_double(4), Catch::Matchers::WithinAbs(0., eps) );   // [2][1]
  REQUIRE_THAT( rpn.stack.peek_double(5), Catch::Matchers::WithinAbs(0., eps) );   // [2][0]
  rpn.stack.clear();

  // + add two matrices
  REQUIRE( ev("1. 2. 3. 4. 2 2 ->MATRIX  10. 20. 30. 40. 2 2 ->MATRIX + MATRIX->") == ok );
  REQUIRE_THAT( rpn.stack.peek_double(3), Catch::Matchers::WithinAbs(44., eps) );
  REQUIRE_THAT( rpn.stack.peek_double(6), Catch::Matchers::WithinAbs(11., eps) );
  rpn.stack.clear();

  // * matrix-matrix: [[1,2],[3,4]] * [[5,6],[7,8]] = [[19,22],[43,50]]
  REQUIRE( ev("1. 2. 3. 4. 2 2 ->MATRIX  5. 6. 7. 8. 2 2 ->MATRIX  * MATRIX->") == ok );
  REQUIRE_THAT( rpn.stack.peek_double(3), Catch::Matchers::WithinAbs(50., eps) ); // [1][1]
  REQUIRE_THAT( rpn.stack.peek_double(4), Catch::Matchers::WithinAbs(43., eps) ); // [1][0]
  REQUIRE_THAT( rpn.stack.peek_double(5), Catch::Matchers::WithinAbs(22., eps) ); // [0][1]
  REQUIRE_THAT( rpn.stack.peek_double(6), Catch::Matchers::WithinAbs(19., eps) ); // [0][0]
  rpn.stack.clear();

  // * matrix-vector: [[1,2],[3,4]] * [5,6] = [17,39]
  REQUIRE( ev("1. 2. 3. 4. 2 2 ->MATRIX  5. 6. 2 ->VEC  * VEC->") == ok );
  REQUIRE( rpn.stack.peek_as_integer(1) == 2 );
  REQUIRE_THAT( rpn.stack.peek_double(2), Catch::Matchers::WithinAbs(39., eps) ); // v[1]
  REQUIRE_THAT( rpn.stack.peek_double(3), Catch::Matchers::WithinAbs(17., eps) ); // v[0]
  rpn.stack.clear();

  // * scalar scale
  REQUIRE( ev("1. 2. 3. 4. 2 2 ->MATRIX 2. * MATRIX->") == ok );
  REQUIRE_THAT( rpn.stack.peek_double(3), Catch::Matchers::WithinAbs(8., eps) );
  REQUIRE_THAT( rpn.stack.peek_double(6), Catch::Matchers::WithinAbs(2., eps) );
  rpn.stack.clear();
}

TEST_CASE("stats words", "stats") {
  rpn::Interp rpn(false);
  using Catch::Matchers::WithinAbs;
  auto ev = [&](const std::string &s) { return rpn.sync_eval(s); };
  constexpr double eps = 1e-10;
  auto ok = rpn::WordDefinition::Result::ok;

  // MEAN: [1,2,3,4,5] → 3.0
  REQUIRE( ev("1. 2. 3. 4. 5. 5 ->VEC MEAN") == ok );
  REQUIRE_THAT( rpn.stack.peek_double(1), Catch::Matchers::WithinAbs(3.0, eps) );
  rpn.stack.clear();

  // VARIANCE: [2,4,4,4,5,5,7,9] → 4.571428... (sample)
  REQUIRE( ev("2. 4. 4. 4. 5. 5. 7. 9. 8 ->VEC VARIANCE") == ok );
  REQUIRE_THAT( rpn.stack.peek_double(1), Catch::Matchers::WithinAbs(32.0/7.0, eps) );
  rpn.stack.clear();

  // STDDEV: same data → sqrt(4.5714...)
  REQUIRE( ev("2. 4. 4. 4. 5. 5. 7. 9. 8 ->VEC STDDEV") == ok );
  REQUIRE_THAT( rpn.stack.peek_double(1), Catch::Matchers::WithinAbs(std::sqrt(32.0/7.0), eps) );
  rpn.stack.clear();

  // CORRELATION: perfect positive correlation
  REQUIRE( ev("1. 2. 3. 4. 5. 5 ->VEC  1. 2. 3. 4. 5. 5 ->VEC  CORRELATION") == ok );
  REQUIRE_THAT( rpn.stack.peek_double(1), Catch::Matchers::WithinAbs(1.0, eps) );
  rpn.stack.clear();

  // CORRELATION: perfect negative correlation
  REQUIRE( ev("1. 2. 3. 4. 5. 5 ->VEC  5. 4. 3. 2. 1. 5 ->VEC  CORRELATION") == ok );
  REQUIRE_THAT( rpn.stack.peek_double(1), Catch::Matchers::WithinAbs(-1.0, eps) );
  rpn.stack.clear();

  // LINFIT: y = 2x + 1 → slope=2, intercept=1
  // x=[1,2,3,4,5], y=[3,5,7,9,11]
  REQUIRE( ev("1. 2. 3. 4. 5. 5 ->VEC  3. 5. 7. 9. 11. 5 ->VEC  LINFIT") == ok );
  REQUIRE( rpn.stack.depth() == 2 );
  REQUIRE_THAT( rpn.stack.peek_double(1), Catch::Matchers::WithinAbs(2.0, eps) );   // slope  (TOS)
  REQUIRE_THAT( rpn.stack.peek_double(2), Catch::Matchers::WithinAbs(1.0, eps) );   // intercept (NOS)
  rpn.stack.clear();

  // OLS: same as LINFIT but via matrix interface
  // X = [1 x] (intercept column), y = [3,5,7,9,11]
  REQUIRE( ev("1. 1. 1. 2. 1. 3. 1. 4. 1. 5. 5 2 ->MATRIX  3. 5. 7. 9. 11. 5 ->VEC  OLS") == ok );
  REQUIRE( rpn.stack.depth() == 1 );
  {
    auto obj = rpn.stack.pop();
    auto &res = POP_CAST(stack::Object, obj);
    const auto &beta_v = PEEK_CAST(::stack::Vector, res.member("beta"));
    REQUIRE_THAT( beta_v.get(0), Catch::Matchers::WithinAbs(1.0, eps) );  // intercept
    REQUIRE_THAT( beta_v.get(1), Catch::Matchers::WithinAbs(2.0, eps) );  // slope
    REQUIRE_THAT( PEEK_CAST(stack::Double, res.member("rsq")).operator double(), Catch::Matchers::WithinAbs(1.0, eps) );
    REQUIRE_THAT( PEEK_CAST(stack::Double, res.member("ess")).operator double(), Catch::Matchers::WithinAbs(0.0, 1e-8) );
  }
  rpn.stack.clear();
}

TEST_CASE("marker and collection literals", "types") {
  auto &rpn = g_rpn();
  rpn.stack.clear();
  auto ok = rpn::WordDefinition::Result::ok;
  auto ev = [&](const std::string &s) { return rpn.sync_eval(s); };

  // --- MARK / FIND-MARK primitives ---

  // MARK creates a marker from a string label
  REQUIRE( ev("\"[\" MARK") == ok );
  REQUIRE( rpn.stack.depth() == 1 );
  {
    auto m = rpn.stack.pop();
    REQUIRE( dynamic_cast<const stack::Marker*>(m.get()) != nullptr );
    REQUIRE( dynamic_cast<const stack::Marker*>(m.get())->label() == "[" );
  }

  // FIND-MARK returns count of items above marker
  REQUIRE( ev("\"[\" MARK 1. 2. 3. \"[\" FIND-MARK") == ok );
  REQUIRE( rpn.stack.depth() == 5 );           // marker + 3 values + count
  REQUIRE( rpn.stack.peek_integer(1) == 3 );   // 3 items above marker
  rpn.stack.clear();

  // FIND-MARK throws on missing marker
  REQUIRE( ev("\"[\" FIND-MARK") != ok );
  rpn.stack.clear();

  // --- [ ] vector literal ---

  REQUIRE( ev("[ 1. 2. 3. ]") == ok );
  REQUIRE( rpn.stack.depth() == 1 );
  {
    auto sv = rpn.stack.pop();
    auto &vec = POP_CAST(stack::Vector, sv);
    REQUIRE( vec.size() == 3 );
    REQUIRE_THAT( vec.get(0), Catch::Matchers::WithinAbs(1.0, 1e-12) );
    REQUIRE_THAT( vec.get(1), Catch::Matchers::WithinAbs(2.0, 1e-12) );
    REQUIRE_THAT( vec.get(2), Catch::Matchers::WithinAbs(3.0, 1e-12) );
  }

  // Single-element vector
  REQUIRE( ev("[ 42. ]") == ok );
  REQUIRE( rpn.stack.depth() == 1 );
  {
    auto sv = rpn.stack.pop();
    auto &vec = POP_CAST(stack::Vector, sv);
    REQUIRE( vec.size() == 1 );
    REQUIRE_THAT( vec.get(0), Catch::Matchers::WithinAbs(42.0, 1e-12) );
  }

  // Values already on stack are unaffected by the literal
  REQUIRE( ev("99. [ 1. 2. ]") == ok );
  REQUIRE( rpn.stack.depth() == 2 );
  REQUIRE_THAT( rpn.stack.peek_double(2), Catch::Matchers::WithinAbs(99.0, 1e-12) );
  {
    auto sv = rpn.stack.pop();
    auto &vec = POP_CAST(stack::Vector, sv);
    REQUIRE( vec.size() == 2 );
  }
  rpn.stack.clear();

  // --- { } object literal ---

  REQUIRE( ev("{ \"x\" 1. \"y\" 2. }") == ok );
  REQUIRE( rpn.stack.depth() == 1 );
  {
    auto so = rpn.stack.pop();
    auto &obj = POP_CAST(stack::Object, so);
    REQUIRE( obj.has_member("x") );
    REQUIRE( obj.has_member("y") );
    REQUIRE_THAT( PEEK_CAST(stack::Double, obj.member("x")).operator double(), Catch::Matchers::WithinAbs(1.0, 1e-12) );
    REQUIRE_THAT( PEEK_CAST(stack::Double, obj.member("y")).operator double(), Catch::Matchers::WithinAbs(2.0, 1e-12) );
  }

  // Single key/value pair
  REQUIRE( ev("{ \"name\" \"alice\" }") == ok );
  REQUIRE( rpn.stack.depth() == 1 );
  {
    auto so = rpn.stack.pop();
    auto &obj = POP_CAST(stack::Object, so);
    REQUIRE( obj.has_member("name") );
    REQUIRE( std::string(PEEK_CAST(stack::String, obj.member("name"))) == "alice" );
  }

  // Odd item count → error
  REQUIRE( ev("{ \"x\" 1. \"y\" }") != ok );
  rpn.stack.clear();

  // --- [ ] inside a compiled word ---

  REQUIRE( ev(": make-vec [ 1. 2. 3. ] ; make-vec") == ok );
  REQUIRE( rpn.stack.depth() == 1 );
  {
    auto sv = rpn.stack.pop();
    auto &vec = POP_CAST(stack::Vector, sv);
    REQUIRE( vec.size() == 3 );
  }
  rpn.stack.clear();

  // --- [ [ ] [ ] ] matrix literal ---

  // 2x3 matrix: [ [1 2 3] [4 5 6] ]
  REQUIRE( ev("[ [ 1. 2. 3. ] [ 4. 5. 6. ] ]") == ok );
  REQUIRE( rpn.stack.depth() == 1 );
  {
    auto sv = rpn.stack.pop();
    auto &mat = POP_CAST(stack::Matrix, sv);
    REQUIRE( mat.rows() == 2 );
    REQUIRE( mat.cols() == 3 );
    REQUIRE_THAT( mat.get(0, 0), Catch::Matchers::WithinAbs(1.0, 1e-12) );
    REQUIRE_THAT( mat.get(0, 2), Catch::Matchers::WithinAbs(3.0, 1e-12) );
    REQUIRE_THAT( mat.get(1, 0), Catch::Matchers::WithinAbs(4.0, 1e-12) );
    REQUIRE_THAT( mat.get(1, 2), Catch::Matchers::WithinAbs(6.0, 1e-12) );
  }
  rpn.stack.clear();

  // Non-conformant rows (different lengths) → error
  REQUIRE( ev("[ [ 1. 2. ] [ 3. 4. 5. ] ]") != ok );
  rpn.stack.clear();
}

// Regression: ->RADIX / ->PRECISION run on the async interpreter's
// main_loop thread, but the stack is rendered (to_latex / to_string)
// on the caller's thread.  The display context backing those render
// functions must therefore be visible cross-thread — it was once
// thread_local, so a radix/precision set by a word was never seen by
// renders on another thread.
TEST_CASE("display context set on the engine thread is visible to renders elsewhere", "[radix][precision][threading]") {
  auto runOnEngineThread = [](rpn::Interp &rpn, const std::string &line) {
    std::promise<void> done;
    rpn.eval(line, [&done](rpn::WordDefinition::Result) { done.set_value(); });
    done.get_future().wait();
  };

  SECTION("radix") {
    rpn::Interp rpn(true);   // async → words execute on the main_loop thread
    runOnEngineThread(rpn, "0d255 0d16 ->RADIX");
    // Render the stacked integer on THIS (test) thread.
    REQUIRE( rpn.stack.peek(1).to_latex() == "FF_{16}" );
  }

  SECTION("precision") {
    rpn::Interp rpn(true);
    runOnEngineThread(rpn, "1.23456789 0d3 ->PRECISION");
    REQUIRE( rpn.stack.peek(1).to_latex() == "1.235" );
  }
}

TEST_CASE("finance: TVM blank constructor", "[finance]") {
  rpn::Interp rpn(false);
  REQUIRE( rpn.sync_eval("TVM") == rpn::WordDefinition::Result::ok );
  REQUIRE( rpn.stack.depth() == 1 );
  const auto &t = PEEK_CAST(stack::Tvm, rpn.stack.peek(1));
  REQUIRE( t.type_name() == "tvm" );
  REQUIRE( t.n == 0.0 );
  REQUIRE( t.i == 0.0 );
  REQUIRE( t.pv == 0.0 );
  REQUIRE( t.pmt == 0.0 );
  REQUIRE( t.fv == 0.0 );
  REQUIRE( t.begin == false );
  REQUIRE( t.solveFor == stack::Tvm::SolveFor::none );
}

TEST_CASE("finance: ->TVM constructor and TVM-> decompose", "[finance]") {
  rpn::Interp rpn(false);

  SECTION("->TVM fills every field") {
    REQUIRE( rpn.sync_eval("10. 5. -1000. 0. 1628.89 TRUE ->TVM")
             == rpn::WordDefinition::Result::ok );
    REQUIRE( rpn.stack.depth() == 1 );
    const auto &t = PEEK_CAST(stack::Tvm, rpn.stack.peek(1));
    REQUIRE( t.n == 10.0 );
    REQUIRE( t.i == 5.0 );
    REQUIRE( t.pv == -1000.0 );
    REQUIRE( t.pmt == 0.0 );
    REQUIRE_THAT( t.fv, Catch::Matchers::WithinAbs(1628.89, 1e-9) );
    REQUIRE( t.begin == true );
    REQUIRE( t.solveFor == stack::Tvm::SolveFor::none );
  }

  SECTION("TVM-> is the inverse of ->TVM") {
    REQUIRE( rpn.sync_eval("10. 5. -1000. 0. 1628.89 FALSE ->TVM TVM->")
             == rpn::WordDefinition::Result::ok );
    REQUIRE( rpn.stack.depth() == 6 );
    REQUIRE( rpn.stack.peek_boolean(1) == false );           // begin (TOS)
    REQUIRE_THAT( rpn.stack.peek_double(2), Catch::Matchers::WithinAbs(1628.89, 1e-9) ); // fv
    REQUIRE( rpn.stack.peek_double(3) == 0.0 );              // pmt
    REQUIRE( rpn.stack.peek_double(4) == -1000.0 );          // pv
    REQUIRE( rpn.stack.peek_double(5) == 5.0 );              // i
    REQUIRE( rpn.stack.peek_double(6) == 10.0 );             // n
  }

  SECTION("deparse round-trips") {
    REQUIRE( rpn.sync_eval("10. 5. -1000. 0. 1628.89 FALSE ->TVM")
             == rpn::WordDefinition::Result::ok );
    auto original = rpn.stack.peek(1).deep_copy();
    REQUIRE( rpn.sync_eval("DEPARSE EVAL") == rpn::WordDefinition::Result::ok );
    REQUIRE( rpn.stack.depth() == 1 );
    REQUIRE( *original == rpn.stack.peek(1) );
  }
}

TEST_CASE("finance: closed-form TVM solvers", "[finance]") {
  rpn::Interp rpn(false);
  using Catch::Matchers::WithinAbs;

  SECTION("SOLVE-FV") {
    rpn.sync_eval("10. 5. -1000. 0. 0. FALSE ->TVM SOLVE-FV");
    const auto &t = PEEK_CAST(stack::Tvm, rpn.stack.peek(1));
    REQUIRE_THAT( t.fv, WithinAbs(1628.8946268, 1e-4) );
    REQUIRE( t.solveFor == stack::Tvm::SolveFor::fv );
  }
  SECTION("SOLVE-PV") {
    rpn.sync_eval("10. 5. 0. 0. 1628.8946268 FALSE ->TVM SOLVE-PV");
    const auto &t = PEEK_CAST(stack::Tvm, rpn.stack.peek(1));
    REQUIRE_THAT( t.pv, WithinAbs(-1000.0, 1e-4) );
  }
  SECTION("SOLVE-N") {
    rpn.sync_eval("0. 5. -1000. 0. 1628.8946268 FALSE ->TVM SOLVE-N");
    const auto &t = PEEK_CAST(stack::Tvm, rpn.stack.peek(1));
    REQUIRE_THAT( t.n, WithinAbs(10.0, 1e-6) );
  }
  SECTION("SOLVE-PMT on a 48-month loan") {
    // 20000 borrowed, 48 months, 5%/yr = 5/12 %/month, fv 0.
    rpn.sync_eval("48. 5. 12. / 20000. 0. 0. FALSE ->TVM SOLVE-PMT");
    const auto &t = PEEK_CAST(stack::Tvm, rpn.stack.peek(1));
    REQUIRE_THAT( t.pmt, WithinAbs(-460.59, 0.01) );
  }
  SECTION("zero-rate degenerate case") {
    rpn.sync_eval("10. 0. -1000. 0. 0. FALSE ->TVM SOLVE-FV");
    const auto &t = PEEK_CAST(stack::Tvm, rpn.stack.peek(1));
    REQUIRE_THAT( t.fv, WithinAbs(1000.0, 1e-9) );  // 0 = pv + fv
  }
  SECTION("deparse round-trips a solved tvm") {
    // a solveFor != none tvm exercises deparse()'s SOLVE-<X> suffix
    rpn.sync_eval("10. 5. -1000. 0. 0. FALSE ->TVM SOLVE-FV");
    auto solved = rpn.stack.peek(1).deep_copy();
    REQUIRE( rpn.sync_eval("DEPARSE EVAL") == rpn::WordDefinition::Result::ok );
    REQUIRE( rpn.stack.depth() == 1 );
    REQUIRE( *solved == rpn.stack.peek(1) );
  }
}

TEST_CASE("finance: iterative SOLVE-I", "[finance]") {
  rpn::Interp rpn(false);
  using Catch::Matchers::WithinAbs;

  SECTION("recovers a known 5% rate") {
    rpn.sync_eval("10. 0. -1000. 0. 1628.8946268 FALSE ->TVM SOLVE-I");
    REQUIRE( rpn.stack.depth() == 1 );
    const auto &t = PEEK_CAST(stack::Tvm, rpn.stack.peek(1));
    REQUIRE_THAT( t.i, WithinAbs(5.0, 1e-4) );
    REQUIRE( t.solveFor == stack::Tvm::SolveFor::i );
  }
  SECTION("loan rate") {
    // 20000 over 48 months at -460.59/month implies 5/12 %/month.
    rpn.sync_eval("48. 0. 20000. -460.59 0. FALSE ->TVM SOLVE-I");
    const auto &t = PEEK_CAST(stack::Tvm, rpn.stack.peek(1));
    REQUIRE_THAT( t.i, WithinAbs(5.0/12.0, 1e-3) );
  }
  SECTION("zero-rate, begin mode — bracket collapses to a tight root") {
    // annuity-due: 100/period x10 == 1000 principal at 0% → i ≈ 0.
    rpn.sync_eval("10. 0. -1000. 100. 0. TRUE ->TVM SOLVE-I");
    REQUIRE( rpn.stack.depth() == 1 );
    const auto &t = PEEK_CAST(stack::Tvm, rpn.stack.peek(1));
    REQUIRE_THAT( t.i, WithinAbs(0.0, 1e-4) );
  }
  SECTION("deparse round-trips a solved-i tvm") {
    rpn.sync_eval("10. 0. -1000. 0. 1628.8946268 FALSE ->TVM SOLVE-I");
    auto solved = rpn.stack.peek(1).deep_copy();
    REQUIRE( rpn.sync_eval("DEPARSE EVAL") == rpn::WordDefinition::Result::ok );
    REQUIRE( rpn.stack.depth() == 1 );
    REQUIRE( *solved == rpn.stack.peek(1) );
  }
}

TEST_CASE("finance: TVM field setters with auto-re-solve", "[finance]") {
  rpn::Interp rpn(false);
  using Catch::Matchers::WithinAbs;

  SECTION("changing an input re-solves the dependent field") {
    // 20000 / 48mo / 5%apr, solve pmt; then change n to 60 → pmt updates.
    rpn.sync_eval("48. 5. 12. / 20000. 0. 0. FALSE ->TVM SOLVE-PMT");
    const auto &t1 = PEEK_CAST(stack::Tvm, rpn.stack.peek(1));
    REQUIRE_THAT( t1.pmt, WithinAbs(-460.59, 0.01) );
    rpn.sync_eval("60. TVM-N");
    const auto &t2 = PEEK_CAST(stack::Tvm, rpn.stack.peek(1));
    REQUIRE( t2.n == 60.0 );
    REQUIRE_THAT( t2.pmt, WithinAbs(-377.42, 0.01) );        // re-solved
    REQUIRE( t2.solveFor == stack::Tvm::SolveFor::pmt );
  }
  SECTION("setting the dependent field itself clears solveFor") {
    rpn.sync_eval("48. 5. 12. / 20000. 0. 0. FALSE ->TVM SOLVE-PMT");
    rpn.sync_eval("-500. TVM-PMT");
    const auto &t = PEEK_CAST(stack::Tvm, rpn.stack.peek(1));
    REQUIRE( t.pmt == -500.0 );
    REQUIRE( t.solveFor == stack::Tvm::SolveFor::none );
  }
  SECTION("TVM-BEGIN / TVM-END set payment timing") {
    rpn.sync_eval("TVM TVM-BEGIN");
    REQUIRE( PEEK_CAST(stack::Tvm, rpn.stack.peek(1)).begin == true );
    rpn.sync_eval("TVM-END");
    REQUIRE( PEEK_CAST(stack::Tvm, rpn.stack.peek(1)).begin == false );
  }
}

TEST_CASE("finance: AMORT schedule", "[finance]") {
  rpn::Interp rpn(false);
  using Catch::Matchers::WithinAbs;

  SECTION("end-of-period loan") {
    // 1000 borrowed, 3 periods, 10%/period, fv 0 → pmt ≈ -402.1148
    rpn.sync_eval("3. 10. 1000. 0. 0. FALSE ->TVM SOLVE-PMT AMORT");
    REQUIRE( rpn.stack.depth() == 2 );           // tvm (NOS) + obj (TOS)

    const auto &obj = PEEK_CAST(stack::Object, rpn.stack.peek(1));
    const auto &interest  = PEEK_CAST(stack::Vector, obj.member("interest"));
    const auto &principal = PEEK_CAST(stack::Vector, obj.member("principal"));
    const auto &balance   = PEEK_CAST(stack::Vector, obj.member("balance"));

    REQUIRE( interest.vec().size() == 3 );
    REQUIRE_THAT( interest.vec()(0),  WithinAbs(100.0,   0.01) );
    REQUIRE_THAT( interest.vec()(1),  WithinAbs(69.789,  0.01) );
    REQUIRE_THAT( principal.vec()(0), WithinAbs(302.115, 0.01) );
    REQUIRE_THAT( balance.vec()(2),   WithinAbs(0.0,     0.01) );

    const auto &totInt  = PEEK_CAST(stack::Double, obj.member("total-interest"));
    const auto &totPrin = PEEK_CAST(stack::Double, obj.member("total-principal"));
    REQUIRE_THAT( (double)totInt,  WithinAbs(206.345,  0.01) );
    REQUIRE_THAT( (double)totPrin, WithinAbs(1000.0,   0.01) );  // == pv

    // the tvm is still underneath, untouched
    REQUIRE( PEEK_CAST(stack::Tvm, rpn.stack.peek(2)).n == 3.0 );
  }

  SECTION("begin-of-period loan accrues interest post-payment") {
    // same loan, begin mode → pmt ≈ -365.559; interest is lower because
    // each payment lands before the period's interest accrues.
    rpn.sync_eval("3. 10. 1000. 0. 0. TRUE ->TVM SOLVE-PMT AMORT");
    const auto &obj = PEEK_CAST(stack::Object, rpn.stack.peek(1));
    const auto &interest = PEEK_CAST(stack::Vector, obj.member("interest"));
    const auto &balance  = PEEK_CAST(stack::Vector, obj.member("balance"));

    REQUIRE_THAT( interest.vec()(0), WithinAbs(63.444, 0.01) );
    REQUIRE_THAT( interest.vec()(2), WithinAbs(0.0,    0.01) );
    REQUIRE_THAT( balance.vec()(2),  WithinAbs(0.0,    0.01) );  // fully amortized

    const auto &totInt  = PEEK_CAST(stack::Double, obj.member("total-interest"));
    const auto &totPrin = PEEK_CAST(stack::Double, obj.member("total-principal"));
    REQUIRE_THAT( (double)totInt,  WithinAbs(96.677, 0.01) );
    REQUIRE_THAT( (double)totPrin, WithinAbs(1000.0, 0.01) );    // == pv
  }
}

TEST_CASE("finance: NPV", "[finance]") {
  rpn::Interp rpn(false);
  using Catch::Matchers::WithinAbs;

  SECTION("textbook discounting") {
    // -1000 then 200 for five periods, discounted at 8%/period
    rpn.sync_eval("[ -1000. 200. 200. 200. 200. 200. ] 8. NPV");
    REQUIRE( rpn.stack.depth() == 1 );
    REQUIRE_THAT( rpn.stack.peek_double(1), WithinAbs(-201.46, 0.01) );
  }
  SECTION("zero rate is the plain sum") {
    rpn.sync_eval("[ -1000. 200. 200. 200. 200. 200. ] 0. NPV");
    REQUIRE_THAT( rpn.stack.peek_double(1), WithinAbs(0.0, 1e-9) );
  }
}

TEST_CASE("finance: IRR", "[finance]") {
  rpn::Interp rpn(false);
  using Catch::Matchers::WithinAbs;

  SECTION("known rate") {
    // -1000 then 500 x3 → IRR ≈ 23.375%
    rpn.sync_eval("[ -1000. 500. 500. 500. ] IRR");
    REQUIRE( rpn.stack.depth() == 1 );
    REQUIRE_THAT( rpn.stack.peek_double(1), WithinAbs(23.375, 0.05) );
  }
  SECTION("IRR zeroes the NPV (round-trip)") {
    rpn.sync_eval("[ -1000. 500. 500. 500. ] DUP IRR NPV");
    REQUIRE_THAT( rpn.stack.peek_double(1), WithinAbs(0.0, 1e-6) );
  }
  SECTION("no sign change → param_error") {
    auto r = rpn.sync_eval("[ 100. 200. 300. ] IRR");
    REQUIRE( r == rpn::WordDefinition::Result::param_error );
  }
}

TEST_CASE("finance: interest-rate conversion", "[finance]") {
  rpn::Interp rpn(false);
  using Catch::Matchers::WithinAbs;

  SECTION("nominal <-> effective") {
    rpn.sync_eval("12. 12. NOM->EFF");
    REQUIRE_THAT( rpn.stack.peek_double(1), WithinAbs(12.6825, 0.001) );
    rpn.sync_eval("CLEAR 12.6825 12. EFF->NOM");
    REQUIRE_THAT( rpn.stack.peek_double(1), WithinAbs(12.0, 0.001) );
  }
  SECTION("continuous <-> effective") {
    rpn.sync_eval("5. CONT->EFF");
    REQUIRE_THAT( rpn.stack.peek_double(1), WithinAbs(5.1271, 0.001) );
    rpn.sync_eval("CLEAR 5.1271 EFF->CONT");
    REQUIRE_THAT( rpn.stack.peek_double(1), WithinAbs(5.0, 0.001) );
  }
  SECTION("Fisher real <-> nominal") {
    rpn.sync_eval("3. 2. REAL->NOM");
    REQUIRE_THAT( rpn.stack.peek_double(1), WithinAbs(5.06, 0.001) );
    rpn.sync_eval("CLEAR 5.06 2. NOM->REAL");
    REQUIRE_THAT( rpn.stack.peek_double(1), WithinAbs(3.0, 0.001) );
  }
}

TEST_CASE("finance: depreciation", "[finance]") {
  rpn::Interp rpn(false);
  using Catch::Matchers::WithinAbs;

  SECTION("straight-line") {
    rpn.sync_eval("10000. 2000. 5. DEP-SL");
    const auto &obj = PEEK_CAST(stack::Object, rpn.stack.peek(1));
    const auto &dep  = PEEK_CAST(stack::Vector, obj.member("depreciation"));
    const auto &book = PEEK_CAST(stack::Vector, obj.member("book"));
    REQUIRE( dep.vec().size() == 5 );
    REQUIRE_THAT( dep.vec()(0),  WithinAbs(1600.0, 0.01) );
    REQUIRE_THAT( book.vec()(4), WithinAbs(2000.0, 0.01) );
  }
  SECTION("sum-of-years-digits") {
    rpn.sync_eval("10000. 2000. 5. DEP-SOYD");
    const auto &obj = PEEK_CAST(stack::Object, rpn.stack.peek(1));
    const auto &dep = PEEK_CAST(stack::Vector, obj.member("depreciation"));
    REQUIRE_THAT( dep.vec()(0), WithinAbs(2666.667, 0.01) ); // 8000*5/15
    REQUIRE_THAT( dep.vec()(4), WithinAbs(533.333,  0.01) ); // 8000*1/15
  }
  SECTION("declining-balance, floored at salvage") {
    rpn.sync_eval("10000. 2000. 5. 2. DEP-DB");
    const auto &obj = PEEK_CAST(stack::Object, rpn.stack.peek(1));
    const auto &dep  = PEEK_CAST(stack::Vector, obj.member("depreciation"));
    const auto &book = PEEK_CAST(stack::Vector, obj.member("book"));
    REQUIRE_THAT( dep.vec()(0),  WithinAbs(4000.0, 0.01) ); // 10000*0.4
    REQUIRE_THAT( dep.vec()(1),  WithinAbs(2400.0, 0.01) ); // 6000*0.4
    REQUIRE_THAT( book.vec()(4), WithinAbs(2000.0, 0.01) ); // never below salvage
  }
}

TEST_CASE("finance: percentage math", "[finance]") {
  rpn::Interp rpn(false);
  using Catch::Matchers::WithinAbs;
  rpn.sync_eval("200. 8. %");
  REQUIRE_THAT( rpn.stack.peek_double(1), WithinAbs(16.0, 1e-9) );
  rpn.sync_eval("CLEAR 200. 250. %CHG");
  REQUIRE_THAT( rpn.stack.peek_double(1), WithinAbs(25.0, 1e-9) );
  rpn.sync_eval("CLEAR 800. 200. %T");
  REQUIRE_THAT( rpn.stack.peek_double(1), WithinAbs(25.0, 1e-9) );
  rpn.sync_eval("CLEAR 80. 100. MU-COST");
  REQUIRE_THAT( rpn.stack.peek_double(1), WithinAbs(25.0, 1e-9) );
  rpn.sync_eval("CLEAR 80. 100. MU-PRICE");
  REQUIRE_THAT( rpn.stack.peek_double(1), WithinAbs(20.0, 1e-9) );
}

TEST_CASE("finance: historical inflation", "[finance]") {
  rpn::Interp rpn(false);
  using Catch::Matchers::WithinAbs;

  SECTION("CPI in range vs out of range") {
    REQUIRE( rpn.sync_eval("1913 CPI") == rpn::WordDefinition::Result::ok );
    REQUIRE( rpn.sync_eval("CLEAR 1850 CPI") == rpn::WordDefinition::Result::param_error );
    REQUIRE( rpn.sync_eval("CLEAR 2100 CPI") == rpn::WordDefinition::Result::param_error );
  }
  SECTION("anchor values (loose — guards transcription)") {
    rpn.sync_eval("CLEAR 1923 CPI");
    REQUIRE_THAT( rpn.stack.peek_double(1), WithinAbs(17.1, 1.0) );
    rpn.sync_eval("CLEAR 2020 CPI");
    REQUIRE_THAT( rpn.stack.peek_double(1), WithinAbs(258.8, 2.0) );
  }
  SECTION("INFL-ADJUST same year is identity") {
    rpn.sync_eval("CLEAR 100. 1950 1950 INFL-ADJUST");
    REQUIRE_THAT( rpn.stack.peek_double(1), WithinAbs(100.0, 1e-9) );
  }
  SECTION("INFL-ADJUST round-trips (data-independent)") {
    rpn.sync_eval("CLEAR 100. 1920 2000 INFL-ADJUST 2000 1920 INFL-ADJUST");
    REQUIRE_THAT( rpn.stack.peek_double(1), WithinAbs(100.0, 1e-6) );
  }
  SECTION("INFL-ADJUST out-of-range year → param_error") {
    auto r = rpn.sync_eval("CLEAR 100. 1850 2000 INFL-ADJUST");
    REQUIRE( r == rpn::WordDefinition::Result::param_error );
  }
}

/* end of qinc/rpn-lang/tests/runtime-test.cpp */
