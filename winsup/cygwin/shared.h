/* shared.h: shared info for cygwin

   Copyright 1998, 1999, 2000 Cygnus Solutions.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

/******** Functions declarations for use in methods below ********/

/* Printf type functions */
extern "C" void __api_fatal (const char *, ...) __attribute__ ((noreturn));
extern "C" int __small_sprintf (char *dst, const char *fmt, ...);
extern "C" int __small_vsprintf (char *dst, const char *fmt, va_list ap);

/******** Deletion Queue Class ********/

/* First pass at a file deletion queue structure.

   We can't keep this list in the per-process info, since
   one process may open a file, and outlive a process which
   wanted to unlink the file - and the data would go away.

   Perhaps the FILE_FLAG_DELETE_ON_CLOSE would be ok,
   but brief experimentation didn't get too far.
*/

#define MAX_DELQUEUES_PENDING 100

class delqueue_list
{
  char name[MAX_DELQUEUES_PENDING][MAX_PATH];
  char inuse[MAX_DELQUEUES_PENDING];
  int empty;

public:
  void init ();
  void queue_file (const char *dosname);
  void process_queue ();
};

/* non-NULL if this process is a child of a cygwin process */
extern HANDLE parent_alive;

/******** Registry Access ********/

class reg_key
{
private:

  HKEY key;
  LONG key_is_invalid;

public:

  reg_key (HKEY toplev, REGSAM access, ...);
  reg_key (REGSAM access, ...);
  reg_key (REGSAM access = KEY_ALL_ACCESS);

  void *operator new (size_t, void *p) {return p;}
  void build_reg (HKEY key, REGSAM access, va_list av);

  int error () {return key == (HKEY) INVALID_HANDLE_VALUE;}

  int kill (const char *child);
  int killvalue (const char *name);

  HKEY get_key ();
  int get_int (const char *,int def);
  int get_string (const char *, char *buf, size_t len, const char *def);
  int set_string (const char *,const char *);
  int set_int (const char *, int val);

  ~reg_key ();
};

/* Evaluates path to the directory of the local user registry hive */
char *__stdcall get_registry_hive_path (const PSID psid, char *path);
void __stdcall load_registry_hive (PSID psid);

/******** Mount Table ********/

/* Mount table entry */

class mount_item
{
public:
  /* FIXME: Nasty static allocation.  Need to have a heap in the shared
     area [with the user being able to configure at runtime the max size].  */

  /* Win32-style mounted partition source ("C:\foo\bar").
     native_path[0] == 0 for unused entries.  */
  char native_path[MAX_PATH];
  int native_pathlen;

  /* POSIX-style mount point ("/foo/bar") */
  char posix_path[MAX_PATH];
  int posix_pathlen;

  unsigned flags;

  void init (const char *dev, const char *path, unsigned flags);

  struct mntent *getmntent ();
};

/* Warning: Decreasing this value will cause cygwin.dll to ignore existing
   higher numbered registry entries.  Don't change this number willy-nilly.
   What we need is to have a more dynamic allocation scheme, but the current
   scheme should be satisfactory for a long while yet.  */
#define MAX_MOUNTS 30

class mount_info
{
  int posix_sorted[MAX_MOUNTS];
  int native_sorted[MAX_MOUNTS];
public:
  int nmounts;
  mount_item mount[MAX_MOUNTS];

  /* Strings used by getmntent(). */
  char mnt_type[20];
  char mnt_opts[20];
  char mnt_fsname[MAX_PATH];
  char mnt_dir[MAX_PATH];

  /* cygdrive_prefix is used as the root of the path automatically
     prepended to a path when the path has no associated mount.
     cygdrive_flags are the default flags for the cygdrives. */
  char cygdrive[MAX_PATH];
  size_t cygdrive_len;
  unsigned cygdrive_flags;

  /* Increment when setting up a reg_key if mounts area had to be
     created so we know when we need to import old mount tables. */
  int had_to_create_mount_areas;

  void init ();
  int add_item (const char *dev, const char *path, unsigned flags, int reg_p);
  int del_item (const char *path, unsigned flags, int reg_p);

  void from_registry ();
  int add_reg_mount (const char * native_path, const char * posix_path,
		      unsigned mountflags);
  int del_reg_mount (const char * posix_path, unsigned mountflags);

  unsigned set_flags_from_win32_path (const char *path);
  int conv_to_win32_path (const char *src_path, char *win32_path,
			  char *full_win32_path, DWORD &devn, int &unit,
			  unsigned *flags = NULL);
  int conv_to_posix_path (const char *src_path, char *posix_path,
			  int keep_rel_p);
  struct mntent *getmntent (int x);

  int write_cygdrive_info_to_registry (const char *cygdrive_prefix, unsigned flags);
  int remove_cygdrive_info_from_registry (const char *cygdrive_prefix, unsigned flags);
  int get_cygdrive_prefixes (char *user, char *system);

  void import_v1_mounts ();

private:

  void sort ();
  void read_mounts (reg_key& r);
  void read_v1_mounts (reg_key r, unsigned which);
  void mount_slash ();
  void to_registry ();

  int cygdrive_win32_path (const char *src, char *dst, int trailing_slash_p);
  void cygdrive_posix_path (const char *src, char *dst, int trailing_slash_p);
  void slash_drive_to_win32_path (const char *path, char *buf, int trailing_slash_p);
  void read_cygdrive_info_from_registry ();
};

/******** TTY Support ********/

/* tty tables */

#define INP_BUFFER_SIZE 256
#define OUT_BUFFER_SIZE 256
#define NTTYS		128
#define TTY_CONSOLE	0x40000000
#define tty_attached(p)	((p)->ctty >= 0 && (p)->ctty != TTY_CONSOLE)

/* Input/Output/ioctl events */

#define OUTPUT_DONE_EVENT	"cygtty%d.output.done"
#define IOCTL_REQUEST_EVENT	"cygtty%d.ioctl.request"
#define IOCTL_DONE_EVENT	"cygtty%d.ioctl.done"
#define RESTART_OUTPUT_EVENT	"cygtty%d.output.restart"
#define OUTPUT_MUTEX		"cygtty%d.output.mutex"
#define TTY_SLAVE_ALIVE		"cygtty%x.slave_alive"
#define TTY_MASTER_ALIVE	"cygtty%x.master_alive"

#include <sys/termios.h>

enum
{
  TTY_INITIALIZED = 1,		/* Set if tty is initialized */
  TTY_RSTCONS = 2		/* Set if console needs to be set to "non-cooked" */
};

#define TTYISSETF(x)	__ISSETF (tc, x, TTY)
#define TTYSETF(x)	__SETF (tc, x, TTY)
#define TTYCLEARF(x)	__CLEARF (tc, x, TTY)
#define TTYCONDSETF(n, x) __CONDSETF(n, tc, x, TTY)

#ifndef MIN_CTRL_C_SLOP
#define MIN_CTRL_C_SLOP 50
#endif

class tty_min
{
  pid_t sid;	/* Session ID of tty */
public:
  DWORD status;
  pid_t pgid;
  int OutputStopped;
  int ntty;
  DWORD last_ctrl_c;	// tick count of last ctrl-c

  tty_min (int t = -1, pid_t s = -1) : sid (s), ntty (t) {}
  void setntty (int n) {ntty = n;}
  pid_t getpgid () {return pgid;}
  void setpgid (int pid) {pgid = pid;}
  int getsid () {return sid;}
  void setsid (pid_t tsid) {sid = tsid;}
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

  int write_retval;
};

class fhandler_pty_master;

class tty: public tty_min
{
  HANDLE get_event (const char *fmt, BOOL inherit);
public:
  HWND  hwnd;	/* Console window handle tty belongs to */

  DWORD master_pid;	/* Win32 PID of tty master process */

  HANDLE from_master, to_slave;
  HANDLE from_slave, to_master;

  int read_retval;
  BOOL was_opened;	/* True if opened at least once. */

  void init ();
  HANDLE create_inuse (const char *);
  BOOL common_init (fhandler_pty_master *);
  BOOL alive (const char *fmt);
  BOOL slave_alive ();
  BOOL master_alive ();
  HWND gethwnd () {return hwnd;}
  void sethwnd (HWND wnd) {hwnd = wnd;}
  int make_pipes (fhandler_pty_master *ptym);
  HANDLE open_output_mutex (BOOL inherit = FALSE)
  {
    char buf[80];
    __small_sprintf (buf, OUTPUT_MUTEX, ntty);
    return OpenMutex (MUTEX_ALL_ACCESS, inherit, buf);
  }
  BOOL exists ()
  {
    HANDLE h = open_output_mutex ();
    if (h)
      {
	CloseHandle (h);
	return 1;
      }
    return slave_alive ();
  }
};

class tty_list
{
  tty ttys[NTTYS];

public:
  tty * operator [](int n) {return ttys + n;}
  int allocate_tty (int n); /* n non zero if allocate a tty, pty otherwise */
  int connect_tty (int);
  void terminate ();
  void init ();
  tty_min *get_tty (int n);
};

void __stdcall tty_init ();
void __stdcall tty_terminate ();
int __stdcall attach_tty (int);
void __stdcall create_tty_master (int);
extern "C" int ttyslot (void);

/******** Shared Info ********/
/* Data accessible to all tasks */

class shared_info
{
  DWORD inited;

public:
  /* FIXME: Doesn't work if more than one user on system. */
  mount_info mount;

  int heap_chunk_in_mb;
  unsigned heap_chunk_size (void);

  tty_list tty;
  delqueue_list delqueue;
  void initialize (void);
};

/* Various types of security attributes for use in Create* functions. */
extern SECURITY_ATTRIBUTES sec_none, sec_none_nih, sec_all, sec_all_nih;
extern SECURITY_ATTRIBUTES *__stdcall sec_user (PVOID sa_buf, PSID sid2 = NULL, BOOL inherit = TRUE);
extern SECURITY_ATTRIBUTES *__stdcall sec_user_nih (PVOID sa_buf, PSID sid2 = NULL);

extern shared_info *cygwin_shared;
extern HANDLE cygwin_shared_h;
extern HANDLE console_shared_h;
extern int __stdcall set_console_state_for_spawn ();

void __stdcall shared_init (void);
void __stdcall shared_terminate (void);

char *__stdcall shared_name (const char *, int);
void *__stdcall open_shared (const char *name, HANDLE &shared_h, DWORD size, void *addr);

extern "C" {
/* This is for programs that want to access the shared data. */
class shared_info *cygwin_getshared (void);

struct cygwin_version_info
{
  unsigned short api_major;
  unsigned short api_minor;
  unsigned short dll_major;
  unsigned short dll_minor;
  unsigned short shared_data;
  unsigned short mount_registry;
  const char *dll_build_date;
  char shared_id[sizeof (CYGWIN_VERSION_DLL_IDENTIFIER) + 64];
};
}

extern cygwin_version_info cygwin_version;
extern const char *cygwin_version_strings;
