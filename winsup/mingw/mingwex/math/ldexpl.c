#include <math.h>
#include <errno.h>
  
long double ldexpl(long double x, int expn)
{
  long double res;
  if (!isfinite (x) || x == 0.0L)
    return x;

  __asm__ ("fscale"
  	    : "=t" (res)
	    : "0" (x), "u" ((long double) expn));

  if (!isfinite (res) || res == 0.0L)
    errno = ERANGE;

  return res;
}

