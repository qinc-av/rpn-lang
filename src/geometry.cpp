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
#include "../rpn-matrix.h"

/*
 * Estimate (or calculate) a circle based on points. Here we use an OLS regression model
 * to calculate the center (xc,yc) and radius of a circle that is a closest fit to the
 * points.  If we only pass in 3 points, it's an exact fit.
 *
 *
 * Derrivation of regression model, starting with a translated circle at (x0,y0):
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
  int n = (int)points.size();
  Eigen::MatrixXd xx(n, 3);
  Eigen::VectorXd yy(n);

  for (int i = 0; i < n; i++) {
    double x = points[i]._x, y = points[i]._y;
    xx(i, 0) = 1.0;  xx(i, 1) = x;  xx(i, 2) = y;
    yy(i) = x*x + y*y;
  }

  Eigen::VectorXd beta = (xx.transpose() * xx).inverse() * (xx.transpose() * yy);

  xc = beta(1) / 2.0;
  yc = beta(2) / 2.0;
  r = std::sqrt(beta(0) + xc*xc + yc*yc);

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
