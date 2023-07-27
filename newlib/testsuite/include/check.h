/*
Copyright (C) 2002 by Red Hat, Incorporated. All rights reserved.

Permission to use, copy, modify, and distribute this software
is freely granted, provided that this notice is preserved.
 */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#define CHECK(a) { \
  if (!(a)) \
    { \
      int err = errno; \
      printf ("Failed " #a " in <%s> at line %d\n", __FILE__, __LINE__); \
      fflush(stdout); \
      exit(err == ENOMEM ? 77 : 1); \
    } \
}
