/* shm.cc: Single unix specification IPC interface for Cygwin

   Copyright 2001 Red Hat, Inc.

   Originally written by Robert Collins <robert.collins@hotmail.com>

   This file is part of Cygwin.

   This software is a copyrighted work licensed under the terms of the
   Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
   details. */

#include "winsup.h"
#include <sys/stat.h>
#include <errno.h>
#include "cygerrno.h"
#include <unistd.h>
#include "security.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include <stdio.h>
#include "thread.h"
#include <sys/shm.h>
#include "perprocess.h"
#include "cygserver_shm.h"

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
  shmid_ds *shmtemp = new shmid_ds;
  if (!shmtemp)
    {
      system_printf ("failed to malloc shm node\n");
      set_errno (ENOMEM);
      UnmapViewOfFile (mapptr);
      CloseHandle (filemap);
      CloseHandle (attachmap);
      /* exit mutex */
      return NULL;
    }

  /* get the system node data */
  *shmtemp = *(shmid_ds *) mapptr;

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
	  system_printf ("failed to reattach to shm control file view %x\n",
			 tempnode);
	  return 1;
	}
      tempnode->shmds = (class shmid_ds *) newshmds;
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
	      system_printf ("failed to reattach to mapped file view %x\n",
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
      client_request_shm_get *req =
	new client_request_shm_get (shmid, GetCurrentProcessId ());

      int rc;
      if ((rc = cygserver_request (req)))
	{
	  delete req;
	  set_errno (ENOSYS);	/* daemon communication failed */
	  return (void *) -1;
	}

      if (req->header.error_code)	/* shm_get failed in the daemon */
	{
	  set_errno (req->header.error_code);
	  delete req;
	  return (void *) -1;
	}

      /* we've got the id, now we open the memory area ourselves.
       * This tests security automagically
       * FIXME: make this a method of shmnode ?
       */
      tempnode =
	build_inprocess_shmds (req->parameters.out.filemap,
			       req->parameters.out.attachmap,
			       req->parameters.out.key,
			       req->parameters.out.shm_id);
      delete req;
      if (!tempnode)
	return (void *) -1;

    }

  class shmid_ds *shm = tempnode->shmds;

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

  InterlockedIncrement (&shm->shm_nattch);
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
      client_request_shm_get *req =
	new client_request_shm_get (shmid, GetCurrentProcessId ());

      int rc;
      if ((rc = cygserver_request (req)))
	{
	  delete req;
	  set_errno (ENOSYS);	/* daemon communication failed */
	  return -1;
	}

      if (req->header.error_code)	/* shm_get failed in the daemon */
	{
	  set_errno (req->header.error_code);
	  delete req;
	  return -1;
	}

      /* we've got the id, now we open the memory area ourselves.
       * This tests security automagically
       * FIXME: make this a method of shmnode ?
       */
      tempnode =
	build_inprocess_shmds (req->parameters.out.filemap,
			       req->parameters.out.attachmap,
			       req->parameters.out.key,
			       req->parameters.out.shm_id);
      delete req;
      if (!tempnode)
	return -1;
    }

  switch (cmd)
    {
    case IPC_STAT:
      buf->shm_perm = tempnode->shmds->shm_perm;
      buf->shm_segsz = tempnode->shmds->shm_segsz;
      buf->shm_lpid = tempnode->shmds->shm_lpid;
      buf->shm_cpid = tempnode->shmds->shm_cpid;
      buf->shm_nattch = tempnode->shmds->shm_nattch;
      buf->shm_atime = tempnode->shmds->shm_atime;
      buf->shm_dtime = tempnode->shmds->shm_dtime;
      buf->shm_ctime = tempnode->shmds->shm_ctime;
      break;
    case IPC_RMID:
      {
	/* TODO: check permissions. Or possibly, the daemon gets to be the only 
	 * one with write access to the memory area?
	 */
	if (tempnode->shmds->shm_nattch)
	  system_printf
	    ("call to shmctl with cmd= IPC_RMID when memory area still has"
	     " attachees\n");
	/* how does this work? 
	   * we mark the ds area as "deleted", and the at and get calls all fail from now on
	   * on, when nattch becomes 0, the mapped data area is destroyed.
	   * and each process, as they touch this area detaches. eventually only the 
	   * daemon has an attach. The daemon gets asked to detach immediately.
	 */
#if 0
//waiting for the daemon to handle terminating process's
	client_request_shm_get *req =
	  new client_request_shm_get (SHM_DEL, shmid, GetCurrentProcessId ());
	int rc;
	if ((rc = cygserver_request (req)))
	  {
	    delete req;
	    set_errno (ENOSYS);	/* daemon communication failed */
	    return -1;
	  }

	if (req->header.error_code)	/* shm_del failed in the daemon */
	  {
	    set_errno (req->header.error_code);
	    delete req;
	    return -1;
	  }

	/* the daemon has deleted it's references */
	/* now for us */
	
#endif	

      }
      break;
    case IPC_SET:
    default:
      set_errno (EINVAL);
      return -1;
    }
  return 0;
}

/* FIXME: evaluate getuid() and getgid() against the requested mode. Then
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
  DWORD sd_size = 4096;
  char sd_buf[4096];
  PSECURITY_DESCRIPTOR psd = (PSECURITY_DESCRIPTOR) sd_buf;
  /* create a sd for our open requests based on shmflag & 0x01ff */
  psd = alloc_sd (getuid (), getgid (), cygheap->user.logsrv (),
		  shmflg & 0x01ff, psd, &sd_size);

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
	  if (size && tempnode->shmds->shm_segsz < size)
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
  client_request_shm_get *req =
    new client_request_shm_get (key, size, shmflg, sd_buf,
				GetCurrentProcessId ());

  int rc;
  if ((rc = cygserver_request (req)))
    {
      delete req;
      set_errno (ENOSYS);	/* daemon communication failed */
      return -1;
    }

  if (req->header.error_code)	/* shm_get failed in the daemon */
    {
      set_errno (req->header.error_code);
      delete req;
      return -1;
    }

  /* we've got the id, now we open the memory area ourselves.
   * This tests security automagically
   * FIXME: make this a method of shmnode ?
   */
  shmnode *shmtemp = build_inprocess_shmds (req->parameters.out.filemap,
					    req->parameters.out.attachmap,
					    key,
					    req->parameters.out.shm_id);
  delete req;
  if (shmtemp)
    return shmtemp->shm_id;
  return -1;


#if 0
  /* fill out the node data */
  shmtemp->shm_perm.cuid = getuid ();
  shmtemp->shm_perm.uid = shmtemp->shm_perm.cuid;
  shmtemp->shm_perm.cgid = getgid ();
  shmtemp->shm_perm.gid = shmtemp->shm_perm.cgid;
  shmtemp->shm_perm.mode = shmflg & 0x01ff;
  shmtemp->shm_lpid = 0;
  shmtemp->shm_nattch = 0;
  shmtemp->shm_atime = 0;
  shmtemp->shm_dtime = 0;
  shmtemp->shm_ctime = time (NULL);
  shmtemp->shm_segsz = size;
  *(shmid_ds *) mapptr = *shmtemp;
  shmtemp->filemap = filemap;
  shmtemp->attachmap = attachmap;
  shmtemp->mapptr = mapptr;

#endif
}
