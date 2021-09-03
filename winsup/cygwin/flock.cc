/* flock.cc.  NT specific implementation of advisory file locking.

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

/* The lockf function is based upon FreeBSD sources with the following
 * copyright.
 */
/*
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
 */

#include "winsup.h"
#include <assert.h>
#include <sys/file.h>
#include <unistd.h>
#include <stdlib.h>
#include "cygerrno.h"
#include "security.h"
#include "shared_info.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include "pinfo.h"
#include "sigproc.h"
#include "cygtls.h"
#include "tls_pbuf.h"
#include "miscfuncs.h"
#include "ntdll.h"
#include <sys/queue.h>
#include <wchar.h>

#define F_WAIT 0x10	/* Wait until lock is granted */
#define F_FLOCK 0x20	/* Use flock(2) semantics for lock */
#define F_POSIX	0x40	/* Use POSIX semantics for lock */

#ifndef OFF_MAX
#define OFF_MAX LLONG_MAX
#endif

static NO_COPY muto lockf_guard;

#define INODE_LIST_LOCK()	(lockf_guard.init ("lockf_guard")->acquire ())
#define INODE_LIST_UNLOCK()	(lockf_guard.release ())

#define LOCK_DIR_NAME_FMT	L"flock-%08x-%016X"
#define LOCK_DIR_NAME_LEN	31	/* Length of the resulting name */
#define LOCK_DIR_NAME_DEV_OFF	 6	/* Offset to device number */
#define LOCK_DIR_NAME_INO_OFF	15	/* Offset to inode number */

/* Don't change format without also changing lockf_t::from_obj_name! */
#define LOCK_OBJ_NAME_FMT	L"%02x-%01x-%016X-%016X-%016X-%08x-%04x"
#define LOCK_OBJ_NAME_LEN	69	/* Length of the resulting name */

#define FLOCK_INODE_DIR_ACCESS	(DIRECTORY_QUERY \
				 | DIRECTORY_TRAVERSE \
				 | DIRECTORY_CREATE_OBJECT \
				 | READ_CONTROL)

#define FLOCK_EVENT_ACCESS	(EVENT_QUERY_STATE \
				 | SYNCHRONIZE \
				 | READ_CONTROL)

/* This function takes the own process security descriptor DACL and adds
   SYNCHRONIZE permissions for everyone.  This allows all processes
   to wait for this process to die when blocking in a F_SETLKW on a lock
   which is hold by this process. */
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

  /* Get this process DACL.  We use a rather small stack buffer here which
     should be more than sufficient for process ACLs.  Can't use tls functions
     at this point because this gets called during initialization when the tls
     is not really available.  */
#define MAX_PROCESS_SD_SIZE	3072
  PISECURITY_DESCRIPTOR sd = (PISECURITY_DESCRIPTOR) alloca (MAX_PROCESS_SD_SIZE);
  status = NtQuerySecurityObject (NtCurrentProcess (),
				  DACL_SECURITY_INFORMATION, sd,
				  MAX_PROCESS_SD_SIZE, &len);
  if (!NT_SUCCESS (status))
    {
      debug_printf ("NtQuerySecurityObject: %y", status);
      return;
    }
  /* Create a valid dacl pointer and set its size to be as big as
     there's room in the temporary buffer.  Note that the descriptor
     is in self-relative format. */
  BOOLEAN present, defaulted;
  RtlGetDaclSecurityDescriptor (sd, &present, &dacl, &defaulted);
  if (!present) /* If so, dacl has undefined value. */
    {
      dacl = (PACL) (sd + 1);
      RtlCreateAcl (dacl, MAX_PROCESS_SD_SIZE - sizeof *sd, ACL_REVISION);
    }
  else if (dacl == NULL) /* Everyone has all access anyway */
    {
      done = true;
      return;
    }
  else
    {
      dacl->AclSize = MAX_PROCESS_SD_SIZE - ((PBYTE) dacl - (PBYTE) sd);
    }
  /* Allow everyone to SYNCHRONIZE with this process. */
  status = RtlAddAccessAllowedAce (dacl, ACL_REVISION, SYNCHRONIZE,
				   well_known_world_sid);
  if (!NT_SUCCESS (status))
    {
      debug_printf ("RtlAddAccessAllowedAce: %y", status);
      return;
    }
  /* Set the size of the DACL correctly. */
  status = RtlFirstFreeAce (dacl, &ace);
  if (!NT_SUCCESS (status))
    {
      debug_printf ("RtlFirstFreeAce: %y", status);
      return;
    }
  dacl->AclSize = (char *) ace - (char *) dacl;
  /* Write the DACL back. */
  status = NtSetSecurityObject (NtCurrentProcess (), DACL_SECURITY_INFORMATION, sd);
  if (!NT_SUCCESS (status))
    {
      debug_printf ("NtSetSecurityObject: %y", status);
      return;
    }
  done = true;
}

/* Helper struct to construct a local OBJECT_ATTRIBUTES on the stack. */
struct lockfattr_t
{
  OBJECT_ATTRIBUTES attr;
  UNICODE_STRING uname;
  WCHAR name[LOCK_OBJ_NAME_LEN + 1];
};

/* Per lock class. */
class lockf_t
{
  public:
    uint16_t	    lf_flags; /* Semantics: F_POSIX, F_FLOCK, F_WAIT */
    uint16_t	    lf_type;  /* Lock type: F_RDLCK, F_WRLCK */
    off_t	    lf_start; /* Byte # of the start of the lock */
    off_t	    lf_end;   /* Byte # of the end of the lock (-1=EOF) */
    int64_t         lf_id;    /* Cygwin PID for POSIX locks, a unique id per
				 file table entry for BSD flock locks. */
    DWORD	    lf_wid;   /* Win PID of the resource holding the lock */
    uint16_t	    lf_ver;   /* Version number of the lock.  If a released
				 lock event yet exists because another process
				 is still waiting for it, we use the version
				 field to distinguish old from new locks. */
    class lockf_t **lf_head;  /* Back pointer to the head of the lockf_t list */
    class inode_t  *lf_inode; /* Back pointer to the inode_t */
    class lockf_t  *lf_next;  /* Pointer to the next lock on this inode_t */
    HANDLE	    lf_obj;   /* Handle to the lock event object. */

    lockf_t ()
    : lf_flags (0), lf_type (0), lf_start (0), lf_end (0), lf_id (0),
      lf_wid (0), lf_ver (0), lf_head (NULL), lf_inode (NULL),
      lf_next (NULL), lf_obj (NULL)
    {}
    lockf_t (class inode_t *node, class lockf_t **head,
	     short flags, short type, off_t start, off_t end,
	     long long id, DWORD wid, uint16_t ver)
    : lf_flags (flags), lf_type (type), lf_start (start), lf_end (end),
      lf_id (id), lf_wid (wid), lf_ver (ver), lf_head (head), lf_inode (node),
      lf_next (NULL), lf_obj (NULL)
    {}
    ~lockf_t ();

    bool from_obj_name (class inode_t *node, class lockf_t **head,
			const wchar_t *name);

    /* Used to create all locks list in a given TLS buffer. */
    void *operator new (size_t size, void *p)
    { return p; }
    /* Used to store own lock list in the cygheap. */
    void *operator new (size_t size)
    { return cmalloc (HEAP_FHANDLER, sizeof (lockf_t)); }
    /* Never call on node->i_all_lf! */
    void operator delete (void *p)
    { cfree (p); }

    POBJECT_ATTRIBUTES create_lock_obj_attr (lockfattr_t *attr,
					     ULONG flags, void *sd_buf);

    void create_lock_obj ();
    bool open_lock_obj ();
    void close_lock_obj () { NtClose (lf_obj); lf_obj = NULL; }
    void del_lock_obj (HANDLE fhdl, bool signal = false);
};

/* Per inode_t class */
class inode_t
{
  friend class lockf_t;

  public:
    LIST_ENTRY (inode_t) i_next;
    lockf_t		*i_lockf;  /* List of locks of this process. */
    lockf_t		*i_all_lf; /* Temp list of all locks for this file. */

    dev_t		 i_dev;    /* Device ID */
    ino_t		 i_ino;    /* inode number */

  private:
    HANDLE		 i_dir;
    HANDLE		 i_mtx;
    uint32_t		 i_cnt;    /* # of threads referencing this instance. */

  public:
    inode_t (dev_t dev, ino_t ino);
    ~inode_t ();

    void *operator new (size_t size)
    { return cmalloc (HEAP_FHANDLER, sizeof (inode_t)); }
    void operator delete (void *p)
    { cfree (p); }

    static inode_t *get (dev_t dev, ino_t ino,
			 bool create_if_missing, bool lock);

    void LOCK () { WaitForSingleObject (i_mtx, INFINITE); }
    void UNLOCK () { ReleaseMutex (i_mtx); }

    void use () { ++i_cnt; }
    void unuse () { if (i_cnt > 0) --i_cnt; }
    bool inuse () { return i_cnt > 0; }
    void notused () { i_cnt = 0; }

    void unlock_and_remove_if_unused ();

    lockf_t *get_all_locks_list ();

    bool del_my_locks (long long id, HANDLE fhdl);
};

inode_t::~inode_t ()
{
  lockf_t *lock, *n_lock;
  for (lock = i_lockf; lock && (n_lock = lock->lf_next, 1); lock = n_lock)
    delete lock;
  NtClose (i_mtx);
  NtClose (i_dir);
}

void
inode_t::unlock_and_remove_if_unused ()
{
  UNLOCK ();
  INODE_LIST_LOCK ();
  unuse ();
  if (i_lockf == NULL && !inuse ())
    {
      LIST_REMOVE (this, i_next);
      delete this;
    }
  INODE_LIST_UNLOCK ();
}

bool
inode_t::del_my_locks (long long id, HANDLE fhdl)
{
  lockf_t *lock, *n_lock;
  lockf_t **prev = &i_lockf;
  for (lock = *prev; lock && (n_lock = lock->lf_next, 1); lock = n_lock)
    {
      if (lock->lf_flags & F_POSIX)
	{
	  /* Delete all POSIX locks. */
	  *prev = n_lock;
	  /* When called during fork, the POSIX lock must get deleted but
	     *not* signalled.  The lock is still active and locked in the
	     parent.  So in case of fork, we call close_lock_obj explicitely,
	     since del_lock_obj is called from the destructor. */
	  if (!id)
	    lock->close_lock_obj ();
	  delete lock;
	}
      else if (id && lock->lf_id == id)
	{
	  int cnt = 0;
	  cygheap_fdenum cfd (true);
	  while (cfd.next () >= 0)
	    if (cfd->get_unique_id () == lock->lf_id && ++cnt > 1)
	      break;
	  /* Delete BSD flock lock when no other fd in this process references
	     it anymore. */
	  if (cnt <= 1)
	    {
	      *prev = n_lock;
	      lock->del_lock_obj (fhdl);
	      delete lock;
	    }
	}
      else
	prev = &lock->lf_next;
    }
  return i_lockf == NULL;
}

/* Used to delete the locks on a file hold by this process.  Called from
   close(2) and fixup_after_fork, as well as from fixup_after_exec in
   case the close_on_exec flag is set.  The whole inode is deleted as
   soon as no lock exists on it anymore. */
void
fhandler_base::del_my_locks (del_lock_called_from from)
{
  inode_t *node = inode_t::get (get_dev (), get_ino (), false, true);
  if (node)
    {
      /* When we're called from fixup_after_exec, the fhandler is a
	 close-on-exec fhandler.  In this case our io handle is already
	 invalid.  We can't use it to test for the object reference count.
	 However, that shouldn't be necessary for the following reason.
	 After exec, there are no threads in the current process waiting for
	 the lock.  So, either we're the only process accessing the file table
	 entry and there are no threads which require signalling, or we have
	 a parent process still accessing the file object and signalling the
	 lock event would be premature. */
      node->del_my_locks (from == after_fork ? 0 : get_unique_id (),
			  from == after_exec ? NULL : get_handle ());
      node->unlock_and_remove_if_unused ();
    }
}

/* Called in an execed child.  The exec'ed process must allow SYNCHRONIZE
   access to everyone if at least one inode exists.
   The lock owner's Windows PID changed and all POSIX lock event objects
   have to be relabeled so that waiting processes know which process to
   wait on.  If the node has been abandoned due to close_on_exec on the
   referencing fhandlers, remove the inode entirely. */
void
fixup_lockf_after_exec (bool exec)
{
  inode_t *node, *next_node;

  INODE_LIST_LOCK ();
  if (LIST_FIRST (&cygheap->inode_list))
    allow_others_to_sync ();
  LIST_FOREACH_SAFE (node, &cygheap->inode_list, i_next, next_node)
    {
      node->notused ();
      int cnt = 0;
      cygheap_fdenum cfd (true);
      while (cfd.next () >= 0)
	if (cfd->get_dev () == node->i_dev
	    && cfd->get_ino () == node->i_ino
	    && ++cnt >= 1)
	  break;
      if (cnt == 0)
	{
	  LIST_REMOVE (node, i_next);
	  delete node;
	}
      else
	{
	  node->LOCK ();
	  lockf_t *lock, *n_lock;
	  lockf_t **prev = &node->i_lockf;
	  for (lock = *prev; lock && (n_lock = lock->lf_next, 1); lock = n_lock)
	    if (lock->lf_flags & F_POSIX)
	      {
		if (exec)
		  {
		    /* The parent called exec.  The lock is passed to the child.
		       Recreate lock object with changed ownership. */
		    lock->del_lock_obj (NULL);
		    lock->lf_wid = myself->dwProcessId;
		    lock->lf_ver = 0;
		    lock->create_lock_obj ();
		  }
		else
		  {
		    /* The parent called spawn.  The parent continues to hold
		       the POSIX lock, ownership is not passed to the child.
		       Give up the lock in the child. */
		    *prev = n_lock;
		    lock->close_lock_obj ();
		    delete lock;
		  }
	      }
	  node->UNLOCK ();
	}
    }
  INODE_LIST_UNLOCK ();
}

/* static method to return a pointer to the inode_t structure for a specific
   file.  The file is specified by the device and inode_t number.  If inode_t
   doesn't exist, create it. */
inode_t *
inode_t::get (dev_t dev, ino_t ino, bool create_if_missing, bool lock)
{
  inode_t *node;

  INODE_LIST_LOCK ();
  LIST_FOREACH (node, &cygheap->inode_list, i_next)
    if (node->i_dev == dev && node->i_ino == ino)
      break;
  if (!node && create_if_missing)
    {
      node = new inode_t (dev, ino);
      if (node)
	LIST_INSERT_HEAD (&cygheap->inode_list, node, i_next);
    }
  if (node)
    node->use ();
  INODE_LIST_UNLOCK ();
  if (node && lock)
    node->LOCK ();
  return node;
}

inode_t::inode_t (dev_t dev, ino_t ino)
: i_lockf (NULL), i_all_lf (NULL), i_dev (dev), i_ino (ino), i_cnt (0L)
{
  HANDLE parent_dir;
  WCHAR name[48];
  UNICODE_STRING uname;
  OBJECT_ATTRIBUTES attr;
  NTSTATUS status;

  parent_dir = get_shared_parent_dir ();
  /* Create a subdir which is named after the device and inode_t numbers
     of the given file, in hex notation. */
  int len = __small_swprintf (name, LOCK_DIR_NAME_FMT, dev, ino);
  RtlInitCountedUnicodeString (&uname, name, len * sizeof (WCHAR));
  InitializeObjectAttributes (&attr, &uname, OBJ_INHERIT | OBJ_OPENIF,
			      parent_dir, everyone_sd (FLOCK_INODE_DIR_ACCESS));
  status = NtCreateDirectoryObject (&i_dir, FLOCK_INODE_DIR_ACCESS, &attr);
  if (!NT_SUCCESS (status))
    api_fatal ("NtCreateDirectoryObject(inode): %y", status);
  /* Create a mutex object in the file specific dir, which is used for
     access synchronization on the dir and its objects. */
  InitializeObjectAttributes (&attr, &ro_u_mtx, OBJ_INHERIT | OBJ_OPENIF, i_dir,
			      everyone_sd (CYG_MUTANT_ACCESS));
  status = NtCreateMutant (&i_mtx, CYG_MUTANT_ACCESS, &attr, FALSE);
  if (!NT_SUCCESS (status))
    api_fatal ("NtCreateMutant(inode): %y", status);
}

/* Enumerate all lock event objects for this file and create a lockf_t
   list in the i_all_lf member.  This list is searched in lf_getblock
   for locks which potentially block our lock request. */

/* Number of lockf_t structs which fit in the temporary buffer. */
#define MAX_LOCKF_CNT	((intptr_t)((NT_MAX_PATH * sizeof (WCHAR)) \
				    / sizeof (lockf_t)))

bool
lockf_t::from_obj_name (inode_t *node, lockf_t **head, const wchar_t *name)
{
  wchar_t *endptr;

  /* "%02x-%01x-%016X-%016X-%016X-%08x-%04x",
     lf_flags, lf_type, lf_start, lf_end, lf_id, lf_wid, lf_ver */
  lf_flags = wcstol (name, &endptr, 16);
  if ((lf_flags & ~(F_FLOCK | F_POSIX)) != 0
      || ((lf_flags & (F_FLOCK | F_POSIX)) == (F_FLOCK | F_POSIX)))
    return false;
  lf_type = wcstol (endptr + 1, &endptr, 16);
  if ((lf_type != F_RDLCK && lf_type != F_WRLCK) || !endptr || *endptr != L'-')
    return false;
  lf_start = (off_t) wcstoull (endptr + 1, &endptr, 16);
  if (lf_start < 0 || !endptr || *endptr != L'-')
    return false;
  lf_end = (off_t) wcstoull (endptr + 1, &endptr, 16);
  if (lf_end < -1LL
      || (lf_end > 0 && lf_end < lf_start)
      || !endptr || *endptr != L'-')
    return false;
  lf_id = wcstoll (endptr + 1, &endptr, 16);
  if (!endptr || *endptr != L'-'
      || ((lf_flags & F_POSIX) && (lf_id < 1 || lf_id > UINT32_MAX)))
    return false;
  lf_wid = wcstoul (endptr + 1, &endptr, 16);
  if (!endptr || *endptr != L'-')
    return false;
  lf_ver = wcstoul (endptr + 1, &endptr, 16);
  if (endptr && *endptr != L'\0')
    return false;
  lf_head = head;
  lf_inode = node;
  lf_next = NULL;
  lf_obj = NULL;
  return true;
}

lockf_t *
inode_t::get_all_locks_list ()
{
  tmp_pathbuf tp;
  ULONG context;
  NTSTATUS status;
  BOOLEAN restart = TRUE;
  bool last_run = false;
  lockf_t newlock, *lock = i_all_lf;

  PDIRECTORY_BASIC_INFORMATION dbi_buf = (PDIRECTORY_BASIC_INFORMATION)
					 tp.w_get ();
  while (!last_run)
    {
      status = NtQueryDirectoryObject (i_dir, dbi_buf, 65536, FALSE, restart,
				       &context, NULL);
      if (!NT_SUCCESS (status))
	{
	  debug_printf ("NtQueryDirectoryObject, status %y", status);
	  break;
	}
      if (status != STATUS_MORE_ENTRIES)
	last_run = true;
      restart = FALSE;
      for (PDIRECTORY_BASIC_INFORMATION dbi = dbi_buf;
	   dbi->ObjectName.Length > 0;
	   dbi++)
	{
	  if (dbi->ObjectName.Length != LOCK_OBJ_NAME_LEN * sizeof (WCHAR))
	    continue;
	  dbi->ObjectName.Buffer[LOCK_OBJ_NAME_LEN] = L'\0';
	  if (!newlock.from_obj_name (this, &i_all_lf, dbi->ObjectName.Buffer))
	    continue;
	  if (lock - i_all_lf >= MAX_LOCKF_CNT)
	    {
	      system_printf ("Warning, can't handle more than %d locks per file.",
			     MAX_LOCKF_CNT);
	      break;
	    }
	  if (lock > i_all_lf)
	    lock[-1].lf_next = lock;
	  new (lock++) lockf_t (newlock);
	}
    }
  /* If no lock has been found, return NULL. */
  if (lock == i_all_lf)
    return NULL;
  return i_all_lf;
}

/* Create the lock object name.  The name is constructed from the lock
   properties which identify it uniquely, all values in hex. */
POBJECT_ATTRIBUTES
lockf_t::create_lock_obj_attr (lockfattr_t *attr, ULONG flags, void *sd_buf)
{
  __small_swprintf (attr->name, LOCK_OBJ_NAME_FMT,
		    lf_flags & (F_POSIX | F_FLOCK), lf_type, lf_start, lf_end,
		    lf_id, lf_wid, lf_ver);
  RtlInitCountedUnicodeString (&attr->uname, attr->name,
			       LOCK_OBJ_NAME_LEN * sizeof (WCHAR));
  InitializeObjectAttributes (&attr->attr, &attr->uname, flags, lf_inode->i_dir,
			      _everyone_sd (sd_buf, FLOCK_EVENT_ACCESS));
  return &attr->attr;
}

DWORD WINAPI
create_lock_in_parent (PVOID param)
{
  HANDLE lf_obj;
  ULONG size;
  OBJECT_NAME_INFORMATION *ntfn;
  NTSTATUS status;
  wchar_t *lockname, *inodename, *endptr;
  dev_t dev;
  ino_t ino;
  inode_t *node;
  lockf_t newlock, *lock;
  int cnt;

  /* param is the handle to the lock object, created by caller. */
  lf_obj = (HANDLE) param;
  /* Fetch object path from handle.  Typically the length of the path
     is 146 characters, starting with
     "\BaseNamedObject\cygwin-1S5-<16-hex-digits>\..." */
  size = sizeof (OBJECT_NAME_INFORMATION) + 256 * sizeof (WCHAR);
  ntfn = (OBJECT_NAME_INFORMATION *) alloca (size);
  memset (ntfn, 0, size);
  status = NtQueryObject (lf_obj, ObjectNameInformation, ntfn, size, &size);
  if (!NT_SUCCESS (status))
    goto err;
  ntfn->Name.Buffer[ntfn->Name.Length / sizeof (WCHAR)] = L'\0';
  /* Sanity check so that we don't peek into unchartered territory. */
  if (ntfn->Name.Length < LOCK_OBJ_NAME_LEN + LOCK_DIR_NAME_LEN + 1)
    goto err;
  /* The names have fixed size, so we know where the substrings start. */
  lockname = ntfn->Name.Buffer + ntfn->Name.Length / sizeof (WCHAR)
			       - LOCK_OBJ_NAME_LEN;
  inodename = lockname - LOCK_DIR_NAME_LEN - 1;
  dev = wcstoul (inodename + LOCK_DIR_NAME_DEV_OFF, &endptr, 16);
  if (*endptr != L'-')
    goto err;
  ino = wcstoull (inodename + LOCK_DIR_NAME_INO_OFF, &endptr, 16);
  if (*endptr != L'\\')
    goto err;
  if (!newlock.from_obj_name (NULL, NULL, lockname))
    goto err;
  /* Check if we have an open file handle with the same unique id. */
  {
    cnt = 0;
    cygheap_fdenum cfd (true);
    while (cfd.next () >= 0)
      if (cfd->get_unique_id () == newlock.lf_id && ++cnt > 0)
	break;
  }
  /* If not, close handle and return. */
  if (!cnt)
    {
      NtClose (lf_obj);
      return 0;
    }
  /* The handle gets created non-inheritable.  That's fine, unless the parent
     starts another process accessing this object.  So, after it's clear we
     have to store the handle for further use, make sure it gets inheritable
     by child processes. */
  if (!SetHandleInformation (lf_obj, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT))
    goto err;
  /* otherwise generate inode from directory name... */
  node = inode_t::get (dev, ino, true, false);
  /* ...and generate lock from object name. */
  lock = new lockf_t (newlock);
  lock->lf_inode = node;
  lock->lf_head = &node->i_lockf;
  lock->lf_next = node->i_lockf;
  lock->lf_obj = lf_obj;
  node->i_lockf = lock;
  node->unuse ();
  return 0;

err:
  system_printf ("Adding <%S> lock failed", &ntfn->Name);
  NtClose (lf_obj);
  return 1;
}

DWORD WINAPI
delete_lock_in_parent (PVOID param)
{
  inode_t *node, *next_node;
  lockf_t *lock, **prev;

  /* Scan list of all inodes, and reap stale BSD lock if lf_id matches.
     Remove inode if empty. */
  INODE_LIST_LOCK ();
  LIST_FOREACH_SAFE (node, &cygheap->inode_list, i_next, next_node)
    if (!node->inuse ())
      {
	for (prev = &node->i_lockf, lock = *prev; lock; lock = *prev)
	  {
	    if ((lock->lf_flags & F_FLOCK) && IsEventSignalled (lock->lf_obj))
	      {
		*prev = lock->lf_next;
		delete lock;
	      }
	    else
	      prev = &lock->lf_next;
	  }
	if (node->i_lockf == NULL)
	  {
	    LIST_REMOVE (node, i_next);
	    delete node;
	  }
      }
  INODE_LIST_UNLOCK ();
  return 0;
}

/* Create the lock event object in the file's subdir in the NT global
   namespace. */
void
lockf_t::create_lock_obj ()
{
  lockfattr_t attr;
  NTSTATUS status;
  PSECURITY_DESCRIPTOR sd_buf = alloca (SD_MIN_SIZE);
  POBJECT_ATTRIBUTES lock_obj_attr;

  do
    {
      lock_obj_attr = create_lock_obj_attr (&attr, OBJ_INHERIT, sd_buf);
      status = NtCreateEvent (&lf_obj, CYG_EVENT_ACCESS, lock_obj_attr,
			      NotificationEvent, FALSE);
      if (!NT_SUCCESS (status))
	{
	  if (status != STATUS_OBJECT_NAME_COLLISION)
	    api_fatal ("NtCreateEvent(lock): %y", status);
	  /* If we get a STATUS_OBJECT_NAME_COLLISION, the event still exists
	     because some other process is waiting for it in lf_setlock.
	     If so, check the event's signal state.  If we can't open it, it
	     has been closed in the meantime, so just try again.  If we can
	     open it and the object is not signalled, it's surely a bug in the
	     code somewhere.  Otherwise, close the event and retry to create
	     a new event with another name. */
	  if (open_lock_obj ())
	    {
	      if (!IsEventSignalled (lf_obj))
		api_fatal ("NtCreateEvent(lock): %y", status);
	      close_lock_obj ();
	      /* Increment the lf_ver field until we have no collision. */
	      ++lf_ver;
	    }
	}
    }
  while (!NT_SUCCESS (status));
  /* For BSD locks, notify the parent process. */
  if (lf_flags & F_FLOCK)
    {
      HANDLE parent_proc, parent_thread, parent_lf_obj;

      pinfo p (myself->ppid);
      if (!p)	/* No access or not a Cygwin parent. */
	return;

      parent_proc = OpenProcess (PROCESS_DUP_HANDLE
				 | PROCESS_CREATE_THREAD
				 | PROCESS_QUERY_INFORMATION
				 | PROCESS_VM_OPERATION
				 | PROCESS_VM_WRITE
				 | PROCESS_VM_READ,
				 FALSE, p->dwProcessId);
      if (!parent_proc)
	{
	  debug_printf ("OpenProcess (%u): %E", p->dwProcessId);
	  return;
	}
      if (!DuplicateHandle (GetCurrentProcess (), lf_obj, parent_proc,
			    &parent_lf_obj, TRUE, FALSE, DUPLICATE_SAME_ACCESS))
	debug_printf ("DuplicateHandle (lf_obj): %E");
      else
	{
	  parent_thread = CreateRemoteThread (parent_proc, NULL, 256 * 1024,
					      create_lock_in_parent,
					      parent_lf_obj,
					      STACK_SIZE_PARAM_IS_A_RESERVATION,
					      NULL);
	  if (!parent_thread)
	    {
	      debug_printf ("CreateRemoteThread: %E");
	      /* If thread didn't get started, close object handle in parent,
		 otherwise suffer handle leaks. */
	      DuplicateHandle (parent_proc, parent_lf_obj, parent_proc,
			       NULL, 0, FALSE, DUPLICATE_CLOSE_SOURCE);
	    }
	  else
	    {
	      /* Must wait to avoid race conditions. */
	      WaitForSingleObject (parent_thread, INFINITE);
	      CloseHandle (parent_thread);
	    }
	}
      CloseHandle (parent_proc);
    }
}

/* Open a lock event object for SYNCHRONIZE access (to wait for it). */
bool
lockf_t::open_lock_obj ()
{
  lockfattr_t attr;
  NTSTATUS status;

  status = NtOpenEvent (&lf_obj, FLOCK_EVENT_ACCESS,
			create_lock_obj_attr (&attr, 0, alloca (SD_MIN_SIZE)));
  if (!NT_SUCCESS (status))
    {
      SetLastError (RtlNtStatusToDosError (status));
      lf_obj = NULL; /* Paranoia... */
    }
  return lf_obj != NULL;
}

/* Delete a lock event handle.  The important thing here is to signal it
   before closing the handle.  This way all threads waiting for this lock
   can wake up. */
void
lockf_t::del_lock_obj (HANDLE fhdl, bool signal)
{
  if (lf_obj)
    {
      /* Only signal the event if it's either a POSIX lock, or, in case of
	 BSD flock locks, if it's an explicit unlock or if the calling fhandler
	 holds the last reference to the file table entry.  The file table
	 entry in UNIX terms is equivalent to the FILE_OBJECT in Windows NT
	 terms.  It's what the handle/descriptor references when calling
	 CreateFile/open.  Calling DuplicateHandle/dup only creates a new
	 handle/descriptor to the same FILE_OBJECT/file table entry. */
      if ((lf_flags & F_POSIX) || signal
	  || (fhdl && get_obj_handle_count (fhdl) <= 1))
	{
	  NTSTATUS status = NtSetEvent (lf_obj, NULL);
	  if (!NT_SUCCESS (status))
	    system_printf ("NtSetEvent, %y", status);
	  /* For BSD locks, notify the parent process. */
	  if (lf_flags & F_FLOCK)
	    {
	      HANDLE parent_proc, parent_thread;

	      pinfo p (myself->ppid);
	      if (p && (parent_proc = OpenProcess (PROCESS_CREATE_THREAD
					     | PROCESS_QUERY_INFORMATION
					     | PROCESS_VM_OPERATION
					     | PROCESS_VM_WRITE
					     | PROCESS_VM_READ,
					     FALSE, p->dwProcessId)))
		{
		  parent_thread = CreateRemoteThread (parent_proc, NULL,
					      256 * 1024, delete_lock_in_parent,
					      NULL,
					      STACK_SIZE_PARAM_IS_A_RESERVATION,
					      NULL);
		  if (parent_thread)
		    {
		      /* Must wait to avoid race conditions. */
		      WaitForSingleObject (parent_thread, INFINITE);
		      CloseHandle (parent_thread);
		    }
		  CloseHandle (parent_proc);
		}
	    }
	}
      close_lock_obj ();
    }
}

lockf_t::~lockf_t ()
{
  del_lock_obj (NULL);
}

/*
 * This variable controls the maximum number of processes that will
 * be checked in doing deadlock detection.
 */
#ifndef __CYGWIN__
#define MAXDEPTH 50
static int maxlockdepth = MAXDEPTH;
#endif

#define NOLOCKF (struct lockf_t *)0
#define SELF    0x1
#define OTHERS  0x2
static int      lf_clearlock (lockf_t *, lockf_t **, HANDLE);
static int      lf_findoverlap (lockf_t *, lockf_t *, int, lockf_t ***, lockf_t **);
static lockf_t *lf_getblock (lockf_t *, inode_t *node);
static int      lf_getlock (lockf_t *, inode_t *, struct flock *);
static int      lf_setlock (lockf_t *, inode_t *, lockf_t **, HANDLE);
static void     lf_split (lockf_t *, lockf_t *, lockf_t **);
static void     lf_wakelock (lockf_t *, HANDLE);

/* This is the fcntl advisory lock implementation.  For the implementation
   of mandatory locks using the Windows mandatory locking functions, see the
   fhandler_disk_file::mand_lock method at the end of this file. */
int
fhandler_base::lock (int a_op, struct flock *fl)
{
  off_t start, end, oadd;
  int error = 0;

  short a_flags = fl->l_type & (F_POSIX | F_FLOCK);
  short type = fl->l_type & (F_RDLCK | F_WRLCK | F_UNLCK);

  if (!a_flags)
    a_flags = F_POSIX; /* default */

  /* FIXME: For BSD flock(2) we need a valid, per file table entry OS handle.
     Therefore we can't allow using flock(2) on nohandle devices and
     pre-Windows 8 console handles (recognized by their odd handle value). */
  if ((a_flags & F_FLOCK)
      && (nohandle () || (((uintptr_t) get_handle () & 0x3) == 0x3)))
    {
      set_errno (EINVAL);
      debug_printf ("BSD locking on nohandle and old-style console devices "
		    "not supported");
      return -1;
    }

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
	/* flock semantics don't specify a requirement that the file has
	   been opened with a specific open mode, in contrast to POSIX locks
	   which require that a file is opened for reading to place a read
	   lock and opened for writing to place a write lock. */
	/* CV 2013-10-22: Test POSIX R/W mode flags rather than Windows R/W
	   access flags.  The reason is that POSIX mode flags are set for
	   all types of fhandlers, while Windows access flags are only set
	   for most of the actual Windows device backed fhandlers. */
	if ((a_flags & F_POSIX)
	    && ((get_flags () & O_ACCMODE) == O_WRONLY))
	  {
	    debug_printf ("request F_RDLCK on O_WRONLY file: EBADF");
	    set_errno (EBADF);
	    return -1;
	  }
	break;
      case F_WRLCK:
	/* See above comment. */
	if ((a_flags & F_POSIX)
	    && ((get_flags () & O_ACCMODE) == O_RDONLY))
	  {
	    debug_printf ("request F_WRLCK on O_RDONLY file: EBADF");
	    set_errno (EBADF);
	    return -1;
	  }
	break;
      default:
	set_errno (EINVAL);
	return -1;
      }

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
	start = 0;
      break;

    case SEEK_END:
      if (get_device () != FH_FS)
	start = 0;
      else
	{
	  NTSTATUS status;
	  IO_STATUS_BLOCK io;
	  FILE_STANDARD_INFORMATION fsi;

	  status = NtQueryInformationFile (get_handle (), &io, &fsi, sizeof fsi,
					   FileStandardInformation);
	  if (!NT_SUCCESS (status))
	    {
	      __seterrno_from_nt_status (status);
	      return -1;
	    }
	  if (fl->l_start > 0 && fsi.EndOfFile.QuadPart > OFF_MAX - fl->l_start)
	    {
	      set_errno (EOVERFLOW);
	      return -1;
	    }
	  start = fsi.EndOfFile.QuadPart + fl->l_start;
	}
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

restart:	/* Entry point after a restartable signal came in. */

  inode_t *node = inode_t::get (get_dev (), get_ino (), true, true);
  if (!node)
    {
      set_errno (ENOLCK);
      return -1;
    }

  /* Unlock the fd table which has been locked in fcntl_worker/lock_worker,
     otherwise a blocking F_SETLKW never wakes up on a signal. */
  cygheap->fdtab.unlock ();

  lockf_t **head = &node->i_lockf;

#if 0
  /*
   * Avoid the common case of unlocking when inode_t has no locks.
   *
   * This shortcut is invalid for Cygwin because the above inode_t::get
   * call returns with an empty lock list if this process has no locks
   * on the file yet.
   */
  if (*head == NULL)
    {
      if (a_op != F_SETLK)
	{
	  node->UNLOCK ();
	  fl->l_type = F_UNLCK;
	  return 0;
	}
    }
#endif
  /*
   * Allocate a spare structure in case we have to split.
   */
  lockf_t *clean = NULL;
  if (a_op == F_SETLK || a_op == F_UNLCK)
    {
      clean = new lockf_t ();
      if (!clean)
	{
	  node->unlock_and_remove_if_unused ();
	  set_errno (ENOLCK);
	  return -1;
	}
    }
  /*
   * Create the lockf_t structure
   */
  lockf_t *lock = new lockf_t (node, head, a_flags, type, start, end,
			       (a_flags & F_FLOCK) ? get_unique_id ()
						   : getpid (),
			       myself->dwProcessId, 0);
  if (!lock)
    {
      node->unlock_and_remove_if_unused ();
      set_errno (ENOLCK);
      return -1;
    }

  switch (a_op)
    {
    case F_SETLK:
      error = lf_setlock (lock, node, &clean, get_handle ());
      break;

    case F_UNLCK:
      error = lf_clearlock (lock, &clean, get_handle ());
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
      lockf_t *n = lock->lf_next;
      lock->del_lock_obj (get_handle (), a_op == F_UNLCK);
      delete lock;
      lock = n;
    }
  node->unlock_and_remove_if_unused ();
  switch (error)
    {
    case 0:		/* All is well. */
      need_fork_fixup (true);
      return 0;
    case EINTR:		/* Signal came in. */
      if (_my_tls.call_signal_handler ())
	goto restart;
      break;
    case ECANCELED:	/* The thread has been sent a cancellation request. */
      pthread::static_cancel_self ();
      /*NOTREACHED*/
    default:
      break;
    }
  set_errno (error);
  return -1;
}

/*
 * Set a byte-range lock.
 */
static int
lf_setlock (lockf_t *lock, inode_t *node, lockf_t **clean, HANDLE fhdl)
{
  lockf_t *block;
  lockf_t **head = lock->lf_head;
  lockf_t **prev, *overlap;
  int ovcase, priority, old_prio, needtolink;
  tmp_pathbuf tp;

  /*
   * Set the priority
   */
  priority = old_prio = GetThreadPriority (GetCurrentThread ());
  if (lock->lf_type == F_WRLCK && priority <= THREAD_PRIORITY_ABOVE_NORMAL)
    priority = THREAD_PRIORITY_HIGHEST;
  /*
   * Scan lock list for this file looking for locks that would block us.
   */
  /* Create temporary space for the all locks list. */
  node->i_all_lf = (lockf_t *) (void *) tp.w_get ();
  while ((block = lf_getblock(lock, node)))
    {
      HANDLE obj = block->lf_obj;
      block->lf_obj = NULL;

      /*
       * Free the structure and return if nonblocking.
       */
      if ((lock->lf_flags & F_WAIT) == 0)
	{
	  NtClose (obj);
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
      if (lock->lf_flags & F_POSIX)
	for (lockf_t *lk = node->i_lockf; lk; lk = lk->lf_next)
	  if ((lk->lf_flags & F_POSIX) && get_obj_handle_count (lk->lf_obj) > 1)
	    {
	      NtClose (obj);
	      return EDEADLK;
	    }

      /*
       * For flock type locks, we must first remove
       * any shared locks that we hold before we sleep
       * waiting for an exclusive lock.
       */
      if ((lock->lf_flags & F_FLOCK) && lock->lf_type == F_WRLCK)
	{
	  lock->lf_type = F_UNLCK;
	  (void) lf_clearlock (lock, clean, fhdl);
	  lock->lf_type = F_WRLCK;
	}

      /*
       * Add our lock to the blocked list and sleep until we're free.
       * Remember who blocked us (for deadlock detection).
       */
      /* Cygwin:  No locked list.  See deadlock recognition above. */

      node->UNLOCK ();

      /* Create list of objects to wait for. */
      HANDLE w4[4] = { obj, NULL, NULL, NULL };
      DWORD wait_count = 1;

      DWORD timeout;
      HANDLE proc = NULL;
      if (lock->lf_flags & F_POSIX)
	{
	  proc = OpenProcess (SYNCHRONIZE, FALSE, block->lf_wid);
	  if (!proc)
	    timeout = 0L;
	  else
	    {
	      w4[wait_count++] = proc;
	      timeout = INFINITE;
	    }
	}
      else
	timeout = 100L;

      DWORD WAIT_SIGNAL_ARRIVED = WAIT_OBJECT_0 + wait_count;
      wait_signal_arrived here (w4[wait_count++]);

      DWORD WAIT_THREAD_CANCELED = WAIT_TIMEOUT + 1;
      HANDLE cancel_event = pthread::get_cancel_event ();
      if (cancel_event)
	{
	  WAIT_THREAD_CANCELED = WAIT_OBJECT_0 + wait_count;
	  w4[wait_count++] = cancel_event;
	}

      /* Wait for the blocking object and, for POSIX locks, its holding process.
	 Unfortunately, since BSD flock locks are not attached to a specific
	 process, we can't recognize an abandoned lock by sync'ing with the
	 creator process.  We have to make sure the event object is in a
	 signalled state, or that it has gone away.  The latter we can only
	 recognize by retrying to fetch the block list, so we must not wait
	 infinitely.  For POSIX locks, if the process has already exited,
	 just check if a signal or a thread cancel request arrived. */
      SetThreadPriority (GetCurrentThread (), priority);
      DWORD ret = WaitForMultipleObjects (wait_count, w4, FALSE, timeout);
      SetThreadPriority (GetCurrentThread (), old_prio);
      if (proc)
	CloseHandle (proc);
      node->LOCK ();
      /* Never close lock object handle outside of node lock! */
      NtClose (obj);
      if (ret == WAIT_SIGNAL_ARRIVED)
	{
	  /* A signal came in. */
	  lock->lf_next = *clean;
	  *clean = lock;
	  return EINTR;
	}
      else if (ret == WAIT_THREAD_CANCELED)
	{
	  /* The thread has been sent a cancellation request. */
	  lock->lf_next = *clean;
	  *clean = lock;
	  return ECANCELED;
	}
      else
	/* The lock object has been set to signalled or ...
	   for POSIX locks, the process holding the lock has exited, or ...
	   just a timeout.  Just retry. */
	continue;
    }
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
	  lf_wakelock (overlap, fhdl);
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
	  lf_wakelock (overlap, fhdl);
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
	  lf_wakelock (overlap, fhdl);
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
	  lf_wakelock (overlap, fhdl);
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
	  lf_wakelock (overlap, fhdl);
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
lf_clearlock (lockf_t *unlock, lockf_t **clean, HANDLE fhdl)
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
      lf_wakelock (overlap, fhdl);

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
lf_getlock (lockf_t *lock, inode_t *node, struct flock *fl)
{
  lockf_t *block;
  tmp_pathbuf tp;

  /* Create temporary space for the all locks list. */
  node->i_all_lf = (lockf_t *) (void * ) tp.w_get ();
  if ((block = lf_getblock (lock, node)))
    {
      if (block->lf_obj)
	block->close_lock_obj ();
      fl->l_type = block->lf_type;
      fl->l_whence = SEEK_SET;
      fl->l_start = block->lf_start;
      if (block->lf_end == -1)
	fl->l_len = 0;
      else
	fl->l_len = block->lf_end - block->lf_start + 1;
      if (block->lf_flags & F_POSIX)
	fl->l_pid = (pid_t) block->lf_id;
      else
	fl->l_pid = -1;
    }
  else
    fl->l_type = F_UNLCK;
  return 0;
}

/*
 * Walk the list of locks for an inode_t and
 * return the first blocking lock.
 */
static lockf_t *
lf_getblock (lockf_t *lock, inode_t *node)
{
  lockf_t **prev, *overlap;
  lockf_t *lf = node->get_all_locks_list ();
  int ovcase;

  prev = lock->lf_head;
  while ((ovcase = lf_findoverlap (lf, lock, OTHERS, &prev, &overlap)))
    {
      /*
       * We've found an overlap, see if it blocks us
       */
      if ((lock->lf_type == F_WRLCK || overlap->lf_type == F_WRLCK))
	{
	  /* Open the event object for synchronization. */
	  if (overlap->open_lock_obj ())
	    {
	      /* Check if the event object is signalled.  If so, the overlap
		 doesn't actually exist anymore.  There are just a few open
		 handles left. */
	      if (!IsEventSignalled (overlap->lf_obj))
		return overlap;
	      overlap->close_lock_obj ();
	    }
	}
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
  off_t start, end;

  *overlap = lf;
  if (lf == NOLOCKF)
    return 0;

  start = lock->lf_start;
  end = lock->lf_end;
  while (lf != NOLOCKF)
    {
      if (((type & SELF) && lf->lf_id != lock->lf_id)
	  || ((type & OTHERS) && lf->lf_id == lock->lf_id)
	  /* As on Linux: POSIX locks and BSD flock locks don't interact. */
	  || (lf->lf_flags & (F_POSIX | F_FLOCK))
	     != (lock->lf_flags & (F_POSIX | F_FLOCK)))
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
  memcpy ((void *) splitlock, lock1, sizeof *splitlock);
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
lf_wakelock (lockf_t *listhead, HANDLE fhdl)
{
  listhead->del_lock_obj (fhdl, true);
}

extern "C" int
flock (int fd, int operation)
{
  int res = -1;
  int cmd;
  struct flock fl = { 0, SEEK_SET, 0, 0, 0 };

  __try
    {
      cygheap_fdget cfd (fd);
      if (cfd < 0)
	__leave;

      cmd = (operation & LOCK_NB) ? F_SETLK : F_SETLKW;
      switch (operation & (~LOCK_NB))
	{
	case LOCK_EX:
	  fl.l_type = F_WRLCK;
	  break;
	case LOCK_SH:
	  fl.l_type = F_RDLCK;
	  break;
	case LOCK_UN:
	  fl.l_type = F_UNLCK;
	  break;
	default:
	  set_errno (EINVAL);
	  __leave;
	}
      if (!cfd->mandatory_locking ())
	fl.l_type |= F_FLOCK;
      res = cfd->mandatory_locking () ? cfd->mand_lock (cmd, &fl)
				      : cfd->lock (cmd, &fl);
      if ((res == -1) && ((get_errno () == EAGAIN) || (get_errno () == EACCES)))
	set_errno (EWOULDBLOCK);
    }
  __except (EFAULT) {}
  __endtry
  syscall_printf ("%R = flock(%d, %d)", res, fd, operation);
  return res;
}

extern "C" int
lockf (int filedes, int function, off_t size)
{
  int res = -1;
  int cmd;
  struct flock fl;

  pthread_testcancel ();

  __try
    {
      cygheap_fdget cfd (filedes);
      if (cfd < 0)
	__leave;

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
	  if (cfd->lock (F_GETLK, &fl) == -1)
	    __leave;
	  if (fl.l_type == F_UNLCK || fl.l_pid == getpid ())
	    res = 0;
	  else
	    errno = EAGAIN;
	  __leave;
	  /* NOTREACHED */
	default:
	  errno = EINVAL;
	  __leave;
	  /* NOTREACHED */
	}
      res = cfd->mandatory_locking () ? cfd->mand_lock (cmd, &fl)
				      : cfd->lock (cmd, &fl);
    }
  __except (EFAULT) {}
  __endtry
  syscall_printf ("%R = lockf(%d, %d, %D)", res, filedes, function, size);
  return res;
}

/* This is the fcntl lock implementation for mandatory locks using the
   Windows mandatory locking functions.  For the UNIX-like advisory locking
   implementation see the fhandler_disk_file::lock method earlier in this
   file. */
struct lock_parms {
  HANDLE	   h;
  PIO_STATUS_BLOCK pio;
  PLARGE_INTEGER   poff;
  PLARGE_INTEGER   plen;
  BOOL		   type;
  NTSTATUS	   status;
};

static DWORD WINAPI
blocking_lock_thr (LPVOID param)
{
  struct lock_parms *lp = (struct lock_parms *) param;
  lp->status = NtLockFile (lp->h, NULL, NULL, NULL, lp->pio, lp->poff,
			   lp->plen, 0, FALSE, lp->type);
  return 0;
}

int
fhandler_base::mand_lock (int, struct flock *)
{
  set_errno (EINVAL);
  return -1;
}

int
fhandler_disk_file::mand_lock (int a_op, struct flock *fl)
{
  NTSTATUS status;
  IO_STATUS_BLOCK io;
  FILE_POSITION_INFORMATION fpi;
  FILE_STANDARD_INFORMATION fsi;
  off_t startpos;
  LARGE_INTEGER offset;
  LARGE_INTEGER length;

  /* Calculate where to start from, then adjust this by fl->l_start. */
  switch (fl->l_whence)
  {
    case SEEK_SET:
      startpos = 0;
      break;
    case SEEK_CUR:
      status = NtQueryInformationFile (get_handle (), &io, &fpi, sizeof fpi,
				       FilePositionInformation);
      if (!NT_SUCCESS (status))
	{
	  __seterrno_from_nt_status (status);
	  return -1;
	}
      startpos = fpi.CurrentByteOffset.QuadPart;
      break;
    case SEEK_END:
      status = NtQueryInformationFile (get_handle (), &io, &fsi, sizeof fsi,
				       FileStandardInformation);
      if (!NT_SUCCESS (status))
	{
	  __seterrno_from_nt_status (status);
	  return -1;
	}
      startpos = fsi.EndOfFile.QuadPart;
      break;
    default:
      set_errno (EINVAL);
      return -1;
  }
  /* Adjust start and length until they make sense. */
  offset.QuadPart = startpos + fl->l_start;
  if (fl->l_len < 0)
    {
      offset.QuadPart -= fl->l_len;
      length.QuadPart = -fl->l_len;
    }
  else
    length.QuadPart = fl->l_len;
  if (offset.QuadPart < 0)
    {
      length.QuadPart -= -offset.QuadPart;
      if (length.QuadPart <= 0)
        {
          set_errno (EINVAL);
          return -1;
        }
      offset.QuadPart = 0;
    }
  /* Special case if len == 0.  For POSIX this means lock to the end of
     the entire file, even when file grows later. */
  if (length.QuadPart == 0)
    length.QuadPart = UINT64_MAX;
  /* Action! */
  if (fl->l_type == F_UNLCK)
    {
      status = NtUnlockFile (get_handle (), &io, &offset, &length, 0);
      if (status == STATUS_RANGE_NOT_LOCKED)	/* Not an error */
	status = STATUS_SUCCESS;
    }
  else if (a_op == F_SETLKW)
    {
      /* We open file handles synchronously.  To allow asynchronous operation
	 the file locking functions require a file handle opened in asynchronous
	 mode.  Since Windows locks are per-process/per-file object, we can't
	 open another handle asynchrously and lock/unlock using that handle:
	 The original file handle would not have placed the lock and would be
	 restricted by the lock like any other file handle.
	 So, what we do here is to start a thread which calls the potentially
	 blocking NtLockFile call.  Then we wait for thread completion in an
	 interruptible fashion. */
      OBJECT_ATTRIBUTES attr;
      HANDLE evt;
      struct lock_parms lp = { get_handle (), &io, &offset, &length,
			       fl->l_type == F_WRLCK, 0 };
      cygthread *thr = NULL;

      InitializeObjectAttributes (&attr, NULL, 0, NULL, NULL);
      status = NtCreateEvent (&evt, EVENT_ALL_ACCESS, &attr,
			      NotificationEvent, FALSE);
      if (evt)
	thr = new cygthread (blocking_lock_thr, &lp, "blk_lock", evt);
      if (!thr)
	{
	  /* Thread creation failed.  Fall back to blocking lock call. */
	  if (evt)
	    NtClose (evt);
	  status = NtLockFile (get_handle (), NULL, NULL, NULL, &io, &offset,
			       &length, 0, FALSE, fl->l_type == F_WRLCK);
	}
      else
	{
	  /* F_SETLKW and lock cannot be established.  Wait until the lock can
	     be established, or a signal request arrived.  We deliberately
	     don't handle thread cancel requests here. */
	  DWORD wait_res = cygwait (evt, INFINITE, cw_sig | cw_sig_eintr);
	  NtClose (evt);
	  switch (wait_res)
	    {
	    case WAIT_OBJECT_0:
	      /* Fetch completion status. */
	      status = lp.status;
	      thr->detach ();
	      break;
	    default:
	      /* Signal arrived.
		 If CancelSynchronousIo works we wait for the thread to exit.
		 lp.status will be either STATUS_SUCCESS, or STATUS_CANCELLED.
		 We only call NtUnlockFile in the first case.
		 If CancelSynchronousIo fails we terminated the thread and
		 call NtUnlockFile since lp.status was 0 to begin with. */
	      if (CancelSynchronousIo (thr->thread_handle ()))
		thr->detach ();
	      else
		thr->terminate_thread ();
	      if (NT_SUCCESS (lp.status))
		NtUnlockFile (get_handle (), &io, &offset, &length, 0);
	      /* Per SUSv4: If a signal is received while fcntl is waiting,
		 fcntl shall be interrupted.  Upon return from the signal
		 handler, fcntl shall return -1 with errno set to EINTR,
		 and the lock operation shall not be done. */
	      _my_tls.call_signal_handler ();
	      set_errno (EINTR);
	      return -1;
	    }
	}
    }
  else
    {
      status = NtLockFile (get_handle (), NULL, NULL, NULL, &io, &offset,
			   &length, 0, TRUE, fl->l_type == F_WRLCK);
      if (a_op == F_GETLK)
	{
	  /* This is non-atomic, but there's no other way on Windows to detect
	     if another lock is blocking our lock, other than trying to place
	     the lock, and then having to unlock it again. */
	  if (NT_SUCCESS (status))
	    {
	      NtUnlockFile (get_handle (), &io, &offset, &length, 0);
	      fl->l_type = F_UNLCK;
	    }
	  else
	    {
	      /* FAKE! FAKE! FAKE! */
	      fl->l_type = F_WRLCK;
	      fl->l_whence = SEEK_SET;
	      fl->l_start = offset.QuadPart;
	      fl->l_len = length.QuadPart;
	      fl->l_pid = (pid_t) -1;
	    }
	  status = STATUS_SUCCESS;
	}
    }
  if (!NT_SUCCESS (status))
    {
      __seterrno_from_nt_status (status);
      return -1;
    }
  return 0;
}
