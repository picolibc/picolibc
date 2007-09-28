#include "headers/asinf4.h"

static __inline float _asinf(float x)
{
  return spu_extract(_asinf4(spu_promote(x, 0)), 0);
}
