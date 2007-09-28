#include "headers/acoshd2.h"

static __inline double _acosh(double x)
{
  return spu_extract(_acoshd2(spu_promote(x, 0)), 0);
}
