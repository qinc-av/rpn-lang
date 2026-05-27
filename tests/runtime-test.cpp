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

/* end of qinc/rpn-lang/tests/runtime-test.cpp */
