/***************************************************
 * file: rpn-lang/tests/stats-test.cpp
 *
 * @brief  Stats dictionary tests. Each TEST_CASE constructs an Interp and
 *         calls addStatsDictionary().
 */
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "../rpn.h"
#include "../rpn-matrix.h"

TEST_CASE("stats words", "stats") {
  rpn::Interp rpn(false);
  rpn.addStatsDictionary();
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

/* end of rpn-lang/tests/stats-test.cpp */
