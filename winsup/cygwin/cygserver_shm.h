/* cygserver_shm.h

   Copyright 2001, 2002 Red Hat Inc.
   Written by Robert Collins <rbtcollins@hotmail.com>

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include <sys/types.h>

#include "cygwin_shm.h"

#include "cygwin/cygserver.h"

/* Values for the client_request_shm::parameters.in.type field. */
#define SHM_CREATE   0
#define SHM_REATTACH 1
#define SHM_ATTACH   2
#define SHM_DETACH   3
#define SHM_DEL      4

struct _shmattach
{
  void *data;
  int shmflg;
  struct _shmattach *next;
};

struct int_shmid_ds
{
  struct shmid_ds ds;
  void *mapptr;
};

struct shmnode
{
  struct int_shmid_ds *shmds;
  int shm_id;
  struct shmnode *next;
  key_t key;
  HANDLE filemap;
  HANDLE attachmap;
  struct _shmattach *attachhead;
};

class client_request_shm : public client_request
{
public:
#ifdef __INSIDE_CYGWIN__
  client_request_shm (key_t ntype, size_t nsize, int shmflg);
  client_request_shm (int ntype, int nshmid);
#else
  client_request_shm ();
#endif

  union
  {
    struct
    {
      int type;
      pid_t cygpid;
      DWORD winpid;
      int shm_id;
      key_t key;
      size_t size;
      int shmflg;
      char sd_buf[4096];	// Must be the last item (variable length).
    } in;
    struct
    {
      int shm_id;
      HANDLE filemap;
      HANDLE attachmap;
      key_t key;
    } out;
  } parameters;

private:
#ifndef __INSIDE_CYGWIN__
  virtual void serve (transport_layer_base *, process_cache *);
#endif
};
