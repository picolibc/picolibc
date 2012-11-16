/* profil.h: gprof profiling header file

   Copyright 1998, 1999, 2000, 2001, 2012 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include <stdint.h>

/* profiling frequency.  (No larger than 1000) */
#define PROF_HZ			100

/* convert an addr to an index */
#define PROFIDX(pc, base, scale)	\
  ({									\
    size_t i = (pc - base) / 2;				\
    if (sizeof (unsigned long long int) > sizeof (size_t))		\
      i = (unsigned long long int) i * scale / 65536;			\
    else								\
      i = i / 65536 * scale + i % 65536 * scale / 65536;		\
    i;									\
  })

/* convert an index into an address */
#define PROFADDR(idx, base, scale)	\
	((base) + ((((unsigned long long)(idx) << 16) / (scale)) << 1))

/* convert a bin size into a scale */
#define PROFSCALE(range, bins)		(((bins) << 16) / ((range) >> 1))

typedef void *_WINHANDLE;

struct profinfo {
    _WINHANDLE targthr;			/* thread to profile */
    _WINHANDLE profthr;			/* profiling thread */
    uint16_t *counter;			/* profiling counters */
    uintptr_t lowpc, highpc;		/* range to be profiled */
    unsigned int scale;			/* scale value of bins */
};

int profile_ctl (struct profinfo *, char *, size_t, size_t, unsigned int);
int profil (char *, size_t, size_t, unsigned int);

