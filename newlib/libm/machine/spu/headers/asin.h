#include "headers/asind2.h"

static __inline double _asin(double x)
{
  return spu_extract(_asind2(spu_promote(x, 0)), 0);
}
