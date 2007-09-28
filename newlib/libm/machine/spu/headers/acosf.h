#include "headers/acosf4.h"

static __inline float _acosf(float x)
{
  return spu_extract(_acosf4(spu_promote(x, 0)), 0);
}
