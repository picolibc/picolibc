#include "headers/sqrtf4.h"

static __inline float _sqrtf(float in)
{
  return spu_extract(_sqrtf4(spu_promote(in, 0)), 0);
}
