/* cygserver.h

   Written by Egor Duda <deo@logos-m.ru>

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _CYGSERVER_H_
#define _CYGSERVER_H_

#define CYGWIN_SERVER_VERSION_MAJOR	1
#define CYGWIN_SERVER_VERSION_API	4
#define CYGWIN_SERVER_VERSION_MINOR	0
#define CYGWIN_SERVER_VERSION_PATCH	0

typedef enum {
  CYGSERVER_UNKNOWN = 0,
  CYGSERVER_OK,
  CYGSERVER_UNAVAIL
} cygserver_states;

/*---------------------------------------------------------------------------*
 * class client_request
 *---------------------------------------------------------------------------*/

class transport_layer_base;

#ifndef __INSIDE_CYGWIN__
class process_cache;
#endif

class client_request
{
protected:
  typedef enum {
    CYGSERVER_REQUEST_INVALID,
    CYGSERVER_REQUEST_GET_VERSION,
    CYGSERVER_REQUEST_SHUTDOWN,
    CYGSERVER_REQUEST_ATTACH_TTY,
    CYGSERVER_REQUEST_MSG,
    CYGSERVER_REQUEST_SEM,
    CYGSERVER_REQUEST_SHM,
    CYGSERVER_REQUEST_SETPWD,
    CYGSERVER_REQUEST_PWDGRP,
    CYGSERVER_REQUEST_LAST
  } request_code_t;

  struct header_t
  {
    size_t msglen;
    union
    {
      request_code_t request_code;
      int error_code;
    };

    header_t () {};
    header_t (request_code_t, size_t);
  };

public:
#ifndef __INSIDE_CYGWIN__
  static void handle_request (transport_layer_base *, process_cache *);
#endif

  client_request (request_code_t request_code,
		  void *buf = NULL,
		  size_t bufsiz = 0);
  virtual ~client_request ();

  request_code_t request_code () const { return _header.request_code; }

  int error_code () const { return _header.error_code; };
  void error_code (int error_code) { _header.error_code = error_code; };

  size_t msglen () const { return _header.msglen; };
  void msglen (size_t len) { _header.msglen = len; };

  int make_request ();

protected:
  virtual void send (transport_layer_base *);

private:
  header_t _header;
  void * const _buf;
  const size_t _buflen;

#ifndef __INSIDE_CYGWIN__
  void handle (transport_layer_base *, process_cache *);
  virtual void serve (transport_layer_base *, process_cache *) = 0;
#endif
};

/*---------------------------------------------------------------------------*
 * class client_request_get_version
 *---------------------------------------------------------------------------*/

class client_request_get_version : public client_request
{
private:
  struct request_get_version
  {
    DWORD major, api, minor, patch;
  };

public:
  client_request_get_version ();
  bool check_version () const;

private:
  struct request_get_version version;

#ifndef __INSIDE_CYGWIN__
  virtual void serve (transport_layer_base *, process_cache *);
#endif
};

/*---------------------------------------------------------------------------*
 * class client_request_shutdown
 *
 * Nb. This whole class is only !__INSIDE_CYGWIN__ since it is used
 * solely by cygserver itself.
 *---------------------------------------------------------------------------*/

#ifndef __INSIDE_CYGWIN__

class client_request_shutdown : public client_request
{
public:
  client_request_shutdown ();

private:
  virtual void serve (transport_layer_base *, process_cache *);
};

#endif /* !__INSIDE_CYGWIN__ */

/*---------------------------------------------------------------------------*
 * class client_request_attach_tty
 *---------------------------------------------------------------------------*/

class client_request_attach_tty : public client_request
{
private:
  struct request_attach_tty
  {
    DWORD pid, master_pid;
    HANDLE from_master, to_master;
  };

public:
#ifdef __INSIDE_CYGWIN__
  client_request_attach_tty (DWORD nmaster_pid,
			     HANDLE nfrom_master, HANDLE nto_master);
#else
  client_request_attach_tty ();
#endif

  HANDLE from_master () const { return req.from_master; };
  HANDLE to_master () const { return req.to_master; };

protected:
  virtual void send (transport_layer_base *);

private:
  struct request_attach_tty req;

#ifndef __INSIDE_CYGWIN__
  virtual void serve (transport_layer_base *, process_cache *);
#endif
};

#ifndef __INSIDE_CYGWIN__
extern PSID admininstrator_group_sid;
#endif

extern bool check_cygserver_available ();
extern void cygserver_init ();

#endif /* _CYGSERVER_H_ */
