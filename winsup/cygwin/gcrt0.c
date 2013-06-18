/* gcrt0.c

   Copyright 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

/*
 * This file is taken from Cygwin distribution. Please keep it in sync.
 * The differences should be within __MINGW32__ guard.
 */

#include <sys/types.h>
#include <stdlib.h>

#ifdef __MINGW32__
#include <_bsd_types.h>
#endif

extern u_char etext asm ("etext");
extern u_char eprol asm ("__eprol");
extern void _mcleanup (void);
extern void monstartup (size_t, size_t);
void _monstartup (void) __attribute__((__constructor__));

/* startup initialization for -pg support */

void
_monstartup (void)
{
  static int called;

  /* Guard against multiple calls that may happen if DLLs are linked
     with profile option set as well. Addede side benefit is that it
     makes profiling backward compatible (GCC used to emit a call to
     _monstartup when compiling main with profiling enabled).  */
  if (called++)
    return;

  monstartup ((size_t) &eprol, (size_t) &etext);
  atexit (&_mcleanup);
}

asm (".text");
asm ("__eprol:");

