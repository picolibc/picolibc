/* dirname.c
 *
 * $Id$
 *
 * Provides an implementation of the "dirname" function, conforming
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

__cdecl char *dirname( char *path )
{
  static char retfail[] = "?:.";
  char *retname, *basename, *copyptr = retfail;

  if( path && *path )
  {
    retname = path;

    /* SUSv3 identifies a special case, where path is exactly equal to "//";
     * (we will also accept "\\" in the Win32 context, but not "/\" or "\/",
     *  and neither will we consider paths with an initial drive designator).
     * For this special case, SUSv3 allows the implementation to choose to
     * return "/" or "//", (or "\" or "\\", since this is Win32); we will
     * simply return the path unchanged, (i.e. "//" or "\\").
     */

    if( (*path == '/') || (*path == '\\') )
    {
      if( (path[1] == *retname) && (path[2] == '\0') )
	return retname;
    }

    /* For all other cases ...
     * step over the drive designator, if present, copying it to retfail ...
     * (FIXME: maybe should confirm *path is a valid drive designator).
     */

    else if( *path && (path[1] == ':') )
    {
      *copyptr++ = *path++;
      *copyptr++ = *path++;
    }

    if( *path )
    {
      /* reproduce the scanning logic of the "basename" function
       * to locate the basename component of the current path string,
       * (but also remember where the dirname component starts).
       */
      
      for( retname = basename = path ; *path ; ++path )
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

	    basename = path;

	  else

	    /* we struck an early termination of the path string,
	     * with trailing dir separators following the base name,
	     * so break out of the for loop, to avoid overrun.
	     */

	    break;
	}
      }

      /* now check,
       * to confirm that we have distinct dirname and basename components
       */

      if( basename > retname )
      {
	/* and, when we do ...
	 * backtrack over all trailing separators on the dirname component,
	 * (but preserve exactly two initial dirname separators, if identical),
	 * and add a NULL terminator in their place.
	 */
	
	--basename;
	while( (basename > retname) && ((*basename == '/') || (*basename == '\\')) )
	  --basename;
	if( (basename == retname) && ((*retname == '/') || (*retname == '\\'))
	&&  (retname[1] == *retname) && (retname[2] != '/') && (retname[2] != '\\') )
	  ++basename;
	*++basename = '\0';

	/* adjust the start point of the dirname,
	 * to accommodate the Win32 drive designator, if it was present.
	 */

	if( copyptr > retfail )
	  retname -= 2;

	/* if the resultant dirname begins with EXACTLY two dir separators,
	 * AND both are identical, then we preserve them.
	 */

	path = copyptr = retname;
	while( ((*path == '/') || (*path == '\\')) )
	  ++path;
	if( ((path - retname) == 2) && (*++copyptr == *retname) )
	  ++copyptr;

	/* and finally ...
	 * we remove any residual, redundantly duplicated separators from the dirname,
	 * reterminate, and return it.
	 */

	path = copyptr;
	while( *path )
	{
	  if( ((*copyptr++ = *path) == '/') || (*path++ == '\\') )
	  {
	    while( (*path == '/') || (*path == '\\') )
	      ++path;
	  }
	}
	*copyptr = '\0';
	return retname;
      }

      else if( (*retname == '/') || (*retname == '\\') )
      {
	*copyptr++ = *retname;
	*copyptr = '\0';
	return retfail;
      }
    }
  }

  /* path is NULL, or an empty string; default return value is "." ...
   * return this in our own static buffer, but strcpy it, just in case
   * the caller trashed it after a previous call.
   */

  strcpy( copyptr, "." );
  return retfail;
}

/* $RCSfile$: end of file */
