/*
Copyright (c) 1990 Regents of the University of California.
All rights reserved.
 */
#include <stdlib.h>
#include <_ansi.h>

float
atoff (const char *s)
{
  return strtof (s, NULL);
}
