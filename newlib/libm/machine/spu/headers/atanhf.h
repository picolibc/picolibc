#include "headers/atanhf4.h"

static __inline float _atanhf(float x)
{
  return spu_extract(_atanhf4(spu_promote(x, 0)), 0);
}
