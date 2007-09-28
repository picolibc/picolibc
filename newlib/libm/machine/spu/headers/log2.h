#include "headers/log2d2.h"

static __inline double _log2(double vx)
{
  return spu_extract(_log2d2(spu_promote(vx, 0)), 0);
}
