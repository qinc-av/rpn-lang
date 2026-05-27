/***************************************************
 * file: rpn-lang/tests/numeric-test.cpp
 *
 * @brief  Numeric (matrix + vec3 + mx3) dictionary tests. Each TEST_CASE
 *         constructs an Interp and calls addNumericDictionaries() so the
 *         binary proves the numeric bundle is self-sufficient on core.
 */
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "../rpn.h"
#include "../rpn-matrix.h"

TEST_CASE( "vec3", "types" ) {
}

TEST_CASE("vector type", "matrix") {
  rpn::Interp rpn(false);
  rpn.addNumericDictionaries();
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
  rpn.addNumericDictionaries();
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

/* end of rpn-lang/tests/numeric-test.cpp */
