#include <math.h>

float rintf (float x){
  float retval;
  __asm__ ("frndint;": "=t" (retval) : "0" (x));
  return retval;
}
