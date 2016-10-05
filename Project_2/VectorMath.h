#ifndef _VECTORMATH_H_
#define _VECTORMATH_H_

#include <math.h>

/**
 * Define our 3D vectors as an array of doubles
 */
typedef double *V3;

/**
 * Returns the double of the input double times itself (x^2)
 * @param v input double
 * @return double which is the input multiplied by itself
 */
static inline double sqr(double v) { return v * v; }

/**
 * Performs a 3D vector addition
 * @param a
 * @param b
 * @param c the result of the addition
 */
static inline void vectorAdd(V3 a, V3 b, V3 c) {
  c[0] = a[0] + b[0];
  c[1] = a[1] + b[1];
  c[2] = a[2] + b[2];
}

/**
 * Performs a 3D vector subtraction
 * @param a
 * @param b
 * @param c the result of the subtraction
 */
static inline void vectorSubtract(V3 a, V3 b, V3 c) {
  c[0] = a[0] - b[0];
  c[1] = a[1] - b[1];
  c[2] = a[2] - b[2];
}

/**
 * Performs a 3D vector scale (multiplication by a constant)
 * @param a
 * @param s
 * @param c the result of the scale operation
 */
static inline void vectorScale(V3 a, double s, V3 c) {
  c[0] = s * a[0];
  c[1] = s * a[1];
  c[2] = s * a[2];
}

/**
 * Performs a 3D vector dot-product
 * @param a
 * @param b
 * @return returns the dot product of the two input 3D vectors
 */
static inline double vectorDot(V3 a, V3 b) {
  return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

/**
 * Performs a 3D vector cross product
 * @param a
 * @param b
 * @param c the result of the cross product of 3D vectors a and b
 */
static inline void vectorCross(V3 a, V3 b, V3 c) {
  c[0] = a[1] * b[2] - a[2] * b[1];
  c[1] = a[2] * b[0] - a[0] * b[2];
  c[2] = a[0] * b[1] - a[1] * b[0];
}

/**
 * Creates a copy of the 'from' vector at the given 'to' vector
 * @param to
 * @param from
 */
static inline void vectorCopy(V3 to, V3 from) {
  to[0] = from[0];
  to[1] = from[1];
  to[2] = from[2];
}

/**
 *
 * @param v
 */
static inline void normalize(double *v) {
  double len = sqrt(sqr(v[0]) + sqr(v[1]) + sqr(v[2]));
  v[0] /= len;
  v[1] /= len;
  v[2] /= len;
}

#endif
