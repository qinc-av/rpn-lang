/***************************************************
 * file: rpn-lang/tests/timecode-test.cpp
 *
 * @brief  Timecode dictionary tests. Each TEST_CASE constructs an
 *         Interp and calls addTimecodeDictionary().
 *         Round-trip SECTIONs for stack::Timecode are owned here per
 *         the project's deparse-round-trip discipline (filled in by
 *         Task 15 of the modularity plan).
 */
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "../rpn.h"
#include "../rpn-matrix.h"
#include "../src/timecode.h"

// stack::Timecode deparse round-trip — owned here per the project's
// per-domain deparse-round-trip discipline. Needs the fraction dictionary
// too, since ->TC's argument is built with ->FRAC.
TEST_CASE("timecode deparse round-trips", "[timecode]") {
  rpn::Interp rpn(false);
  rpn.addTimecodeDictionary();
  rpn.addFractionDictionary();
  // Timecode — ->TC takes d2_frac_double (NOS:Fraction, TOS:Double).
  // Round-trip exercises that deparse emits tokens in the right order.
  {
    rpn.stack.clear();
    rpn.sync_eval("0d30000 0d1001 ->FRAC 89915. ->TC DEPARSE EVAL");
    REQUIRE( rpn.stack.depth() == 1 );
    auto obj = rpn.stack.pop();
    auto *tc = dynamic_cast<const stack::Timecode*>(obj.get());
    REQUIRE( tc != nullptr );
    REQUIRE( tc->to_frames() == 89915 );
  }
}

/* end of rpn-lang/tests/timecode-test.cpp */
