
/* @(#)z_powf.c 1.0 98/08/13 */
#include <float.h>
#include "fdlibm.h"
#include "zmath.h"

float powf (float x, float y)
{
  float d, t, r = 1.0;
  int n, k, sign = 0;
  __int32_t px;

  GET_FLOAT_WORD (px, x);

  k = modff (y, &d);
  if (k == 0.0) 
    {
      if (modff (ldexpf (y, -1), &t))
        sign = 0;
      else
        sign = 1; 
    }

  if (x == 0.0 && y <= 0.0) 
    errno = EDOM;

  else if ((t = y * log (fabsf (x))) >= BIGX) 
    {
      errno = ERANGE;
      if (px & 0x80000000) 
        {
          if (!k) 
            {
              errno = EDOM;
              x = 0.0;
            }
          else if (sign)
            x = -z_infinity_f.f;
          else
            x =  z_infinity_f.f;
        }

    else 
      x = z_infinity_f.f;
  }

  else if (t < SMALLX)
    {
      errno = ERANGE;
      x = 0.0;
    }

  else 
    {
      if ( k && fabsf (d) <= 32767 ) 
        {
          n = (int) d;

          if (sign = (n < 0))
            n = -n;

          while ( n > 0 ) 
            {
              if ((unsigned int) n % 2) 
                r *= x;
              x *= x;
              n = (unsigned int) n / 2;
            }

          if (sign)
            r = 1.0 / r;

          return r;
        }
		
      else 
        {
          if ( px & 0x80000000 ) 
            {
              if ( !k ) 
                {
                  errno = EDOM;
                  return 0.0;
                }
            }

          x = exp (t);

          if ( sign ) 
            { 
              px ^= 0x80000000;
              SET_FLOAT_WORD (x, px);
            }
        }
      }

  return x;
}

#ifdef _DOUBLE_IS_32BITS

double pow (double x, double y)
{
  return (double) powf ((float) x, (float) y);
}

#endif /* defined(_DOUBLE_IS_32BITS) */ 
