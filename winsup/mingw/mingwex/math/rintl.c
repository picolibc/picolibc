#include <math.h>

long double rintl (long double x){
  long double retval;
  __asm__ ("frndint;": "=t" (retval) : "0" (x));
  return retval;
}
