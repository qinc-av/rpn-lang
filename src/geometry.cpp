/***************************************************
 * file: QInc/Projects/rpn-lang/src/geometry.cpp
 *
 * @file    geometry.cpp
 * @author  Eric L. Hernes
 * @version V1.0
 * @born_on   Sunday, January 19, 2025
 * @copyright (C) Copyright Eric L. Hernes 2025
 * @copyright (C) Copyright Q, Inc. 2025
 *
 * @brief   An Eric L. Hernes Signature Series C++ module
 *
 */

#include "geometry.h"
#include "matrix.h"

/*
 * Estimate (or calculate) a circle based on points. Here we use an OLS regression model
 * to calculate the center (xc,yc) and radius of a circle that is a closest fit to the
 * points.  If we only pass in 3 points, it's an exact fit.
 *
 *
 * Derrivation of regression model:
 *
 *   (X - x0)^2 + (Y-y0)^2 = R^2
 *   X^2 - 2*x0*X + x0^2 + Y^2 - 2*y0*Y + y0^2 = R^2
 *   X^2 - 2*x0*X + Y^2 - 2*y0*Y = R^2 - x0^2 - y0^2
 *
 *   setting a=R^2 - x0^2 - y0^2
 *
 *   X^2 - 2*x0*X + Y^2 - 2*y0*Y = a
 *   X^2 + Y^2 = a + 2*x0*X + 2*y0*Y
 *
 *   for OLS estimation:
 *   [Y] =  [X^2 + Y^2]
 *
 *   [X] =  [ 1 2*X 2*X ]
 *
 *    Beta = (X'X)^-1 (X'Y)
 *
 *    x0 = Beta[1]
 *    y0 = Beta[2]
 *    R = sqrt(Beta[0] + x0^2 + y0^2)
 *
 * https://planetcalc.com/8116/ - three point ref
 */

bool
geometry::estimate_circle(double &r, double &xc, double &yc, const std::vector<StVec3> &points) {

  math::matrix<double> xx(points.size(),3), yy(points.size(), 1);

  for(size_t r=0; r< points.size(); r++) {
    double x=points[r]._x, y=points[r]._y;

    xx(r, 0) = 1; xx(r,1) = x; xx(r,2) = y;
    yy(r,0) = x*x+y*y;
  }

  math::matrix<double> beta = (~xx * xx).Inv() * (~xx * yy);

  xc = beta(1,0)/2.;
  yc = beta(2,0)/2.;
  r = sqrt(beta(0,0) + xc*xc + yc*yc);

  return true;
}

#if 0

std::string rpn::to_string(const double &d) {
  return std::to_string(d);
}

int
main(int ac, char **av) {
  double r, xc, yc;
  geometry::estimate_circle(r,xc,yc, {
      { 0.25, 0.125 },
      { 2.875, 1.9 },
      { -3.0, 0.}
    });

  printf("Circle: (%f,%f) R=%f\n", xc,yc, r);
  /*
    beta: Matrix[3x1]
    [ [ -0.545419  ]
    [ -3.18181  ]
    [ 11.352  ]
    ]

    R=5.848277 @ (-1.590903, 5.675984)
  */
  return 0;
}
#endif

/* end of QInc/Projects/rpn-lang/src/geometry.cpp */
