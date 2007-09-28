#include "headers/log1pf4.h"

static __inline float _log1pf(float x)
{
  return spu_extract(_log1pf4(spu_promote(x, 0)), 0);
}
