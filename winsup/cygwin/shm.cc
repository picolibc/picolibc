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

#include <errno.h>
#include <stdio.h>
#include <unistd.h>

#include "cygerrno.h"

#include "cygserver_ipc.h"
#include "cygserver_shm.h"

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
  CRITICAL_SECTION _segments_lock;
  static segment_t *_segments_head; // A list sorted by shmaddr.

  client_shmmgr ();
  ~client_shmmgr ();

  // Undefined (as this class is a singleton):
  client_shmmgr (const client_shmmgr &);
  client_shmmgr & operator= (const client_shmmgr &);

  segment_t *find (const void *, segment_t **previous = NULL);

  void *attach (int shmid, const void *, int shmflg, HANDLE & hFileMap);

  segment_t *new_segment (int shmid, const void *, int shmflg, HANDLE);
};

client_shmmgr::segment_t *client_shmmgr::_segments_head;

/*---------------------------------------------------------------------------*
 * client_shmmgr::instance ()
 *---------------------------------------------------------------------------*/

client_shmmgr &
client_shmmgr::instance ()
{
  static NO_COPY client_shmmgr instance;

  return instance;
}

/*---------------------------------------------------------------------------*
 * client_shmmgr::shmat ()
 *---------------------------------------------------------------------------*/

void *
client_shmmgr::shmat (const int shmid,
		      const void *const shmaddr,
		      const int shmflg)
{
  HANDLE hFileMap = NULL;

  void *const ptr = attach (shmid, shmaddr, shmflg, hFileMap);

  if (ptr)
    new_segment (shmid, ptr, shmflg, hFileMap);

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
  client_request_shm request (shmid, cmd, buf);

  if (request.make_request () == -1 || request.error_code ())
    {
      set_errno (request.error_code ());
      return -1;
    }

  // Some commands require special processing, e.g. for out parameters.

  switch (cmd)
    {
    case IPC_STAT:
      *buf = request.ds ();
      break;
    }

  return 0;
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
  segment_t *previous = NULL;

  segment_t *const segptr = find (shmaddr, &previous);

  if (!segptr)
    {
      set_errno (EINVAL);
      return -1;
    }

  assert (previous ? previous->next == segptr : _segments_head == segptr);

  if (previous)
    previous->next = segptr->next;
  else
    _segments_head = segptr->next;

  if (!UnmapViewOfFile ((void *) shmaddr))
    with_strerr (msg,
		 syscall_printf (("failed to unmap view "
				  "[shmid = %d, handle = %p, shmaddr = %p]:"
				  "%s"),
				 segptr->shmid, segptr->hFileMap, shmaddr,
				 msg));

  assert (segptr->hFileMap);

  if (!CloseHandle (segptr->hFileMap))
    with_strerr (msg,
		 syscall_printf (("failed to close file map handle "
				  "[shmid = %d, handle = %p]:"
				  "%s"),
				 segptr->shmid, segptr->hFileMap,
				 msg));

  client_request_shm request (segptr->shmid);

  if (request.make_request () == -1 || request.error_code ())
    syscall_printf ("shmdt request failed [shmid = %d, handle = %p]: %s",
		    segptr->shmid, segptr->hFileMap,
		    strerror (request.error_code ()));

  delete segptr;

  return 0;
}

/*---------------------------------------------------------------------------*
 * client_shmmgr::shmget ()
 *---------------------------------------------------------------------------*/

int
client_shmmgr::shmget (const key_t key, const size_t size, const int shmflg)
{
  client_request_shm request (key, size, shmflg);

  if (request.make_request () == -1 || request.error_code ())
    {
      set_errno (request.error_code ());
      return -1;
    }

  return request.shmid ();
}

/*---------------------------------------------------------------------------*
 * client_shmmgr::fixup_shms_after_fork ()
 *
 * The hFileMap handles are non-inheritable: so the 
 *---------------------------------------------------------------------------*/

int
client_shmmgr::fixup_shms_after_fork ()
{
  for (segment_t *segptr = _segments_head; segptr; segptr = segptr->next)
    if (!attach (segptr->shmid,
		 segptr->shmaddr,
		 segptr->shmflg & ~SHM_RND,
		 segptr->hFileMap))
      {
	system_printf ("fatal error re-attaching to shared memory segments");
	return 1;
      }

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
      with_strerr (msg,
		   syscall_printf (("failed to map view "
				    "[shmid = %d, handle = %p, shmaddr = %p]:"
				    "%s"),
				   shmid, request.hFileMap (), shmaddr,
				   msg));
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
	with_strerr (msg,
		     syscall_printf (("failed to close file map handle "
				      "[shmid = %d, handle = %p]:"
				      "%s"),
				     shmid, request.hFileMap (),
				     msg));

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

  segment_t *const segptr = new segment_t (shmid, shmaddr, shmflg, hFileMap);

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

  return segptr;
}

/*---------------------------------------------------------------------------*
 * shmat ()
 *---------------------------------------------------------------------------*/

extern "C" void *
shmat (const int shmid, const void *const shmaddr, const int shmflg)
{
  return shmmgr.shmat (shmid, shmaddr, shmflg);
}

/*---------------------------------------------------------------------------*
 * shmctl ()
 *---------------------------------------------------------------------------*/

extern "C" int
shmctl (const int shmid, const int cmd, struct shmid_ds *const buf)
{
  return shmmgr.shmctl (shmid, cmd, buf);
}

/*---------------------------------------------------------------------------*
 * shmdt ()
 *---------------------------------------------------------------------------*/

extern "C" int
shmdt (const void *const shmaddr)
{
  return shmmgr.shmdt (shmaddr);
}

/*---------------------------------------------------------------------------*
 * shmget ()
 *---------------------------------------------------------------------------*/

extern "C" int
shmget (const key_t key, const size_t size, const int shmflg)
{
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

  _parameters.shmid = shmid;
  _parameters.in.shmflg = shmflg;

  _parameters.in.cygpid = getpid ();
  _parameters.in.winpid = GetCurrentProcessId ();
  _parameters.in.uid = geteuid ();
  _parameters.in.gid = getegid ();
}

/*---------------------------------------------------------------------------*
 * client_request_shm::client_request_shm ()
 *---------------------------------------------------------------------------*/

client_request_shm::client_request_shm (const int shmid,
					const int cmd,
					const struct shmid_ds * const buf)
  : client_request (CYGSERVER_REQUEST_SHM, &_parameters, sizeof (_parameters))
{
  _parameters.in.shmop = SHMOP_shmctl;

  _parameters.shmid = shmid;
  if (cmd == IPC_SET)
    _parameters.ds = *buf;
  _parameters.in.cmd = cmd;

  _parameters.in.cygpid = getpid ();
  _parameters.in.winpid = GetCurrentProcessId ();
  _parameters.in.uid = geteuid ();
  _parameters.in.gid = getegid ();
}

/*---------------------------------------------------------------------------*
 * client_request_shm::client_request_shm ()
 *---------------------------------------------------------------------------*/

client_request_shm::client_request_shm (const int shmid)
  : client_request (CYGSERVER_REQUEST_SHM, &_parameters, sizeof (_parameters))
{
  _parameters.in.shmop = SHMOP_shmdt;

  _parameters.shmid = shmid;

  _parameters.in.cygpid = getpid ();
  _parameters.in.winpid = GetCurrentProcessId ();
  _parameters.in.uid = geteuid ();
  _parameters.in.gid = getegid ();
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
}
