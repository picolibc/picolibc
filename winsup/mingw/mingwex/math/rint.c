#include <math.h>
double rint (double x){
  double retval;
  __asm__ ("frndint;" : "=t" (retval) : "0" (x));
  return retval;
}
