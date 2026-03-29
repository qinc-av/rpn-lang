/***************************************************
 * file: QInc/Projects/rpn-lang/src/geometry.h
 *
 * @file    geometry.h
 * @author  Eric L. Hernes
 * @born_on   Sunday, January 19, 2025
 * @copyright (C) Copyright Eric L. Hernes 2025
 * @copyright (C) Copyright Q, Inc. 2025
 *
 * @brief   An Eric L. Hernes Signature Series C/C++ header
 *
 * $Id$
 */

#pragma once

#include "../rpn-matrix.h"

namespace geometry {
  void addWords(rpn::Interp &rpn);

  bool estimate_circle(double &r, double &xc, double &yc, const std::vector<stack::Vec3> &points);
}

/* end of QInc/Projects/rpn-lang/src/geometry.h */
