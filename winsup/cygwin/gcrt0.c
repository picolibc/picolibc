/* gcrt0.c

   Copyright 1998 Cygnus Solutions.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include <sys/types.h>
#include <stdlib.h>

extern u_char etext asm ("etext");
extern u_char eprol asm ("__eprol");
extern void _mcleanup (void);
extern void monstartup (u_long, u_long);

foo void _monstartup (void) __attribute__((__constructor__));

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

  monstartup ((u_long) &eprol, (u_long) &etext);
  atexit (&_mcleanup);
}

asm (".text");
asm ("__eprol:");

