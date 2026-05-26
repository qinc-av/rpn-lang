/***************************************************
 * file: rpn-lang/tests/core-test.cpp
 *
 * @brief  Core dictionary tests — parser, stack, math, logic, type,
 *         markers, stdlib, threading, validators. The Interp constructor
 *         alone registers everything this binary exercises.
 */
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "../rpn.h"
#include "../rpn-matrix.h"

TEST_CASE("core-test smoke", "[core]") {
  rpn::Interp rpn(false);
  rpn.sync_eval("2 3 +");
  REQUIRE(rpn.stack.depth() == 1);
  REQUIRE(rpn.stack.peek_as_integer(1).value_or(0) == 5);
}

/* end of rpn-lang/tests/core-test.cpp */
