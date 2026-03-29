/***************************************************
 * file: vecmx3.cpp
 */
/**
 * @file vecmx3.cpp
 * @brief   classes and methods for 3x3 and 3x1 matrix/vector math
 *
 */

#include <math.h>
#include <stdio.h>

#include "../vecmx3.h"

static double
det_2x2(double a, double b,
        double c, double d) {
  return (a * d)  - (b * c);
}

double
q::Vec3::emin() {
  return fmin(fmin(_v[0], _v[1]), _v[2]);
}
double
q::Vec3::emax() {
  return fmax(fmax(_v[0], _v[1]), _v[2]);
}

void
q::Vec3::set(double a, double b, double c) {
  _v[0] = a;
  _v[1] = b;
  _v[2] = c;
}

double
q::Vec3::sum() {
  return (_v[0] + _v[1] + _v[2]);

}

double 
q::Vec3::operator[](unsigned ix) const {
  if (ix>2) throw("index out of range");
  return _v[ix];
}

double &
q::Vec3::operator[](unsigned ix) {
  if (ix>2) throw("index out of range");
  return _v[ix];
}

// element wise division
q::Vec3
q::Vec3::operator/(const q::Vec3 &rhs) const {
  return q::Vec3(_v[0] / rhs._v[0],
                 _v[1] / rhs._v[1],
                 _v[2] / rhs._v[2]);
}

q::Vec3
q::Vec3::operator/(double d) const {
  return q::Vec3(_v[0] / d,
                 _v[1] / d,
                 _v[2] / d);
}

q::Mx3 operator*(const q::Mx3 &lhs, const q::Mx3 &rhs); // standard matrix multiply
q::Vec3 operator*(const q::Mx3 &lhs, const q::Vec3 &rhs); //  matrix times vector

// 2D methods, these ignore the third element of the V3, could probably be Vec2 class
q::Vec3
q::Vec3::slope_intercept_2d(const q::Vec3 &p1, const q::Vec3 &p2) {

  //
  // ax + by = c
  // ax - c = -by
  // (-a/b)*x + (c/b) = y
        
  // y = mx + b
  // solve for m and b

  // m = (y2-y1)/(x2-x1)
  double m = (p2._v[1] - p1._v[1]) / (p2._v[0] - p1._v[0]);
  // b = y1 - x1 m
  double b = (p1._v[1] - p1._v[0] * m);

  // now we have the result in
  // y = mx + b form
  // for cramer's rule, we want in ax + by = c form:
  return Vec3(m, -1, -b);
}

q::Vec3
q::Vec3::solve_2d(const Vec3 &line1, const Vec3 &line2) {
  // cramer's method
  // https://www.chilimath.com/lessons/advanced-algebra/cramers-rule-with-two-variables/
        
  // line ==> a x + b y = c;
  // a,b,c are elements of the Vec3
  double dd = det_2x2(line1._v[0], line1._v[1],
                      line2._v[0], line2._v[1]);
  double dx = det_2x2(line1._v[2], line1._v[1],
                      line2._v[2], line2._v[1]);
  double dy = det_2x2(line1._v[0], line1._v[2],
                      line2._v[0], line2._v[2]);
  return Vec3(dx/dd, dy/dd, 0);
}

double
q::Vec3::area(const Vec3 &p0, const Vec3 &p1, const Vec3 &p2) {
  return 0.5 *(-p1._v[1]*p2._v[0] +
               p0._v[1]*(-p1._v[0] + p2._v[0]) +
               p0._v[0]*(p1._v[1] - p2._v[1]) +
               p1._v[0]*p2._v[1]);
}

q::Mx3::Mx3(const Vec3 &r1, const Vec3 &r2, const Vec3 &r3) {
  _mx[0*3+0]=r1._v[0]; _mx[0*3+1]=r1._v[1]; _mx[0*3+2]=r1._v[2];
  _mx[1*3+0]=r2._v[0]; _mx[1*3+1]=r2._v[1]; _mx[1*3+2]=r2._v[2];
  _mx[2*3+0]=r3._v[0]; _mx[2*3+1]=r3._v[1]; _mx[2*3+2]=r3._v[2];
}

// identity is default constructor
q::Mx3::Mx3(double e11, double e12, double e13,
            double e21, double e22, double e23,
            double e31, double e32, double e33) {
  _mx[0*3+0]=e11; _mx[0*3+1]=e12; _mx[0*3+2]=e13;
  _mx[1*3+0]=e21; _mx[1*3+1]=e22; _mx[1*3+2]=e23;
  _mx[2*3+0]=e31; _mx[2*3+1]=e32; _mx[2*3+2]=e33;
}

q::Mx3 &
q::Mx3::assign(double e11, double e12, double e13,
               double e21, double e22, double e23,
               double e31, double e32, double e33) {
  _mx[0*3+0]=e11; _mx[0*3+1]=e12; _mx[0*3+2]=e13;
  _mx[1*3+0]=e21; _mx[1*3+1]=e22; _mx[1*3+2]=e23;
  _mx[2*3+0]=e31; _mx[2*3+1]=e32; _mx[2*3+2]=e33;
  return *this;
}

q::Mx3
q::Mx3::identity() {
  return Mx3();
}

q::Mx3
q::Mx3::diag(const Vec3 &v) {
  return Mx3(v._v[0], 0., 0.,
             0., v._v[1], 0.,
             0., 0., v._v[2]);
}

double &
q::Mx3::operator()(int r, int c) {
  return _mx[r*3+c];
}

double
q::Mx3::operator()(int r, int c) const {
  return _mx[r*3+c];
}

q::Mx3
&q::Mx3::transpose() {
  assign(_mx[0*3+0],_mx[1*3+0],_mx[2*3+0],
         _mx[0*3+1],_mx[1*3+1],_mx[2*3+1],
         _mx[0*3+2],_mx[1*3+2],_mx[2*3+2]);
  return *this;
}

q::Mx3
q::Mx3::inverse() const {
/* emacs calc
  { {mx[0], mx[1], mx[2]},
    {mx[3], mx[4], mx[5]}, 
    {mx[6], mx[7], mx[8]} }
*/
  double d = (_mx[0]*(_mx[4]*_mx[8]) + _mx[1]*(_mx[5]*_mx[6]) 
                  + _mx[2]*(_mx[3]*_mx[7]) - _mx[2]*(_mx[4]*_mx[6]) 
                  - _mx[0]*(_mx[5]*_mx[7]) - _mx[1]*(_mx[3]*_mx[8]));
  
  return q::Mx3((_mx[4]*_mx[8] - _mx[5]*_mx[7])/d,
                (_mx[2]*_mx[7] - _mx[1]*_mx[8])/d,
                (_mx[1]*_mx[5] - _mx[2]*_mx[4])/d, 

                (_mx[5]*_mx[6] - _mx[3]*_mx[8])/d,
                (_mx[0]*_mx[8] - _mx[2]*_mx[6])/d, 
                ( _mx[2]*_mx[3] - _mx[0]*_mx[5])/d, 

                (_mx[3]*_mx[7] - _mx[4]*_mx[6])/d,
                (_mx[1]*_mx[6] - _mx[0]*_mx[7])/d, 
                (_mx[0]*_mx[4] - _mx[1]*_mx[3])/d);
}

// matrix times scalar
q::Mx3
q::Mx3::operator*(double rhs) const {
  return q::Mx3(_mx[0*3+0]*rhs, _mx[0*3+1]*rhs, _mx[0*3+2]*rhs,
                _mx[1*3+0]*rhs, _mx[1*3+1]*rhs, _mx[1*3+2]*rhs,
                _mx[2*3+0]*rhs, _mx[2*3+1]*rhs, _mx[2*3+2]*rhs);
}

//  matrix times vector
q::Vec3
q::Mx3::operator*(const Vec3 &rhs) const {
  /* emacs calc
  { {mx[0], mx[1], mx[2]},
    {mx[3], mx[4], mx[5]}, 
    {mx[6], mx[7], mx[8]} }

    { v[0], v[1], v[2] }
   */

  return q::Vec3(_mx[0]*rhs._v[0] + _mx[1]*rhs._v[1] + _mx[2]*rhs._v[2], 
                 _mx[3]*rhs._v[0] + _mx[4]*rhs._v[1] + _mx[5]*rhs._v[2], 
                 _mx[6]*rhs._v[0] + _mx[7]*rhs._v[1] + _mx[8]*rhs._v[2]);
}

// standard matrix multiply
q::Mx3
q::Mx3::operator*(const Mx3 &rhs) const {
  /* emacs calc
  { {mx[0], mx[1], mx[2]},
    {mx[3], mx[4], mx[5]}, 
    {mx[6], mx[7], mx[8]} }
  { {rhsmx[0], rhsmx[1], rhsmx[2]},
    {rhsmx[3], rhsmx[4], rhsmx[5]}, 
    {rhsmx[6], rhsmx[7], rhsmx[8]} }
   */
  return q::Mx3(_mx[0]*rhs._mx[0] + _mx[1]*rhs._mx[3] + _mx[2]*rhs._mx[6], 
                _mx[0]*rhs._mx[1] + _mx[1]*rhs._mx[4] + _mx[2]*rhs._mx[7], 
                _mx[0]*rhs._mx[2] + _mx[1]*rhs._mx[5] + _mx[2]*rhs._mx[8],

                _mx[3]*rhs._mx[0] + _mx[4]*rhs._mx[3] + _mx[5]*rhs._mx[6], 
                _mx[3]*rhs._mx[1] + _mx[4]*rhs._mx[4] + _mx[5]*rhs._mx[7], 
                _mx[3]*rhs._mx[2] + _mx[4]*rhs._mx[5] + _mx[5]*rhs._mx[8],

                _mx[6]*rhs._mx[0] + _mx[7]*rhs._mx[3] + _mx[8]*rhs._mx[6], 
                _mx[6]*rhs._mx[1] + _mx[7]*rhs._mx[4] + _mx[8]*rhs._mx[7], 
                _mx[6]*rhs._mx[2] + _mx[7]*rhs._mx[5] + _mx[8]*rhs._mx[8]);
}

// probably doesn't belong here
void
q::Vec3::print(const std::string &name, FILE *fp) const {
  if (fp==nullptr) fp = stdout;
  int n=fprintf(fp, "%s: [ %f %f %f ]\n", name.c_str(), _v[0], _v[1], _v[2]);
}

void
q::Mx3::print(const std::string &name, FILE *fp) const {

  if (fp==nullptr) fp = stdout;
  
  int n=fprintf(fp, "%s: [ ", name.c_str());
  const char* spaces="                                                            ";
  fprintf(fp, "[ %f %f %f ]\n",
          _mx[0*3+0], _mx[0*3+1], _mx[0*3+2]);
  fprintf(fp, "%.*s[ %f %f %f ]\n", n, spaces,
          _mx[1*3+0], _mx[1*3+1], _mx[1*3+2]);
  fprintf(fp, "%.*s[ %f %f %f ] ]\n", n, spaces,
          _mx[2*3+0], _mx[2*3+1], _mx[2*3+2]);
}

/* end of vecmx3.cpp */
