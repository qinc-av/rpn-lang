/***************************************************
 * file: rpn-lang/tests/finance-test.cpp
 *
 * @brief  Finance dictionary tests — TVM, AMORT, NPV, IRR, rate
 *         conversion, depreciation, percentage math, historical
 *         inflation. Each TEST_CASE constructs an Interp and calls
 *         addFinanceDictionary() so the binary proves the finance
 *         dictionary is self-sufficient on core.
 */
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "../rpn.h"
#include "../rpn-matrix.h"
#include "../src/finance.h"

TEST_CASE("finance: TVM blank constructor", "[finance]") {
  rpn::Interp rpn(false);
  rpn.addFinanceDictionary();
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
  rpn.addFinanceDictionary();

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
  rpn.addFinanceDictionary();
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
  rpn.addFinanceDictionary();
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
  rpn.addFinanceDictionary();
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
  rpn.addFinanceDictionary();
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
  rpn.addFinanceDictionary();
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
  rpn.addFinanceDictionary();
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
  rpn.addFinanceDictionary();
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
  rpn.addFinanceDictionary();
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
  rpn.addFinanceDictionary();
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
  rpn.addFinanceDictionary();
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

// stack::Tvm deparse round-trip — owned here per the project's per-domain
// deparse-round-trip discipline.
TEST_CASE("finance deparse round-trips", "[finance]") {
  rpn::Interp rpn(false);
  rpn.addFinanceDictionary();
  // Tvm
  {
    rpn.stack.clear();
    rpn.sync_eval("36. 0.5 200000. -1199.10 0. FALSE ->TVM");
    auto before = rpn.stack.peek(1).deep_copy();
    rpn.sync_eval("DEPARSE EVAL");
    REQUIRE( *before == rpn.stack.peek(1) );
  }
}

TEST_CASE("finance: structured signatures land in wordHelp", "[finance]") {
  rpn::Interp rpn(false);
  rpn.addFinanceDictionary();
  auto h = rpn.wordHelp("NPV");
  REQUIRE(h.effects.size() == 1);
  REQUIRE(h.effects[0] == "( cashflows:vector rate:number -- npv:number )");

  auto htvm = rpn.wordHelp("->TVM");
  REQUIRE(htvm.effects.size() == 1);
  REQUIRE(htvm.effects[0].find("n:number") != std::string::npos);
  REQUIRE(htvm.effects[0].find("-- tvm:tvm") != std::string::npos);
}

/* end of rpn-lang/tests/finance-test.cpp */
