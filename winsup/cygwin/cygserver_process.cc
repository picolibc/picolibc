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
#include <pthread.h>
#include <cygwin/cygserver_process.h>

#define debug_printf if (DEBUG) printf
#define DEBUG 1

/* the cache structures and classes are designed for one cache per server process.
 * To make multiple process caches, a redesign will be needed
 */

/* process cache */
process_cache::process_cache () : head (NULL)
{
  /* there can only be one */
  InitializeCriticalSection (&cache_write_access);
}

class process *
process_cache::process (long pid)
{
  class process_entry *entry = head;
  /* TODO: make this more granular, so a search doesn't involve the write lock */
  EnterCriticalSection (&cache_write_access);
  while (entry && entry->process.winpid != pid)
    entry = entry->next;
  if (!entry)
    {
      entry = new process_entry (pid);
      entry->next = (process_entry *) InterlockedExchangePointer (&head, entry);
    }
  LeaveCriticalSection (&cache_write_access);
  return &entry->process;
}

/* process cache entries */
process_entry::process_entry (long pid) : next (NULL), process (pid)
{
}

/* process's */
static CRITICAL_SECTION process_access;
static pthread_once_t process_init;

void
do_process_init (void)
{
  InitializeCriticalSection (&process_access);
  /* we don't have a cache shutdown capability today */
}

process::process (long pid) : winpid (pid)
{
  pthread_once(&process_init, do_process_init);
  EnterCriticalSection (&process_access);
  thehandle = OpenProcess (PROCESS_ALL_ACCESS, FALSE, pid);
  debug_printf ("Got handle %p for new cache process %ld\n", thehandle, pid);
  LeaveCriticalSection (&process_access);
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
      CloseHandle (thehandle);
      debug_printf ("Process id %ld has terminated, attempting to open a new handle\n", winpid);
      thehandle = OpenProcess (PROCESS_ALL_ACCESS, FALSE, winpid);
      /* FIXME: what if this fails */
    }
  return thehandle;
}
