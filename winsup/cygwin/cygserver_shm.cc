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
	  system_printf ("failed to free memory at 0x%p, error = %lu",	\
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

    static long sequence;

    const int intid;
    const int shmid;
    shmid_ds ds;
    int flg;
    const HANDLE hFileMap;

    segment_t *next;

    segment_t (const key_t key, const int intid, const HANDLE hFileMap)
      : intid (intid),
	shmid (ipc_int2ext (intid, IPC_SHMOP, sequence)),
	flg (0),
	hFileMap (hFileMap),
	next (NULL)
    {
      assert (0 <= intid && intid < SHMMNI);

      memset (&ds, '\0', sizeof (ds));
      ds.shm_perm.key = key;
    }
  };

public:
  static server_shmmgr & instance ();

  int shmat (HANDLE & hFileMap,
	     int shmid, int shmflg, pid_t, process_cache *, DWORD winpid);
  int shmctl (int & out_shmid, shmid_ds & out_ds, shminfo & out_info,
	      const int shmid, int cmd, const shmid_ds &, pid_t);
  int shmdt (int shmid, pid_t);
  int shmget (int & out_shmid, key_t, size_t, int shmflg, pid_t, uid_t, gid_t);

private:
  static server_shmmgr *_instance;
  static pthread_once_t _instance_once;

  static void initialise_instance ();

  CRITICAL_SECTION _segments_lock;
  segment_t *_segments_head;	// A list sorted by int_id.

  int _shmseg_cnt;		// Number of shm segments (for ipcs(8)).
  int _intid_max;		// Highest intid yet allocated (for ipcs(8)).

  server_shmmgr ();
  ~server_shmmgr ();

  // Undefined (as this class is a singleton):
  server_shmmgr (const server_shmmgr &);
  server_shmmgr & operator= (const server_shmmgr &);

  segment_t *find_by_key (key_t);
  segment_t *find (int intid, segment_t **previous = NULL);

  int new_segment (key_t, size_t, int shmflg, pid_t, uid_t, gid_t);

  segment_t *new_segment (key_t, HANDLE);
  void delete_segment (segment_t *);
};

/* static */ long server_shmmgr::segment_t::sequence = 0;

/* static */ server_shmmgr *server_shmmgr::_instance = NULL;
/* static */ pthread_once_t server_shmmgr::_instance_once = PTHREAD_ONCE_INIT;

/*---------------------------------------------------------------------------*
 * server_shmmgr::instance ()
 *---------------------------------------------------------------------------*/

/* static */ server_shmmgr &
server_shmmgr::instance ()
{
  pthread_once (&_instance_once, &initialise_instance);

  assert (_instance);

  return *_instance;
}

/*---------------------------------------------------------------------------*
 * server_shmmgr::shmat ()
 *---------------------------------------------------------------------------*/

int
server_shmmgr::shmat (HANDLE & hFileMap,
		      const int shmid, const int shmflg,
		      const pid_t cygpid,
		      process_cache *const cache, const DWORD winpid)
{
  syscall_printf ("shmat (shmid = %d, shmflg = 0%o) for %d(%lu)",
		  shmid, shmflg, cygpid, winpid);

  process *const client = cache->process (winpid);

  if (!client)
    {
      syscall_printf (("-1 [%d] = shmat (shmid = %d, shmflg = 0%o) "
		       "for %d(%lu)"),
		      0, EAGAIN, shmid, shmflg,
		      cygpid, winpid);
      return -EAGAIN;
    }

  int result = 0;
  EnterCriticalSection (&_segments_lock);

  segment_t *const segptr = find (ipc_ext2int (shmid, IPC_SHMOP));

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
				    "[key = 0x%016llx, shmid = %d, handle = 0x%x]:"
				    "%s"),
				   segptr->ds.shm_perm.key, segptr->shmid,
				   segptr->hFileMap, msg));

      result = -EACCES;		// FIXME
    }
  else
    {
      segptr->ds.shm_lpid  = cygpid;
      segptr->ds.shm_nattch += 1;
      segptr->ds.shm_atime = time (NULL); // FIXME: sub-second times.
    }

  LeaveCriticalSection (&_segments_lock);

  client->release ();

  if (result < 0)
    syscall_printf ("-1 [%d] = shmat (shmid = %d, shmflg = 0%o) for %d(%lu)",
		    -result, shmid, shmflg, cygpid, winpid);
  else
    syscall_printf ("0x%x = shmat (shmid = %d, shmflg = 0%o) for %d(%lu)",
		    hFileMap, shmid, shmflg, cygpid, winpid);

  return result;
}

/*---------------------------------------------------------------------------*
 * server_shmmgr::shmctl ()
 *---------------------------------------------------------------------------*/

int
server_shmmgr::shmctl (int & out_shmid, shmid_ds & out_ds, shminfo & out_info,
		       const int shmid, const int cmd, const shmid_ds & ds,
		       const pid_t cygpid)
{
  syscall_printf ("shmctl (shmid = %d, cmd = 0x%x) for %d",
		  shmid, cmd, cygpid);

  int result = 0;
  EnterCriticalSection (&_segments_lock);

  switch (cmd)
    {
    case IPC_STAT:
    case SHM_STAT:		// Uses intids rather than shmids.
    case IPC_SET:
    case IPC_RMID:
      {
	int intid;

	if (cmd == SHM_STAT)
	  intid = shmid;
	else
	  intid = ipc_ext2int (shmid, IPC_SHMOP);

	segment_t *const segptr = find (intid);

	if (!segptr)
	  result = -EINVAL;
	else
	  switch (cmd)
	    {
	    case IPC_STAT:
	      out_ds = segptr->ds;
	      break;

	    case IPC_SET:
	      segptr->ds.shm_perm.uid = ds.shm_perm.uid;
	      segptr->ds.shm_perm.gid = ds.shm_perm.gid;
	      segptr->ds.shm_perm.mode = ds.shm_perm.mode & 0777;
	      segptr->ds.shm_lpid = cygpid;
	      segptr->ds.shm_ctime = time (NULL); // FIXME: sub-second times.
	      break;

	    case IPC_RMID:
	      if (segptr->flg & segment_t::REMOVED)
		result = -EIDRM;
	      else
		{
		  segptr->flg |= segment_t::REMOVED;
		  if (!segptr->ds.shm_nattch)
		    delete_segment (segptr);
		}
	      break;

	    case SHM_STAT:	// ipcs(8) i'face.
	      out_ds = segptr->ds;
	      out_shmid = segptr->shmid;
	      break;
	    }
      }
      break;

    case IPC_INFO:
      out_info.shmmax = SHMMAX;
      out_info.shmmin = SHMMIN;
      out_info.shmmni = SHMMNI;
      out_info.shmseg = SHMSEG;
      out_info.shmall = SHMALL;
      break;

    case SHM_INFO:		// ipcs(8) i'face.
      out_shmid = _intid_max;
      break;

    default:
      result = -EINVAL;
      break;
    }

  LeaveCriticalSection (&_segments_lock);

  if (result < 0)
    syscall_printf (("-1 [%d] = "
		     "shmctl (shmid = %d, cmd = 0x%x) for %d"),
		    -result,
		    shmid, cmd, cygpid);
  else
    syscall_printf (("%d = "
		     "shmctl (shmid = %d, cmd = 0x%x) for %d"),
		    ((cmd == SHM_STAT || cmd == SHM_INFO)
		     ? out_shmid
		     : result),
		    shmid, cmd, cygpid);

  return result;
}

/*---------------------------------------------------------------------------*
 * server_shmmgr::shmdt ()
 *---------------------------------------------------------------------------*/

int
server_shmmgr::shmdt (const int shmid, const pid_t cygpid)
{
  syscall_printf ("shmdt (shmid = %d) for %d",
		  shmid, cygpid);

  int result = 0;
  EnterCriticalSection (&_segments_lock);

  segment_t *const segptr = find (ipc_ext2int (shmid, IPC_SHMOP));

  if (!segptr)
    result = -EINVAL;
  else
    {
      assert (segptr->ds.shm_nattch > 0);

      segptr->ds.shm_lpid  = cygpid;
      segptr->ds.shm_nattch -= 1;
      segptr->ds.shm_dtime = time (NULL); // FIXME: sub-second times.

      if (!segptr->ds.shm_nattch && (segptr->flg & segment_t::REMOVED))
	delete_segment (segptr);
    }

  LeaveCriticalSection (&_segments_lock);

  if (result < 0)
    syscall_printf ("-1 [%d] = shmdt (shmid = %d) for %d",
		    -result, shmid, cygpid);
  else
    syscall_printf ("%d = shmdt (shmid = %d) for %d",
		    result, shmid, cygpid);

  return result;
}

/*---------------------------------------------------------------------------*
 * server_shmmgr::shmget ()
 *---------------------------------------------------------------------------*/

int
server_shmmgr::shmget (int & out_shmid,
		       const key_t key, const size_t size, const int shmflg,
		       const pid_t cygpid, const uid_t uid, const gid_t gid)
{
  syscall_printf ("shmget (key = 0x%016llx, size = %u, shmflg = 0%o) for %d",
		  key, size, shmflg,
		  cygpid);

  int result = 0;
  EnterCriticalSection (&_segments_lock);

  /* Does a segment already exist with that key? */
  if (key == IPC_PRIVATE)
    result = new_segment (key, size, shmflg, cygpid, uid, gid);
  else
    {
      segment_t *const segptr = find_by_key (key);

      if (!segptr)
	if (shmflg & IPC_CREAT)
	  result = new_segment (key, size, shmflg, cygpid, uid, gid);
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

  if (result >= 0)
    {
      out_shmid = result;
      result = 0;
    }

  if (result < 0)
    syscall_printf (("-1 [%d] = "
		     "shmget (key = 0x%016llx, size = %u, shmflg = 0%o) "
		     "for %d"),
		    -result,
		    key, size, shmflg,
		    cygpid);
  else
    syscall_printf (("%d = "
		     "shmget (key = 0x%016llx, size = %u, shmflg = 0%o) "
		     "for %d"),
		    out_shmid,
		    key, size, shmflg,
		    cygpid);

  return result;
}

/*---------------------------------------------------------------------------*
 * server_shmmgr::initialise_instance ()
 *---------------------------------------------------------------------------*/

/* static */ void
server_shmmgr::initialise_instance ()
{
  assert (!_instance);

  _instance = safe_new0 (server_shmmgr);
}

/*---------------------------------------------------------------------------*
 * server_shmmgr::server_shmmgr ()
 *---------------------------------------------------------------------------*/

server_shmmgr::server_shmmgr ()
  : _segments_head (NULL),
    _shmseg_cnt (0),
    _intid_max (0)
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
    if (segptr->ds.shm_perm.key == key)
      return segptr;

  return NULL;
}

/*---------------------------------------------------------------------------*
 * server_shmmgr::find ()
 *---------------------------------------------------------------------------*/

server_shmmgr::segment_t *
server_shmmgr::find (const int intid, segment_t **previous)
{
  if (previous)
    *previous = NULL;

  for (segment_t *segptr = _segments_head; segptr; segptr = segptr->next)
    if (segptr->intid == intid)
      return segptr;
    else if (segptr->intid > intid) // The list is sorted by intid.
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
			    const pid_t cygpid,
			    const uid_t uid,
			    const gid_t gid)
{
  if (size < SHMMIN || size > SHMMAX)
      return -EINVAL;

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

  if (!segptr)
    {
      (void) CloseHandle (hFileMap);
      return -ENOSPC;
    }

  segptr->ds.shm_perm.cuid = segptr->ds.shm_perm.uid = uid;
  segptr->ds.shm_perm.cgid = segptr->ds.shm_perm.gid = gid;
  segptr->ds.shm_perm.mode = shmflg & 0777;
  segptr->ds.shm_segsz = size;
  segptr->ds.shm_cpid = cygpid;
  segptr->ds.shm_ctime = time (NULL); // FIXME: sub-second times.

  return segptr->shmid;
}

/*---------------------------------------------------------------------------*
 * server_shmmgr::new_segment ()
 *
 * Allocate a new segment for the given key and file map with the
 * lowest available intid and insert into the segment map.
 *---------------------------------------------------------------------------*/

server_shmmgr::segment_t *
server_shmmgr::new_segment (const key_t key, const HANDLE hFileMap)
{
  int intid = 0;		// Next expected intid value.
  segment_t *previous = NULL;	// Insert pointer.

  // Find first unallocated intid.
  for (segment_t *segptr = _segments_head;
       segptr && segptr->intid == intid;
       segptr = segptr->next, intid++)
    {
      previous = segptr;
    }

  /* By the time this condition is reached (given the default value of
   * SHMMNI), the linear searches should all replaced by something
   * just a *little* cleverer . . .
   */
  if (intid >= SHMMNI)
    return NULL;

  segment_t *const segptr = safe_new (segment_t, key, intid, hFileMap);

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

  _shmseg_cnt += 1;
  if (intid > _intid_max)
    _intid_max = intid;

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

  const segment_t *const tmp = find (segptr->intid, &previous);

  assert (tmp == segptr);
  assert (previous ? previous->next == segptr : _segments_head == segptr);

  if (previous)
    previous->next = segptr->next;
  else
    _segments_head = segptr->next;

  if (!CloseHandle (segptr->hFileMap))
    with_strerr (msg,
		 syscall_printf (("failed to close file map "
				  "[handle = 0x%x]: %s"),
				 segptr->hFileMap, msg));

  assert (_shmseg_cnt > 0);
  _shmseg_cnt -= 1;

  safe_delete (segment_t, segptr);
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

  if (msglen () != sizeof (_parameters.in))
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
      result = shmmgr.shmget (_parameters.out.shmid,
			      _parameters.in.key, _parameters.in.size,
			      _parameters.in.shmflg, _parameters.in.cygpid,
			      _parameters.in.uid, _parameters.in.gid);
      break;

    case SHMOP_shmat:
      result = shmmgr.shmat (_parameters.out.hFileMap,
			     _parameters.in.shmid, _parameters.in.shmflg,
			     _parameters.in.cygpid,
			     cache, _parameters.in.winpid);
      break;

    case SHMOP_shmdt:
      result = shmmgr.shmdt (_parameters.in.shmid, _parameters.in.cygpid);
      break;

    case SHMOP_shmctl:
      result = shmmgr.shmctl (_parameters.out.shmid,
			      _parameters.out.ds, _parameters.out.info,
			      _parameters.in.shmid, _parameters.in.cmd,
			      _parameters.in.ds, _parameters.in.cygpid);
      break;
    }

  conn->revert_to_self ();

  if (result < 0)
    {
      error_code (-result);
      msglen (0);
    }
  else
    msglen (sizeof (_parameters.out));
}
