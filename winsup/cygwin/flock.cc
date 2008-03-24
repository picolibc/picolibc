/* flock.cc.  NT specific implementation of advisory file locking.

   Copyright 2003, 2008 Red Hat, Inc.

   This file is part of Cygwin.

   This software is a copyrighted work licensed under the terms of the
   Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
   details. */

/* The basic mechanism as well as the datastructures used in the below
   implementation are taken from the FreeBSD repository on 2008-03-18.
   The essential code of the lf_XXX functions has been taken from the
   module src/sys/kern/kern_lockf.c.  It has been adapted to use NT
   global namespace subdirs and event objects for synchronization
   purposes. 
   
   So, the following copyright applies to most of the code in the lf_XXX
   functions.

 * Copyright (c) 1982, 1986, 1989, 1993
 *      The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Scooter Morris at Genentech Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *      @(#)ufs_lockf.c 8.3 (Berkeley) 1/6/94
*/   

/*
 * The flock() function is based upon source taken from the Red Hat
 * implementation used in their imap-2002d SRPM.
 *
 * $RH: flock.c,v 1.2 2000/08/23 17:07:00 nalin Exp $
 */

/* The lockf function has been taken from FreeBSD with the following
 * copyright.
 *
 * Copyright (c) 1997 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Klaus Klein.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *        This product includes software developed by the NetBSD
 *        Foundation, Inc. and its contributors.
 * 4. Neither the name of The NetBSD Foundation nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 *
 * $NetBSD: lockf.c,v 1.1 1997/12/20 20:23:18 kleink Exp $
*/

#include "winsup.h"
#include <assert.h>
#include <sys/file.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include "cygerrno.h"
#include "perprocess.h"
#include "security.h"
#include "cygwin/version.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include "shared_info.h"
#include "pinfo.h"
#include "sigproc.h"
#include "cygtls.h"
#include "tls_pbuf.h"
#include "ntdll.h"
#include <sys/queue.h>
#include <wchar.h>

/* Right now we implement flock(2) locks using the POSIX semantics
   in terms of inheritance and removal of locks.

   TODO: How to implement real BSD flock semantics?

	 From the Linux man page:

	 Locks created by flock() are associated with an open file table
	 entry.  This means that duplicate file descriptors (created by,
	 for example, fork(2) or dup(2)) refer to the same lock, and
	 this lock may be modified or released using any of these
	 descriptors.  Furthermore, the lock is released either by an
	 explicit LOCK_UN operation on any of these duplicate
	 descriptors, or when all such descriptors have been closed.

	 If a process uses open(2) (or similar) to obtain more than one
	 descrip- tor for the same file, these descriptors are treated
	 independently by flock().  An attempt to lock the file using
	 one of these file descriptors may be denied by a lock that the
	 calling process has already placed via another descriptor.  */

#define F_WAIT 0x10	/* Wait until lock is granted */
#define F_FLOCK 0x20	/* Use flock(2) semantics for lock */
#define F_POSIX	0x40	/* Use POSIX semantics for lock */

#ifndef OFF_MAX
#define OFF_MAX LLONG_MAX
#endif

static NO_COPY muto lockf_guard;

#define INODE_LIST_LOCK()	(lockf_guard.init ("lockf_guard")->acquire ())
#define INODE_LIST_UNLOCK()	(lockf_guard.release ())

#define LOCK_OBJ_NAME_LEN	56

/* This function takes the own process security descriptor DACL and adds
   SYNCHRONIZE permissions for everyone.  This allows to wait any process
   to wait for this process to set the event object to signalled in case
   the lock gets removed or replaced. */
static void
allow_others_to_sync ()
{
  static NO_COPY bool done;

  if (done)
    return;

  NTSTATUS status;
  PACL dacl;
  LPVOID ace;
  ULONG len;

  /* Get this process DACL.  We use a temporary path buffer in TLS space
     to avoid having to alloc 64K from the stack. */
  tmp_pathbuf tp;
  PSECURITY_DESCRIPTOR sd = (PSECURITY_DESCRIPTOR) tp.w_get ();
  status = NtQuerySecurityObject (hMainProc, DACL_SECURITY_INFORMATION, sd,
				  NT_MAX_PATH * sizeof (WCHAR), &len);
  if (!NT_SUCCESS (status))
    {
      debug_printf ("NtQuerySecurityObject: %p", status);
      return;
    }
  /* Create a valid dacl pointer and set it's size to be as big as
     there's room in the temporary buffer.  Note that the descriptor
     is in self-relative format. */
  dacl = (PACL) ((char *) sd + (uintptr_t) sd->Dacl);
  dacl->AclSize = NT_MAX_PATH * sizeof (WCHAR) - ((char *) dacl - (char *) sd);
  /* Allow everyone to SYNCHRONIZE with this process. */
  if (!AddAccessAllowedAce (dacl, ACL_REVISION, SYNCHRONIZE,
			    well_known_world_sid))
    {
      debug_printf ("AddAccessAllowedAce: %lu", GetLastError ());
      return;
    }
  /* Set the size of the DACL correctly. */
  if (!FindFirstFreeAce (dacl, &ace))
    {
      debug_printf ("FindFirstFreeAce: %lu", GetLastError ());
      return;
    }
  dacl->AclSize = (char *) ace - (char *) dacl;
  /* Write the DACL back. */
  status = NtSetSecurityObject (hMainProc, DACL_SECURITY_INFORMATION, sd);
  if (!NT_SUCCESS (status))
    {
      debug_printf ("NtSetSecurityObject: %p", status);
      return;
    }
  done = true;
}

/* Helper function to create an event security descriptor which only allows
   SYNCHRONIZE access to everyone.  Only the creating process has all access
   rights. */
static PSECURITY_DESCRIPTOR
everyone_sync_sd ()
{
  static PSECURITY_DESCRIPTOR psd;

  if (!psd)
    {
      const size_t acl_len = sizeof (ACL) +
			     sizeof (ACCESS_ALLOWED_ACE) + MAX_SID_LEN;
      psd = (PSECURITY_DESCRIPTOR)
	    malloc (sizeof (SECURITY_DESCRIPTOR) + acl_len);
      InitializeSecurityDescriptor (psd, SECURITY_DESCRIPTOR_REVISION);
      PACL dacl = (PACL) (psd + 1);
      InitializeAcl (dacl, acl_len, ACL_REVISION);
      if (!AddAccessAllowedAce (dacl, ACL_REVISION, SYNCHRONIZE, 
				well_known_world_sid))
	{
	  debug_printf ("AddAccessAllowedAce: %lu", GetLastError ());
	  return NULL;
	}
      LPVOID ace;
      if (!FindFirstFreeAce (dacl, &ace))
	{
	  debug_printf ("FindFirstFreeAce: %lu", GetLastError ());
	  return NULL;
	}
      dacl->AclSize = (char *) ace - (char *) dacl;
      SetSecurityDescriptorDacl (psd, TRUE, dacl, FALSE);
    }
  return psd;
}

/* This function returns a handle to the top-level directory in the global
   NT namespace used to implement advisory locking. */
static HANDLE
get_lock_parent_dir ()
{
  static HANDLE dir;
  UNICODE_STRING uname;
  OBJECT_ATTRIBUTES attr;
  NTSTATUS status;

  INODE_LIST_LOCK();
  if (!dir)
    {
      RtlInitUnicodeString (&uname, L"\\BaseNamedObjects\\cygwin-fcntl-lk");
      InitializeObjectAttributes (&attr, &uname, OBJ_INHERIT, NULL,
				  sec_all.lpSecurityDescriptor);
      status = NtOpenDirectoryObject (&dir, DIRECTORY_ALL_ACCESS, &attr);
      if (!NT_SUCCESS (status))
	{
	  status = NtCreateDirectoryObject (&dir, DIRECTORY_ALL_ACCESS, &attr);
	  if (!NT_SUCCESS (status))
	    api_fatal ("NtCreateDirectoryObject: %p", status);
	}
    }
  INODE_LIST_UNLOCK ();
  return dir;
}

/* Per lock class. */
class lockf_t
{
  public:
    short           lf_flags; /* Semantics: F_POSIX, F_FLOCK, F_WAIT */
    short           lf_type;  /* Lock type: F_RDLCK, F_WRLCK */
    _off64_t        lf_start; /* Byte # of the start of the lock */
    _off64_t        lf_end;   /* Byte # of the end of the lock (-1=EOF) */
    /* We need the Cygwin PID for F_GETLK, the Win PID for synchronization. */
    pid_t           lf_id;    /* (P)Id of the resource holding the lock */
    DWORD           lf_wid;   /* Win PID of the resource holding the lock */
    class lockf_t **lf_head;  /* Back pointer to the head of the lockf_t list */
    class inode_t  *lf_inode; /* Back pointer to the inode_t */
    class lockf_t  *lf_next;  /* Pointer to the next lock on this inode_t */
    HANDLE          lf_obj;   /* Handle to the lock event object. */

    lockf_t ()
    : lf_flags (0), lf_type (0), lf_start (0), lf_end (0), lf_id (0),
      lf_wid (0), lf_head (NULL), lf_inode (NULL), lf_next (NULL), lf_obj (NULL)
    {}
    lockf_t (class inode_t *node, class lockf_t **head, short flags, short type,
	   _off64_t start, _off64_t end, pid_t id, DWORD wid)
    : lf_flags (flags), lf_type (type), lf_start (start), lf_end (end),
      lf_id (id), lf_wid (wid), lf_head (head), lf_inode (node),
      lf_next (NULL), lf_obj (NULL)
    {}
    ~lockf_t ();

    void *operator new (size_t size)
    { return ccalloc (HEAP_FHANDLER, 1, sizeof (lockf_t)); }
    void operator delete (void *p)
    { cfree (p); }

    void create_lock_obj ();
    HANDLE open_lock_obj () const;
    ULONG get_lock_obj_handle_count () const;
    void del_lock_obj ();
};

/* Per inode_t class */
class inode_t
{
  friend class lockf_t;

  public:
    LIST_ENTRY (inode_t)  i_next;
    lockf_t              *i_lockf;  /* List of locks of this process. */
    lockf_t              *i_all_lf; /* Temp list of all locks for this file. */

    dev_t  i_dev;
    ino_t  i_ino;
  private:
    HANDLE i_dir; /* Not inherited! */
    HANDLE i_mtx; /* Not inherited! */

    void del_locks (lockf_t **head);

  public:
    inode_t (dev_t dev, ino_t ino);
    ~inode_t ();

    void *operator new (size_t size)
    { return ccalloc (HEAP_FHANDLER, 1, sizeof (inode_t)); }
    void operator delete (void *p)
    { cfree (p); }

    static inode_t *get (dev_t dev, ino_t ino);

    void LOCK () { WaitForSingleObject (i_mtx, INFINITE); }
    void UNLOCK () { ReleaseMutex (i_mtx); }

    void get_all_locks_list ();
    void del_all_locks_list () { del_locks (&i_all_lf); }

    void del_my_locks () { LOCK (); del_locks (&i_lockf); UNLOCK ();
}

};

/* Used to delete all locks on a file hold by this process.  Called from
   close(2).  This implements fcntl semantics.
   TODO: flock(2) semantics. */
void
del_my_locks (inode_t *node)
{
  INODE_LIST_LOCK ();
  node->del_my_locks ();
  LIST_REMOVE (node, i_next);
  INODE_LIST_UNLOCK ();
}

/* The global inode_t list header.  inode_t structs are created when a lock
   is requested on a file the first time from this process. */
/* Called in a forked child to get rid of all inodes and locks hold by the
   parent process.  Child processes don't inherit locks. */
void
fixup_lockf_after_fork ()
{
  inode_t *node, *next_node;

  LIST_FOREACH_SAFE (node, &cygheap->inode_list, i_next, next_node)
    delete node;
  LIST_INIT (&cygheap->inode_list);
}

/* Called in an execed child.  The exec'ed process must allow SYNCHRONIZE
   access to everyone if at least one inode exists.
   The lock owner's Windows PID changed and all lock event objects have to
   be relabeled so that waiting processes know which process to wait on. */
void
fixup_lockf_after_exec ()
{
  inode_t *node;

  INODE_LIST_LOCK ();
  if (LIST_FIRST (&cygheap->inode_list))
    allow_others_to_sync ();
  LIST_FOREACH (node, &cygheap->inode_list, i_next)
    {
      node->LOCK ();
      for (lockf_t *lock = node->i_lockf; lock; lock = lock->lf_next)
	{
	  lock->del_lock_obj ();
	  lock->lf_wid = myself->dwProcessId;
	  lock->create_lock_obj ();
	}
      node->UNLOCK ();
    }
  LIST_INIT (&cygheap->inode_list);
  INODE_LIST_UNLOCK ();
}

/* static method to return a pointer to the inode_t structure for a specific
   file.  The file is specified by the device and inode_t number.  If inode_t
   doesn't exist, create it. */
inode_t *
inode_t::get (dev_t dev, ino_t ino)
{
  inode_t *node;

  INODE_LIST_LOCK ();
  LIST_FOREACH (node, &cygheap->inode_list, i_next)
    if (node->i_dev == dev && node->i_ino == ino)
      break;
  if (!node)
    {
      node = new inode_t (dev, ino);
      LIST_INSERT_HEAD (&cygheap->inode_list, node, i_next);
    }
  INODE_LIST_UNLOCK ();
  return node;
}

inode_t::inode_t (dev_t dev, ino_t ino)
: i_lockf (NULL), i_all_lf (NULL), i_dev (dev), i_ino (ino)
{
  HANDLE parent_dir;
  WCHAR name[32];
  UNICODE_STRING uname;
  OBJECT_ATTRIBUTES attr;
  NTSTATUS status;

  parent_dir = get_lock_parent_dir ();
  /* Create a subdir which is named after the device and inode_t numbers
     of the given file, in hex notation. */
  int len = __small_swprintf (name, L"%08x-%016X", dev, ino);
  RtlInitCountedUnicodeString (&uname, name, len * sizeof (WCHAR));
  InitializeObjectAttributes (&attr, &uname, OBJ_INHERIT, parent_dir,
			      sec_all.lpSecurityDescriptor);
  status = NtOpenDirectoryObject (&i_dir, DIRECTORY_ALL_ACCESS, &attr);
  if (!NT_SUCCESS (status))
    {
      status = NtCreateDirectoryObject (&i_dir, DIRECTORY_ALL_ACCESS, &attr);
      if (!NT_SUCCESS (status))
	api_fatal ("NtCreateDirectoryObject: %p", status);
    }
  /* Create a mutex object in the file specific dir, which is used for
     access synchronization on the dir and its objects. */
  RtlInitUnicodeString (&uname, L"mtx");
  InitializeObjectAttributes (&attr, &uname, OBJ_INHERIT, i_dir,
			      sec_all.lpSecurityDescriptor);
  status = NtOpenMutant (&i_mtx, MUTANT_ALL_ACCESS, &attr);
  if (!NT_SUCCESS (status))
    {
      status = NtCreateMutant (&i_mtx, MUTANT_ALL_ACCESS, &attr, FALSE);
      if (!NT_SUCCESS (status))
	api_fatal ("NtCreateMutant: %p", status);
    }
}

inode_t::~inode_t ()
{
  del_locks (&i_lockf);
}

void
inode_t::del_locks (lockf_t **head)
{
  lockf_t *lock, *n_lock;
  for (lock = *head; lock && (n_lock = lock->lf_next, 1); lock = n_lock)
    delete lock;
  *head = NULL;
}

/* Enumerate all lock event objects for this file and create a lockf_t
   list in the i_all_lf member.  This list is searched in lf_getblock
   for locks which potentially block our lock request. */
void
inode_t::get_all_locks_list ()
{
  struct fdbi
  {
    DIRECTORY_BASIC_INFORMATION dbi;
    WCHAR buf[2][NAME_MAX + 1];
  } f;
  ULONG context;
  NTSTATUS status;

  del_all_locks_list ();
  for (BOOLEAN restart = TRUE;
       NT_SUCCESS (status = NtQueryDirectoryObject (i_dir, &f, sizeof f, TRUE,
						    restart, &context, NULL));
       restart = FALSE)
    {
      if (f.dbi.ObjectName.Length != LOCK_OBJ_NAME_LEN * sizeof (WCHAR))
	continue;
      wchar_t *wc = f.dbi.ObjectName.Buffer, *endptr;
      /* "%02x-%01x-%016X-%016X-%08x-%08x",
	 lf_flags, lf_type, lf_start, lf_end, lf_id, lf_wid */
      wc[LOCK_OBJ_NAME_LEN] = L'\0';
      short flags = wcstol (wc, &endptr, 16);
      if ((flags & ~(F_WAIT | F_FLOCK | F_POSIX)) != 0
	  || (flags & (F_FLOCK | F_POSIX) == (F_FLOCK | F_POSIX)))
	continue;
      short type = wcstol (endptr + 1, &endptr, 16);
      if (type != F_RDLCK && type != F_WRLCK || !endptr || *endptr != L'-')
        continue;
      _off64_t start = (_off64_t) wcstoull (endptr + 1, &endptr, 16);
      if (start < 0 || !endptr || *endptr != L'-')
        continue;
      _off64_t end = (_off64_t) wcstoull (endptr + 1, &endptr, 16);
      if (end < -1LL || (end > 0 && end < start) || !endptr || *endptr != L'-')
      	continue;
      pid_t id = wcstoul (endptr + 1, &endptr, 16);
      if (!endptr || *endptr != L'-')
      	continue;
      DWORD wid = wcstoul (endptr + 1, &endptr, 16);
      if (endptr && *endptr != L'\0')
      	continue;
      lockf_t *lock = new lockf_t (this, &i_all_lf, flags, type, start, end,
				   id, wid);
      if (i_all_lf)
	lock->lf_next = i_all_lf;
      i_all_lf = lock;
    }
}

/* Create the lock event object in the file's subdir in the NT global
   namespace.  The name is constructed from the lock properties which
   identify it uniquely, all values in hex.  See the __small_swprintf
   call right at the start.  */
void
lockf_t::create_lock_obj ()
{
  WCHAR name[LOCK_OBJ_NAME_LEN];
  UNICODE_STRING uname;
  OBJECT_ATTRIBUTES attr;
  NTSTATUS status;

  __small_swprintf (name, L"%02x-%01x-%016X-%016X-%08x-%08x",
			  lf_flags, lf_type, lf_start, lf_end, lf_id, lf_wid);
  RtlInitCountedUnicodeString (&uname, name,
			       LOCK_OBJ_NAME_LEN * sizeof (WCHAR));
  InitializeObjectAttributes (&attr, &uname, OBJ_INHERIT, lf_inode->i_dir,
			      everyone_sync_sd ());
  status = NtCreateEvent (&lf_obj, EVENT_ALL_ACCESS, &attr,
			  NotificationEvent, FALSE);
  if (!NT_SUCCESS (status))
    api_fatal ("NtCreateEvent: %p", status);
}

/* Open a lock event object for SYNCHRONIZE access (to wait for it). */
HANDLE
lockf_t::open_lock_obj () const
{
  WCHAR name[LOCK_OBJ_NAME_LEN];
  UNICODE_STRING uname;
  OBJECT_ATTRIBUTES attr;
  NTSTATUS status;
  HANDLE obj;

  __small_swprintf (name, L"%02x-%01x-%016X-%016X-%08x-%08x",
			  lf_flags, lf_type, lf_start, lf_end, lf_id, lf_wid);
  RtlInitCountedUnicodeString (&uname, name,
			       LOCK_OBJ_NAME_LEN * sizeof (WCHAR));
  InitializeObjectAttributes (&attr, &uname, OBJ_INHERIT, lf_inode->i_dir,
			      NULL);
  status = NtOpenEvent (&obj, SYNCHRONIZE, &attr);
  if (!NT_SUCCESS (status))
    api_fatal ("NtOpenEvent: %p", status);
  return obj;
}

/* Get the handle count of a lock object. */
ULONG
lockf_t::get_lock_obj_handle_count () const
{
  OBJECT_BASIC_INFORMATION obi;
  NTSTATUS status;
  ULONG hdl_cnt = 0;

  if (lf_obj)
    {
      status = NtQueryObject (lf_obj, ObjectBasicInformation,
			      &obi, sizeof obi, NULL);
      if (!NT_SUCCESS (status))
	small_printf ("NtQueryObject: %p\n", status);
      else
      	hdl_cnt = obi.HandleCount;
    }
  return hdl_cnt;
}

/* Close a lock event handle.  The important thing here is to signal it
   before closing the handle.  This way all threads waiting for this
   lock can wake up. */
void
lockf_t::del_lock_obj ()
{
  if (lf_obj)
    {
      SetEvent (lf_obj);
      NtClose (lf_obj);
      lf_obj = NULL;
    }
}

lockf_t::~lockf_t ()
{
  del_lock_obj ();
}

/*
 * This variable controls the maximum number of processes that will
 * be checked in doing deadlock detection.
 */
#if 0 /*TODO*/
#define MAXDEPTH 50
static int maxlockdepth = MAXDEPTH;
#endif

#define NOLOCKF (struct lockf_t *)0
#define SELF    0x1
#define OTHERS  0x2
static int      lf_clearlock (lockf_t *, lockf_t **);
static int      lf_findoverlap (lockf_t *, lockf_t *, int, lockf_t ***, lockf_t **);
static lockf_t   *lf_getblock (lockf_t *, inode_t *node);
static int      lf_getlock (lockf_t *, inode_t *, struct __flock64 *);
static int      lf_setlock (lockf_t *, inode_t *, lockf_t **);
static void     lf_split (lockf_t *, lockf_t *, lockf_t **);
static void     lf_wakelock (lockf_t *);

int
fhandler_disk_file::lock (int a_op, struct __flock64 *fl)
{
  _off64_t start, end, oadd;
  lockf_t *n;
  int error = 0;
  struct __stat64 stat;

  short a_flags = fl->l_type & (F_POSIX | F_FLOCK);
  short type = fl->l_type & (F_RDLCK | F_WRLCK | F_UNLCK);
  
  if (!a_flags)
    a_flags = F_POSIX; /* default */
  if (a_op == F_SETLKW)
    {
      a_op = F_SETLK;
      a_flags |= F_WAIT;
    }
  if (a_op == F_SETLK)
    switch (type)
      {
      case F_UNLCK:
	a_op = F_UNLCK;
	break;
      case F_RDLCK:
        if (!(get_access () & GENERIC_READ))
	  {
	    set_errno (EBADF);
	    return -1;
	  }
	break;
      case F_WRLCK:
        if (!(get_access () & GENERIC_WRITE))
	  {
	    set_errno (EBADF);
	    return -1;
	  }
	break;
      default:
      	set_errno (EINVAL);
	return -1;
      }

  if (fstat_by_handle (&stat) == -1)
    return -1;

  /*
   * Convert the flock structure into a start and end.
   */
  switch (fl->l_whence)
    {
    case SEEK_SET:
      start = fl->l_start;
      break;

    case SEEK_CUR:
      if ((start = lseek (0, SEEK_CUR)) == ILLEGAL_SEEK)
	return -1;
      break;

    case SEEK_END:
      if (fl->l_start > 0 && stat.st_size > OFF_MAX - fl->l_start)
	{
	  set_errno (EOVERFLOW);
	  return -1;
	}
      start = stat.st_size + fl->l_start;
      break;

    default:
      return (EINVAL);
    }
  if (start < 0)
    {
      set_errno (EINVAL);
      return -1;
    }
  if (fl->l_len < 0)
    {
      if (start == 0)
	{
	  set_errno (EINVAL);
	  return -1;
	}
      end = start - 1;
      start += fl->l_len;
      if (start < 0)
	{
	  set_errno (EINVAL);
	  return -1;
	}
    }
  else if (fl->l_len == 0)
    end = -1;
  else
    {
      oadd = fl->l_len - 1;
      if (oadd > OFF_MAX - start)
	{
	  set_errno (EOVERFLOW);
	  return -1;
	}
      end = start + oadd;
    }

  if (!node)
    {
      node = inode_t::get (stat.st_dev, stat.st_ino);
      need_fork_fixup (true);
    }
  /* Unlock the fd table which has been locked in fcntl_worker, otherwise
     a F_SETLKW waits forever... */
  cygheap->fdtab.unlock ();

  lockf_t **head = &node->i_lockf;

  /*
   * Avoid the common case of unlocking when inode_t has no locks.
   */
  if (*head == NULL)
    {
      if (a_op != F_SETLK)
	{
	  fl->l_type = F_UNLCK;
	  return 0;
	}
    }
  /*
   * Allocate a spare structure in case we have to split.
   */
  lockf_t *clean = NULL;
  if (a_op == F_SETLK || a_op == F_UNLCK)
    clean = new lockf_t ();
  /*
   * Create the lockf_t structure
   */
  lockf_t *lock = new lockf_t (node, head, a_flags, type, start, end,
			       getpid (), myself->dwProcessId);

  node->LOCK ();
  switch (a_op)
    {
    case F_SETLK:
      error = lf_setlock (lock, node, &clean);
      break;

    case F_UNLCK:
      error = lf_clearlock (lock, &clean);
      lock->lf_next = clean;
      clean = lock; 
      break;
    
    case F_GETLK:
      error = lf_getlock (lock, node, fl);
      lock->lf_next = clean;
      clean = lock;
      break;

    default:
      lock->lf_next = clean;
      clean = lock;
      error = EINVAL;
      break;
    }
  for (lock = clean; lock != NULL; )
    {
      n = lock->lf_next;
      delete lock;
      lock = n;
    }
  node->UNLOCK ();
  if (error)
    {
      set_errno (error);
      return -1;
    }
  return 0;
}

/*
 * Set a byte-range lock.
 */
static int
lf_setlock (lockf_t *lock, inode_t *node, lockf_t **clean)
{ 
  lockf_t *block;
  lockf_t **head = lock->lf_head;
  lockf_t **prev, *overlap;
  int ovcase, priority, old_prio, needtolink;

  /*
   * Set the priority
   */
  priority = old_prio = GetThreadPriority (GetCurrentThread ());
  if (lock->lf_type == F_WRLCK && priority <= THREAD_PRIORITY_ABOVE_NORMAL)
    priority = THREAD_PRIORITY_HIGHEST;
  /*
   * Scan lock list for this file looking for locks that would block us.
   */
  while ((block = lf_getblock(lock, node)))
    {
      /*
       * Free the structure and return if nonblocking.
       */
      if ((lock->lf_flags & F_WAIT) == 0)
	{
	  node->del_all_locks_list ();
	  lock->lf_next = *clean;
	  *clean = lock;
	  return EAGAIN;
	}
      /*
       * We are blocked. Since flock style locks cover
       * the whole file, there is no chance for deadlock.
       * For byte-range locks we must check for deadlock.
       *
       * Deadlock detection is done by looking through the
       * wait channels to see if there are any cycles that
       * involve us. MAXDEPTH is set just to make sure we
       * do not go off into neverland.
       */
      /* FIXME: We check the handle count of all the lock event objects
                this process holds.  If it's > 1, another process is
		waiting for one of our locks.  This method isn't overly
		intelligent.  If it turns out to be too dumb, we might
		have to remove it or to find another method. */
      for (lockf_t *lk = node->i_lockf; lk; lk = lk->lf_next)
	if (lk->get_lock_obj_handle_count () > 1)
	  return EDEADLK;

      /*
       * For flock type locks, we must first remove
       * any shared locks that we hold before we sleep
       * waiting for an exclusive lock.
       */
      if ((lock->lf_flags & F_FLOCK) && lock->lf_type == F_WRLCK)
	{
	  lock->lf_type = F_UNLCK;
	  (void) lf_clearlock (lock, clean);
	  lock->lf_type = F_WRLCK;
	}

      /*
       * Add our lock to the blocked list and sleep until we're free.
       * Remember who blocked us (for deadlock detection).
       */
      /* TODO */

      /* Wait for the blocking object and its holding process. */
      HANDLE obj = block->open_lock_obj ();
      HANDLE proc = OpenProcess (SYNCHRONIZE, FALSE, block->lf_wid);
      if (!proc)
        api_fatal ("OpenProcess: %E");
      HANDLE w4[3] = { obj, proc, signal_arrived };
      node->del_all_locks_list ();
      //SetThreadPriority (GetCurrentThread (), priority);
      node->UNLOCK ();
      DWORD ret = WaitForMultipleObjects (3, w4, FALSE, INFINITE);
      NtClose (proc);
      NtClose (obj);
      node->LOCK ();
      //SetThreadPriority (GetCurrentThread (), old_prio);
      switch (ret)
	{
	case WAIT_OBJECT_0:
	case WAIT_OBJECT_0 + 1:
	  /* The lock object has been set to signalled or the process
	     holding the lock has exited. */ 
	  break;
	case WAIT_OBJECT_0 + 2:
	  /* A signal came in. */
	  _my_tls.call_signal_handler ();
	  return EINTR;
	default:
	  return geterrno_from_win_error ();
	}
    }
  node->del_all_locks_list ();
  allow_others_to_sync ();
  /*
   * No blocks!!  Add the lock.  Note that we will
   * downgrade or upgrade any overlapping locks this
   * process already owns.
   *
   * Handle any locks that overlap.
   */
  prev = head;
  block = *head;
  needtolink = 1;
  for (;;)
    {
      ovcase = lf_findoverlap (block, lock, SELF, &prev, &overlap);
      if (ovcase)
	block = overlap->lf_next;
      /*
       * Six cases:
       *  0) no overlap
       *  1) overlap == lock
       *  2) overlap contains lock
       *  3) lock contains overlap
       *  4) overlap starts before lock
       *  5) overlap ends after lock
       */
      switch (ovcase)
	{
        case 0: /* no overlap */
	  if (needtolink)
	    {
	      *prev = lock;
	      lock->lf_next = overlap;
	      lock->create_lock_obj ();
            }
            break;

        case 1: /* overlap == lock */
	  /*
	   * If downgrading lock, others may be
	   * able to acquire it.
	   * Cygwin: Always wake lock.
	   */
	  lf_wakelock (overlap);
	  overlap->lf_type = lock->lf_type;
	  overlap->create_lock_obj ();
	  lock->lf_next = *clean;
	  *clean = lock;
	  break;

        case 2: /* overlap contains lock */
	  /*
	   * Check for common starting point and different types.
	   */
	  if (overlap->lf_type == lock->lf_type)
	    {
	      lock->lf_next = *clean;
	      *clean = lock;
	      break;
	    }
	  if (overlap->lf_start == lock->lf_start)
	    {
	      *prev = lock;
	      lock->lf_next = overlap;
	      overlap->lf_start = lock->lf_end + 1;
	    }
	  else
	    lf_split (overlap, lock, clean);
	  lf_wakelock (overlap);
	  overlap->create_lock_obj ();
	  lock->create_lock_obj ();
	  if (lock->lf_next && !lock->lf_next->lf_obj)
	    lock->lf_next->create_lock_obj ();
	  break;

        case 3: /* lock contains overlap */
	  /*
	   * If downgrading lock, others may be able to
	   * acquire it, otherwise take the list.
	   * Cygwin: Always wake old lock and create new lock.
	   */
	  lf_wakelock (overlap);
	  /*
	   * Add the new lock if necessary and delete the overlap.
	   */
	  if (needtolink)
	    {
	      *prev = lock;
	      lock->lf_next = overlap->lf_next;
	      prev = &lock->lf_next;
	      lock->create_lock_obj ();
	      needtolink = 0;
	    }
	  else
	    *prev = overlap->lf_next;
	  overlap->lf_next = *clean;
	  *clean = overlap;
	  continue;

        case 4: /* overlap starts before lock */
	  /*
	   * Add lock after overlap on the list.
	   */
	  lock->lf_next = overlap->lf_next;
	  overlap->lf_next = lock;
	  overlap->lf_end = lock->lf_start - 1;
	  prev = &lock->lf_next;
	  lf_wakelock (overlap);
	  overlap->create_lock_obj ();
	  lock->create_lock_obj ();
	  needtolink = 0;
	  continue;

        case 5: /* overlap ends after lock */
	  /*
	   * Add the new lock before overlap.
	   */
	  if (needtolink) {
	      *prev = lock;
	      lock->lf_next = overlap;
	  }
	  overlap->lf_start = lock->lf_end + 1;
	  lf_wakelock (overlap);
	  lock->create_lock_obj ();
	  overlap->create_lock_obj ();
	  break;
	}
      break;
    }
  return 0;
}

/*
 * Remove a byte-range lock on an inode_t.
 *
 * Generally, find the lock (or an overlap to that lock)
 * and remove it (or shrink it), then wakeup anyone we can.
 */
static int
lf_clearlock (lockf_t *unlock, lockf_t **clean)
{
  lockf_t **head = unlock->lf_head;
  lockf_t *lf = *head;
  lockf_t *overlap, **prev;
  int ovcase;

  if (lf == NOLOCKF)
    return 0;
  prev = head;
  while ((ovcase = lf_findoverlap (lf, unlock, SELF, &prev, &overlap)))
    {
      /*
       * Wakeup the list of locks to be retried.
       */
      lf_wakelock (overlap);

      switch (ovcase)
	{
        case 1: /* overlap == lock */
	  *prev = overlap->lf_next;
	  overlap->lf_next = *clean;
	  *clean = overlap;
	  break;

        case 2: /* overlap contains lock: split it */
	  if (overlap->lf_start == unlock->lf_start)
	    {
	      overlap->lf_start = unlock->lf_end + 1;
	      overlap->create_lock_obj ();
	      break;
	    }
	  lf_split (overlap, unlock, clean);
	  overlap->lf_next = unlock->lf_next;
	  overlap->create_lock_obj ();
	  if (overlap->lf_next && !overlap->lf_next->lf_obj)
	    overlap->lf_next->create_lock_obj ();
	  break;

        case 3: /* lock contains overlap */
	  *prev = overlap->lf_next;
	  lf = overlap->lf_next;
	  overlap->lf_next = *clean;
	  *clean = overlap;
	  continue;

        case 4: /* overlap starts before lock */
            overlap->lf_end = unlock->lf_start - 1;
            prev = &overlap->lf_next;
            lf = overlap->lf_next;
	    overlap->create_lock_obj ();
            continue;

        case 5: /* overlap ends after lock */
            overlap->lf_start = unlock->lf_end + 1;
	    overlap->create_lock_obj ();
            break;
	}
      break;
    }
  return 0;
}

/*
 * Check whether there is a blocking lock,
 * and if so return its process identifier.
 */
static int
lf_getlock (lockf_t *lock, inode_t *node, struct __flock64 *fl)
{
  lockf_t *block;

  if ((block = lf_getblock (lock, node)))
    {
      fl->l_type = block->lf_type;
      fl->l_whence = SEEK_SET;
      fl->l_start = block->lf_start;
      if (block->lf_end == -1)
	fl->l_len = 0;
      else
	fl->l_len = block->lf_end - block->lf_start + 1;
      if (block->lf_flags & F_POSIX)
	fl->l_pid = block->lf_id;
      else
	fl->l_pid = -1;
    }
  else
    fl->l_type = F_UNLCK;
  node->del_all_locks_list ();
  return 0;
}

/*
 * Walk the list of locks for an inode_t and
 * return the first blocking lock.
 */
static lockf_t *
lf_getblock (lockf_t *lock, inode_t *node)
{   
  node->get_all_locks_list ();
  lockf_t **prev, *overlap, *lf = node->i_all_lf;
  int ovcase;

  prev = lock->lf_head;
  while ((ovcase = lf_findoverlap (lf, lock, OTHERS, &prev, &overlap)))
    {
      /*
       * We've found an overlap, see if it blocks us
       */
      if ((lock->lf_type == F_WRLCK || overlap->lf_type == F_WRLCK))
	  return overlap;
      /*
       * Nope, point to the next one on the list and
       * see if it blocks us
       */
      lf = overlap->lf_next;
    }
  return NOLOCKF;
}

/*
 * Walk the list of locks for an inode_t to
 * find an overlapping lock (if any).
 *
 * NOTE: this returns only the FIRST overlapping lock.  There
 *   may be more than one.
 */ 
static int
lf_findoverlap (lockf_t *lf, lockf_t *lock, int type, lockf_t ***prev,
		lockf_t **overlap)
{
  _off64_t start, end;

  *overlap = lf;
  if (lf == NOLOCKF)
    return 0;

  start = lock->lf_start;
  end = lock->lf_end;
  while (lf != NOLOCKF)
    {
      if ((type & OTHERS) && lf->lf_id == lock->lf_id)
	{
	  *prev = &lf->lf_next;
	  *overlap = lf = lf->lf_next;
	  continue;
        }
      /*
       * OK, check for overlap
       *
       * Six cases:
       *  0) no overlap
       *  1) overlap == lock
       *  2) overlap contains lock
       *  3) lock contains overlap
       *  4) overlap starts before lock
       *  5) overlap ends after lock
       */
      if ((lf->lf_end != -1 && start > lf->lf_end) ||
	  (end != -1 && lf->lf_start > end))
	{
	  /* Case 0 */
	  if ((type & SELF) && end != -1 && lf->lf_start > end)
	    return 0;
	  *prev = &lf->lf_next;
	  *overlap = lf = lf->lf_next;
	  continue;
        }
      if ((lf->lf_start == start) && (lf->lf_end == end))
	{
	  /* Case 1 */
	  return 1;
	}
      if ((lf->lf_start <= start) && (end != -1) &&
	  ((lf->lf_end >= end) || (lf->lf_end == -1)))
	{
	  /* Case 2 */
	  return 2;
	}
      if (start <= lf->lf_start && (end == -1 ||
	  (lf->lf_end != -1 && end >= lf->lf_end)))
	{
	  /* Case 3 */
	  return 3;
	}
      if ((lf->lf_start < start) &&
	  ((lf->lf_end >= start) || (lf->lf_end == -1)))
	{
	  /* Case 4 */
	  return 4;
	}
      if ((lf->lf_start > start) && (end != -1) &&
	  ((lf->lf_end > end) || (lf->lf_end == -1)))
	{
	  /* Case 5 */
	  return 5;
	}
      api_fatal ("lf_findoverlap: default\n");
    }
  return 0;
}

/*
 * Split a lock and a contained region into
 * two or three locks as necessary.
 */
static void
lf_split (lockf_t *lock1, lockf_t *lock2, lockf_t **split)
{
  lockf_t *splitlock;

  /* 
   * Check to see if spliting into only two pieces.
   */
  if (lock1->lf_start == lock2->lf_start)
    {
      lock1->lf_start = lock2->lf_end + 1;
      lock2->lf_next = lock1;
      return;
    }
  if (lock1->lf_end == lock2->lf_end)
    {
      lock1->lf_end = lock2->lf_start - 1;
      lock2->lf_next = lock1->lf_next;
      lock1->lf_next = lock2;
      return;
    }
  /*
   * Make a new lock consisting of the last part of
   * the encompassing lock.  We use the preallocated
   * splitlock so we don't have to block.
   */
  splitlock = *split;
  assert (splitlock != NULL);
  *split = splitlock->lf_next;
  memcpy (splitlock, lock1, sizeof *splitlock);
  /* We have to unset the obj HANDLE here which has been copied by the
     above memcpy, so that the calling function recognizes the new object.
     See post-lf_split handling in lf_setlock and lf_clearlock. */
  splitlock->lf_obj = NULL;
  splitlock->lf_start = lock2->lf_end + 1;
  lock1->lf_end = lock2->lf_start - 1;
  /*
   * OK, now link it in
   */
  splitlock->lf_next = lock1->lf_next;
  lock2->lf_next = splitlock;
  lock1->lf_next = lock2;
}

/*
 * Wakeup a blocklist
 * Cygwin: Just signal the lock which gets removed.  This unblocks
 * all threads waiting for this lock.
 */
static void
lf_wakelock (lockf_t *listhead)
{
  listhead->del_lock_obj ();
}

int
flock (int fd, int operation)
{
  int i, cmd;
  struct __flock64 l = { 0, 0, 0, 0, 0 };
  if (operation & LOCK_NB)
    {
      cmd = F_SETLK;
    }
  else
    {
      cmd = F_SETLKW;
    }
  l.l_whence = SEEK_SET;
  switch (operation & (~LOCK_NB))
    {
    case LOCK_EX:
      l.l_type = F_WRLCK | F_FLOCK;
      i = fcntl_worker (fd, cmd, &l);
      if (i == -1)
	{
	  if ((get_errno () == EAGAIN) || (get_errno () == EACCES))
	    {
	      set_errno (EWOULDBLOCK);
	    }
	}
      break;
    case LOCK_SH:
      l.l_type = F_RDLCK | F_FLOCK;
      i = fcntl_worker (fd, cmd, &l);
      if (i == -1)
	{
	  if ((get_errno () == EAGAIN) || (get_errno () == EACCES))
	    {
	      set_errno (EWOULDBLOCK);
	    }
	}
      break;
    case LOCK_UN:
      l.l_type = F_UNLCK | F_FLOCK;
      i = fcntl_worker (fd, cmd, &l);
      if (i == -1)
	{
	  if ((get_errno () == EAGAIN) || (get_errno () == EACCES))
	    {
	      set_errno (EWOULDBLOCK);
	    }
	}
      break;
    default:
      i = -1;
      set_errno (EINVAL);
      break;
    }
  return i;
}

extern "C" int
lockf (int filedes, int function, _off64_t size)
{
  struct flock fl;
  int cmd;

  fl.l_start = 0;
  fl.l_len = size;
  fl.l_whence = SEEK_CUR;

  switch (function)
    {
    case F_ULOCK:
      cmd = F_SETLK;
      fl.l_type = F_UNLCK;
      break;
    case F_LOCK:
      cmd = F_SETLKW;
      fl.l_type = F_WRLCK;
      break;
    case F_TLOCK:
      cmd = F_SETLK;
      fl.l_type = F_WRLCK;
      break;
    case F_TEST:
      fl.l_type = F_WRLCK;
      if (fcntl_worker (filedes, F_GETLK, &fl) == -1)
	return -1;
      if (fl.l_type == F_UNLCK || fl.l_pid == getpid ())
	return 0;
      errno = EAGAIN;
      return -1;
      /* NOTREACHED */
    default:
      errno = EINVAL;
      return -1;
      /* NOTREACHED */
    }

  return fcntl_worker (filedes, cmd, &fl);
}
