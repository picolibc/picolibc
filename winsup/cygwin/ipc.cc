/* ipc.cc: Single unix specification IPC interface for Cygwin

   Copyright 2001 Red Hat, Inc.

   Originally written by Robert Collins <robert.collins@hotmail.com>

   This file is part of Cygwin.

   This software is a copyrighted work licensed under the terms of the
   Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
   details. */

#include "winsup.h"
#include <sys/ipc.h>
#include <sys/stat.h>

extern "C"
{

/* Notes: we return a valid key even if id's low order 8 bits are 0. */
key_t
ftok(const char *path, int id)
{
  struct stat statbuf;
  if (stat(path, &statbuf))
    {
      /* stat set the appropriate errno for us */
      return (key_t) -1;
    }
  
  /* dev_t is short for cygwin 
   * ino_t is long for cygwin
   * and we need 8 bits for the id.
   * thus key_t is long long. 
   */
  return ((long long) statbuf.st_dev << (5*8)) | (statbuf.st_ino << (8) ) | (id & 0x00ff);
}

}
