/* cygserver.h

   Copyright 2001 Red Hat Inc.

   Written by Egor Duda <deo@logos-m.ru>

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _CYGSERVER_H_
#define _CYGSERVER_H_

#define MAX_REQUEST_SIZE 128

#define CYGWIN_SERVER_VERSION_MAJOR	1
#define CYGWIN_SERVER_VERSION_API	1
#define CYGWIN_SERVER_VERSION_MINOR	0
#define CYGWIN_SERVER_VERSION_PATCH	0


typedef enum {
  CYGSERVER_UNKNOWN=0,
  CYGSERVER_OK=1,
  CYGSERVER_DEAD=2
} cygserver_states;

typedef enum {
  CYGSERVER_REQUEST_INVALID = 0,
  CYGSERVER_REQUEST_GET_VERSION,
  CYGSERVER_REQUEST_ATTACH_TTY,
  CYGSERVER_REQUEST_SHUTDOWN,
  CYGSERVER_REQUEST_SHM_GET,
  CYGSERVER_REQUEST_LAST
} cygserver_request_code;

class request_header
{
  public:
    ssize_t cb;
    cygserver_request_code req_id;
    ssize_t error_code;
    request_header (cygserver_request_code id, ssize_t ncb) : cb (ncb), req_id (id), error_code (0) {} ;
}
#ifdef __GNUC__
  __attribute__ ((packed))
#endif
;

extern void cygserver_init ();

#define INIT_REQUEST(req,id) \
	(req).header.cb = sizeof (req); \
	(req).header.req_id = id;

struct request_get_version
{
  DWORD major, api, minor, patch;
}
#ifdef __GNUC__
  __attribute__ ((packed))
#endif
;

struct request_shutdown
{
  int foo;
}
#ifdef __GNUC__
  __attribute__ ((packed))
#endif
;

struct request_attach_tty
{
  DWORD pid, master_pid;
  HANDLE from_master, to_master;
}
#ifdef __GNUC__
  __attribute__ ((packed))
#endif
;

class client_request
{
  public:
  client_request (cygserver_request_code id, ssize_t data_size);
  virtual void send (transport_layer_base *conn);
#ifndef __INSIDE_CYGWIN__
  virtual void serve (transport_layer_base *conn, class process_cache *cache);
#endif
  virtual operator struct request_header ();
  cygserver_request_code req_id () {return header.req_id;};
  virtual ~client_request();
  request_header header;
  char *buffer;
};

class client_request_get_version : public client_request
{
  public:
#ifndef __INSIDE_CYGWIN__
  virtual void serve (transport_layer_base *conn, class process_cache *cache);
#endif
  client_request_get_version::client_request_get_version();
  struct request_get_version version;
};

class client_request_shutdown : public client_request
{
  public:
#ifndef __INSIDE_CYGWIN__
  virtual void serve (transport_layer_base *conn, class process_cache *cache);
#endif
  client_request_shutdown ();
};

class client_request_attach_tty : public client_request
{
  public:
#ifndef __INSIDE_CYGWIN__
  virtual void serve (transport_layer_base *conn, class process_cache *cache);
#endif
  client_request_attach_tty ();
  client_request_attach_tty (DWORD npid, DWORD nmaster_pid, HANDLE nfrom_master, HANDLE nto_master);
  HANDLE from_master () {return req.from_master;};
  HANDLE to_master () {return req.to_master;};
  struct request_attach_tty req;
};

extern int cygserver_request (client_request *);

#endif /* _CYGSERVER+H+ */
