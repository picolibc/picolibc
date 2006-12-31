/* basename.c
 *
 * $Id$
 *
 * Provides an implementation of the "basename" function, conforming
 * to SUSv3, with extensions to accommodate Win32 drive designators,
 * and suitable for use on native Microsoft(R) Win32 platforms.
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
#include <string.h>
#include <libgen.h>

#ifndef __cdecl  /* If compiling on any non-Win32 platform ... */
#define __cdecl  /* this may not be defined.                   */
#endif

__cdecl char *basename( char *path )
{
  char *retname;
  static char retfail[] = ".";

  if( path && *path )
  {
    /* step over the drive designator, if present ...
     * (FIXME: maybe should confirm *path is a valid drive designator).
     */

    if( path[1] == ':' )
      path += 2;

    /* check again, just to ensure we still have a non-empty path name ... */

    if( *path )
    {
      /* and, when we do ...
       * scan from left to right, to the char after the final dir separator
       */

      for( retname = path ; *path ; ++path )
      {
	if( (*path == '/') || (*path == '\\') )
	{
	  /* we found a dir separator ...
	   * step over it, and any others which immediately follow it
	   */

	  while( (*path == '/') || (*path == '\\') )
	    ++path;

	  /* if we didn't reach the end of the path string ... */

	  if( *path )

	    /* then we have a new candidate for the base name */

	    retname = path;

	  /* otherwise ...
	   * strip off any trailing dir separators which we found
	   */

	  else while( (path > retname) && ((*--path == '/') || (*path == '\\')) )
	    *path = '\0';
	}
      }

      /* retname now points at the resolved base name ...
       * if it's not empty, then we return it as it is, otherwise ...
       * we must have had only dir separators in the original path name,
       * so we return "/".
       */

      return *retname ? retname : strcpy( retfail, "/" );
    }

    /* or we had an empty residual path name, after the drive designator,
     * in which case we simply fall through ...
     */
  }

  /* and, if we get to here ...
   * the path name is either NULL, or it decomposes to an empty string;
   * in either case, we return the default value of "." in our static buffer,
   * (but strcpy it, just in case the caller trashed it after a previous call).
   */

  return strcpy( retfail, "." );
}

/* $RCSfile$: end of file */
