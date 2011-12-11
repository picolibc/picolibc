/* select.h

   Copyright 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004,
   2005, 2006, 2007, 2008, 2009, 2010, 2011 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _SELECT_H_
#define _SELECT_H_

struct select_record
{
  int fd;
  HANDLE h;
  fhandler_base *fh;
  int thread_errno;
  bool windows_handle;
  bool read_ready, write_ready, except_ready;
  bool read_selected, write_selected, except_selected;
  bool except_on_write;
  int (*startup) (select_record *me, class select_stuff *stuff);
  int (*peek) (select_record *, bool);
  int (*verify) (select_record *me, fd_set *readfds, fd_set *writefds,
		 fd_set *exceptfds);
  void (*cleanup) (select_record *me, class select_stuff *stuff);
  struct select_record *next;
  void set_select_errno () {__seterrno (); thread_errno = errno;}
  int saw_error () {return thread_errno;}
  select_record () {}
  select_record (int): fd (0), h (NULL), fh (NULL), thread_errno (0),
    windows_handle (false), read_ready (false), write_ready (false),
    except_ready (false), read_selected (false), write_selected (false),
    except_selected (false), except_on_write (false),
    startup (NULL), peek (NULL), verify (NULL), cleanup (NULL),
    next (NULL) {}
};

struct select_info
{
  cygthread *thread;
  bool stop_thread;
  select_record *start;
  select_info () {}
};

struct select_pipe_info: public select_info
{
};

struct select_socket_info: public select_info
{
  int num_w4;
  LONG *ser_num;
  HANDLE *w4;
};

struct select_serial_info: public select_info
{
};

struct select_mailslot_info: public select_info
{
};

class select_stuff
{
public:
  ~select_stuff ();
  bool always_ready, windows_used;
  select_record start;

  select_pipe_info *device_specific_pipe;
  select_socket_info *device_specific_socket;
  select_serial_info *device_specific_serial;
  select_mailslot_info *device_specific_mailslot;

  bool test_and_set (int i, fd_set *readfds, fd_set *writefds,
		     fd_set *exceptfds);
  int poll (fd_set *readfds, fd_set *writefds, fd_set *exceptfds);
  int wait (fd_set *readfds, fd_set *writefds, fd_set *exceptfds, DWORD ms);
  void cleanup ();
  void destroy ();
  select_stuff (): always_ready (0), windows_used (0), start (0),
		   device_specific_pipe (0),
		   device_specific_socket (0),
		   device_specific_serial (0),
		   device_specific_mailslot (0) {}
};
#endif /* _SELECT_H_ */
