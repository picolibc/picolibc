/* shm.cc: Single unix specification IPC interface for Cygwin

   Copyright 2001, 2002 Red Hat, Inc.

   Originally written by Robert Collins <robert.collins@hotmail.com>

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <sys/stat.h>
#include <assert.h>
#include <errno.h>
#include "cygerrno.h"
#include <limits.h>
#include <unistd.h>
#include "security.h"
#include "fhandler.h"
#include "path.h"
#include "dtable.h"
#include "cygheap.h"
#include "thread.h"
#include "cygwin_shm.h"
#include "cygserver_shm.h"

/*
 * FIXME: These must be defined somewhere? i.e. get the high and low
 * double words of a quad word?  They are required here to allow keys
 * to be printed out via XXX_printf (i.e. small_printf()).  Note that
 * even if the key is only 32 bits (the old default), these routine
 * should still give sensible results, i.e. hi_ulong() => 0.
 */

inline long int hi_ulong (const key_t key)
{
  return (unsigned long)
    (((unsigned long long) key >> 32) & ULONG_MAX);
}

inline long int lo_ulong (const key_t key)
{
  return (unsigned long)
    ((unsigned long long) key & ULONG_MAX);
}

// FIXME IS THIS CORRECT
/* Implementation notes: We use two shared memory regions per key:
 * One for the control structure, and one for the shared memory.
 * While this has a higher overhead tham a single shared area,
 * It allows more flexability. As the entire code is transparent to the user
 * We can merge these in the future should it be needed.
 */
extern "C" size_t
getsystemallocgranularity ()
{
  SYSTEM_INFO sysinfo;
  static size_t buffer_offset = 0;
  if (buffer_offset)
    return buffer_offset;
  GetSystemInfo (&sysinfo);
  buffer_offset = sysinfo.dwAllocationGranularity;
  return buffer_offset;
}

/*
 * Used for: SHM_ATTACH, SHM_DETACH, SHM_REATTACH, and SHM_DEL.
 */
client_request_shm::client_request_shm (int ntype, int nshmid)
  : client_request (CYGSERVER_REQUEST_SHM, &parameters, sizeof (parameters))
{
  assert (ntype == SHM_REATTACH			\
	  || ntype == SHM_ATTACH		\
	  || ntype == SHM_DETACH		\
	  || ntype == SHM_DEL);

  parameters.in.type = ntype;
  parameters.in.cygpid = getpid ();
  parameters.in.winpid = GetCurrentProcessId ();
  parameters.in.shm_id = nshmid;

  assert (parameters.in.cygpid > 0);
  assert (parameters.in.winpid != 0);

  msglen (sizeof (parameters.in) - sizeof (parameters.in.sd_buf));

  syscall_printf ("created: type = %d, shmid = %d",
		  parameters.in.type, parameters.in.shm_id);
}

/*
 * Used for: SHM_CREATE.
 */
client_request_shm::client_request_shm (key_t nkey, size_t nsize, int nshmflg)
  : client_request (CYGSERVER_REQUEST_SHM, &parameters, sizeof (parameters))
{
  assert (nkey != (key_t) -1);
  assert (nsize >= 0);

  parameters.in.type = SHM_CREATE;
  parameters.in.cygpid = getpid ();
  parameters.in.winpid = GetCurrentProcessId ();
  parameters.in.key = nkey;
  parameters.in.size = nsize;
  parameters.in.shmflg = nshmflg;

  assert (parameters.in.cygpid > 0);
  assert (parameters.in.winpid != 0);

  DWORD sd_buf_size = 0;

  if (wincap.has_security ())
    {
      SECURITY_ATTRIBUTES sa = sec_none;
      sd_buf_size = sizeof (parameters.in.sd_buf);

      PSECURITY_DESCRIPTOR psd = (PSECURITY_DESCRIPTOR) parameters.in.sd_buf;

      InitializeSecurityDescriptor (psd, SECURITY_DESCRIPTOR_REVISION);

      alloc_sd (geteuid32 (), getegid32 (), parameters.in.shmflg & 0777,
		psd, &sd_buf_size);
    }

  msglen (sizeof (parameters.in) - sizeof (parameters.in.sd_buf)
	  + sd_buf_size);

  syscall_printf (("created: type = %d, "
		   "key = 0x%x%08x, size = %ld, shmflg = %o"),
		  parameters.in.type,
		  hi_ulong(parameters.in.key), lo_ulong(parameters.in.key),
		  parameters.in.size, parameters.in.shmflg);
}

static shmnode *shm_head = NULL;

static shmnode *
build_inprocess_shmds (HANDLE hfilemap, HANDLE hattachmap, key_t key,
		       int shm_id)
{
  HANDLE filemap = hfilemap;
  void *mapptr = MapViewOfFile (filemap, FILE_MAP_WRITE, 0, 0, 0);

  if (!mapptr)
    {
      CloseHandle (hfilemap);
      CloseHandle (hattachmap);
      //FIXME: close filemap and free the mutex
      /* we couldn't access the mapped area with the requested permissions */
      set_errno (EACCES);
      return NULL;
    }

  /* Now get the user data */
  HANDLE attachmap = hattachmap;
  int_shmid_ds *shmtemp = new int_shmid_ds;
  if (!shmtemp)
    {
      system_printf ("failed to malloc shm node");
      set_errno (ENOMEM);
      UnmapViewOfFile (mapptr);
      CloseHandle (filemap);
      CloseHandle (attachmap);
      /* exit mutex */
      return NULL;
    }

  /* get the system node data */
  shmtemp->ds = *(shmid_ds *) mapptr;

  /* process local data */
  shmnode *tempnode = new shmnode;

  tempnode->filemap = filemap;
  tempnode->attachmap = attachmap;
  shmtemp->mapptr = mapptr;

  /* no need for InterlockedExchange here, we're serialised by the global mutex */
  tempnode->shmds = shmtemp;
  tempnode->shm_id = shm_id;
  tempnode->key = key;
  tempnode->next = shm_head;
  tempnode->attachhead = NULL;
  shm_head = tempnode;

  /* FIXME: leave the system wide shm mutex */

  return tempnode;
}

static void
delete_inprocess_shmds (shmnode **nodeptr)
{
  shmnode *node = *nodeptr;

  // remove from the list
  if (node == shm_head)
    shm_head = shm_head->next;
  else
    {
      shmnode *tempnode = shm_head;
      while (tempnode && tempnode->next != node)
	tempnode = tempnode->next;
      if (tempnode)
	tempnode->next = node->next;
      // else log the unexpected !
    }

  // release the shared data view
  UnmapViewOfFile (node->shmds);
  CloseHandle (node->filemap);
  CloseHandle (node->attachmap);

  // free the memory
  delete node;
  nodeptr = NULL;
}

int __stdcall
fixup_shms_after_fork ()
{
  shmnode *tempnode = shm_head;
  while (tempnode)
    {
      void *newshmds =
	MapViewOfFile (tempnode->filemap, FILE_MAP_WRITE, 0, 0, 0);
      if (!newshmds)
	{
	  /* don't worry about handle cleanup, we're dying! */
	  system_printf ("failed to reattach to shm control file view %x",
			 tempnode);
	  return 1;
	}
      tempnode->shmds->ds = *(shmid_ds *) newshmds;
      tempnode->shmds->mapptr = newshmds;
      _shmattach *attachnode = tempnode->attachhead;
      while (attachnode)
	{
	  void *newdata = MapViewOfFileEx (tempnode->attachmap,
					   (attachnode->shmflg & SHM_RDONLY) ?
					   FILE_MAP_READ : FILE_MAP_WRITE, 0,
					   0, 0, attachnode->data);
	  if (newdata != attachnode->data)
	    {
	      /* don't worry about handle cleanup, we're dying! */
	      system_printf ("failed to reattach to mapped file view %x",
			     attachnode->data);
	      return 1;
	    }
	  attachnode = attachnode->next;
	}
      tempnode = tempnode->next;
    }
  return 0;
}

/* this is ugly. Yes, I know that.
 * FIXME: abstract the lookup functionality,
 * So that it can be an array, list, whatever without us being worried
 */

/* FIXME: after fork, every memory area needs to have the attach count
 * incremented. This should be done in the server?
 */

/* FIXME: tell the daemon when we attach, so at process close it can clean up
 * the attach count
 */
extern "C" void *
shmat (int shmid, const void *shmaddr, int shmflg)
{
  shmnode *tempnode = shm_head;
  while (tempnode && tempnode->shm_id != shmid)
    tempnode = tempnode->next;

  if (!tempnode)
    {
      /* couldn't find a currently open shm control area for the key - probably because
       * shmget hasn't been called.
       * Allocate a new control block - this has to be handled by the daemon */
      client_request_shm req (SHM_REATTACH, shmid);

      if (req.make_request () == -1)
	{
	  set_errno (ENOSYS);	/* daemon communication failed */
	  return (void *) -1;
	}

      if (req.error_code ())	/* shm_get failed in the daemon */
	{
	  set_errno (req.error_code ());
	  return (void *) -1;
	}

      /* we've got the id, now we open the memory area ourselves.
       * This tests security automagically
       * FIXME: make this a method of shmnode ?
       */
      tempnode =
	build_inprocess_shmds (req.parameters.out.filemap,
			       req.parameters.out.attachmap,
			       req.parameters.out.key,
			       req.parameters.out.shm_id);
      if (!tempnode)
	return (void *) -1;

    }

  // class shmid_ds *shm = tempnode->shmds;

  if (shmaddr)
    {
      //FIXME: requested base address ?! (Don't forget to fix the fixup_after_fork too)
      set_errno (EINVAL);
      return (void *) -1;
    }

  void *rv = MapViewOfFile (tempnode->attachmap,
			    (shmflg & SHM_RDONLY) ? FILE_MAP_READ :
			    FILE_MAP_WRITE, 0, 0, 0);

  if (!rv)
    {
      //FIXME: translate GetLastError()
      set_errno (EACCES);
      return (void *) -1;
    }
  /* tell the daemon we have attached */
  client_request_shm req (SHM_ATTACH, shmid);

  if (req.make_request () == -1)
    {
      debug_printf ("failed to tell daemon that we have attached");
    }

  _shmattach *attachnode = new _shmattach;
  attachnode->data = rv;
  attachnode->shmflg = shmflg;
  attachnode->next =
    (_shmattach *) InterlockedExchangePointer (&tempnode->attachhead,
					       attachnode);


  return rv;
}

/* FIXME: tell the daemon when we detach so it doesn't cleanup incorrectly.
 */
extern "C" int
shmdt (const void *shmaddr)
{
  /* this should be "rare" so a hefty search is ok. If this is common, then we
   * should alter the data structs to allow more optimisation
   */
  shmnode *tempnode = shm_head;
  _shmattach *attachnode;
  while (tempnode)
    {
      // FIXME: Race potential
      attachnode = tempnode->attachhead;
      while (attachnode && attachnode->data != shmaddr)
	attachnode = attachnode->next;
      if (attachnode)
	break;
      tempnode = tempnode->next;
    }
  if (!tempnode)
    {
      // dt cannot be called by an app that hasn't alreadu at'd
      set_errno (EINVAL);
      return -1;
    }

  UnmapViewOfFile (attachnode->data);
  /* tell the daemon we have attached */
  client_request_shm req (SHM_DETACH, tempnode->shm_id);

  if (req.make_request () == -1)
    {
      debug_printf ("failed to tell daemon that we have detached");
    }

  return 0;
}

//FIXME: who is allowed to perform STAT?
extern "C" int
shmctl (int shmid, int cmd, struct shmid_ds *buf)
{
  shmnode *tempnode = shm_head;
  while (tempnode && tempnode->shm_id != shmid)
    tempnode = tempnode->next;
  if (!tempnode)
    {
      /* couldn't find a currently open shm control area for the key - probably because
       * shmget hasn't been called.
       * Allocate a new control block - this has to be handled by the daemon */
      client_request_shm req (SHM_REATTACH, shmid);

      if (req.make_request () == -1)
	{
	  set_errno (ENOSYS);	/* daemon communication failed */
	  return -1;
	}

      if (req.error_code ())	/* shm_get failed in the daemon */
	{
	  set_errno (req.error_code ());
	  return -1;
	}

      /* we've got the id, now we open the memory area ourselves.
       * This tests security automagically
       * FIXME: make this a method of shmnode ?
       */
      tempnode =
	build_inprocess_shmds (req.parameters.out.filemap,
			       req.parameters.out.attachmap,
			       req.parameters.out.key,
			       req.parameters.out.shm_id);
      if (!tempnode)
	return -1;
    }

  switch (cmd)
    {
    case IPC_STAT:
      *buf = tempnode->shmds->ds;
      break;
    case IPC_RMID:
      {
	/* TODO: check permissions. Or possibly, the daemon gets to be the only
	 * one with write access to the memory area?
	 */
	if (tempnode->shmds->ds.shm_nattch)
	  system_printf
	    ("call to shmctl with cmd= IPC_RMID when memory area still has"
	     " attachees");
	/* how does this work?
	   * we mark the ds area as "deleted", and the at and get calls all fail from now on
	   * on, when nattch becomes 0, the mapped data area is destroyed.
	   * and each process, as they touch this area detaches. eventually only the
	   * daemon has an attach. The daemon gets asked to detach immediately.
	 */
	//waiting for the daemon to handle terminating process's
	client_request_shm req (SHM_DEL, shmid);

	if (req.make_request () == -1)
	  {
	    set_errno (ENOSYS);	/* daemon communication failed */
	    return -1;
	  }

	if (req.error_code ())	/* shm_del failed in the daemon */
	  {
	    set_errno (req.error_code ());
	    return -1;
	  }

	/* the daemon has deleted it's references */
	/* now for us */

	// FIXME: create a destructor
	delete_inprocess_shmds (&tempnode);

      }
      break;
    case IPC_SET:
    default:
      set_errno (EINVAL);
      return -1;
    }
  return 0;
}

/* FIXME: evaluate getuid32() and getgid32() against the requested mode. Then
 * choose PAGE_READWRITE | PAGE_READONLY and FILE_MAP_WRITE  |  FILE_MAP_READ
 * appropriately
 */

/* FIXME: shmid should be a verifyable object
 */

/* FIXME: on NT we should check everything against the SD. On 95 we just emulate.
 */
extern "C" int
shmget (key_t key, size_t size, int shmflg)
{
  if (key == (key_t) - 1)
    {
      set_errno (ENOENT);
      return -1;
    }

  /* FIXME: enter the checking for existing keys mutex. This mutex _must_ be system wide
   * to prevent races on shmget.
   */

  /* walk the list of currently open keys and return the id if found
   */
  shmnode *tempnode = shm_head;
  while (tempnode)
    {
      if (tempnode->key == key && key != IPC_PRIVATE)
	{
	  // FIXME: free the mutex
	  if (size && tempnode->shmds->ds.shm_segsz < size)
	    {
	      set_errno (EINVAL);
	      return -1;
	    }
	  if ((shmflg & IPC_CREAT) && (shmflg & IPC_EXCL))
	    {
	      set_errno (EEXIST);
	      // FIXME: free the mutex
	      return -1;
	    }
	  // FIXME: do we need to other tests of the requested mode with the
	  // tempnode->shmid mode ? testcase on unix needed.
	  // FIXME do we need a security test? We are only examining the keys we already have open.
	  // FIXME: what are the sec implications for fork () if we don't check here?
	  return tempnode->shm_id;
	}
      tempnode = tempnode->next;
    }
  /* couldn't find a currently open shm control area for the key.
   * Allocate a new control block - this has to be handled by the daemon */
  client_request_shm req (key, size, shmflg);

  if (req.make_request () == -1)
    {
      set_errno (ENOSYS);	/* daemon communication failed */
      return -1;
    }

  if (req.error_code ())	/* shm_get failed in the daemon */
    {
      set_errno (req.error_code ());
      return -1;
    }

  /* we've got the id, now we open the memory area ourselves.
   * This tests security automagically
   * FIXME: make this a method of shmnode ?
   */
  shmnode *shmtemp = build_inprocess_shmds (req.parameters.out.filemap,
					    req.parameters.out.attachmap,
					    key,
					    req.parameters.out.shm_id);
  if (shmtemp)
    return shmtemp->shm_id;
  return -1;
}
