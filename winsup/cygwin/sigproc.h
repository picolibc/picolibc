/* sigproc.h

   Copyright 1997, 1998, 2000 Cygnus Solutions.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#define EXIT_SIGNAL    	 0x010000
#define EXIT_REPARENTING 0x020000
#define EXIT_NOCLOSEALL  0x040000

enum procstuff
{
  PROC_ADDCHILD		= 1,	// add a new subprocess to list
  PROC_CHILDSTOPPED	= 2,	// a child stopped
  PROC_CHILDTERMINATED	= 3,	// a child died
  PROC_CLEARWAIT	= 4,	// clear all waits - signal arrived
  PROC_WAIT		= 5	// setup for wait() for subproc
};

typedef struct struct_waitq
{
  int pid;
  int options;
  int status;
  HANDLE ev;
  void *rusage;			/* pointer to potential rusage */
  struct struct_waitq *next;
  HANDLE thread_ev;
} waitq;

class muto;

struct sigthread
{
  DWORD id;
  DWORD frame;
  muto *lock;
  sigthread () : id (0), frame (0), lock (0) {}
  void init (const char *s)
  {
    lock = new_muto (FALSE, s);
    id = GetCurrentThreadId ();
  }
};

class sigframe
{
private:
  sigthread *st;

public:
  void set (sigthread &t, int up = 1)
  {
    t.lock->acquire ();
    st = &t;
    t.frame = (DWORD) (up ? __builtin_frame_address (1) :
			   __builtin_frame_address (0));
    t.lock->release ();
  }

  sigframe () {st = NULL;}
  sigframe (sigthread &t, int up = 1)
  {
    if (!t.frame && t.id == GetCurrentThreadId ())
      set (t, up);
    else
      st = NULL;
  }
  ~sigframe ()
  {
    if (st)
      {
	st->lock->acquire ();
	st->frame = 0;
	st->lock->release ();
	st = NULL;
      }
  }
};

extern sigthread mainthread;
extern HANDLE signal_arrived;

BOOL __stdcall my_parent_is_alive ();
extern "C" int __stdcall sig_dispatch_pending (int force = FALSE) __asm__ ("sig_dispatch_pending");
extern "C" void __stdcall set_process_mask (sigset_t newmask);
int __stdcall sig_handle (int);
void __stdcall sig_clear (int);
void __stdcall sig_set_pending (int);
int __stdcall handle_sigsuspend (sigset_t);

void __stdcall proc_terminate ();
void __stdcall sigproc_init ();
void __stdcall subproc_init ();
void __stdcall sigproc_terminate ();
BOOL __stdcall proc_exists (pinfo *);
int __stdcall proc_subproc (DWORD, DWORD);
int __stdcall sig_send (pinfo *, int, DWORD ebp = 0);
void __stdcall signal_fixup_after_fork ();

extern char myself_nowait_dummy[];
extern char myself_nowait_nonmain_dummy[];
extern HANDLE hExeced;		// Process handle of new window
				//  process created by spawn_guts()

#define WAIT_SIG_EXITING (WAIT_OBJECT_0 + 1)

#define allow_sig_dispatch(n) __allow_sig_dispatch (__FILE__, __LINE__, (n))

#define myself_nowait ((pinfo *)myself_nowait_dummy)
#define myself_nowait_nonmain ((pinfo *)myself_nowait_nonmain_dummy)
#define proc_register(child) \
	proc_subproc (PROC_ADDCHILD, (DWORD) (child))
