#include <math.h>

float
nextafterf (float x, float y)
{
  union
  {
    float f;
    unsigned int i;
  } u;
  if (isnan (y) || isnan (x))
    return x + y;
  if (x == y )
     return x;
  u.f = x; 
  if (u.i == 0u)
    {
      if (y > 0.0F)
	u.i = 1;
      else
	u.i = 0x80000001;
      return u.f;
    }
   if (((x > 0.0F) ^ (y > x)) == 0)
    u.i++;
  else
    u.i--;
  return u.f;
}
