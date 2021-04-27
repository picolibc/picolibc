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
#define PCON_MUTEX		"cygtty.pcon.mutex"
#define TTY_SLAVE_ALIVE		"cygtty.slave_alive"
#define TTY_SLAVE_READING	"cygtty.slave_reading"

#include <sys/termios.h>

#ifndef MIN_CTRL_C_SLOP
#define MIN_CTRL_C_SLOP 50
#endif

typedef void *HPCON;

#include "devices.h"
class tty_min
{
  pid_t sid;	/* Session ID of tty */
  struct status_flags
  {
    unsigned initialized : 1;	/* Set if tty is initialized */
  } status;

public:
  pid_t pgid;
  bool output_stopped;		/* FIXME: Maybe do this with a mutex someday? */
  fh_devices ntty;
  ULONGLONG last_ctrl_c;	/* tick count of last ctrl-c */
  bool is_console;
  int last_sig;

  IMPLEMENT_STATUS_FLAG (bool, initialized)

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
  void setpgid (int pid);
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

  /* Transfer direction for fhandler_pty_slave::transfer_input() */
  enum xfer_dir
  {
    to_cyg,
    to_nat
  };
  enum cons_mode
  {
    restore, /* For restoring when exit from cygwin. */
    cygwin,  /* For cygwin apps */
    native   /* For native apps executed from cygwin. */
  };

private:
  HANDLE _from_master_nat;
  HANDLE _from_master;
  HANDLE _to_master_nat;
  HANDLE _to_master;
  HANDLE _to_slave_nat;
  HANDLE _to_slave;
  bool pcon_activated;
  bool pcon_start;
  pid_t pcon_start_pid;
  bool switch_to_pcon_in;
  DWORD pcon_pid;
  UINT term_code_page;
  DWORD pcon_last_time;
  HANDLE h_pcon_write_pipe;
  HANDLE h_pcon_condrv_reference;
  HANDLE h_pcon_conhost_process;
  HANDLE h_pcon_in;
  HANDLE h_pcon_out;
  bool pcon_cap_checked;
  bool has_csi6n;
  bool need_invisible_console;
  pid_t invisible_console_pid;
  UINT previous_code_page;
  UINT previous_output_code_page;
  bool master_is_running_as_service;
  bool req_xfer_input;
  xfer_dir pcon_input_state;
  bool mask_flusho;
  bool discard_input;

public:
  HANDLE from_master_nat () const { return _from_master_nat; }
  HANDLE from_master () const { return _from_master; }
  HANDLE to_master_nat () const { return _to_master_nat; }
  HANDLE to_master () const { return _to_master; }
  HANDLE to_slave_nat () const { return _to_slave_nat; }
  HANDLE to_slave () const { return _to_slave; }
  void set_from_master_nat (HANDLE h) { _from_master_nat = h; }
  void set_from_master (HANDLE h) { _from_master = h; }
  void set_to_master_nat (HANDLE h) { _to_master_nat = h; }
  void set_to_master (HANDLE h) { _to_master = h; }
  void set_to_slave_nat (HANDLE h) { _to_slave_nat = h; }
  void set_to_slave (HANDLE h) { _to_slave = h; }

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
  void wait_pcon_fwd (bool init = true);
  bool pcon_input_state_eq (xfer_dir x) { return pcon_input_state == x; }
  bool pcon_fg (pid_t pgid);
  friend class fhandler_pty_common;
  friend class fhandler_pty_master;
  friend class fhandler_pty_slave;
  friend class tty_min;
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
