#include <math.h>
#include <errno.h>


long double ldexpl(long double x, int expn)
{
  if (isfinite (x) && x != 0.0L)
    {
      x = scalbnl (x , expn);
      if (!isfinite (x) || x == 0.0) errno = ERANGE;
    }
  return x;
}

