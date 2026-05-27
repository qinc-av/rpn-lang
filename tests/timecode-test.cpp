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

TEST_CASE("timecode-test smoke", "[timecode]") {
  rpn::Interp rpn(false);
  rpn.addTimecodeDictionary();
  REQUIRE(rpn.wordExists("->TC"));
}

/* end of rpn-lang/tests/timecode-test.cpp */
