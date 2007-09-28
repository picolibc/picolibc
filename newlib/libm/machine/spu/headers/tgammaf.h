#include "headers/tgammaf4.h"

static __inline float _tgammaf(float x)
{
  return spu_extract(_tgammaf4(spu_promote(x, 0)), 0);
}
