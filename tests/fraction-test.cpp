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

// stack::Fraction deparse round-trip — owned here per the project's
// per-domain deparse-round-trip discipline.
TEST_CASE("fraction deparse round-trips", "[fraction]") {
  rpn::Interp rpn(false);
  rpn.addFractionDictionary();
  // Fraction (use 0d prefix to create integers — ->FRAC expects d2_integer_integer)
  {
    rpn.stack.clear();
    rpn.sync_eval("0d3 0d4 ->FRAC DEPARSE EVAL");
    REQUIRE( rpn.stack.depth() == 1 );
    REQUIRE_THAT( rpn.stack.peek_as_double(1).value(), Catch::Matchers::WithinAbs(0.75, 1e-10) );
  }
}

/* end of rpn-lang/tests/fraction-test.cpp */
