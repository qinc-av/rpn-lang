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
rpn::Interp::addStdlibDictionary() {
  if (_alreadyRegistered("stdlib")) return;

  // -------------------------------------------------------------------------
  // Stack utilities (previously defined via sync_eval in stack-dict.cpp)
  setWordCategory("stack");

  // Structured-signature note for compiled words: the inline `( name:type -- name:type )`
  // comments below are parsed by ct_SEMICOLON (via parse_typed_signature) and DO populate
  // WordDefinition.signature — help-tooltip rendering picks up the structured form.
  // Separately, parse_input_types strips the `name:` prefix when building the
  // StrictTypeValidator, so type-checked dispatch also works.
  addCompiledWord("DUP2",  "( a:any b:any -- a:any b:any a:any b:any ) OVER OVER");
  addWordMetadata("DUP2",  "Duplicate the top two items (equivalent to OVER OVER).");

  addCompiledWord("DROP2", "( a:any b:any -- ) DROP DROP");
  addWordMetadata("DROP2", "Drop the top two items (equivalent to DROP DROP).");

  // -------------------------------------------------------------------------
  // Math composites (previously defined as C++ in math-dict.cpp)
  setWordCategory("math");

  addCompiledWord("SQ",    "( n:number -- result:number ) DUP *");
  addWordMetadata("SQ",    "Square: TOS².");

  addCompiledWord("HYPOT", "( a:number b:number -- c:number ) SQ SWAP SQ + SQRT");
  addWordMetadata("HYPOT", "Euclidean distance: sqrt(a² + b²).");

  // -------------------------------------------------------------------------
  // Hyperbolic trigonometry
  // sinh(x) = (e^x - e^(-x)) / 2
  addCompiledWord("SINH",  "( x:number -- result:number ) DUP EXP SWAP CHS EXP - 2. /");
  addWordMetadata("SINH",  "Hyperbolic sine.");

  // cosh(x) = (e^x + e^(-x)) / 2
  addCompiledWord("COSH",  "( x:number -- result:number ) DUP EXP SWAP CHS EXP + 2. /");
  addWordMetadata("COSH",  "Hyperbolic cosine.");

  // tanh(x) = sinh(x) / cosh(x)
  addCompiledWord("TANH",  "( x:number -- result:number ) DUP SINH SWAP COSH /");
  addWordMetadata("TANH",  "Hyperbolic tangent.");

  // asinh(x) = ln(x + sqrt(x^2 + 1))
  addCompiledWord("ASINH", "( x:number -- result:number ) DUP DUP * 1. + SQRT + LN");
  addWordMetadata("ASINH", "Inverse hyperbolic sine.");

  // acosh(x) = ln(x + sqrt(x^2 - 1))
  addCompiledWord("ACOSH", "( x:number -- result:number ) DUP DUP * 1. - SQRT + LN");
  addWordMetadata("ACOSH", "Inverse hyperbolic cosine. Domain: x >= 1.");

  // atanh(x) = ln((1+x)/(1-x)) / 2
  addCompiledWord("ATANH", "( x:number -- result:number ) DUP 1. + SWAP CHS 1. + / LN 2. /");
  addWordMetadata("ATANH", "Inverse hyperbolic arctangent. Domain: |x| < 1.");

  // -------------------------------------------------------------------------
  // Factorial and combinatorics
  // ! generalizes to real arguments via the gamma function: n! = Gamma(n+1)
  addCompiledWord("!",    "( n:number -- result:number ) 1. + GAMMA");
  addWordMetadata("!",    "Factorial (generalized). n! = Gamma(n+1). Works for real n.");

  addCompiledWord("FACT", "( n:number -- result:number ) !");
  addWordMetadata("FACT", "Factorial. Alias for !.");

  // nCr(n,r) = n! / (r! * (n-r)!)
  addCompiledWord("nCr",
    "( n:number r:number -- result:number ) OVER OVER - ! SWAP ! * SWAP ! SWAP /");
  addWordMetadata("nCr", "Combinations: n choose r = n! / (r! * (n-r)!).");

  // nPr(n,r) = n! / (n-r)!
  addCompiledWord("nPr",
    "( n:number r:number -- result:number ) OVER OVER - ! SWAP DROP SWAP ! SWAP /");
  addWordMetadata("nPr", "Permutations: n P r = n! / (n-r)!.");

  // -------------------------------------------------------------------------
  // Number theory
  // GCD: Euclidean algorithm using UNTIL loop.
  // Loop body: if b==0 drop b and exit; else replace (a,b) with (b, a mod b).
  // At exit the result is the remaining non-zero value (originally a).
  addCompiledWord("GCD",
    "( a:number b:number -- result:number ) "
    "BEGIN "
      "DUP 0. == IF DROP TRUE ELSE OVER OVER MOD ROTU DROP FALSE END "
    "UNTIL");
  addWordMetadata("GCD", "Greatest common divisor (Euclidean algorithm).");

  // LCM(a,b) = (b / gcd(a,b)) * a  — divide before multiply to keep values small
  addCompiledWord("LCM",
    "( a:number b:number -- result:number ) OVER OVER GCD / *");
  addWordMetadata("LCM", "Least common multiple.");

  // -------------------------------------------------------------------------
  // Geometry composites (previously defined via sync_eval in types-dict.cpp)
  setWordCategory("geometry");

  // Extract x and y from a VEC3 and build a new VEC3 with those two components.
  addCompiledWord("VEC3->{xy}",
    "( v:vec3 -- result:vec3 ) VEC3-> DROP ->VEC3y SWAP ->VEC3x +");
  addWordMetadata("VEC3->{xy}", "Project a VEC3 onto the XY plane (Z dropped).");

  // Build a VEC3 from x (NOS) and y (TOS).
  addCompiledWord("->{xy}",
    "( x:number y:number -- v:vec3 ) ->VEC3x SWAP ->VEC3y +");
  addWordMetadata("->{xy}",     "Create a VEC3 from x (NOS) and y (TOS) components.");

  setWordCategory(""); // reset

  // -------------------------------------------------------------------------
  // Collection literal close words — consume marker and build collection
  setWordCategory("types");

  // }  ( {... k1 v1 .. kn vn -- obj )
  // ] is a native C++ word registered in addMarkerDictionary() (types-dict.cpp)
  addCompiledWord("}", "( -- obj:object ) \"{\" FIND-MARK ->OBJECT SWAP DROP");
  addWordMetadata("}", "Close an object literal started with `{`.  Collects key/value pairs above the `{` marker into an object.");

  setWordCategory(""); // reset
}

/* end of qinc/rpn-lang/src/rpn-stdlib.cpp */
