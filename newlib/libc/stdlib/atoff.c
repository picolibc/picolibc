#include <stdlib.h>
#include <_ansi.h>

float
atoff (const char *s)
{
  return strtof (s, NULL);
}
