/* cygserver_process.cc

   Copyright 2001 Red Hat Inc.

   Written by Robert Collins <rbtcollins@hotmail.com>

   This file is part of Cygwin.

   This software is a copyrighted work licensed under the terms of the
   Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
   details. */

#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <windows.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include "wincap.h"
#include <cygwin/cygserver_process.h>

#define debug_printf if (DEBUG) printf
#define DEBUG 1

/* process cache */
process_cache::process_cache () : head (NULL)
{
}

class process *
process_cache::process (long pid)
{
  class process_entry *entry = head;
  while (entry && entry->process.winpid != pid)
    entry = entry->next;
  if (!entry)
    return NULL;
  return &entry->process;
}

/* process cache entries */
process_entry::process_entry (long pid) : next (NULL), process (pid)
{
}

/* process's */
process::process (long pid) : winpid (pid)
{
  thehandle = OpenProcess (PROCESS_ALL_ACCESS, FALSE, pid);
  debug_printf ("Got handle %p for new cache process %ld\n", thehandle, pid);
}

HANDLE
process::handle ()
{
  unsigned long exitstate;
  bool err = GetExitCodeProcess (thehandle, &exitstate);
  if (!err)
    {
      /* FIXME: */
      thehandle = INVALID_HANDLE_VALUE;
      return INVALID_HANDLE_VALUE;
    }
  if (exitstate != STILL_ACTIVE)
    {
      /* FIXME: call the cleanup list */
      thehandle = OpenProcess (PROCESS_ALL_ACCESS, FALSE, winpid);
      /* FIXME: what if this fails */
    }
  return thehandle;
}
