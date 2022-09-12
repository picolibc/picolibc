/*
Copyright (c) 1994 Cygnus Support.
All rights reserved.

Redistribution and use in source and binary forms are permitted
provided that the above copyright notice and this paragraph are
duplicated in all such forms and that any documentation,
and/or other materials related to such
distribution and use acknowledge that the software was developed
at Cygnus Support, Inc.  Cygnus Support, Inc. may not be used to
endorse or promote products derived from this software without
specific prior written permission.
THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/
/*
FUNCTION
	<<reent>>---definition of impure data.
	
INDEX
	reent

DESCRIPTION
	This module defines the impure data area used by the
	non-reentrant functions, such as strtok.
*/

#define _DEFAULT_SOURCE

#include <stdlib.h>
#ifndef TINY_STDIO
#include "../stdio/local.h"
#endif

void
_reclaim_reent (void *ptr)
{
  (void) ptr;
#ifndef _REENT_THREAD_LOCAL
  if (ptr != _impure_ptr)
#endif
    {
#ifndef TINY_STDIO
      if (_REENT_CLEANUP(ptr))
	{
	  /* cleanup won't reclaim memory 'coz usually it's run
	     before the program exits, and who wants to wait for that? */
	  _REENT_CLEANUP(ptr) ();
	}
#endif
      /* Malloc memory not reclaimed; no good way to return memory anyway. */

    }
}
