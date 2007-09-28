#include "headers/atanhd2.h"

static __inline double _atanh(double x)
{
  return spu_extract(_atanhd2(spu_promote(x, 0)), 0);
}
