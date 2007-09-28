#include "headers/acosd2.h"

static __inline double _acos(double x)
{
  return spu_extract(_acosd2(spu_promote(x, 0)), 0);
}
