/*
 * round_generic.c
 *
 * $Id$
 *
 * Provides a generic implementation for the `round()', `roundf()'
 * and `roundl()' functions; compile with `-D FUNCTION=name', with
 * `name' set to each of these three in turn, to create separate
 * object files for each of the three functions.
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
 * Valid FUNCTION names are `round', `roundf' and `roundl'; specifying
 * anything else will most likely cause a compilation error.  If user
 * did not specify any FUNCTION name, default to `round'.
 */
#define FUNCTION round
#endif

#include "round_internal.h"

/* Generic implementation.
 * The user is required to specify the FUNCTION name;
 * the RETURN_TYPE and INPUT_TYPE macros resolve to appropriate
 * type declarations, to match the selected FUNCTION prototype.
 */
RETURN_TYPE FUNCTION( INPUT_TYPE x )
{
  /* Round to nearest integer, away from zero for half-way.
   *
   * We split it with the `round_internal()' function in
   * a private header file, so that it may be shared by this,
   * `lround()' and `llround()' implementations.
   */
  return isfinite( x ) ? round_internal( x ) : x;
}

/* $RCSfile$$Revision$: end of file */
