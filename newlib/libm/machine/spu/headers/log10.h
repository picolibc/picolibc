#include "headers/log10d2.h"

static __inline double _log10(double x)
{
  return spu_extract(_log10d2(spu_promote(x, 0)), 0);
}
