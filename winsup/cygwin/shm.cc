/* shm.cc: Single unix specification IPC interface for Cygwin.

   Copyright 2002 Red Hat, Inc.

   Written by Conrad Scott <conrad.scott@dsl.pipex.com>.
   Based on code by Robert Collins <robert.collins@hotmail.com>.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"

#include <sys/types.h>

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>

#include "cygerrno.h"
#include "safe_memory.h"
#include "sigproc.h"

#include "cygserver_ipc.h"
#include "cygserver_shm.h"

/*---------------------------------------------------------------------------*
 * class client_shmmgr
 *
 * A singleton class.
 *---------------------------------------------------------------------------*/

#define shmmgr (client_shmmgr::instance ())

class client_shmmgr
{
private:
  class segment_t
  {
  public:
    const int shmid;
    const void *const shmaddr;
    const int shmflg;
    HANDLE hFileMap;		// Updated by fixup_shms_after_fork ().

    segment_t *next;

    segment_t (const int shmid, const void *const shmaddr, const int shmflg,
	       const HANDLE hFileMap)
      : shmid (shmid), shmaddr (shmaddr), shmflg (shmflg), hFileMap (hFileMap),
	next (NULL)
    {}
  };

public:
  static client_shmmgr & instance ();

  void *shmat (int shmid, const void *, int shmflg);
  int shmctl (int shmid, int cmd, struct shmid_ds *);
  int shmdt (const void *);
  int shmget (key_t, size_t, int shmflg);

  int fixup_shms_after_fork ();

private:
  static NO_COPY client_shmmgr *_instance;

  CRITICAL_SECTION _segments_lock;
  static segment_t *_segments_head; // List of attached segs by shmaddr.

  static long _shmat_cnt;	// No. of attached segs; for info. only.

  client_shmmgr ();
  ~client_shmmgr ();

  // Undefined (as this class is a singleton):
  client_shmmgr (const client_shmmgr &);
  client_shmmgr & operator= (const client_shmmgr &);

  segment_t *find (const void *, segment_t **previous = NULL);

  void *attach (int shmid, const void *, int shmflg, HANDLE & hFileMap);

  segment_t *new_segment (int shmid, const void *, int shmflg, HANDLE);
};

/* static */ NO_COPY client_shmmgr *client_shmmgr::_instance;

/* The following two variables must be inherited by child processes
 * since they are used by fixup_shms_after_fork () to re-attach to the
 * parent's shm segments.
 */
/* static */ client_shmmgr::segment_t *client_shmmgr::_segments_head;
/* static */ long client_shmmgr::_shmat_cnt;

/*---------------------------------------------------------------------------*
 * client_shmmgr::instance ()
 *---------------------------------------------------------------------------*/

client_shmmgr &
client_shmmgr::instance ()
{
  if (!_instance)
    _instance = safe_new0 (client_shmmgr);

  assert (_instance);

  return *_instance;
}

/*---------------------------------------------------------------------------*
 * client_shmmgr::shmat ()
 *---------------------------------------------------------------------------*/

void *
client_shmmgr::shmat (const int shmid,
		      const void *const shmaddr,
		      const int shmflg)
{
  syscall_printf ("shmat (shmid = %d, shmaddr = %p, shmflg = 0%o)",
		  shmid, shmaddr, shmflg);

  EnterCriticalSection (&_segments_lock);

  HANDLE hFileMap = NULL;

  void *const ptr = attach (shmid, shmaddr, shmflg, hFileMap);

  if (ptr)
    new_segment (shmid, ptr, shmflg, hFileMap);

  LeaveCriticalSection (&_segments_lock);

  if (ptr)
    syscall_printf ("%p = shmat (shmid = %d, shmaddr = %p, shmflg = 0%o)",
		    ptr, shmid, shmaddr, shmflg);
  // else
    // See the syscall_printf in client_shmmgr::attach ().

  return (ptr ? ptr : (void *) -1);
}

/*---------------------------------------------------------------------------*
 * client_shmmgr::shmctl ()
 *---------------------------------------------------------------------------*/

int
client_shmmgr::shmctl (const int shmid,
		       const int cmd,
		       struct shmid_ds *const buf)
{
  syscall_printf ("shmctl (shmid = %d, cmd = 0x%x, buf = %p)",
		  shmid, cmd, buf);

  // Check parameters and set up in parameters as required.

  const struct shmid_ds *in_buf = NULL;

  switch (cmd)
    {
    case IPC_SET:
      if (__check_invalid_read_ptr_errno (buf, sizeof (struct shmid_ds)))
	{
	  syscall_printf (("-1 [EFAULT] = "
			   "shmctl (shmid = %d, cmd = 0x%x, buf = %p)"),
			  shmid, cmd, buf);
	  set_errno (EFAULT);
	  return -1;
	}
      in_buf = buf;
      break;

    case IPC_STAT:
    case SHM_STAT:
      if (__check_null_invalid_struct_errno (buf, sizeof (struct shmid_ds)))
	{
	  syscall_printf (("-1 [EFAULT] = "
			   "shmctl (shmid = %d, cmd = 0x%x, buf = %p)"),
			  shmid, cmd, buf);
	  set_errno (EFAULT);
	  return -1;
	}
      break;

    case IPC_INFO:
      if (__check_null_invalid_struct_errno (buf, sizeof (struct shminfo)))
	{
	  syscall_printf (("-1 [EFAULT] = "
			   "shmctl (shmid = %d, cmd = 0x%x, buf = %p)"),
			  shmid, cmd, buf);
	  set_errno (EFAULT);
	  return -1;
	}
      break;

    case SHM_INFO:
      if (__check_null_invalid_struct_errno (buf, sizeof (struct shm_info)))
	{
	  syscall_printf (("-1 [EFAULT] = "
			   "shmctl (shmid = %d, cmd = 0x%x, buf = %p)"),
			  shmid, cmd, buf);
	  set_errno (EFAULT);
	  return -1;
	}
      break;
    }

  // Create and issue the command.

  client_request_shm request (shmid, cmd, in_buf);

  if (request.make_request () == -1 || request.error_code ())
    {
      syscall_printf (("-1 [%d] = "
		       "shmctl (shmid = %d, cmd = 0x%x, buf = %p)"),
		      request.error_code (), shmid, cmd, buf);
      set_errno (request.error_code ());
      return -1;
    }

  // Some commands require special processing for their out parameters.

  int result = 0;

  switch (cmd)
    {
    case IPC_STAT:
      *buf = request.ds ();
      break;

    case IPC_INFO:
      *(struct shminfo *) buf = request.shminfo ();
      break;

    case SHM_STAT:		// ipcs(8) i'face.
      result = request.shmid ();
      *buf = request.ds ();
      break;

    case SHM_INFO:		// ipcs(8) i'face.
      result = request.shmid ();
      *(struct shm_info *) buf = request.shm_info ();
      break;
    }

  syscall_printf ("%d = shmctl (shmid = %d, cmd = 0x%x, buf = %p)",
		  result, shmid, cmd, buf);

  return result;
}

/*---------------------------------------------------------------------------*
 * client_shmmgr::shmdt ()
 *
 * According to Posix, the only error condition for this system call
 * is EINVAL if shmaddr is not the address of the start of an attached
 * shared memory segment.  Given that, all other errors just generate
 * tracing noise.
 *---------------------------------------------------------------------------*/

int
client_shmmgr::shmdt (const void *const shmaddr)
{
  syscall_printf ("shmdt (shmaddr = %p)", shmaddr);

  EnterCriticalSection (&_segments_lock);

  segment_t *previous = NULL;

  segment_t *const segptr = find (shmaddr, &previous);

  if (!segptr)
    {
      LeaveCriticalSection (&_segments_lock);
      syscall_printf ("-1 [EINVAL] = shmdt (shmaddr = %p)", shmaddr);
      set_errno (EINVAL);
      return -1;
    }

  assert (previous ? previous->next == segptr : _segments_head == segptr);

  if (previous)
    previous->next = segptr->next;
  else
    _segments_head = segptr->next;

  LeaveCriticalSection (&_segments_lock);

  const long cnt = InterlockedDecrement (&_shmat_cnt);
  assert (cnt >= 0);

  if (!UnmapViewOfFile ((void *) shmaddr))
    syscall_printf (("failed to unmap view "
		     "[shmid = %d, handle = %p, shmaddr = %p]:"
		     "%E"),
		    segptr->shmid, segptr->hFileMap, shmaddr);

  assert (segptr->hFileMap);

  if (!CloseHandle (segptr->hFileMap))
    syscall_printf (("failed to close file map handle "
		     "[shmid = %d, handle = %p]: %E"),
		    segptr->shmid, segptr->hFileMap);

  client_request_shm request (segptr->shmid);

  if (request.make_request () == -1 || request.error_code ())
    syscall_printf ("shmdt request failed [shmid = %d, handle = %p]: %s",
		    segptr->shmid, segptr->hFileMap,
		    strerror (request.error_code ()));

  safe_delete (segptr);

  syscall_printf ("0 = shmdt (shmaddr = %p)", shmaddr);

  return 0;
}

/*---------------------------------------------------------------------------*
 * client_shmmgr::shmget ()
 *---------------------------------------------------------------------------*/

int
client_shmmgr::shmget (const key_t key, const size_t size, const int shmflg)
{
  syscall_printf ("shmget (key = 0x%016X, size = %u, shmflg = 0%o)",
		  key, size, shmflg);

  client_request_shm request (key, size, shmflg);

  if (request.make_request () == -1 || request.error_code ())
    {
      syscall_printf (("-1 [%d] = "
		       "shmget (key = 0x%016X, size = %u, shmflg = 0%o)"),
		      request.error_code (),
		      key, size, shmflg);
      set_errno (request.error_code ());
      return -1;
    }

  syscall_printf (("%d = shmget (key = 0x%016X, size = %u, shmflg = 0%o)"),
		  request.shmid (),
		  key, size, shmflg);

  return request.shmid ();
}

/*---------------------------------------------------------------------------*
 * client_shmmgr::fixup_shms_after_fork ()
 *
 * The hFileMap handles are non-inheritable: so they have to be
 * re-acquired from cygserver.
 *
 * Nb. This routine need not be thread-safe as it is only called at startup.
 *---------------------------------------------------------------------------*/

int
client_shmmgr::fixup_shms_after_fork ()
{
  debug_printf ("re-attaching to shm segments: %d attached", _shmat_cnt);

  {
    int length = 0;
    for (segment_t *segptr = _segments_head; segptr; segptr = segptr->next)
      length += 1;

    if (_shmat_cnt != length)
      {
	system_printf (("state inconsistent: "
			"_shmat_cnt = %d, length of segments list = %d"),
		       _shmat_cnt, length);
	return 1;
      }
  }

  for (segment_t *segptr = _segments_head; segptr; segptr = segptr->next)
    if (!attach (segptr->shmid,
		 segptr->shmaddr,
		 segptr->shmflg & ~SHM_RND,
		 segptr->hFileMap))
      {
	system_printf ("fatal error re-attaching to shm segment %d",
		       segptr->shmid);
	return 1;
      }

  if (_shmat_cnt)
    debug_printf ("re-attached all %d shm segments", _shmat_cnt);

  return 0;
}

/*---------------------------------------------------------------------------*
 * client_shmmgr::client_shmmgr ()
 *---------------------------------------------------------------------------*/

client_shmmgr::client_shmmgr ()
{
  InitializeCriticalSection (&_segments_lock);
}

/*---------------------------------------------------------------------------*
 * client_shmmgr::~client_shmmgr ()
 *---------------------------------------------------------------------------*/

client_shmmgr::~client_shmmgr ()
{
  DeleteCriticalSection (&_segments_lock);
}

/*---------------------------------------------------------------------------*
 * client_shmmgr::find ()
 *---------------------------------------------------------------------------*/

client_shmmgr::segment_t *
client_shmmgr::find (const void *const shmaddr, segment_t **previous)
{
  if (previous)
    *previous = NULL;

  for (segment_t *segptr = _segments_head; segptr; segptr = segptr->next)
    if (segptr->shmaddr == shmaddr)
      return segptr;
    else if (segptr->shmaddr > shmaddr) // The list is sorted by shmaddr.
      return NULL;
    else if (previous)
      *previous = segptr;

  return NULL;
}

/*---------------------------------------------------------------------------*
 * client_shmmgr::attach ()
 *
 * The body of shmat (), also used by fixup_shms_after_fork ().
 *---------------------------------------------------------------------------*/

void *
client_shmmgr::attach (const int shmid,
		       const void *shmaddr,
		       const int shmflg,
		       HANDLE & hFileMap)
{
  client_request_shm request (shmid, shmflg);

  if (request.make_request () == -1 || request.error_code ())
    {
      syscall_printf (("-1 [%d] = "
		       "shmat (shmid = %d, shmaddr = %p, shmflg = 0%o)"),
		      request.error_code (), shmid, shmaddr, shmflg);
      set_errno (request.error_code ());
      return NULL;
    }

  int result = 0;

  const DWORD access = (shmflg & SHM_RDONLY) ? FILE_MAP_READ : FILE_MAP_WRITE;

  if (shmaddr && (shmflg & SHM_RND))
    shmaddr = (char *) shmaddr - ((ssize_t) shmaddr % SHMLBA);

  void *const ptr =
    MapViewOfFileEx (request.hFileMap (), access, 0, 0, 0, (void *) shmaddr);

  if (!ptr)
    {
      syscall_printf (("failed to map view "
		       "[shmid = %d, handle = %p, shmaddr = %p]: %E"),
		      shmid, request.hFileMap (), shmaddr);
      result = EINVAL;		// FIXME
    }
  else if (shmaddr && ptr != shmaddr)
    {
      syscall_printf (("failed to map view at requested address "
		       "[shmid = %d, handle = %p]: "
		       "requested address = %p, mapped address = %p"),
		      shmid, request.hFileMap (),
		      shmaddr, ptr);
      result = EINVAL;		// FIXME
    }

  if (result != 0)
    {
      if (!CloseHandle (request.hFileMap ()))
	syscall_printf (("failed to close file map handle "
			 "[shmid = %d, handle = %p]: %E"),
			shmid, request.hFileMap ());

      client_request_shm dt_req (shmid);

      if (dt_req.make_request () == -1 || dt_req.error_code ())
	syscall_printf ("shmdt request failed [shmid = %d, handle = %p]: %s",
			shmid, request.hFileMap (),
			strerror (dt_req.error_code ()));

      set_errno (result);
      return NULL;
    }

  hFileMap = request.hFileMap ();
  return ptr;
}

/*---------------------------------------------------------------------------*
 * client_shmmgr::new_segment ()
 *
 * Allocate a new segment for the given shmid, file map and address
 * and insert into the segment map.
 *---------------------------------------------------------------------------*/

client_shmmgr::segment_t *
client_shmmgr::new_segment (const int shmid,
			    const void *const shmaddr,
			    const int shmflg,
			    const HANDLE hFileMap)
{
  assert (ipc_ext2int_subsys (shmid) == IPC_SHMOP);
  assert (hFileMap);
  assert (shmaddr);

  segment_t *previous = NULL;	// Insert pointer.

  const segment_t *const tmp = find (shmaddr, &previous);

  assert (!tmp);
  assert (previous							\
	  ? (!previous->next || previous->next->shmaddr > shmaddr)	\
	  : (!_segments_head || _segments_head->shmaddr > shmaddr));

  segment_t *const segptr =
    safe_new (segment_t, shmid, shmaddr, shmflg, hFileMap);

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

  const long cnt = InterlockedIncrement (&_shmat_cnt);
  assert (cnt > 0);

  return segptr;
}

/*---------------------------------------------------------------------------*
 * shmat ()
 *---------------------------------------------------------------------------*/

extern "C" void *
shmat (const int shmid, const void *const shmaddr, const int shmflg)
{
  sigframe thisframe (mainthread);
  return shmmgr.shmat (shmid, shmaddr, shmflg);
}

/*---------------------------------------------------------------------------*
 * shmctl ()
 *---------------------------------------------------------------------------*/

extern "C" int
shmctl (const int shmid, const int cmd, struct shmid_ds *const buf)
{
  sigframe thisframe (mainthread);
  return shmmgr.shmctl (shmid, cmd, buf);
}

/*---------------------------------------------------------------------------*
 * shmdt ()
 *---------------------------------------------------------------------------*/

extern "C" int
shmdt (const void *const shmaddr)
{
  sigframe thisframe (mainthread);
  return shmmgr.shmdt (shmaddr);
}

/*---------------------------------------------------------------------------*
 * shmget ()
 *---------------------------------------------------------------------------*/

extern "C" int
shmget (const key_t key, const size_t size, const int shmflg)
{
  sigframe thisframe (mainthread);
  return shmmgr.shmget (key, size, shmflg);
}

/*---------------------------------------------------------------------------*
 * fixup_shms_after_fork ()
 *---------------------------------------------------------------------------*/

int __stdcall
fixup_shms_after_fork ()
{
  return shmmgr.fixup_shms_after_fork ();
}

/*---------------------------------------------------------------------------*
 * client_request_shm::client_request_shm ()
 *---------------------------------------------------------------------------*/

client_request_shm::client_request_shm (const int shmid, const int shmflg)
  : client_request (CYGSERVER_REQUEST_SHM, &_parameters, sizeof (_parameters))
{
  _parameters.in.shmop = SHMOP_shmat;

  _parameters.in.shmid = shmid;
  _parameters.in.shmflg = shmflg;

  _parameters.in.cygpid = getpid ();
  _parameters.in.winpid = GetCurrentProcessId ();
  _parameters.in.uid = geteuid ();
  _parameters.in.gid = getegid ();

  msglen (sizeof (_parameters.in));
}

/*---------------------------------------------------------------------------*
 * client_request_shm::client_request_shm ()
 *---------------------------------------------------------------------------*/

client_request_shm::client_request_shm (const int shmid,
					const int cmd,
					const struct shmid_ds *const buf)
  : client_request (CYGSERVER_REQUEST_SHM, &_parameters, sizeof (_parameters))
{
  _parameters.in.shmop = SHMOP_shmctl;

  _parameters.in.shmid = shmid;
  _parameters.in.cmd = cmd;
  if (buf)
    _parameters.in.ds = *buf;

  _parameters.in.cygpid = getpid ();
  _parameters.in.winpid = GetCurrentProcessId ();
  _parameters.in.uid = geteuid ();
  _parameters.in.gid = getegid ();

  msglen (sizeof (_parameters.in));
}

/*---------------------------------------------------------------------------*
 * client_request_shm::client_request_shm ()
 *---------------------------------------------------------------------------*/

client_request_shm::client_request_shm (const int shmid)
  : client_request (CYGSERVER_REQUEST_SHM, &_parameters, sizeof (_parameters))
{
  _parameters.in.shmop = SHMOP_shmdt;

  _parameters.in.shmid = shmid;

  _parameters.in.cygpid = getpid ();
  _parameters.in.winpid = GetCurrentProcessId ();
  _parameters.in.uid = geteuid ();
  _parameters.in.gid = getegid ();

  msglen (sizeof (_parameters.in));
}

/*---------------------------------------------------------------------------*
 * client_request_shm::client_request_shm ()
 *---------------------------------------------------------------------------*/

client_request_shm::client_request_shm (const key_t key,
					const size_t size,
					const int shmflg)
  : client_request (CYGSERVER_REQUEST_SHM, &_parameters, sizeof (_parameters))
{
  _parameters.in.shmop = SHMOP_shmget;

  _parameters.in.key = key;
  _parameters.in.size = size;
  _parameters.in.shmflg = shmflg;

  _parameters.in.cygpid = getpid ();
  _parameters.in.winpid = GetCurrentProcessId ();
  _parameters.in.uid = geteuid ();
  _parameters.in.gid = getegid ();

  msglen (sizeof (_parameters.in));
}
