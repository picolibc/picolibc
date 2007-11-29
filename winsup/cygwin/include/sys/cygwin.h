/* sys/cygwin.h

   Copyright 1997, 1998, 2000, 2001, 2002 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _SYS_CYGWIN_H
#define _SYS_CYGWIN_H

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define _CYGWIN_SIGNAL_STRING "cYgSiGw00f"

extern pid_t cygwin32_winpid_to_pid (int);
extern void cygwin32_win32_to_posix_path_list (const char *, char *);
extern int cygwin32_win32_to_posix_path_list_buf_size (const char *);
extern void cygwin32_posix_to_win32_path_list (const char *, char *);
extern int cygwin32_posix_to_win32_path_list_buf_size (const char *);
extern int cygwin32_conv_to_win32_path (const char *, char *);
extern int cygwin32_conv_to_full_win32_path (const char *, char *);
extern void cygwin32_conv_to_posix_path (const char *, char *);
extern void cygwin32_conv_to_full_posix_path (const char *, char *);
extern int cygwin32_posix_path_list_p (const char *);
extern void cygwin32_split_path (const char *, char *, char *);

extern pid_t cygwin_winpid_to_pid (int);
extern int cygwin_win32_to_posix_path_list (const char *, char *);
extern int cygwin_win32_to_posix_path_list_buf_size (const char *);
extern int cygwin_posix_to_win32_path_list (const char *, char *);
extern int cygwin_posix_to_win32_path_list_buf_size (const char *);
extern int cygwin_conv_to_win32_path (const char *, char *);
extern int cygwin_conv_to_full_win32_path (const char *, char *);
extern int cygwin_conv_to_posix_path (const char *, char *);
extern int cygwin_conv_to_full_posix_path (const char *, char *);
extern int cygwin_posix_path_list_p (const char *);
extern void cygwin_split_path (const char *, char *, char *);

struct __cygwin_perfile
{
  const char *name;
  unsigned flags;
};

/* External interface stuff */

/* Always add at the bottom.  Do not add new values in the middle. */
typedef enum
  {
    CW_LOCK_PINFO,
    CW_UNLOCK_PINFO,
    CW_GETTHREADNAME,
    CW_GETPINFO,
    CW_SETPINFO,
    CW_SETTHREADNAME,
    CW_GETVERSIONINFO,
    CW_READ_V1_MOUNT_TABLES,
    CW_USER_DATA,
    CW_PERFILE,
    CW_GET_CYGDRIVE_PREFIXES,
    CW_GETPINFO_FULL,
    CW_INIT_EXCEPTIONS,
    CW_GET_CYGDRIVE_INFO,
    CW_SET_CYGWIN_REGISTRY_NAME,
    CW_GET_CYGWIN_REGISTRY_NAME,
    CW_STRACE_TOGGLE,
    CW_STRACE_ACTIVE,
    CW_CYGWIN_PID_TO_WINPID,
    CW_EXTRACT_DOMAIN_AND_USER,
    CW_CMDLINE,
    CW_CHECK_NTSEC,
    CW_GET_ERRNO_FROM_WINERROR,
    CW_GET_POSIX_SECURITY_ATTRIBUTE,
    CW_GET_SHMLBA,
    CW_GET_UID_FROM_SID,
    CW_GET_GID_FROM_SID,
    CW_GET_BINMODE,
    CW_HOOK,
    CW_ARGV,
    CW_ENVP,
    CW_DEBUG_SELF,
    CW_SYNC_WINENV,
    CW_CYGTLS_PADSIZE
  } cygwin_getinfo_types;

#define CW_NEXTPID	0x80000000	/* or with pid to get next one */
unsigned long cygwin_internal (cygwin_getinfo_types, ...);

/* Flags associated with process_state */
enum
{
  PID_IN_USE	       = 0x00001, /* Entry in use. */
  PID_UNUSED	       = 0x00002, /* Available. */
  PID_STOPPED	       = 0x00004, /* Waiting for SIGCONT. */
  PID_TTYIN	       = 0x00008, /* Waiting for terminal input. */
  PID_TTYOU	       = 0x00010, /* Waiting for terminal output. */
  PID_ORPHANED	       = 0x00020, /* Member of an orphaned process group. */
  PID_ACTIVE	       = 0x00040, /* Pid accepts signals. */
  PID_CYGPARENT	       = 0x00080, /* Set if parent was a cygwin app. */
  PID_MAP_RW	       = 0x00100, /* Flag to open map rw. */
  PID_MYSELF	       = 0x00200, /* Flag that pid is me. */
  PID_NOCLDSTOP	       = 0x00400, /* Set if no SIGCHLD signal on stop. */
  PID_INITIALIZING     = 0x00800, /* Set until ready to receive signals. */
  PID_USETTY	       = 0x01000, /* Setting this enables or disables cygwin's
				     tty support.  This is inherited by
				     all execed or forked processes. */
  PID_ALLPIDS	       = 0x02000, /* used by pinfo scanner */
  PID_EXECED	       = 0x04000, /* redirect to original pid info block */
  PID_NOREDIR	       = 0x08000, /* don't redirect if execed */
  PID_EXITED	       = 0x80000000 /* Free entry. */
};

#ifdef WINVER
#ifdef _PATH_PASSWD
extern HANDLE cygwin_logon_user (const struct passwd *, const char *);
#endif

/* This lives in the app and is initialized before jumping into the DLL.
   It should only contain stuff which the user's process needs to see, or
   which is needed before the user pointer is initialized, or is needed to
   carry inheritance information from parent to child.  Note that it cannot
   be used to carry inheritance information across exec!

   Remember, this structure is linked into the application's executable.
   Changes to this can invalidate existing executables, so we go to extra
   lengths to avoid having to do it.

   When adding/deleting members, remember to adjust {public,internal}_reserved.
   The size of the class shouldn't change [unless you really are prepared to
   invalidate all existing executables].  The program does a check (using
   SIZEOF_PER_PROCESS) to make sure you remember to make the adjustment.
*/

#ifdef __cplusplus
class MTinterface;
#endif

struct per_process
{
  char *initial_sp;

  /* The offset of these 3 values can never change. */
  /* magic_biscuit is the size of this class and should never change. */
  unsigned long magic_biscuit;
  unsigned long dll_major;
  unsigned long dll_minor;

  struct _reent **impure_ptr_ptr;
  char ***envptr;

  /* Used to point to the memory machine we should use.  Usually these
     point back into the dll, but they can be overridden by the user. */
  void *(*malloc)(size_t);
  void (*free)(void *);
  void *(*realloc)(void *, size_t);

  int *fmode_ptr;

  int (*main)(int, char **, char **);
  void (**ctors)(void);
  void (**dtors)(void);

  /* For fork */
  void *data_start;
  void *data_end;
  void *bss_start;
  void *bss_end;

  void *(*calloc)(size_t, size_t);
  /* For future expansion of values set by the app. */
  void (*premain[4]) (int, char **, struct per_process *);

  /* The rest are *internal* to cygwin.dll.
     Those that are here because we want the child to inherit the value from
     the parent (which happens when bss is copied) are marked as such. */

  /* non-zero of ctors have been run.  Inherited from parent. */
  int run_ctors_p;

  DWORD unused[7];

  /* Non-zero means the task was forked.  The value is the pid.
     Inherited from parent. */
  int forkee;

  HMODULE hmodule;

  DWORD api_major;		/* API version that this program was */
  DWORD api_minor;		/*  linked with */
  /* For future expansion, so apps won't have to be relinked if we
     add an item. */
  DWORD unused2[6];

#if defined (__INSIDE_CYGWIN__) && defined (__cplusplus)
  MTinterface *threadinterface;
#else
  void *threadinterface;
#endif
  struct _reent *impure_ptr;
};
#define per_process_overwrite ((unsigned) &(((struct per_process *) NULL)->threadinterface))

extern void cygwin_premain0 (int argc, char **argv, struct per_process *);
extern void cygwin_premain1 (int argc, char **argv, struct per_process *);
extern void cygwin_premain2 (int argc, char **argv, struct per_process *);
extern void cygwin_premain3 (int argc, char **argv, struct per_process *);

extern void cygwin_set_impersonation_token (const HANDLE);

/* included if <windows.h> is included */
extern int cygwin32_attach_handle_to_fd (char *, int, HANDLE, mode_t, DWORD);
extern int cygwin_attach_handle_to_fd (char *, int, HANDLE, mode_t, DWORD);

#ifdef __CYGWIN__
#include <sys/resource.h>

#define TTY_CONSOLE	0x40000000

#define EXTERNAL_PINFO_VERSION_16_BIT 0
#define EXTERNAL_PINFO_VERSION_32_BIT 1
#define EXTERNAL_PINFO_VERSION EXTERNAL_PINFO_VERSION_32_BIT

#ifndef _SYS_TYPES_H
typedef unsigned short __uid16_t;
typedef unsigned short __gid16_t;
typedef unsigned long __uid32_t;
typedef unsigned long __gid32_t;
#endif

struct external_pinfo
  {
  pid_t pid;
  pid_t ppid;
  DWORD exitcode;
  DWORD dwProcessId, dwSpawnedProcessId;
  __uid16_t uid;
  __gid16_t gid;
  pid_t pgid;
  pid_t sid;
  int ctty;
  mode_t umask;

  long start_time;
  struct rusage rusage_self;
  struct rusage rusage_children;

  char progname[MAX_PATH];

  DWORD strace_mask;
  DWORD version;

  DWORD process_state;

  /* Only available if version >= EXTERNAL_PINFO_VERSION_32_BIT */
  __uid32_t uid32;
  __gid32_t gid32;
};
#endif /*__CYGWIN__*/
#endif /*WINVER*/

#ifdef __cplusplus
};
#endif
#endif /* _SYS_CYGWIN_H */
