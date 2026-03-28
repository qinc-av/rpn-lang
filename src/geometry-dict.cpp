/***************************************************
 * file: QInc/Projects/rpn-lang/src/geometry-dict.cpp
 */
/**
 * @file    geometry-dict.cpp
 * @author  Eric L. Hernes
 * @version V1.0
 * @born_on   Monday, January 13, 2025
 * @copyright (C) Copyright Q, Inc 2025
 *
 * @brief   An Eric L. Hernes Signature Series C++ module
 *
 */

#include "geometry.h"

NATIVE_WORD_DECL(geometry, circle_a) {
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  auto oo  = rpn.stack.pop();
  const auto &array = POP_CAST(stack::Array,oo);
  const auto &vals = array.val();

  std::vector<StVec3> points;

  for(size_t i=0; i<vals.size() && rv == rpn::WordDefinition::Result::ok; i++) {
    auto *vp = OBJECTP_CAST(const StVec3)(vals[i].get());
    if (vp) {
      points.push_back(*vp);
    } else {
      rv = rpn::WordDefinition::Result::param_error;
    }
  }

  if (rv == rpn::WordDefinition::Result::ok) {
    double r=0., xc=0., yc=0.;
    bool st = geometry::estimate_circle(r, xc, yc, points);

    if (st) {
      rv = rpn::WordDefinition::Result::ok;
      rpn.stack.push_double(r);
      rpn.stack.push(StVec3(xc,yc));

    } else {
      rv = rpn::WordDefinition::Result::eval_error;
    }
  }

  return rv;
}

NATIVE_WORD_DECL(geometry, circle_vvv) {
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  std::vector<StVec3> points;
  for(size_t i=0; i<3; i++) {
    auto o = rpn.stack.pop();
    points.push_back(POP_CAST(StVec3,o));
  }

  double r=0., xc=0., yc=0.;
  bool st = geometry::estimate_circle(r, xc, yc, points);

  if (st) {
    rv = rpn::WordDefinition::Result::ok;
    rpn.stack.push_double(r);
    rpn.stack.push(StVec3(xc,yc));

  } else {
    rv = rpn::WordDefinition::Result::eval_error;
  }

  return rv;
}


void
geometry::addWords(rpn::Interp &rpn) {
  rpn.setWordCategory("geometry");
  rpn.addDefinition("CIRCLE", NATIVE_WORD_WDEF(geometry, rpn::StrictTypeValidator::d1_array, circle_a, nullptr));
  rpn.addDefinition("CIRCLE", NATIVE_WORD_WDEF(geometry, rpn::StrictTypeValidator::d3_vec3_vec3_vec3, circle_vvv, nullptr));
  rpn.addWordMetadata("CIRCLE", "Estimate a circle from three vec3 points or an array of vec3 points. Pushes radius (double) and center (vec3).");
}


/* end of QInc/Projects/rpn-lang/src/geometry-dict.cpp */
