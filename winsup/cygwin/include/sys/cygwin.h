#ifndef _SYS_CYGWIN_H
#define _SYS_CYGWIN_H

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

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

extern void cygwin_premain0 (int argc, char **argv);
extern void cygwin_premain1 (int argc, char **argv);
extern void cygwin_premain2 (int argc, char **argv);
extern void cygwin_premain3 (int argc, char **argv);

#ifdef WINVER
extern HANDLE cygwin_logon_user (const struct passwd *, const char *);
extern void cygwin_set_impersonation_token (const HANDLE);

/* included if <windows.h> is included */
extern int cygwin32_attach_handle_to_fd (char *, int, HANDLE, mode_t, DWORD);
extern int cygwin_attach_handle_to_fd (char *, int, HANDLE, mode_t, DWORD);

#include <sys/resource.h>

/* External interface stuff */

typedef enum
  {
    CW_LOCK_PINFO,
    CW_UNLOCK_PINFO,
    CW_GETTHREADNAME,
    CW_GETPINFO,
    CW_SETPINFO,
    CW_SETTHREADNAME,
    CW_GETVERSIONINFO,
    CW_READ_V1_MOUNT_TABLES
  } cygwin_getinfo_types;

struct external_pinfo
  {
  pid_t pid;
  pid_t ppid;
  HANDLE hProcess;
  DWORD dwProcessId, dwSpawnedProcessId;
  uid_t uid;
  gid_t gid;
  pid_t pgid;
  pid_t sid;
  int ctty;
  mode_t umask;

  long start_time;
  struct rusage rusage_self;
  struct rusage rusage_children;

  char progname[MAX_PATH];

  DWORD strace_mask;
  HANDLE strace_file;

  DWORD process_state;
};

DWORD cygwin_internal (cygwin_getinfo_types, ...);
#endif /*WINVER*/

#define CW_NEXTPID 0x80000000	// or with pid to get next one

/* Flags associated with process_state */
enum
{
  PID_NOT_IN_USE       = 0x0000, // Free entry.
  PID_IN_USE	       = 0x0001, // Entry in use.
  PID_ZOMBIE	       = 0x0002, // Child exited: no parent wait.
  PID_STOPPED	       = 0x0004, // Waiting for SIGCONT.
  PID_TTYIN	       = 0x0008, // Waiting for terminal input.
  PID_TTYOU	       = 0x0010, // Waiting for terminal output.
  PID_ORPHANED	       = 0x0020, // Member of an orphaned process group.
  PID_ACTIVE	       = 0x0040, // Pid accepts signals.
  PID_CYGPARENT	       = 0x0080, // Set if parent was a cygwin app.
  PID_SPLIT_HEAP       = 0x0100, // Set if the heap has been split,
				 //  which means we can't fork again.
  PID_CLEAR	       = 0x0200, // Flag that pid should be cleared from parent's
				 //  wait list
  PID_SOCKETS_USED     = 0x0400, // Set if process uses Winsock.
  PID_INITIALIZING     = 0x0800, // Set until ready to receive signals.
  PID_USETTY	       = 0x1000, // Setting this enables or disables cygwin's
				 //  tty support.  This is inherited by
				 //  all execed or forked processes.
  PID_REPARENT	       = 0x2000  // child has execed
};

#ifdef __cplusplus
};
#endif

#endif /* _SYS_CYGWIN_H */
