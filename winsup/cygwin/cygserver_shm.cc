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
 * class server_shmmgr
 *
 * A singleton class.
 *---------------------------------------------------------------------------*/

#define shmmgr (server_shmmgr::instance ())

class server_shmmgr
{
private:
  class attach_t
  {
  public:
    class process *const _client;
    unsigned int _refcnt;

    attach_t *_next;

    attach_t (class process *const client)
      : _client (client),
	_refcnt (0),
	_next (NULL)
    {}
  };

  class segment_t
  {
  private:
    // Bits for the _flg field.
    enum { IS_DELETED = 0x01 };

  public:
    const int _intid;
    const int _shmid;
    struct shmid_ds _ds;

    segment_t *_next;

    segment_t (const key_t key, const int intid, const HANDLE hFileMap);
    ~segment_t ();

    bool is_deleted () const
    {
      return _flg & IS_DELETED;
    }

    bool is_pending_delete () const
    {
      return !_ds.shm_nattch && is_deleted ();
    }

    void mark_deleted ()
    {
      assert (!is_deleted ());

      _flg |= IS_DELETED;
    }

    int attach (class process *, HANDLE & hFileMap);
    int detach (class process *);

  private:
    static long _sequence;

    int _flg;
    const HANDLE _hFileMap;
    attach_t *_attach_head;	// A list sorted by winpid;

    attach_t *find (const class process *, attach_t **previous = NULL);
  };

  class cleanup_t : public cleanup_routine
  {
  public:
    cleanup_t (const segment_t *const segptr)
      : cleanup_routine (reinterpret_cast<void *> (segptr->_shmid))
    {
      assert (key ());
    }

    int shmid () const { return reinterpret_cast<int> (key ()); }

    virtual void cleanup (class process *const client)
    {
      const int res = shmmgr.shmdt (shmid (), client);

      if (res != 0)
	debug_printf ("process cleanup failed [shmid = %d]: %s",
		      shmid (), strerror (-res));
    }
  };

public:
  static server_shmmgr & instance ();

  int shmat (HANDLE & hFileMap,
	     int shmid, int shmflg, class process *);
  int shmctl (int & out_shmid, struct shmid_ds & out_ds,
	      struct shminfo & out_shminfo, struct shm_info & out_shm_info,
	      const int shmid, int cmd, const struct shmid_ds &,
	      class process *);
  int shmdt (int shmid, class process *);
  int shmget (int & out_shmid, key_t, size_t, int shmflg, uid_t, gid_t,
	      class process *);

private:
  static server_shmmgr *_instance;
  static pthread_once_t _instance_once;

  static void initialise_instance ();

  CRITICAL_SECTION _segments_lock;
  segment_t *_segments_head;	// A list sorted by int_id.

  int _shm_ids;			// Number of shm segments (for ipcs(8)).
  int _shm_tot;			// Total bytes of shm segments (for ipcs(8)).
  int _shm_atts;		// Number of attached segments (for ipcs(8)).
  int _intid_max;		// Highest intid yet allocated (for ipcs(8)).

  server_shmmgr ();
  ~server_shmmgr ();

  // Undefined (as this class is a singleton):
  server_shmmgr (const server_shmmgr &);
  server_shmmgr & operator= (const server_shmmgr &);

  segment_t *find_by_key (key_t);
  segment_t *find (int intid, segment_t **previous = NULL);

  int new_segment (key_t, size_t, int shmflg, pid_t, uid_t, gid_t);

  segment_t *new_segment (key_t, size_t, HANDLE);
  void delete_segment (segment_t *);
};

/* static */ long server_shmmgr::segment_t::_sequence = 0;

/* static */ server_shmmgr *server_shmmgr::_instance = NULL;
/* static */ pthread_once_t server_shmmgr::_instance_once = PTHREAD_ONCE_INIT;

/*---------------------------------------------------------------------------*
 * server_shmmgr::segment_t::segment_t ()
 *---------------------------------------------------------------------------*/

server_shmmgr::segment_t::segment_t (const key_t key,
				     const int intid,
				     const HANDLE hFileMap)
  : _intid (intid),
    _shmid (ipc_int2ext (intid, IPC_SHMOP, _sequence)),
    _next (NULL),
    _flg (0),
    _hFileMap (hFileMap),
    _attach_head (NULL)
{
  assert (0 <= _intid && _intid < SHMMNI);

  memset (&_ds, '\0', sizeof (_ds));
  _ds.shm_perm.key = key;
}

/*---------------------------------------------------------------------------*
 * server_shmmgr::segment_t::~segment_t ()
 *---------------------------------------------------------------------------*/

server_shmmgr::segment_t::~segment_t ()
{
  assert (!_attach_head);

  if (!CloseHandle (_hFileMap))
    syscall_printf ("failed to close file map [handle = 0x%x]: %E", _hFileMap);
}

/*---------------------------------------------------------------------------*
 * server_shmmgr::segment_t::attach ()
 *---------------------------------------------------------------------------*/

int
server_shmmgr::segment_t::attach (class process *const client,
				  HANDLE & hFileMap)
{
  assert (client);

  if (!DuplicateHandle (GetCurrentProcess (),
			_hFileMap,
			client->handle (),
			&hFileMap,
			0,
			FALSE, // bInheritHandle
			DUPLICATE_SAME_ACCESS))
    {
      syscall_printf (("failed to duplicate handle for client "
		       "[key = 0x%016llx, shmid = %d, handle = 0x%x]: %E"),
		      _ds.shm_perm.key, _shmid, _hFileMap);

      return -EACCES;	// FIXME: Case analysis?
    }

  _ds.shm_lpid  = client->cygpid ();
  _ds.shm_nattch += 1;
  _ds.shm_atime = time (NULL); // FIXME: sub-second times.

  attach_t *previous = NULL;
  attach_t *attptr = find (client, &previous);

  if (!attptr)
    {
      attptr = safe_new (attach_t, client);

      if (previous)
	{
	  attptr->_next = previous->_next;
	  previous->_next = attptr;
	}
      else
	{
	  attptr->_next = _attach_head;
	  _attach_head = attptr;
	}
    }

  attptr->_refcnt += 1;

  cleanup_t *const cleanup = safe_new (cleanup_t, this);

  // FIXME: ::add should only fail if the process object is already
  // cleaning up; but it can't be doing that since this thread has it
  // locked.

  const bool result = client->add (cleanup);

  assert (result);

  return 0;
}

/*---------------------------------------------------------------------------*
 * server_shmmgr::segment_t::detach ()
 *---------------------------------------------------------------------------*/

int
server_shmmgr::segment_t::detach (class process *const client)
{
  attach_t *previous = NULL;
  attach_t *const attptr = find (client, &previous);

  if (!attptr)
    return -EINVAL;

  if (client->is_active ())
    {
      const cleanup_t key (this);

      if (!client->remove (&key))
	syscall_printf (("failed to remove cleanup routine for %d(%lu) "
			 "[shmid = %d]"),
			client->cygpid (), client->winpid (),
			_shmid);
    }

  attptr->_refcnt -= 1;

  if (!attptr->_refcnt)
    {
      assert (previous ? previous->_next == attptr : _attach_head == attptr);

      if (previous)
	previous->_next = attptr->_next;
      else
	_attach_head = attptr->_next;

      safe_delete (attptr);
    }

  assert (_ds.shm_nattch > 0);

  _ds.shm_lpid  = client->cygpid ();
  _ds.shm_nattch -= 1;
  _ds.shm_dtime = time (NULL); // FIXME: sub-second times.

  return 0;
}

/*---------------------------------------------------------------------------*
 * server_shmmgr::segment_t::find ()
 *---------------------------------------------------------------------------*/

server_shmmgr::attach_t *
server_shmmgr::segment_t::find (const class process *const client,
				attach_t **previous)
{
  if (previous)
    *previous = NULL;

  // Nb. The _attach_head list is sorted by winpid.

  for (attach_t *attptr = _attach_head; attptr; attptr = attptr->_next)
    if (attptr->_client == client)
      return attptr;
    else if (attptr->_client->winpid () > client->winpid ())
      return NULL;
    else if (previous)
      *previous = attptr;

  return NULL;
}

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
		      class process *const client)
{
  syscall_printf ("shmat (shmid = %d, shmflg = 0%o) for %d(%lu)",
		  shmid, shmflg, client->cygpid (), client->winpid ());

  int result = 0;
  EnterCriticalSection (&_segments_lock);

  segment_t *const segptr = find (ipc_ext2int (shmid, IPC_SHMOP));

  if (!segptr)
    result = -EINVAL;
  else
    result = segptr->attach (client, hFileMap);

  if (!result)
    _shm_atts += 1;

  LeaveCriticalSection (&_segments_lock);

  if (result < 0)
    syscall_printf (("-1 [%d] = shmat (shmid = %d, shmflg = 0%o) "
		     "for %d(%lu)"),
		    -result, shmid, shmflg,
		    client->cygpid (), client->winpid ());
  else
    syscall_printf (("0x%x = shmat (shmid = %d, shmflg = 0%o) "
		     "for %d(%lu)"),
		    hFileMap, shmid, shmflg,
		    client->cygpid (), client->winpid ());

  return result;
}

/*---------------------------------------------------------------------------*
 * server_shmmgr::shmctl ()
 *---------------------------------------------------------------------------*/

int
server_shmmgr::shmctl (int & out_shmid,
		       struct shmid_ds & out_ds,
		       struct shminfo & out_shminfo,
		       struct shm_info & out_shm_info,
		       const int shmid, const int cmd,
		       const struct shmid_ds & ds,
		       class process *const client)
{
  syscall_printf ("shmctl (shmid = %d, cmd = 0x%x) for %d(%lu)",
		  shmid, cmd, client->cygpid (), client->winpid ());

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
	      out_ds = segptr->_ds;
	      break;

	    case IPC_SET:
	      segptr->_ds.shm_perm.uid = ds.shm_perm.uid;
	      segptr->_ds.shm_perm.gid = ds.shm_perm.gid;
	      segptr->_ds.shm_perm.mode = ds.shm_perm.mode & 0777;
	      segptr->_ds.shm_lpid = client->cygpid ();
	      segptr->_ds.shm_ctime = time (NULL); // FIXME: sub-second times.
	      break;

	    case IPC_RMID:
	      if (segptr->is_deleted ())
		result = -EIDRM;
	      else
		{
		  segptr->mark_deleted ();
		  if (segptr->is_pending_delete ())
		    delete_segment (segptr);
		}
	      break;

	    case SHM_STAT:	// ipcs(8) i'face.
	      out_ds = segptr->_ds;
	      out_shmid = segptr->_shmid;
	      break;
	    }
      }
      break;

    case IPC_INFO:
      out_shminfo.shmmax = SHMMAX;
      out_shminfo.shmmin = SHMMIN;
      out_shminfo.shmmni = SHMMNI;
      out_shminfo.shmseg = SHMSEG;
      out_shminfo.shmall = SHMALL;
      break;

    case SHM_INFO:		// ipcs(8) i'face.
      out_shmid = _intid_max;
      out_shm_info.shm_ids = _shm_ids;
      out_shm_info.shm_tot = _shm_tot;
      out_shm_info.shm_atts = _shm_atts;
      break;

    default:
      result = -EINVAL;
      break;
    }

  LeaveCriticalSection (&_segments_lock);

  if (result < 0)
    syscall_printf (("-1 [%d] = "
		     "shmctl (shmid = %d, cmd = 0x%x) for %d(%lu)"),
		    -result,
		    shmid, cmd, client->cygpid (), client->winpid ());
  else
    syscall_printf (("%d = "
		     "shmctl (shmid = %d, cmd = 0x%x) for %d(%lu)"),
		    ((cmd == SHM_STAT || cmd == SHM_INFO)
		     ? out_shmid
		     : result),
		    shmid, cmd, client->cygpid (), client->winpid ());

  return result;
}

/*---------------------------------------------------------------------------*
 * server_shmmgr::shmdt ()
 *---------------------------------------------------------------------------*/

int
server_shmmgr::shmdt (const int shmid, class process *const client)
{
  syscall_printf ("shmdt (shmid = %d) for %d(%lu)",
		  shmid, client->cygpid (), client->winpid ());

  int result = 0;
  EnterCriticalSection (&_segments_lock);

  segment_t *const segptr = find (ipc_ext2int (shmid, IPC_SHMOP));

  if (!segptr)
    result = -EINVAL;
  else
    result = segptr->detach (client);

  if (!result)
    _shm_atts -= 1;

  if (!result && segptr->is_pending_delete ())
    delete_segment (segptr);

  LeaveCriticalSection (&_segments_lock);

  if (result < 0)
    syscall_printf ("-1 [%d] = shmdt (shmid = %d) for %d(%lu)",
		    -result, shmid, client->cygpid (), client->winpid ());
  else
    syscall_printf ("%d = shmdt (shmid = %d) for %d(%lu)",
		    result, shmid, client->cygpid (), client->winpid ());

  return result;
}

/*---------------------------------------------------------------------------*
 * server_shmmgr::shmget ()
 *---------------------------------------------------------------------------*/

int
server_shmmgr::shmget (int & out_shmid,
		       const key_t key, const size_t size, const int shmflg,
		       const uid_t uid, const gid_t gid,
		       class process *const client)
{
  syscall_printf (("shmget (key = 0x%016llx, size = %u, shmflg = 0%o) "
		   "for %d(%lu)"),
		  key, size, shmflg,
		  client->cygpid (), client->winpid ());

  int result = 0;
  EnterCriticalSection (&_segments_lock);

  if (key == IPC_PRIVATE)
    result = new_segment (key, size, shmflg,
			  client->cygpid (), uid, gid);
  else
    {
      segment_t *const segptr = find_by_key (key);

      if (!segptr)
	if (shmflg & IPC_CREAT)
	  result = new_segment (key, size, shmflg,
				client->cygpid (), uid, gid);
	else
	  result = -ENOENT;
      else if (segptr->is_deleted ())
	result = -EIDRM;
      else if ((shmflg & IPC_CREAT) && (shmflg & IPC_EXCL))
	result = -EEXIST;
      else if ((shmflg & ~(segptr->_ds.shm_perm.mode)) & 0777)
	result = -EACCES;
      else if (size && segptr->_ds.shm_segsz < size)
	result = -EINVAL;
      else
	result = segptr->_shmid;
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
		     "for %d(%lu)"),
		    -result,
		    key, size, shmflg,
		    client->cygpid (), client->winpid ());
  else
    syscall_printf (("%d = "
		     "shmget (key = 0x%016llx, size = %u, shmflg = 0%o) "
		     "for %d(%lu)"),
		    out_shmid,
		    key, size, shmflg,
		    client->cygpid (), client->winpid ());

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

  assert (_instance);
}

/*---------------------------------------------------------------------------*
 * server_shmmgr::server_shmmgr ()
 *---------------------------------------------------------------------------*/

server_shmmgr::server_shmmgr ()
  : _segments_head (NULL),
    _shm_ids (0),
    _shm_tot (0),
    _shm_atts (0),
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
  for (segment_t *segptr = _segments_head; segptr; segptr = segptr->_next)
    if (segptr->_ds.shm_perm.key == key)
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

  for (segment_t *segptr = _segments_head; segptr; segptr = segptr->_next)
    if (segptr->_intid == intid)
      return segptr;
    else if (segptr->_intid > intid) // The list is sorted by intid.
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
      syscall_printf ("failed to create file mapping [size = %lu]: %E", size);
      return -ENOMEM;		// FIXME
    }

  segment_t *const segptr = new_segment (key, size, hFileMap);

  if (!segptr)
    {
      (void) CloseHandle (hFileMap);
      return -ENOSPC;
    }

  segptr->_ds.shm_perm.cuid = segptr->_ds.shm_perm.uid = uid;
  segptr->_ds.shm_perm.cgid = segptr->_ds.shm_perm.gid = gid;
  segptr->_ds.shm_perm.mode = shmflg & 0777;
  segptr->_ds.shm_segsz = size;
  segptr->_ds.shm_cpid = cygpid;
  segptr->_ds.shm_ctime = time (NULL); // FIXME: sub-second times.

  return segptr->_shmid;
}

/*---------------------------------------------------------------------------*
 * server_shmmgr::new_segment ()
 *
 * Allocate a new segment for the given key and file map with the
 * lowest available intid and insert into the segment map.
 *---------------------------------------------------------------------------*/

server_shmmgr::segment_t *
server_shmmgr::new_segment (const key_t key, const size_t size,
			    const HANDLE hFileMap)
{
  // FIXME: Overflow risk.
  if (_shm_tot + size > SHMALL)
    return NULL;

  int intid = 0;		// Next expected intid value.
  segment_t *previous = NULL;	// Insert pointer.

  // Find first unallocated intid.
  for (segment_t *segptr = _segments_head;
       segptr && segptr->_intid == intid;
       segptr = segptr->_next, intid++)
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
      segptr->_next = previous->_next;
      previous->_next = segptr;
    }
  else
    {
      segptr->_next = _segments_head;
      _segments_head = segptr;
    }

  _shm_ids += 1;
  _shm_tot += size;
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
  assert (segptr->is_pending_delete ());

  segment_t *previous = NULL;

  const segment_t *const tmp = find (segptr->_intid, &previous);

  assert (tmp == segptr);
  assert (previous ? previous->_next == segptr : _segments_head == segptr);

  if (previous)
    previous->_next = segptr->_next;
  else
    _segments_head = segptr->_next;

  assert (_shm_ids > 0);
  _shm_ids -= 1;
  _shm_tot -= segptr->_ds.shm_segsz;

  safe_delete (segptr);
}

/*---------------------------------------------------------------------------*
 * client_request_shm::client_request_shm ()
 *---------------------------------------------------------------------------*/

client_request_shm::client_request_shm ()
  : client_request (CYGSERVER_REQUEST_SHM,
		    &_parameters, sizeof (_parameters))
{
  // verbose: syscall_printf ("created");
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

  class process *const client = cache->process (_parameters.in.cygpid,
						_parameters.in.winpid);

  if (!client)
    {
      error_code (EAGAIN);
      msglen (0);
      return;
    }

  int result = -EINVAL;

  switch (_parameters.in.shmop)
    {
    case SHMOP_shmget:
      result = shmmgr.shmget (_parameters.out.shmid,
			      _parameters.in.key, _parameters.in.size,
			      _parameters.in.shmflg,
			      _parameters.in.uid, _parameters.in.gid,
			      client);
      break;

    case SHMOP_shmat:
      result = shmmgr.shmat (_parameters.out.hFileMap,
			     _parameters.in.shmid, _parameters.in.shmflg,
			     client);
      break;

    case SHMOP_shmdt:
      result = shmmgr.shmdt (_parameters.in.shmid, client);
      break;

    case SHMOP_shmctl:
      result = shmmgr.shmctl (_parameters.out.shmid,
			      _parameters.out.ds, _parameters.out.shminfo,
			      _parameters.out.shm_info,
			      _parameters.in.shmid, _parameters.in.cmd,
			      _parameters.in.ds,
			      client);
      break;
    }

  client->release ();
  conn->revert_to_self ();

  if (result < 0)
    {
      error_code (-result);
      msglen (0);
    }
  else
    msglen (sizeof (_parameters.out));
}
