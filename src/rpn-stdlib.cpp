/***************************************************
 * file: qinc/rpn-lang/src/rpn-stdlib.cpp
 *
 * @file    rpn-stdlib.cpp
 * @author  Eric L. Hernes
 * @version V1.0
 * @born_on   Friday, March 27, 2026
 * @copyright (C) Copyright Eric L. Hernes 2026
 * @copyright (C) Copyright Q, Inc. 2026
 *
 * @brief   RPL standard library — compiled words loaded at Interp init.
 *
 * Primitives required by this library (all in C++ dict files):
 *   EXP LN SQRT MOD GAMMA  — math-dict.cpp
 *   CHS DUP SWAP OVER ROTU DROP  — stack-dict.cpp / math-dict.cpp
 *   IF THEN ELSE END BEGIN UNTIL — rpn-interp.cpp (control flow)
 *   == TRUE FALSE  — logic-dict.cpp / rpn-interp.cpp
 *
 * Stack-effect comments use the Phase 2.3 typed validator convention:
 *   all-registered-type tokens → StrictTypeValidator
 *   HP48-style variable names  → StackSizeValidator (arity only)
 *
 */

#include "../rpn.h"

void
rpn::Interp::addStdlibWords() {

  // -------------------------------------------------------------------------
  // Stack utilities (previously defined via sync_eval in stack-dict.cpp)
  setWordCategory("stack");

  addCompiledWord("DUP2",  "( a b -- a b a b ) OVER OVER");
  addCompiledWord("DROP2", "( a b -- ) DROP DROP");

  addWordMetadata("DUP2",  "Duplicate the top two items (equivalent to OVER OVER).");
  addWordMetadata("DROP2", "Drop the top two items (equivalent to DROP DROP).");

  // -------------------------------------------------------------------------
  // Math composites (previously defined as C++ in math-dict.cpp)
  setWordCategory("math");

  addCompiledWord("SQ",    "( n -- n ) DUP *");
  addCompiledWord("HYPOT", "( a b -- c ) SQ SWAP SQ + SQRT");

  addWordMetadata("SQ",    "Square: TOS².");
  addWordMetadata("HYPOT", "Euclidean distance: sqrt(a² + b²).");

  // -------------------------------------------------------------------------
  // Hyperbolic trigonometry
  // sinh(x) = (e^x - e^(-x)) / 2
  addCompiledWord("SINH",  "( double -- double ) DUP EXP SWAP CHS EXP - 2. /");
  // cosh(x) = (e^x + e^(-x)) / 2
  addCompiledWord("COSH",  "( double -- double ) DUP EXP SWAP CHS EXP + 2. /");
  // tanh(x) = sinh(x) / cosh(x)
  addCompiledWord("TANH",  "( double -- double ) DUP SINH SWAP COSH /");
  // asinh(x) = ln(x + sqrt(x^2 + 1))
  addCompiledWord("ASINH", "( double -- double ) DUP DUP * 1. + SQRT + LN");
  // acosh(x) = ln(x + sqrt(x^2 - 1))
  addCompiledWord("ACOSH", "( double -- double ) DUP DUP * 1. - SQRT + LN");
  // atanh(x) = ln((1+x)/(1-x)) / 2
  addCompiledWord("ATANH", "( double -- double ) DUP 1. + SWAP CHS 1. + / LN 2. /");

  addWordMetadata("SINH",  "Hyperbolic sine.");
  addWordMetadata("COSH",  "Hyperbolic cosine.");
  addWordMetadata("TANH",  "Hyperbolic tangent.");
  addWordMetadata("ASINH", "Inverse hyperbolic sine.");
  addWordMetadata("ACOSH", "Inverse hyperbolic cosine. Domain: x >= 1.");
  addWordMetadata("ATANH", "Inverse hyperbolic arctangent. Domain: |x| < 1.");

  // -------------------------------------------------------------------------
  // Factorial and combinatorics
  // ! generalizes to real arguments via the gamma function: n! = Gamma(n+1)
  addCompiledWord("!",    "( double -- double ) 1. + GAMMA");
  addCompiledWord("FACT", "( double -- double ) !");

  // nCr(n,r) = n! / (r! * (n-r)!)
  addCompiledWord("nCr",
    "( double double -- double ) OVER OVER - ! SWAP ! * SWAP ! SWAP /");

  // nPr(n,r) = n! / (n-r)!
  addCompiledWord("nPr",
    "( double double -- double ) OVER OVER - ! SWAP DROP SWAP ! SWAP /");

  addWordMetadata("!",    "Factorial (generalized). n! = Gamma(n+1). Works for real n.");
  addWordMetadata("FACT", "Factorial. Alias for !.");
  addWordMetadata("nCr", "Combinations: n choose r = n! / (r! * (n-r)!).");
  addWordMetadata("nPr", "Permutations: n P r = n! / (n-r)!.");

  // -------------------------------------------------------------------------
  // Number theory
  // GCD: Euclidean algorithm using UNTIL loop.
  // Loop body: if b==0 drop b and exit; else replace (a,b) with (b, a mod b).
  // At exit the result is the remaining non-zero value (originally a).
  addCompiledWord("GCD",
    "( double double -- double ) "
    "BEGIN "
      "DUP 0. == IF DROP TRUE ELSE OVER OVER MOD ROTU DROP FALSE END "
    "UNTIL");

  // LCM(a,b) = (b / gcd(a,b)) * a  — divide before multiply to keep values small
  addCompiledWord("LCM",
    "( double double -- double ) OVER OVER GCD / *");

  addWordMetadata("GCD", "Greatest common divisor (Euclidean algorithm).");
  addWordMetadata("LCM", "Least common multiple.");

  // -------------------------------------------------------------------------
  // Geometry composites (previously defined via sync_eval in types-dict.cpp)
  setWordCategory("geometry");

  // Extract x and y from a VEC3 and build a new VEC3 with those two components.
  addCompiledWord("VEC3->{xy}",
    "( v3 -- v3' ) VEC3-> DROP ->VEC3y SWAP ->VEC3x +");

  // Build a VEC3 from x (NOS) and y (TOS).
  addCompiledWord("->{xy}",
    "( x y -- v3 ) ->VEC3x SWAP ->VEC3y +");

  addWordMetadata("VEC3->{xy}", "Project a VEC3 onto the XY plane (Z dropped).");
  addWordMetadata("->{xy}",     "Create a VEC3 from x (NOS) and y (TOS) components.");

  setWordCategory(""); // reset
}

/* end of qinc/rpn-lang/src/rpn-stdlib.cpp */
