/* tty.h: shared tty info for cygwin

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _TTY_H
#define _TTY_H
/* tty tables */

#define INP_BUFFER_SIZE 256
#define OUT_BUFFER_SIZE 256
#define NTTYS		128
#define real_tty_attached(p)	((p)->ctty > 0 && !iscons_dev ((p)->ctty))

/* Input/Output/ioctl events */

#define INPUT_AVAILABLE_EVENT	"cygtty.input.avail"
#define OUTPUT_MUTEX		"cygtty.output.mutex"
#define INPUT_MUTEX		"cygtty.input.mutex"
#define TTY_SLAVE_ALIVE		"cygtty.slave_alive"

#include <sys/termios.h>

#ifndef MIN_CTRL_C_SLOP
#define MIN_CTRL_C_SLOP 50
#endif

#include <devices.h>
class tty_min
{
  pid_t sid;	/* Session ID of tty */
  struct status_flags
  {
    unsigned initialized : 1;	/* Set if tty is initialized */
    unsigned rstcons     : 1;	/* Set if console needs to be set to "non-cooked" */
  } status;

public:
  pid_t pgid;
  bool output_stopped;		/* FIXME: Maybe do this with a mutex someday? */
  fh_devices ntty;
  DWORD last_ctrl_c;		/* tick count of last ctrl-c */
  bool is_console;

  IMPLEMENT_STATUS_FLAG (bool, initialized)
  IMPLEMENT_STATUS_FLAG (bool, rstcons)

  struct termios ti;
  struct winsize winsize;

  /* ioctl requests buffer */
  int cmd;
  union
  {
    struct termios termios;
    struct winsize winsize;
    int value;
    pid_t pid;
  } arg;
  /* XXX_retval variables holds master's completion codes. Error are stored as
   * -ERRNO
   */
  int ioctl_retval;

  void setntty (_major_t t, _minor_t n) {ntty = (fh_devices) FHDEV (t, n);}
  dev_t getntty () const {return ntty;}
  _minor_t get_minor () const {return device::minor (ntty);}
  pid_t getpgid () const {return pgid;}
  void setpgid (int pid) {pgid = pid;}
  int getsid () const {return sid;}
  void setsid (pid_t tsid) {sid = tsid;}
  void kill_pgrp (int);
  int is_orphaned_process_group (int);
  const __reg1 char *ttyname () __attribute (());
};

class fhandler_pty_master;

class tty: public tty_min
{
  HANDLE __reg3 get_event (const char *fmt, PSECURITY_ATTRIBUTES sa,
		    BOOL manual_reset = FALSE);
public:
  pid_t master_pid;	/* PID of tty master process */

private:
  HANDLE _from_master;
  HANDLE _to_master;
  HANDLE _to_master_cyg;

public:
  HANDLE from_master() const { return _from_master; }
  HANDLE to_master() const { return _to_master; }
  HANDLE to_master_cyg() const { return _to_master_cyg; }
  void set_from_master (HANDLE h) { _from_master = h; }
  void set_to_master (HANDLE h) { _to_master = h; }
  void set_to_master_cyg (HANDLE h) { _to_master_cyg = h; }

  int read_retval;
  bool was_opened;	/* True if opened at least once. */
  int column;	/* Current Column */

  void init ();
  HANDLE open_inuse (ACCESS_MASK access);
  HANDLE create_inuse (PSECURITY_ATTRIBUTES);
  bool slave_alive ();
  HANDLE open_mutex (const char *mutex, ACCESS_MASK access);
  inline HANDLE open_output_mutex (ACCESS_MASK access)
    { return open_mutex (OUTPUT_MUTEX, access); }
  inline HANDLE open_input_mutex (ACCESS_MASK access)
    { return open_mutex (INPUT_MUTEX, access); }
  bool exists ();
  bool not_allocated (HANDLE&, HANDLE&);
  void set_master_ctl_closed () {master_pid = -1;}
  static void __stdcall create_master (int);
  static void __stdcall init_session ();
  friend class fhandler_pty_master;
};

class tty_list
{
  tty ttys[NTTYS];
  static HANDLE mutex;

public:
  tty * operator [](int n) {return ttys + device::minor (n);}
  int allocate (HANDLE& r, HANDLE& w);	/* allocate a pty */
  int connect (int);
  void init ();
  tty_min *get_cttyp ();
  int __reg2 attach (int n);
  static void __stdcall init_session ();
  friend class lock_ttys;
};

class lock_ttys
{
  bool release_me;
public:
  lock_ttys (DWORD = INFINITE);
  static void release ();
  void dont_release () {release_me = false;}
  ~lock_ttys ()
  {
    if (release_me)
      release ();
  }
};

extern "C" int ttyslot (void);
#endif /*_TTY_H*/
