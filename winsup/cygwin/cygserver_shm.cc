/* cygserver_shm.cc: Single unix specification IPC interface for Cygwin

   Copyright 2001 Red Hat, Inc.

   Originally written by Robert Collins <robert.collins@hotmail.com>

   This file is part of Cygwin.

   This software is a copyrighted work licensed under the terms of the
   Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
   details. */


#ifdef __OUTSIDE_CYGWIN__
#undef __INSIDE_CYGWIN__
#else
#include "winsup.h"
#endif

#ifndef __INSIDE_CYGWIN__
#define DEBUG 0
#define system_printf printf
#define debug_printf if (DEBUG) printf
#define api_fatal printf
#include <stdio.h>
#include <windows.h>
#endif

#include <sys/stat.h>
#include <errno.h>
#include "cygerrno.h"
#include <unistd.h>
#include "security.h"
//#include "fhandler.h"
//#include "dtable.h"
//#include "cygheap.h"
#include <stdio.h>
//#include "thread.h"
#ifndef __INSIDE_CYGWIN__
#define __INSIDE_CYGWIN__
#include <sys/shm.h>
#undef __INSIDE_CYGWIN__
#else
#include <sys/shm.h>
#endif
//#include "perprocess.h"
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
size_t getsystemallocgranularity ()
{
  SYSTEM_INFO sysinfo;
  static size_t buffer_offset = 0;
  if (buffer_offset)
    return buffer_offset;
  GetSystemInfo (&sysinfo);
  buffer_offset = sysinfo.dwAllocationGranularity;
  return buffer_offset;
}

client_request_shm_get::client_request_shm_get () : client_request (CYGSERVER_REQUEST_SHM_GET)
{
  header.cb = sizeof (parameters);
  buffer = (char *) &parameters;
  header.error_code = 0;
}

client_request_shm_get::client_request_shm_get (int nshm_id, pid_t npid) : client_request (CYGSERVER_REQUEST_SHM_GET)
{
  header.cb = sizeof (parameters);
  buffer = (char *) &parameters;
  header.error_code = 0;
  parameters.in.shm_id = nshm_id;
  parameters.in.type = SHM_REATTACH;
  parameters.in.pid = npid;
}

client_request_shm_get::client_request_shm_get (key_t nkey, size_t nsize,
						int nshmflg, char psdbuf[4096], pid_t npid):	client_request (CYGSERVER_REQUEST_SHM_GET)
{
  header.cb = sizeof (parameters);
  buffer = (char *) &parameters;
  parameters.in.key = nkey;
  parameters.in.size = nsize;
  parameters.in.shmflg = nshmflg;
  parameters.in.type = SHM_CREATE;
  parameters.in.pid = npid;
  memcpy (parameters.in.sd_buf, psdbuf, 4096);
}

/* FIXME: If building on a 64-bit compiler, the address->int typecast will fail.
 * Solution: manually calculate the next id value 
 */

#if 0
extern "C" void *
shmat (int shmid, const void *shmaddr, int parameters.in.shmflg)
{
  class shmid_ds *shm = (class shmid_ds *) shmid;	//FIXME: verifyable object test

  if (shmaddr)
    {
      //FIXME: requested base address ?!
      set_errno (EINVAL);
      return (void *) -1;
    }

  void *rv = MapViewOfFile (shm->attachmap,

			    
			    (parameters.in.
			     shmflg & SHM_RDONLY) ? FILE_MAP_READ :
			    FILE_MAP_WRITE, 0,
			    0, 0);

  if (!rv)
    {
      //FIXME: translate GetLastError()
      set_errno (EACCES);
      return (void *) -1;
    }

/* FIXME: this needs to be globally protected to prevent a mismatch betwen
 * attach count and attachees list
 */

  InterlockedIncrement (&shm->shm_nattch);
  _shmattach *attachnode = new _shmattach;

  attachnode->data = rv;
  attachnode->next =
    (_shmattach *) InterlockedExchangePointer ((LONG *) & shm->attachhead,
					       (long int) attachnode);
  return rv;
}
#endif

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

/* for dll size */
#ifdef __INSIDE_CYGWIN__
void
client_request_shm_get::serve (transport_layer_base * conn)
{
}
#else

extern GENERIC_MAPPING access_mapping;

extern int
check_and_dup_handle (HANDLE from_process, HANDLE to_process,
		      HANDLE from_process_token,
                      DWORD access,
                      HANDLE from_handle,
                      HANDLE* to_handle_ptr, BOOL bInheritHandle);

//FIXME: where should this live
static shmnode *shm_head = NULL;
/* must be long for InterlockedIncrement */
static long new_id = 0;
static long new_private_key = 0;

void
client_request_shm_get::serve (transport_layer_base * conn)
{
//  DWORD sd_size = 4096;
//  char sd_buf[4096];
  PSECURITY_DESCRIPTOR psd = (PSECURITY_DESCRIPTOR) parameters.in.sd_buf;
//  /* create a sd for our open requests based on shmflag & 0x01ff */
//  psd = alloc_sd (getuid (), getgid (), cygheap->user.logsrv (),
//                parameters.in.shmflg & 0x01ff, psd, &sd_size);

  HANDLE from_process_handle = NULL;
  HANDLE token_handle = NULL;
  DWORD rc;

  from_process_handle = OpenProcess (PROCESS_DUP_HANDLE, FALSE, parameters.in.pid);
  if (!from_process_handle)
    {
      printf ("error opening process (%lu)\n", GetLastError ());
      header.error_code = EACCES;
      return;
    }

  conn->impersonate_client ();
  
  rc = OpenThreadToken (GetCurrentThread (),
  			TOKEN_QUERY,
                        TRUE,
			&token_handle);

  conn->revert_to_self ();

  if (!rc)
    {
      printf ("error opening thread token (%lu)\n", GetLastError ());
      header.error_code = EACCES;
      CloseHandle (from_process_handle);
      return;
    }


  /* we trust the clients request - we will be doing it as them, and
   * the worst they can do is open their own permissions 
   */


  SECURITY_ATTRIBUTES sa;
  sa.nLength = sizeof (sa);
  sa.lpSecurityDescriptor = psd;
  sa.bInheritHandle = TRUE; /* the memory structures inherit ok */

  char *shmname = NULL, *shmaname = NULL;
  char stringbuf[29], stringbuf1[29];

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
  if (check_and_dup_handle (GetCurrentProcess (),from_process_handle,
                           token_handle,
                           DUPLICATE_SAME_ACCESS,
		    tempnode->filemap, &parameters.out.filemap, TRUE) != 0)
    {
      printf ("error duplicating filemap handle (%lu)\n", GetLastError ());
      header.error_code = EACCES;
      CloseHandle (from_process_handle);
    }
     if (check_and_dup_handle (GetCurrentProcess (),from_process_handle,
		token_handle,
                DUPLICATE_SAME_ACCESS,
          tempnode->attachmap, &parameters.out.attachmap, TRUE) != 0)
    {
      printf ("error duplicating attachmap handle (%lu)\n", GetLastError ());
      header.error_code = EACCES;
      CloseHandle (from_process_handle);
    }
          return;
        }
      tempnode = tempnode->next;
    }     
  header.error_code = EINVAL;
  return;
    }
  /* it's a original request from the users */

  /* FIXME: enter the checking for existing keys mutex. This mutex _must_ be system wide
   * to prevent races on shmget.
   */

  if (parameters.in.key == IPC_PRIVATE)
    {
      /* create the mapping name (CYGWINSHMKPRIVATE_0x01234567 */
      /* The K refers to Key, the actual mapped area has D */
      long private_key = (int) InterlockedIncrement (&new_private_key);
      snprintf (stringbuf , 29, "CYGWINSHMKPRIVATE_0x%0x", private_key);
	shmname = stringbuf;
	snprintf (stringbuf1, 29, "CYGWINSHMDPRIVATE_0x%0x", private_key);
	shmaname = stringbuf1;
    }
  else
    {
  /* create the mapping name (CYGWINSHMK0x0123456789abcdef */
  /* The K refers to Key, the actual mapped area has D */

  snprintf (stringbuf , 29, "CYGWINSHMK0x%0qx", parameters.in.key);
  shmname = stringbuf;
  snprintf (stringbuf1, 29, "CYGWINSHMD0x%0qx", parameters.in.key);
  shmaname = stringbuf1;
  debug_printf ("system id strings are \n%s\n%s\n",shmname,shmaname);
  debug_printf ("key input value is 0x%0qx\n", parameters.in.key);
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
	      && tempnode->shmds->shm_segsz < parameters.in.size)
	    {
	      header.error_code = EINVAL;
	      return;
	    }
	  /* FIXME: can the same process call this twice without error ? test 
	   * on unix
	   */
	  if ((parameters.in.shmflg & IPC_CREAT)
	      && (parameters.in.shmflg & IPC_EXCL))
	    {
	      header.error_code = EEXIST;
	      debug_printf ("attempt to exclusively create already created shm_area with key 0x%0qx\n",parameters.in.key);
	      // FIXME: free the mutex
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
if (check_and_dup_handle (GetCurrentProcess (),from_process_handle,
                           token_handle,
              DUPLICATE_SAME_ACCESS,
                    tempnode->filemap, &parameters.out.filemap,TRUE) != 0)
    {
      printf ("error duplicating filemap handle (%lu)\n", GetLastError ());
      header.error_code = EACCES;
/*mutex*/
      CloseHandle (from_process_handle);
 return;
    }
if (check_and_dup_handle (GetCurrentProcess (),from_process_handle,
                            token_handle,
                            DUPLICATE_SAME_ACCESS,
          tempnode->attachmap, &parameters.out.attachmap, TRUE) != 0)
    {
      printf ("error duplicating attachmap handle (%lu)\n", GetLastError ());
      header.error_code = EACCES;
/*mutex*/
      CloseHandle (from_process_handle);
return;
    }

	  return;
	}
      tempnode = tempnode->next;
    }
  /* couldn't find a currently open shm area. */

  /* create one */
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
  int lasterr = GetLastError ();
  conn->revert_to_self ();

  if (filemap == NULL)
    {
      /* We failed to open the filemapping ? */
      system_printf ("failed to open file mapping: %lu\n", GetLastError ());
      // free the mutex
      // we can assume that it exists, and that it was an access problem.
      header.error_code = EACCES;
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
      system_printf ("failed to get shm attachmap\n");
      header.error_code = ENOMEM;
      UnmapViewOfFile (mapptr);
      CloseHandle (filemap);
      /* FIXME exit the mutex */
      return;
    }

  shmid_ds *shmtemp = new shmid_ds;
  if (!shmtemp)
    {
      system_printf ("failed to malloc shm node\n");
      header.error_code = ENOMEM;
      UnmapViewOfFile (mapptr);
      CloseHandle (filemap);
      CloseHandle (attachmap);
      /* FIXME exit mutex */
      return;
    }

  /* fill out the node data */
  shmtemp->shm_perm.cuid = getuid ();
  shmtemp->shm_perm.uid = shmtemp->shm_perm.cuid;
  shmtemp->shm_perm.cgid = getgid ();
  shmtemp->shm_perm.gid = shmtemp->shm_perm.cgid;
  shmtemp->shm_perm.mode = parameters.in.shmflg & 0x01ff;
  shmtemp->shm_lpid = 0;
  shmtemp->shm_nattch = 0;
  shmtemp->shm_atime = 0;
  shmtemp->shm_dtime = 0;
  shmtemp->shm_ctime = time (NULL);
  shmtemp->shm_segsz = parameters.in.size;
  *(shmid_ds *) mapptr = *shmtemp;
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
if (check_and_dup_handle (GetCurrentProcess (),from_process_handle,
                           token_handle,
              DUPLICATE_SAME_ACCESS,
                    tempnode->filemap, &parameters.out.filemap, TRUE) != 0)
    {
      printf ("error duplicating filemap handle (%lu)\n", GetLastError ());
      header.error_code = EACCES;
      CloseHandle (from_process_handle);
/* mutex et al */
return;
    }
if (check_and_dup_handle (GetCurrentProcess (),from_process_handle,
                            token_handle,
                           DUPLICATE_SAME_ACCESS,
          tempnode->attachmap, &parameters.out.attachmap, TRUE) != 0)
    {
      printf ("error duplicating attachmap handle (%lu)\n", GetLastError ());
      header.error_code = EACCES;
      CloseHandle (from_process_handle);
/* more cleanup... yay! */
return;
    }
  return;
}
#endif
