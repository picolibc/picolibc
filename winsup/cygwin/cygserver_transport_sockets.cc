/* cygserver_transport_sockets.cc

   Copyright 2001, 2002 Red Hat Inc.

   Written by Robert Collins <rbtcollins@hotmail.com>

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

/* to allow this to link into cygwin and the .dll, a little magic is needed. */
#ifdef __OUTSIDE_CYGWIN__
#include "woutsup.h"
#else
#include "winsup.h"
#endif

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>

#include "cygwin/cygserver_transport.h"
#include "cygwin/cygserver_transport_sockets.h"

/* to allow this to link into cygwin and the .dll, a little magic is needed. */
#ifndef __OUTSIDE_CYGWIN__

extern "C" int cygwin_accept (int fd, struct sockaddr *, int *len);
extern "C" int cygwin_bind (int fd, const struct sockaddr *, int len);
extern "C" int cygwin_connect (int fd, const struct sockaddr *, int len);
extern "C" int cygwin_listen (int fd, int backlog);
extern "C" int cygwin_shutdown (int fd, int how);
extern "C" int cygwin_socket (int af, int type, int protocol);

#else /* __OUTSIDE_CYGWIN__ */

#define cygwin_accept(A,B,C)    ::accept (A,B,C)
#define cygwin_bind(A,B,C)      ::bind (A,B,C)
#define cygwin_connect(A,B,C)   ::connect (A,B,C)
#define cygwin_listen(A,B)      ::listen (A,B)
#define cygwin_shutdown(A,B)    ::shutdown (A,B)
#define cygwin_socket(A,B,C)    ::socket (A,B,C)

#endif /* __OUTSIDE_CYGWIN__ */

enum
  {
    MAX_CONNECT_RETRY = 64
  };

transport_layer_sockets::transport_layer_sockets (const int fd)
  : _fd (fd),
    _addr_len (0),
    _is_accepted_endpoint (true),
    _is_listening_endpoint (false)
{
  assert (_fd != -1);

  memset (&_addr, '\0', sizeof (_addr));
}

transport_layer_sockets::transport_layer_sockets ()
  : _fd (-1),
    _addr_len (0),
    _is_accepted_endpoint (false),
    _is_listening_endpoint (false)
{
  memset (&_addr, '\0', sizeof (_addr));

  _addr.sun_family = AF_UNIX;
  strcpy (_addr.sun_path, "/tmp/cygdaemo"); // FIXME: $TMP?
  _addr_len = SUN_LEN (&_addr);
}

transport_layer_sockets::~transport_layer_sockets ()
{
  close ();
}

#ifndef __INSIDE_CYGWIN__

int
transport_layer_sockets::listen ()
{
  assert (_fd == -1);
  assert (!_is_accepted_endpoint);
  assert (!_is_listening_endpoint);

  debug_printf ("listen () [this = %p]", this);

  struct stat sbuf;

  if (stat (_addr.sun_path, &sbuf) == -1)
    {
      if (errno != ENOENT)
	{
	  system_printf ("cannot access socket file `%s': %s",
			 _addr.sun_path, strerror (errno));
	  return -1;
	}
    }
  else if (S_ISSOCK (sbuf.st_mode))
    {
      // The socket already exists: is a duplicate cygserver running?

      const int newfd = cygwin_socket (AF_UNIX, SOCK_STREAM, 0);

      if (newfd == -1)
	{
	  system_printf ("failed to create UNIX domain socket: %s",
			 strerror (errno));
	  return -1;
	}

      if (cygwin_connect (newfd, (struct sockaddr *) &_addr, _addr_len) == 0)
	{
	  system_printf ("the daemon is already running");
	  (void) cygwin_shutdown (newfd, SHUT_WR);
	  char buf[BUFSIZ];
	  while (::read (newfd, buf, sizeof (buf)) > 0)
	    {}
	  (void) ::close (newfd);
	  return -1;
	}

      if (unlink (_addr.sun_path) == -1)
	{
	  system_printf ("failed to remove `%s': %s",
			 _addr.sun_path, strerror (errno));
	  (void) ::close (newfd);
	  return -1;
	}
    }
  else
    {
      system_printf ("cannot create socket `%s': File already exists",
		     _addr.sun_path);
      return -1;
    }

  _fd = cygwin_socket (AF_UNIX, SOCK_STREAM, 0);

  if (_fd == -1)
    {
      system_printf ("failed to create UNIX domain socket: %s",
		     strerror (errno));
      return -1;
    }

  if (cygwin_bind (_fd, (struct sockaddr *) &_addr, _addr_len) == -1)
    {
      const int saved_errno = errno;
      close ();
      errno = saved_errno;
      system_printf ("failed to bind UNIX domain socket `%s': %s",
		     _addr.sun_path, strerror (errno));
      return -1;
    }

  _is_listening_endpoint = true; // i.e. this really means "have bound".

  if (cygwin_listen (_fd, SOMAXCONN) == -1)
    {
      const int saved_errno = errno;
      close ();
      errno = saved_errno;
      system_printf ("failed to listen on UNIX domain socket `%s': %s",
		     _addr.sun_path, strerror (errno));
      return -1;
    }

  debug_printf ("0 = listen () [this = %p, fd = %d]", this, _fd);

  return 0;
}

class transport_layer_sockets *
transport_layer_sockets::accept (bool *const recoverable)
{
  assert (_fd != -1);
  assert (!_is_accepted_endpoint);
  assert (_is_listening_endpoint);

  debug_printf ("accept () [this = %p, fd = %d]", this, _fd);

  struct sockaddr_un client_addr;
  socklen_t client_addr_len = sizeof (client_addr);

  const int accept_fd =
    cygwin_accept (_fd, (struct sockaddr *) &client_addr, &client_addr_len);

  if (accept_fd == -1)
    {
      system_printf ("failed to accept connection: %s", strerror (errno));
      switch (errno)
	{
	case ECONNABORTED:
	case EINTR:
	case EMFILE:
	case ENFILE:
	case ENOBUFS:
	case ENOMEM:
	  *recoverable = true;
	  break;

	default:
	  *recoverable = false;
	  break;
	}
      return NULL;
    }

  debug_printf ("%d = accept () [this = %p, fd = %d]", accept_fd, this, _fd);

  return safe_new (transport_layer_sockets, accept_fd);
}

#endif /* !__INSIDE_CYGWIN__ */

void
transport_layer_sockets::close ()
{
  debug_printf ("close () [this = %p, fd = %d]", this, _fd);

  if (_is_listening_endpoint)
    (void) unlink (_addr.sun_path);

  if (_fd != -1)
    {
      (void) cygwin_shutdown (_fd, SHUT_WR);
      if (!_is_listening_endpoint)
	{
	  char buf[BUFSIZ];
	  while (::read (_fd, buf, sizeof (buf)) > 0)
	    {}
	}
      (void) ::close (_fd);
      _fd = -1;
    }

  _is_listening_endpoint = false;
}

ssize_t
transport_layer_sockets::read (void *const buf, const size_t buf_len)
{
  assert (_fd != -1);
  assert (!_is_listening_endpoint);

  assert (buf);
  assert (buf_len > 0);

  // verbose: debug_printf ("read (buf = %p, len = %u) [this = %p, fd = %d]",
  //		buf, buf_len, this, _fd);

  char *read_buf = static_cast<char *> (buf);
  size_t read_buf_len = buf_len;
  ssize_t res = 0;

  while (read_buf_len != 0
	 && (res = ::read (_fd, read_buf, read_buf_len)) > 0)
    {
      read_buf += res;
      read_buf_len -= res;

      assert (read_buf_len >= 0);
    }

  if (res != -1)
    {
      if (res == 0)
	errno = EIO;		// FIXME?

      res = buf_len - read_buf_len;
    }

  if (res != static_cast<ssize_t> (buf_len))
    debug_printf ("%d = read (buf = %p, len = %u) [this = %p, fd = %d]: %s",
		  res, buf, buf_len, this, _fd,
		  (res == -1 ? strerror (errno) : "EOF"));
  else
    {
      // verbose: debug_printf ("%d = read (buf = %p, len = %u) [this = %p, fd = %d]",
      //		    res, buf, buf_len, this, _fd);
    }

  return res;
}

ssize_t
transport_layer_sockets::write (void *const buf, const size_t buf_len)
{
  assert (_fd != -1);
  assert (!_is_listening_endpoint);

  assert (buf);
  assert (buf_len > 0);

  // verbose: debug_printf ("write (buf = %p, len = %u) [this = %p, fd = %d]",
  //		buf, buf_len, this, _fd);

  char *write_buf = static_cast<char *> (buf);
  size_t write_buf_len = buf_len;
  ssize_t res = 0;

  while (write_buf_len != 0
	 && (res = ::write (_fd, write_buf, write_buf_len)) > 0)
    {
      write_buf += res;
      write_buf_len -= res;

      assert (write_buf_len >= 0);
    }

  if (res != -1)
    {
      if (res == 0)
	errno = EIO;		// FIXME?

      res = buf_len - write_buf_len;
    }

  if (res != static_cast<ssize_t> (buf_len))
    debug_printf ("%d = write (buf = %p, len = %u) [this = %p, fd = %d]: %s",
		  res, buf, buf_len, this, _fd,
		  (res == -1 ? strerror (errno) : "EOF"));
  else
    {
      // verbose: debug_printf ("%d = write (buf = %p, len = %u) [this = %p, fd = %d]",
      //		    res, buf, buf_len, this, _fd);
    }

  return res;
}

int
transport_layer_sockets::connect ()
{
  assert (_fd == -1);
  assert (!_is_accepted_endpoint);
  assert (!_is_listening_endpoint);

  static bool assume_cygserver = false;

  debug_printf ("connect () [this = %p]", this);

  for (int retries = 0; retries != MAX_CONNECT_RETRY; retries++)
    {
      _fd = cygwin_socket (AF_UNIX, SOCK_STREAM, 0);

      if (_fd == -1)
	{
	  system_printf ("failed to create UNIX domain socket: %s",
			 strerror (errno));
	  return -1;
	}

      if (cygwin_connect (_fd, (struct sockaddr *) &_addr, _addr_len) == 0)
	{
	  assume_cygserver = true;
	  debug_printf ("0 = connect () [this = %p, fd = %d]", this, _fd);
	  return 0;
	}

      if (!assume_cygserver || errno != ECONNREFUSED)
	{
	  debug_printf ("failed to connect to server: %s", strerror (errno));
	  (void) ::close (_fd);
	  _fd = -1;
	  return -1;
	}

      (void) ::close (_fd);
      _fd = -1;
      Sleep (0);		// Give the server a chance.
    }

  debug_printf ("failed to connect to server: %s", strerror (errno));
  return -1;
}
