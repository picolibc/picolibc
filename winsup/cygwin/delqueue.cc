/* delqueue.cc

   Copyright 1996, 1998 Cygnus Solutions.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"

/* FIXME: this delqueue module is very flawed and should be rewritten.
	First, having an array of a fixed size for keeping track of the
	unlinked but not yet deleted files is bad.  Second, some programs
	will unlink files and then create a new one in the same location
	and this behavior is not supported in the current code.  Probably
	we should find a move/rename function that will work on open files,
	and move delqueue files to some special location or some such
	hack... */

void
delqueue_list::init ()
{
  empty = 1;
  memset(inuse, 0, MAX_DELQUEUES_PENDING);
}

void
delqueue_list::queue_file (const char *dosname)
{
  char temp[MAX_PATH], *end;
  GetFullPathName (dosname, sizeof (temp), temp, &end);

  /* Note about race conditions: The only time we get to this point is
     when a delete fails because someone's holding the descriptor open.
     In those cases, other programs will be unable to delete the file
     also, so any entries referring to that file will not be removed
     from the queue while we're here. */

  if (!empty)
  {
    /* check for duplicates */
    for (int i=0; i < MAX_DELQUEUES_PENDING; i++)
      if (inuse[i] && strcmp(name[i], temp) == 0)
	return;
  }

  for (int i = 0; i < MAX_DELQUEUES_PENDING; i++)
    if (!inuse[i])
      {
	/* set the name first, in case someone else is running the
	   queue they'll get a valid name */
	strcpy(name[i], temp);
	inuse[i] = 1;
	empty = 0;
	debug_printf ("adding '%s' to queue %d", temp, i);
	return;
      }

  system_printf ("Out of queue slots");
}

void
delqueue_list::process_queue ()
{
  if (empty)
    return;
  /* We set empty to 1 here, rather than later, to avoid a race
     condition - some other program might queue up a file while we're
     processing, and it will zero out empty also. */
  empty = 1; /* but might get set to zero again, below */

  syscall_printf ("Running delqueue");

  for (int i = 0; i < MAX_DELQUEUES_PENDING; i++)
    if (inuse[i])
      {
	if (DeleteFileA (name[i]))
	  {
	    syscall_printf ("Deleted %s", name[i]);
	    inuse[i] = 0;
	  }
	else
	  {
	    int res = GetLastError ();
	    empty = 0;
	    if (res == ERROR_SHARING_VIOLATION ||
		(os_being_run != winNT && res == ERROR_ACCESS_DENIED))
	      {
		/* File still inuse, that's ok */
		syscall_printf ("Still using %s", name[i]);
	      }
	    else
	      {
		syscall_printf ("Hmm, don't know what to do with '%s', %E", name[i]);
		inuse[i] = 0;
	      }
	  }
      }
}
