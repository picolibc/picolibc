/*
Copyright (c) 2002 Thomas Fitzsimmons <fitzsim@redhat.com>
 */
#include <wchar.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

int
mbsinit(const mbstate_t *ps)
{
  if (ps == NULL || ps->__count == 0)
    return 1;
  else
    return 0;
}
