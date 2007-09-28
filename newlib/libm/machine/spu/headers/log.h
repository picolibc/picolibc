#include "headers/logd2.h"

static __inline double _log(double x)
{
  return spu_extract(_logd2(spu_promote(x, 0)), 0);
}
