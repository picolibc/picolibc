/* cygserver_shm.h: Single unix specification IPC interface for Cygwin.

   Copyright 2002 Red Hat, Inc.

   Written by Conrad Scott <conrad.scott@dsl.pipex.com>.
   Based on code by Robert Collins <robert.collins@hotmail.com>.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef __CYGSERVER_SHM_H__
#define __CYGSERVER_SHM_H__

#include <sys/types.h>

#include <assert.h>

#include <sys/shm.h>

#include "cygwin/cygserver.h"

/*---------------------------------------------------------------------------*
 * class client_request_shm
 *---------------------------------------------------------------------------*/

#ifndef __INSIDE_CYGWIN__
class transport_layer_base;
class process_cache;
#endif

class client_request_shm : public client_request
{
  friend class client_request;

public:
  enum shmop_t
    {
      SHMOP_shmat,
      SHMOP_shmctl,
      SHMOP_shmdt,
      SHMOP_shmget
    };

#ifdef __INSIDE_CYGWIN__
  client_request_shm (int shmid, int shmflg); // shmat
  client_request_shm (int shmid, int cmd, const struct shmid_ds *); // shmctl
  client_request_shm (int shmid); // shmdt
  client_request_shm (key_t, size_t, int shmflg); // shmget
#endif

  // Accessors for out parameters.

  int shmid () const
  {
    assert (!error_code ());
    return _parameters.shmid;
  }

  const struct shmid_ds & ds () const
  {
    assert (!error_code ());
    return _parameters.ds;
  }

  HANDLE hFileMap () const
  {
    assert (!error_code ());
    return _parameters.out.hFileMap;
  }

private:
  struct
  {
    int shmid;
    struct shmid_ds ds;

    union
    {
      struct
      {
	shmop_t shmop;
	key_t key;
	size_t size;
	int shmflg;
	int cmd;
	pid_t cygpid;
	DWORD winpid;
	uid_t uid;
	gid_t gid;
      } in;

      struct
      {
	HANDLE hFileMap;
      } out;
    };
  } _parameters;

#ifndef __INSIDE_CYGWIN__
  client_request_shm ();
#endif

#ifndef __INSIDE_CYGWIN__
  virtual void serve (transport_layer_base *, process_cache *);
#endif
};

#endif /* __CYGSERVER_SHM_H__ */
