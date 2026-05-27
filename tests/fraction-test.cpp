/***************************************************
 * file: rpn-lang/tests/fraction-test.cpp
 *
 * @brief  Fraction dictionary tests. Each TEST_CASE constructs an
 *         Interp and calls addFractionDictionary().
 *         Round-trip SECTIONs for stack::Fraction are owned here per
 *         the project's deparse-round-trip discipline (filled in by
 *         Task 15 of the modularity plan).
 */
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "../rpn.h"
#include "../rpn-matrix.h"
#include "../src/fraction.h"

TEST_CASE("fraction-test smoke", "[fraction]") {
  rpn::Interp rpn(false);
  rpn.addFractionDictionary();
  REQUIRE(rpn.wordExists("->FRAC"));
}

/* end of rpn-lang/tests/fraction-test.cpp */
