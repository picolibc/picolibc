#include <wchar.h>
#include <stdlib.h>
#include <stdio.h>
#include <reent.h>
#include <errno.h>

size_t
mbrlen(const char *s, size_t n, mbstate_t *ps)
{
  mbstate_t internal;

  return mbrtowc(NULL, s, n, ps != NULL ? ps : &internal);
}
