/* cygserver_shm.cc: Single unix specification IPC interface for Cygwin

   Copyright 2001, 2002 Red Hat, Inc.

   Originally written by Robert Collins <robert.collins@hotmail.com>

   This file is part of Cygwin.

   This software is a copyrighted work licensed under the terms of the
   Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
   details. */

#include "woutsup.h"

// #include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <time.h>
#include "cygerrno.h"
#include <unistd.h>
#include "security.h"
//#include "fhandler.h"
//#include "dtable.h"
//#include "cygheap.h"
//#include "thread.h"
#include <sys/shm.h>
//#include "perprocess.h"
#include <threaded_queue.h>
#include <cygwin/cygserver_process.h>
#include "cygserver_shm.h"

// FIXME IS THIS CORRECT
/* Implementation notes: We use two shared memory regions per key:
 * One for the control structure, and one for the shared memory.
 * While this has a higher overhead tham a single shared area,
 * It allows more flexability. As the entire code is transparent to the user
 * We can merge these in the future should it be needed.
 * Also, IPC_PRIVATE keys create unique mappings each time. The shm_ids just
 * keep monotonically incrementing - system wide.
 */
size_t
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


client_request_shm::client_request_shm ():client_request (CYGSERVER_REQUEST_SHM_GET,
	       sizeof (parameters))
{
  buffer = (char *) &parameters;
}

/* FIXME: evaluate getuid() and getgid() against the requested mode. Then
 * choose PAGE_READWRITE | PAGE_READONLY and FILE_MAP_WRITE  |  FILE_MAP_READ
 * appropriately
 */

/* Test result from openbsd: shm ids are persistent cross process if a handle is left
 * open. This could lead to resource starvation: we're not copying that behaviour
 * unless we have to. (It will involve acygwin1.dll gloal shared list :[ ).
 */
/* FIXME: shmid should be a verifyable object
 */

/* FIXME: on NT we should check everything against the SD. On 95 we just emulate.
 */

extern GENERIC_MAPPING
  access_mapping;

extern int
check_and_dup_handle (HANDLE from_process, HANDLE to_process,
		      HANDLE from_process_token,
		      DWORD access,
		      HANDLE from_handle,
		      HANDLE * to_handle_ptr, BOOL bInheritHandle);

//FIXME: where should this live
static shmnode *
  shm_head =
  NULL;
//FIXME: ditto.
static shmnode *
  deleted_head = NULL;
/* must be long for InterlockedIncrement */
static long
  new_id =
  0;
static long
  new_private_key =
  0;

static void
delete_shmnode (shmnode **nodeptr)
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
    UnmapViewOfFile (node->shmds->mapptr);
    delete node->shmds;
    CloseHandle (node->filemap);
    CloseHandle (node->attachmap);

    // free the memory
    delete node;
    nodeptr = NULL;
}

void
client_request_shm::serve (transport_layer_base * conn, process_cache * cache)
{
  HANDLE from_process_handle = NULL;
  HANDLE token_handle = NULL;
  DWORD rc;

  from_process_handle = cache->process (parameters.in.winpid)->handle ();
  /* possible TODO: reduce the access on the handle before we use it */
  /* Note that unless we do this, we don't need to call CloseHandle - it's kept open
   * by the process cache until the process terminates.
   * We may need a refcount on the cache however...
   */
  if (!from_process_handle)
    {
      debug_printf ("error opening process (%lu)", GetLastError ());
      header.error_code = EACCES;
      return;
    }

  if (wincap.has_security ())
    {
      conn->impersonate_client ();

      rc = OpenThreadToken (GetCurrentThread (),
			    TOKEN_QUERY, TRUE, &token_handle);

      conn->revert_to_self ();

      if (!rc)
	{
	  debug_printf ("error opening thread token (%lu)", GetLastError ());
	  header.error_code = EACCES;
	  CloseHandle (from_process_handle);
	  return;
	}
    }

  char *shmname = NULL, *shmaname = NULL;
  char stringbuf[29], stringbuf1[29];

  /* TODO: make this code block a function! */
  if (parameters.in.type == SHM_REATTACH)
    {
      /* just find and fill out the existing shm_id */
      shmnode *tempnode = shm_head;
      while (tempnode)
	{
	  if (tempnode->shm_id == parameters.in.shm_id)
	    {
	      parameters.out.shm_id = tempnode->shm_id;
	      parameters.out.key = tempnode->key;
	      if (check_and_dup_handle
		  (GetCurrentProcess (), from_process_handle, token_handle,
		   DUPLICATE_SAME_ACCESS, tempnode->filemap,
		   &parameters.out.filemap, TRUE) != 0)
		{
		  debug_printf ("error duplicating filemap handle (%lu)",
				GetLastError ());
		  header.error_code = EACCES;
		}
	      if (check_and_dup_handle
		  (GetCurrentProcess (), from_process_handle, token_handle,
		   DUPLICATE_SAME_ACCESS, tempnode->attachmap,
		   &parameters.out.attachmap, TRUE) != 0)
		{
		  debug_printf ("error duplicating attachmap handle (%lu)",
				GetLastError ());
		  header.error_code = EACCES;
		}
	      CloseHandle (token_handle);
	      return;
	    }
	  tempnode = tempnode->next;
	}
      header.error_code = EINVAL;
      CloseHandle (token_handle);
      return;
    }

  /* someone attached */
  /* someone can send shm_id's they don't have and currently we will increment those
   * attach counts. If someone wants to fix that, please go ahead.
   * The problem is that shm_get has nothing to do with the ability to attach. Attach
   * requires a permission check, which we get the OS to do in MapViewOfFile.
   */
  if (parameters.in.type == SHM_ATTACH)
    {
      shmnode *tempnode = shm_head;
      while (tempnode)
	{
	  if (tempnode->shm_id == parameters.in.shm_id)
	    {
	      InterlockedIncrement (&tempnode->shmds->ds.shm_nattch);
	      header.error_code = 0;
	      CloseHandle (token_handle);
	      return;
	    }
	  tempnode = tempnode->next;
	}
      header.error_code = EINVAL;
      CloseHandle (token_handle);
      return;
    }

  /* Someone detached */
  if (parameters.in.type == SHM_DETACH)
    {
      shmnode *tempnode = shm_head;
      while (tempnode)
	{
	  if (tempnode->shm_id == parameters.in.shm_id)
	    {
	      InterlockedDecrement (&tempnode->shmds->ds.shm_nattch);
	      header.error_code = 0;
	      CloseHandle (token_handle);
	      return;
	    }
	  tempnode = tempnode->next;
	}
      header.error_code = EINVAL;
      CloseHandle (token_handle);
      return;
    }

  /* Someone wants the ID removed. */
  if (parameters.in.type == SHM_DEL)
    {
      shmnode **tempnode = &shm_head;
      while (*tempnode)
	  {
	    if ((*tempnode)->shm_id == parameters.in.shm_id)
	      {
		// unlink from the accessible node list
		shmnode *temp2 = *tempnode;
		*tempnode = temp2->next;
		// link into the deleted list
		temp2->next = deleted_head;
		deleted_head = temp2;

		// FIXME: when/where do we delete the handles?
		if (temp2->shmds->ds.shm_nattch)
		  {
		    // FIXME: add to a pending queue?
		  }
		else
		  {
		    delete_shmnode (&temp2);
		  }

		header.error_code = 0;
		CloseHandle (token_handle);
		return;
	      }
	    tempnode = &(*tempnode)->next;
	  }
      header.error_code = EINVAL;
      CloseHandle (token_handle);
      return;
    }


  if (parameters.in.type == SHM_CREATE)
    {
      /* FIXME: enter the checking for existing keys mutex. This mutex _must_ be system wide
       * to prevent races on shmget.
       */

      if (parameters.in.key == IPC_PRIVATE)
	{
	  /* create the mapping name (CYGWINSHMKPRIVATE_0x01234567 */
	  /* The K refers to Key, the actual mapped area has D */
	  long private_key = (int) InterlockedIncrement (&new_private_key);
	  snprintf (stringbuf, 29, "CYGWINSHMKPRIVATE_0x%0x", private_key);
	  shmname = stringbuf;
	  snprintf (stringbuf1, 29, "CYGWINSHMDPRIVATE_0x%0x", private_key);
	  shmaname = stringbuf1;
	}
      else
	{
	  /* create the mapping name (CYGWINSHMK0x0123456789abcdef */
	  /* The K refers to Key, the actual mapped area has D */

	  snprintf (stringbuf, 29, "CYGWINSHMK0x%0qx", parameters.in.key);
	  shmname = stringbuf;
	  snprintf (stringbuf1, 29, "CYGWINSHMD0x%0qx", parameters.in.key);
	  shmaname = stringbuf1;
	  debug_printf ("system id strings: %s, %s", shmname,
			shmaname);
	  debug_printf ("key input value is 0x%0qx", parameters.in.key);
	}

      /* attempt to open the key */

      /* get an existing key */
      /* On unix the same shmid identifier is returned on multiple calls to shm_get
       * with the same key and size. Different modes is a ?.
       */



      /* walk the list of known keys and return the id if found. remember, we are
       * authoritative...
       */

      shmnode *tempnode = shm_head;
      while (tempnode)
	{
	  if (tempnode->key == parameters.in.key
	      && parameters.in.key != IPC_PRIVATE)
	    {
	      // FIXME: free the mutex
	      if (parameters.in.size
		  && tempnode->shmds->ds.shm_segsz < parameters.in.size)
		{
		  header.error_code = EINVAL;
		  CloseHandle (token_handle);
		  return;
		}
	      /* FIXME: can the same process call this twice without error ? test
	       * on unix
	       */
	      if ((parameters.in.shmflg & IPC_CREAT)
		  && (parameters.in.shmflg & IPC_EXCL))
		{
		  header.error_code = EEXIST;
		  debug_printf
		    ("attempt to exclusively create already created shm_area with key 0x%0qx",
		     parameters.in.key);
		  // FIXME: free the mutex
		  CloseHandle (token_handle);
		  return;
		}
	      // FIXME: do we need to other tests of the requested mode with the
	      // tempnode->shm_id mode ? testcase on unix needed.
	      // FIXME how do we do the security test? or
	      // do we wait for shmat to bother with that?
	      /* One possibly solution: impersonate the client, and then test we can
	       * reopen the area. In fact we'll probably have to do that to get
	       * handles back to them, alternatively just tell them the id, and then
	       * let them attempt the open.
	       */
	      parameters.out.shm_id = tempnode->shm_id;
	      if (check_and_dup_handle
		  (GetCurrentProcess (), from_process_handle, token_handle,
		   DUPLICATE_SAME_ACCESS, tempnode->filemap,
		   &parameters.out.filemap, TRUE) != 0)
		{
		  system_printf ("error duplicating filemap handle (%lu)",
				 GetLastError ());
		  header.error_code = EACCES;
/*mutex*/
		  CloseHandle (token_handle);
		  return;
		}
	      if (check_and_dup_handle
		  (GetCurrentProcess (), from_process_handle, token_handle,
		   DUPLICATE_SAME_ACCESS, tempnode->attachmap,
		   &parameters.out.attachmap, TRUE) != 0)
		{
		  system_printf ("error duplicating attachmap handle (%lu)",
				 GetLastError ());
		  header.error_code = EACCES;
/*mutex*/
		  CloseHandle (token_handle);
		  return;
		}

	      CloseHandle (token_handle);
	      return;
	    }
	  tempnode = tempnode->next;
	}
      /* couldn't find a currently open shm area. */

      /* create one */

      /* we trust the clients request - we will be doing it as them, and
       * the worst they can do is open their own permissions
       */

      SECURITY_ATTRIBUTES sa;
      sa.nLength = sizeof (sa);
      sa.lpSecurityDescriptor = (PSECURITY_DESCRIPTOR) parameters.in.sd_buf;
      sa.bInheritHandle = TRUE;	/* the memory structures inherit ok */

      /* do this as the client */
      conn->impersonate_client ();
      /* This may need sh_none... it's only a control structure */
      HANDLE filemap = CreateFileMapping (INVALID_HANDLE_VALUE,	// system pagefile.
					  &sa,
					  PAGE_READWRITE,	// protection
					  0x00000000,
					  getsystemallocgranularity (),
					  shmname	// object name
					  );
      DWORD lasterr = GetLastError ();
      conn->revert_to_self ();

      if (filemap == NULL)
	{
	  /* We failed to open the filemapping ? */
	  system_printf ("failed to open file mapping: %lu",
			 GetLastError ());
	  // free the mutex
	  // we can assume that it exists, and that it was an access problem.
	  header.error_code = EACCES;
	  CloseHandle (token_handle);
	  return;
	}

      /* successfully opened the control region mapping */
      /* did we create it ? */
      int oldmapping = lasterr == ERROR_ALREADY_EXISTS;
      if (oldmapping)
	{
	  /* should never happen - we are the global daemon! */
#if 0
	  if ((parameters.in.shmflg & IPC_CREAT)
	      && (parameters.in.shmflg & IPC_EXCL))
#endif
	    {
	      /* FIXME free mutex */
	      CloseHandle (filemap);
	      header.error_code = EEXIST;
	      CloseHandle (token_handle);
	      return;
	    }
	}

      /* we created a new mapping */
      if (parameters.in.key != IPC_PRIVATE &&
	  (parameters.in.shmflg & IPC_CREAT) == 0)
	{
	  CloseHandle (filemap);
	  /* FIXME free mutex */
	  header.error_code = ENOENT;
	  CloseHandle (token_handle);
	  return;
	}

      conn->impersonate_client ();
      void *mapptr = MapViewOfFile (filemap, FILE_MAP_WRITE, 0, 0, 0);
      conn->revert_to_self ();

      if (!mapptr)
	{
	  CloseHandle (filemap);
	  //FIXME: close filemap and free the mutex
	  /* we couldn't access the mapped area with the requested permissions */
	  header.error_code = EACCES;
	  CloseHandle (token_handle);
	  return;
	}

      conn->impersonate_client ();
      /* Now get the user data */
      HANDLE attachmap = CreateFileMapping (INVALID_HANDLE_VALUE,	// system pagefile
					    &sa,
					    PAGE_READWRITE,	// protection (FIXME)
					    0x00000000,
					    parameters.in.size +
					    parameters.in.size %
					    getsystemallocgranularity (),
					    shmaname	// object name
	);
      conn->revert_to_self ();

      if (attachmap == NULL)
	{
	  system_printf ("failed to get shm attachmap");
	  header.error_code = ENOMEM;
	  UnmapViewOfFile (mapptr);
	  CloseHandle (filemap);
	  /* FIXME exit the mutex */
	  CloseHandle (token_handle);
	  return;
	}

      int_shmid_ds *shmtemp = new int_shmid_ds;
      if (!shmtemp)
	{
	  system_printf ("failed to malloc shm node");
	  header.error_code = ENOMEM;
	  UnmapViewOfFile (mapptr);
	  CloseHandle (filemap);
	  CloseHandle (attachmap);
	  /* FIXME exit mutex */
	  CloseHandle (token_handle);
	  return;
	}

      /* fill out the node data */
      shmtemp->ds.shm_perm.cuid = getuid ();
      shmtemp->ds.shm_perm.uid = shmtemp->ds.shm_perm.cuid;
      shmtemp->ds.shm_perm.cgid = getgid ();
      shmtemp->ds.shm_perm.gid = shmtemp->ds.shm_perm.cgid;
      shmtemp->ds.shm_perm.mode = parameters.in.shmflg & 0x01ff;
      shmtemp->ds.shm_lpid = 0;
      shmtemp->ds.shm_nattch = 0;
      shmtemp->ds.shm_atime = 0;
      shmtemp->ds.shm_dtime = 0;
      shmtemp->ds.shm_ctime = time (NULL);
      shmtemp->ds.shm_segsz = parameters.in.size;
      *(shmid_ds *) mapptr = shmtemp->ds;
      shmtemp->mapptr = mapptr;

      /* no need for InterlockedExchange here, we're serialised by the global mutex */
      tempnode = new shmnode;
      tempnode->shmds = shmtemp;
      tempnode->shm_id = (int) InterlockedIncrement (&new_id);
      tempnode->key = parameters.in.key;
      tempnode->filemap = filemap;
      tempnode->attachmap = attachmap;
      tempnode->next = shm_head;
      shm_head = tempnode;

      /* we now have the area in the daemon list, opened.

	 FIXME: leave the system wide shm mutex */

      parameters.out.shm_id = tempnode->shm_id;
      if (check_and_dup_handle (GetCurrentProcess (), from_process_handle,
				token_handle,
				DUPLICATE_SAME_ACCESS,
				tempnode->filemap, &parameters.out.filemap,
				TRUE) != 0)
	{
	  system_printf ("error duplicating filemap handle (%lu)",
			 GetLastError ());
	  header.error_code = EACCES;
	  CloseHandle (token_handle);
/* mutex et al */
	  return;
	}
      if (check_and_dup_handle (GetCurrentProcess (), from_process_handle,
				token_handle,
				DUPLICATE_SAME_ACCESS,
				tempnode->attachmap,
				&parameters.out.attachmap, TRUE) != 0)
	{
	  system_printf ("error duplicating attachmap handle (%lu)",
			 GetLastError ());
	  header.error_code = EACCES;
	  CloseHandle (from_process_handle);
	  CloseHandle (token_handle);
/* more cleanup... yay! */
	  return;
	}
      CloseHandle (token_handle);

      return;
    }

  header.error_code = ENOSYS;
  CloseHandle (token_handle);


  return;
}
