/* cygserver_process.h

   Copyright 2001 Red Hat Inc.

   Written by Robert Collins <rbtcollins@hotmail.com>

   This file is part of Cygwin.

   This software is a copyrighted work licensed under the terms of the
   Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
   details. */

#ifndef _CYGSERVER_PROCESS_
#define _CYGSERVER_PROCESS_

class process
{
  public:
    HANDLE handle ();
    long winpid;
    process (long);
  private:
    HANDLE thehandle;
};

class process_entry
{
  public:
    class process_entry *next;
    class process process;
    process_entry (long);
};

class process_cache
{
  public:
    process_cache ();
    ~process_cache ();
    class process *process (long);
  private:
    class process_entry *head;
    CRITICAL_SECTION cache_write_access;
};

#endif /* _CYGSERVER_PROCESS_ */
