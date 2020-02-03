/*
Copyright (c) 2002 Thomas Fitzsimmons <fitzsim@redhat.com>
 */
#include <newlib.h>
#include <wchar.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

size_t
mbrlen(const char *__restrict s, size_t n, mbstate_t *__restrict ps)
{
  return mbrtowc(NULL, s, n, ps);
}
