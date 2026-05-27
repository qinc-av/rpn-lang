/***************************************************
 * file: rpn-lang/tests/geometry-test.cpp
 *
 * @brief  Geometry dictionary tests. The shared g_rpn() engine has the
 *         geometry dictionary registered on first use (and idempotently
 *         pulls the numeric bundle that geometry depends on).
 */
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "../rpn.h"
#include "../rpn-matrix.h"
#include "../src/geometry.h"

static rpn::Interp& g_rpn() {
  static rpn::Interp i(false);
  // First-use init: register the geometry dictionary on the shared engine.
  // addGeometryDictionary() idempotently pulls addNumericDictionaries().
  static int _init = (i.addGeometryDictionary(), 0);
  (void)_init;
  return i;
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

/* end of rpn-lang/tests/geometry-test.cpp */
