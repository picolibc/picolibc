#ifndef __ZMATH_H
#define __ZMATH_H

#include <errno.h>

#define NUM 3
#define NAN 2
#define INF 1

#define __PI 3.14159265358979323846
#define __SQRT_HALF 0.70710678118654752440
#define __PI_OVER_TWO 1.57079632679489661923132

extern double BIGX;
extern double SMALLX;

typedef const union
{
  long l[2];
  double d;
} udouble;

typedef const union
{
  long l;
  float f;
} ufloat;

extern double BIGX;
extern double SMALLX;

extern udouble z_infinity;
extern udouble z_notanum;
extern double  z_rooteps;

extern ufloat  z_infinity_f;
extern ufloat  z_notanum_f;
extern float   z_rooteps_f;

/* Core math routines. */

int    numtest (double);
int    numtestf (float);
double logarithm (double, int);
float  logarithmf (float, int);
double sine (double, int);
float  sinef (float, int);
double asine (double, int);
float  asinef (float, int);
double atangent (double, double, double, int);
float  atangentf (float, float, float, int);
double sineh (double, int);
float  sinehf (float, int);

#endif /* no __ZMATH_H */
