/* cygserver_shm.cc: Single unix specification IPC interface for Cygwin.

   Copyright 2002 Red Hat, Inc.

   Written by Conrad Scott <conrad.scott@dsl.pipex.com>.
   Based on code by Robert Collins <robert.collins@hotmail.com>.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "woutsup.h"

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "cygserver_ipc.h"
#include "cygserver_shm.h"
#include "security.h"

#include "cygwin/cygserver.h"
#include "cygwin/cygserver_process.h"
#include "cygwin/cygserver_transport.h"

/*---------------------------------------------------------------------------*
 * with_strerr ()
 *---------------------------------------------------------------------------*/

#define with_strerr(MSG, ACTION)					\
  do									\
    {									\
      const DWORD lasterr = GetLastError ();				\
      char *MSG = NULL;							\
      if (!FormatMessage ((FORMAT_MESSAGE_ALLOCATE_BUFFER		\
			   | FORMAT_MESSAGE_FROM_SYSTEM			\
			   | FORMAT_MESSAGE_IGNORE_INSERTS),		\
			  NULL,						\
			  lasterr,					\
			  MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT),	\
			  reinterpret_cast<char *>(&MSG),		\
			  0,						\
			  NULL))					\
	{								\
	  MSG = static_cast<char *>					\
	    (LocalAlloc (LMEM_FIXED, 24)); /* Big enough. */		\
	  if (!MSG)							\
	    {								\
	      system_printf (("failure in LocalAlloc(LMEM_FIXED, 16): "	\
			      "error = %lu"),				\
			     GetLastError ());				\
	    }								\
	  else								\
	    {								\
	      snprintf (MSG, 24, "error = %lu", lasterr);		\
	    }								\
	}								\
      SetLastError (lasterr);						\
      { ACTION; }							\
      if (MSG && !LocalFree (MSG))					\
	{								\
	  system_printf ("failed to free memory at %p, error = %lu",	\
			 MSG, GetLastError ());				\
	}								\
      SetLastError (lasterr);						\
    } while (false)

/*---------------------------------------------------------------------------*
 * class server_shmmgr
 *
 * A singleton class.
 *---------------------------------------------------------------------------*/

#define shmmgr (server_shmmgr::instance ())

class server_shmmgr
{
private:
  class segment_t
  {
  public:
    // Bits for the _flg field.
    enum { REMOVED = 0x01 };

    const key_t key;
    const int shmid;
    struct shmid_ds ds;
    int flg;
    const HANDLE hFileMap;

    segment_t *next;

    segment_t (const key_t key, const int shmid, const HANDLE hFileMap)
      : key (key), shmid (shmid), flg (0), hFileMap (hFileMap), next (NULL)
    {
      bzero (&ds, sizeof (ds));
    }
  };

public:
  static server_shmmgr & instance ();

  int shmat (int shmid, int shmflg,
	     pid_t, HANDLE & hFileMap,
	     process_cache *, DWORD winpid);
  int shmctl (int shmid, int cmd, struct shmid_ds &, pid_t);
  int shmdt (int shmid, pid_t);
  int shmget (key_t, size_t, int shmflg, pid_t, uid_t, gid_t);

private:
  CRITICAL_SECTION _segments_lock;
  segment_t *_segments_head;	// A list sorted by shmid.

  int _shmid_cnt;		// Number of shmid's allocated (for ipcs(8)).
  int _shmid_max;		// Highest shmid yet allocated (for ipcs(8)).

  server_shmmgr ();
  ~server_shmmgr ();

  // Undefined (as this class is a singleton):
  server_shmmgr (const server_shmmgr &);
  server_shmmgr & operator= (const server_shmmgr &);

  segment_t *find_by_key (key_t);
  segment_t *find (int shmid, segment_t **previous = NULL);

  int new_segment (key_t, size_t, int shmflg, pid_t, uid_t, gid_t);

  segment_t *new_segment (key_t, HANDLE);
  void delete_segment (segment_t *);
};

/*---------------------------------------------------------------------------*
 * server_shmmgr::instance ()
 *---------------------------------------------------------------------------*/

server_shmmgr &
server_shmmgr::instance ()
{
  static server_shmmgr instance;

  return instance;
}

/*---------------------------------------------------------------------------*
 * server_shmmgr::shmat ()
 *---------------------------------------------------------------------------*/

int
server_shmmgr::shmat (const int shmid, const int shmflg,
		      const pid_t pid, HANDLE & hFileMap,
		      process_cache *const cache, const DWORD winpid)
{
  process *const client = cache->process (winpid);

  if (!client)
    {
      return -EAGAIN;
    }

  int result;
  EnterCriticalSection (&_segments_lock);

  segment_t *const segptr = find (shmid);

  if (!segptr)
    result = -EINVAL;
  else if (!DuplicateHandle (GetCurrentProcess (),
			     segptr->hFileMap,
			     client->handle (),
			     &hFileMap,
			     0,
			     FALSE, // bInheritHandle
			     DUPLICATE_SAME_ACCESS))
    {
      with_strerr (msg,
		   syscall_printf (("failed to duplicate handle for client "
				    "[key = %lld, shmid = %d, handle = %p]:"
				    "%s"),
				   segptr->key, segptr->shmid,
				   segptr->hFileMap, msg));

      result = -EACCES;		// FIXME
    }
  else
    {
      segptr->ds.shm_lpid  = pid;
      segptr->ds.shm_nattch += 1;
      segptr->ds.shm_atime = time (NULL); // FIXME: sub-second times.

      result = 0;
    }

  LeaveCriticalSection (&_segments_lock);

  client->release ();
  return result;
}

/*---------------------------------------------------------------------------*
 * server_shmmgr::shmctl ()
 *---------------------------------------------------------------------------*/

int
server_shmmgr::shmctl (const int shmid, const int cmd, struct shmid_ds & ds,
		       const pid_t pid)
{
  int result;
  EnterCriticalSection (&_segments_lock);

  segment_t *const segptr = find (shmid);

  if (!segptr)
    result = -EINVAL;
  else
    {
      switch (cmd)
	{
	case IPC_STAT:
	  ds = segptr->ds;
	  result = 0;
	  break;

	case IPC_SET:
	  segptr->ds.shm_perm.uid = ds.shm_perm.uid;
	  segptr->ds.shm_perm.gid = ds.shm_perm.gid;
	  segptr->ds.shm_perm.mode = ds.shm_perm.mode & 0777;
	  segptr->ds.shm_lpid = pid;
	  segptr->ds.shm_ctime = time (NULL); // FIXME: sub-second times.
	  result = 0;
	  break;

	case IPC_RMID:
	  if (segptr->flg & segment_t::REMOVED)
	    result = -EIDRM;
	  else
	    {
	      segptr->flg |= segment_t::REMOVED;
	      if (!segptr->ds.shm_nattch)
		delete_segment (segptr);
	      result = 0;
	    }
	  break;

	case IPC_INFO:
	  result = -EINVAL;
	  break;

	case SHM_STAT:
	  result = -EINVAL;
	  break;

	default:
	  result = -EINVAL;
	  break;
	}
    }

  LeaveCriticalSection (&_segments_lock);
  return result;
}

/*---------------------------------------------------------------------------*
 * server_shmmgr::shmdt ()
 *---------------------------------------------------------------------------*/

int
server_shmmgr::shmdt (const int shmid, const pid_t pid)
{
  int result;
  EnterCriticalSection (&_segments_lock);

  segment_t *const segptr = find (shmid);

  if (!segptr)
    result = -EINVAL;
  else
    {
      assert (segptr->ds.shm_nattch > 0);

      segptr->ds.shm_lpid  = pid;
      segptr->ds.shm_nattch -= 1;
      segptr->ds.shm_dtime = time (NULL); // FIXME: sub-second times.

      if (!segptr->ds.shm_nattch && (segptr->flg & segment_t::REMOVED))
	delete_segment (segptr);

      result = 0;
    }

  LeaveCriticalSection (&_segments_lock);
  return result;
}

/*---------------------------------------------------------------------------*
 * server_shmmgr::shmget ()
 *---------------------------------------------------------------------------*/

int
server_shmmgr::shmget (const key_t key, const size_t size, const int shmflg,
		       const pid_t pid, const uid_t uid, const gid_t gid)
{
  int result;
  EnterCriticalSection (&_segments_lock);

  /* Does a segment already exist with that key? */
  if (key == IPC_PRIVATE)
    result = new_segment (key, size, shmflg, pid, uid, gid);
  else
    {
      segment_t *const segptr = find_by_key (key);

      if (!segptr)
	if (shmflg & IPC_CREAT)
	  result = new_segment (key, size, shmflg, pid, uid, gid);
	else
	  result = -ENOENT;
      else if (segptr->flg & segment_t::REMOVED)
	result = -EIDRM;
      else if ((shmflg & IPC_CREAT) && (shmflg & IPC_EXCL))
	result = -EEXIST;
      else if (size && segptr->ds.shm_segsz < size)
	result = -EINVAL;
      else
	result = segptr->shmid;
    }

  LeaveCriticalSection (&_segments_lock);
  return result;
}

/*---------------------------------------------------------------------------*
 * server_shmmgr::server_shmmgr ()
 *---------------------------------------------------------------------------*/

server_shmmgr::server_shmmgr ()
  : _segments_head (NULL)
{
  InitializeCriticalSection (&_segments_lock);
}

/*---------------------------------------------------------------------------*
 * server_shmmgr::~server_shmmgr ()
 *---------------------------------------------------------------------------*/

server_shmmgr::~server_shmmgr ()
{
  DeleteCriticalSection (&_segments_lock);
}

/*---------------------------------------------------------------------------*
 * server_shmmgr::find_by_key ()
 *---------------------------------------------------------------------------*/

server_shmmgr::segment_t *
server_shmmgr::find_by_key (const key_t key)
{
  for (segment_t *segptr = _segments_head; segptr; segptr = segptr->next)
    if (segptr->key == key)
      return segptr;

  return NULL;
}

/*---------------------------------------------------------------------------*
 * server_shmmgr::find ()
 *---------------------------------------------------------------------------*/

server_shmmgr::segment_t *
server_shmmgr::find (const int shmid, segment_t **previous)
{
  if (previous)
    *previous = NULL;

  for (segment_t *segptr = _segments_head; segptr; segptr = segptr->next)
    if (segptr->shmid == shmid)
      return segptr;
    else if (segptr->shmid > shmid) // The list is sorted by shmid.
      return NULL;
    else if (previous)
      *previous = segptr;

  return NULL;
}

/*---------------------------------------------------------------------------*
 * server_shmmgr::new_segment ()
 *---------------------------------------------------------------------------*/

int
server_shmmgr::new_segment (const key_t key,
			    const size_t size,
			    const int shmflg,
			    const pid_t pid,
			    const uid_t uid,
			    const gid_t gid)
{
  const HANDLE hFileMap = CreateFileMapping (INVALID_HANDLE_VALUE,
					     NULL, PAGE_READWRITE,
					     0, size,
					     NULL);

  if (!hFileMap)
    {
      with_strerr (msg,
		   syscall_printf (("failed to create file mapping "
				    "[size = %lu]: %s"),
				   size, msg));

      return -EINVAL;		// FIXME
    }

  segment_t *const segptr = new_segment (key, hFileMap);

  segptr->ds.shm_perm.cuid = segptr->ds.shm_perm.uid = uid;
  segptr->ds.shm_perm.cgid = segptr->ds.shm_perm.gid = gid;
  segptr->ds.shm_perm.mode = shmflg & 0777;
  segptr->ds.shm_segsz = size;
  segptr->ds.shm_cpid = pid;
  segptr->ds.shm_ctime = time (NULL); // FIXME: sub-second times.

  return segptr->shmid;
}

/*---------------------------------------------------------------------------*
 * server_shmmgr::new_segment ()
 *
 * Allocate a new segment for the given key and file map with the
 * lowest available shmid and insert into the segment map.
 *---------------------------------------------------------------------------*/

server_shmmgr::segment_t *
server_shmmgr::new_segment (const key_t key, const HANDLE hFileMap)
{
  int shmid = ipc_int2ext (0, IPC_SHMOP); // Next expected shmid value.
  segment_t *previous = NULL;	// Insert pointer.

  // Find first unallocated shmid.
  for ( segment_t *segptr = _segments_head;
	segptr && segptr->shmid == shmid;
	segptr = segptr->next, shmid = ipc_inc_id (shmid, IPC_SHMOP))
    {
      previous = segptr;
    }

  segment_t *const segptr = new segment_t (key, shmid, hFileMap);

  assert (segptr);

  if (previous)
    {
      segptr->next = previous->next;
      previous->next = segptr;
    }
  else
    {
      segptr->next = _segments_head;
      _segments_head = segptr;
    }

  _shmid_cnt += 1;
  if (shmid > _shmid_max)
    _shmid_max = shmid;

  return segptr;
}

/*---------------------------------------------------------------------------*
 * server_shmmgr::delete_segment ()
 *---------------------------------------------------------------------------*/

void
server_shmmgr::delete_segment (segment_t *const segptr)
{
  assert (segptr);
  assert (!segptr->ds.shm_nattch);
  assert (segptr->flg & segment_t::REMOVED);

  segment_t *previous = NULL;

  const segment_t *const tmp = find (segptr->shmid, &previous);

  assert (tmp == segptr);
  assert (previous ? previous->next == segptr : _segments_head == segptr);

  if (previous)
    previous->next = segptr->next;
  else
    _segments_head = segptr->next;

  if (!CloseHandle (segptr->hFileMap))
    with_strerr (msg,
		 syscall_printf (("failed to close file map "
				  "[handle = %p]: %s"),
				 segptr->hFileMap, msg));

  assert (_shmid_cnt > 0);
  _shmid_cnt -= 1;

  delete segptr;
}

/*---------------------------------------------------------------------------*
 * client_request_shm::client_request_shm ()
 *---------------------------------------------------------------------------*/

client_request_shm::client_request_shm ()
  : client_request (CYGSERVER_REQUEST_SHM,
		    &_parameters, sizeof (_parameters))
{
  syscall_printf ("created");
}

/*---------------------------------------------------------------------------*
 * client_request_shm::serve ()
 *---------------------------------------------------------------------------*/

void
client_request_shm::serve (transport_layer_base *const conn,
			   process_cache *const cache)
{
  assert (conn);

  assert (!error_code ());

  if (msglen () != sizeof (_parameters))
    {
      syscall_printf ("bad request body length: expecting %lu bytes, got %lu",
		      sizeof (_parameters), msglen ());
      error_code (EINVAL);
      msglen (0);
      return;
    }

  // FIXME: Get a return code out of this and don't continue on error.
  conn->impersonate_client ();

  int result = -EINVAL;

  switch (_parameters.in.shmop)
    {
    case SHMOP_shmget:
      result = shmmgr.shmget (_parameters.in.key, _parameters.in.size,
			      _parameters.in.shmflg, _parameters.in.cygpid,
			      _parameters.in.uid, _parameters.in.gid);
      _parameters.shmid = result;
      break;

    case SHMOP_shmat:
      result = shmmgr.shmat (_parameters.shmid, _parameters.in.shmflg,
			     _parameters.in.cygpid, _parameters.out.hFileMap,
			     cache, _parameters.in.winpid);
      break;

    case SHMOP_shmdt:
      result = shmmgr.shmdt (_parameters.shmid, _parameters.in.cygpid);
      break;

    case SHMOP_shmctl:
      result = shmmgr.shmctl (_parameters.shmid, _parameters.in.cmd,
			      _parameters.ds, _parameters.in.cygpid);
      break;
    }

  conn->revert_to_self ();

  if (result < 0)
    {
      error_code (-result);
      msglen (0);
    }
}
