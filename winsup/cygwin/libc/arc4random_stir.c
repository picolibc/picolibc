/* libc/arc4random_stir.c

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include <sys/types.h>

/* Exported functions removed from OpenBSD in the meantime.  Keep them,
   but make them non-functional.  They don't return a value anyway. */

void
arc4random_stir(void)
{
}

void
arc4random_addrandom(u_int8_t *dat, int datlen)
{
}
