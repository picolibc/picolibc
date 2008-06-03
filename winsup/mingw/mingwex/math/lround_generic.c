/*
 * lround_generic.c
 *
 * $Id$
 *
 * Provides a generic implementation for the `lround()', `lroundf()',
 * `lroundl()', `llround()', `llroundf()' and `llroundl()' functions;
 * compile with `-D FUNCTION=name', with `name' set to each of these
 * six in turn, to create separate object files for each of the six
 * functions.
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
#ifndef FUNCTION
/*
 * Normally specified with `-D FUNCTION=name', on the command line.
 * Valid FUNCTION names are `lround', `lroundf', `lroundl', `llround'
 * `llroundf' and `llroundl'; specifying anything else will most likely
 * cause a compilation error.  If user did not specify an appropriate
 * FUNCTION name, default to `lround'.
 */
#define FUNCTION lround
#endif

#include "round_internal.h"

#include <limits.h>
#include <errno.h>

/* Generic implementation.
 * The user is required to specify the FUNCTION name;
 * the RETURN_TYPE and INPUT_TYPE macros resolve to appropriate
 * type declarations, to match the selected FUNCTION prototype,
 * while RETURN_MAX and RETURN_MIN map to the correspondingly
 * appropriate limits.h manifest values, to establish the
 * valid range for the RETURN_TYPE.
 */
RETURN_TYPE FUNCTION( INPUT_TYPE x )
{
  if( !isfinite( x ) || !isfinite( x = round_internal( x ) )
  ||  (x > MAX_RETURN_VALUE) || (x < MIN_RETURN_VALUE)        )
    /*
     * Undefined behaviour...
     * POSIX requires us to report a domain error; ANSI C99 says we
     * _may_ report a range error, and previous MinGW implementation
     * set `errno = ERANGE' here; we change that, conforming to the
     * stricter requiremment of the POSIX standard.
     */
    errno = EDOM;

  return (RETURN_TYPE)(x);
}

/* $RCSfile$$Revision$: end of file */
