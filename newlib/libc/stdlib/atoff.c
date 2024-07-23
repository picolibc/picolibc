/*
Copyright (c) 1990 Regents of the University of California.
All rights reserved.
 */
#define _DEFAULT_SOURCE
#include <stdlib.h>
#include <sys/cdefs.h>

float
atoff (const char *s)
{
  return strtof (s, NULL);
}
