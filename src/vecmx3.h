/***************************************************
 * file: vecmx3.h
 */
/**
 * @file    vecmx3.h
 * @author  Eric L. Hernes
 * @version V1.0
 * @born_on   unknown
 *
 * @brief   Basic 3x1 and 3x3 vector/matrix library
 *
 */

#ifndef VECMX3_H
#define VECMX3_H

#include <string>

/* basic classes and methods for color math */
namespace q {
  class Mx3;
  class Vec3;

/**
 * @brief Simple 3x1 vector class
 */
  class Vec3 {
  public:
    Vec3(double a=0., double b=0., double c=0.) {
      _v[0] = a;
      _v[1] = b;
      _v[2] = c;
    };

    // area of a 2-space triangular region defined by the first two elements of the
    // three points (as in a triangular gamut)
    static double area(const Vec3 &p0, const Vec3 &p1, const Vec3 &p2);

    // Vec3 as 2d points - ignores last element
    static Vec3 slope_intercept_2d(const Vec3 &p1, const Vec3 &p2);

    // Vec3 as line coefficients: ax + by = c => Vec3(a,b,c)
    static Vec3 solve_2d(const Vec3 &line1, const Vec3 &line2);

    double emin();
    double emax();
    void set(double a=0., double b=0., double c=0.);
    double sum();

    static void solveLinEq(double res[2], const Vec3 &l1, const Vec3 &l2);

    double operator[](unsigned ix) const;
    double &operator[](unsigned ix);
    Vec3 operator/(const Vec3 &rhs) const;
    Vec3 operator/(double rhs) const;
    void print(const std::string &name="", FILE *fp=nullptr) const;

  private:
    double _v[3];
    friend class Mx3;
  };

/**
 * @brief Simple 3x3 matrix class
 */
  class Mx3 {
  public:
    Mx3(const Vec3 &r1, const Vec3 &r2, const Vec3 &r3);

    // identity is default constructor
    Mx3(double e11=1., double e12=0., double e13=0.,
        double e21=0., double e22=1., double e23=0.,
        double e31=0., double e32=0., double e33=1.);

    Mx3 &assign(double e11, double e12, double e13,
                double e21, double e22, double e23,
                double e31, double e32, double e33);

    static Mx3 identity(); // named constructor explicitly returns identity
    static Mx3 diag(const Vec3 &v); // named constructor
    double &operator()(int r, int c);
    double operator()(int r, int c) const;
    Mx3 &transpose();
    Mx3 inverse() const;

    void print(const std::string &name="", FILE *fp=0) const;

    Mx3 operator*(double rhs) const; // scalar multiply
    Vec3 operator*(const Vec3 &rhs) const; //  matrix times vector
    Mx3 operator*(const Mx3 &rhs) const ; // standard matrix multiply

  private:
    double _mx[3*3];

    friend Vec3;
  };
}

#endif /* VECMX3_H */

 /* end of vecmx3.h */
