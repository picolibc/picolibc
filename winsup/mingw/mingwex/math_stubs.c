
#include <math.h>

double copysign	(double x, double y) {return _copysign(x, y);}
float copysignf	(float x, float y) {return  _copysign(x, y);} 
double logb (double x) {return _logb(x);}
float logbf (float x) {return  _logb( x );}
double nextafter(double x, double y) {return _nextafter(x, y);}
float nextafterf(float x, float y) {return _nextafter(x, y);}
double scalb (double x, long i) {return _scalb (x, i);}
float scalbf (float x, long i) {return _scalb(x, i);}

