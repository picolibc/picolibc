#include "headers/acoshf4.h"

static __inline float _acoshf(float x)
{
  return spu_extract(_acoshf4(spu_promote(x, 0)), 0);
}
