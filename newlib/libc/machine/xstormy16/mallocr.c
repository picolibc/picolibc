/* Copyright (c) 2002 Geoffrey Keating <geoffk@geoffk.org> */
#include <picolibc.h>

#include <malloc.h>

void *
_malloc_r (struct _reent *r, size_t sz)
{
  return malloc (sz);
}
