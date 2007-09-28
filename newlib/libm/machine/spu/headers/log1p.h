#include "headers/log1pd2.h"

static __inline double _log1p(double x)
{
  return spu_extract(_log1pd2(spu_promote(x, 0)), 0);
}
