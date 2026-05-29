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

  std::vector<stack::Vec3> points;

  for(size_t i=0; i<vals.size() && rv == rpn::WordDefinition::Result::ok; i++) {
    auto *vp = OBJECTP_CAST(const stack::Vec3)(vals[i].get());
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
      rpn.stack.push(stack::Vec3(xc,yc));

    } else {
      rv = rpn::WordDefinition::Result::eval_error;
    }
  }

  return rv;
}

NATIVE_WORD_DECL(geometry, circle_vvv) {
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  std::vector<stack::Vec3> points;
  for(size_t i=0; i<3; i++) {
    auto o = rpn.stack.pop();
    points.push_back(POP_CAST(stack::Vec3,o));
  }

  double r=0., xc=0., yc=0.;
  bool st = geometry::estimate_circle(r, xc, yc, points);

  if (st) {
    rv = rpn::WordDefinition::Result::ok;
    rpn.stack.push_double(r);
    rpn.stack.push(stack::Vec3(xc,yc));

  } else {
    rv = rpn::WordDefinition::Result::eval_error;
  }

  return rv;
}


void
geometry::addWords(rpn::Interp &rpn) {
  rpn.setWordCategory("geometry");
  rpn.addDefinition("CIRCLE", {
    rpn::StrictTypeValidator::d1_array,
    NATIVE_WORD_FN(geometry, circle_a), nullptr, "",
    rpn::StackEffect{
      {{"points", "array"}},
      {{"radius", "number"}, {"center", "vec3"}}
    }
  });
  rpn.addDefinition("CIRCLE", {
    rpn::StrictTypeValidator::d3_vec3_vec3_vec3,
    NATIVE_WORD_FN(geometry, circle_vvv), nullptr, "",
    rpn::StackEffect{
      {{"p1", "vec3"}, {"p2", "vec3"}, {"p3", "vec3"}},
      {{"radius", "number"}, {"center", "vec3"}}
    }
  });
  rpn.addWordMetadata("CIRCLE", "Estimate a circle from three vec3 points or an array of vec3 points. Pushes radius (double) and center (vec3).");
}

void
rpn::Interp::addGeometryDictionary() {
  if (_alreadyRegistered("geometry")) return;
  addNumericDictionaries();
  geometry::addWords(*this);
}


/* end of QInc/Projects/rpn-lang/src/geometry-dict.cpp */
