/* vsnprintf.c
 *
 * $Id$
 *
 * Provides an implementation of the "vsnprintf" function, conforming
 * generally to C99 and SUSv3/POSIX specifications, with extensions
 * to support Microsoft's non-standard format specifications.  This
 * is included in libmingwex.a, replacing the redirection through
 * libmoldnames.a, to the MSVCRT standard "_vsnprintf" function; (the
 * standard MSVCRT function remains available, and may  be invoked
 * directly, using this fully qualified form of its name).
 *
 * Written by Keith Marshall <keithmarshall@users.sourceforge.net>
 *
 * This is free software.  You may redistribute and/or modify it as you
 * see fit, without restriction of copyright.
 *
 * This software is provided "as is", in the hope that it may be useful,
 * but WITHOUT WARRANTY OF ANY KIND, not even any implied warranty of
 * MERCHANTABILITY, nor of FITNESS FOR ANY PARTICULAR PURPOSE.  At no
 * time will the author accept any form of liability for any damages,
 * however caused, resulting from the use of this software.
 *
 */

#include <stdio.h>
#include <stdarg.h>

#include "pformat.h"

int __cdecl __vsnprintf (char *, size_t, const char *fmt, va_list) __MINGW_NOTHROW;
int __cdecl __mingw_alias(vsnprintf) (char *, size_t, const char *, va_list) __MINGW_NOTHROW;

int __cdecl __vsnprintf( char *buf, size_t length, const char *fmt, va_list argv )
{
  register int retval;

  if( length == (size_t)(0) )
    /*
     * No buffer; simply compute and return the size required,
     * without actually emitting any data.
     */
    return __pformat( 0, buf, 0, fmt, argv );

  /* If we get to here, then we have a buffer...
   * Emit data up to the limit of buffer length less one,
   * then add the requisite NUL terminator.
   */
  retval = __pformat( 0, buf, --length, fmt, argv );
  buf[retval < length ? retval : length] = '\0';

  return retval;
}

/* $RCSfile$Revision$: end of file */
