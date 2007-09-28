#include "headers/tgammad2.h"

static __inline double _tgamma(double x)
{
  return spu_extract(_tgammad2(spu_promote(x, 0)), 0);
}
