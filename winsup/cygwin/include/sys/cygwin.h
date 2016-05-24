
/* sys/cygwin.h

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _SYS_CYGWIN_H
#define _SYS_CYGWIN_H

#include <sys/types.h>
#include <limits.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define _CYGWIN_SIGNAL_STRING "cYgSiGw00f"

#ifndef __x86_64__
/* DEPRECATED INTERFACES.  These are restricted to MAX_PATH length.
   Don't use in modern applications.  They don't exist on x86_64. */
extern int cygwin_win32_to_posix_path_list (const char *, char *)
  __attribute__ ((__deprecated__));
extern int cygwin_win32_to_posix_path_list_buf_size (const char *)
  __attribute__ ((__deprecated__));
extern int cygwin_posix_to_win32_path_list (const char *, char *)
  __attribute__ ((__deprecated__));
extern int cygwin_posix_to_win32_path_list_buf_size (const char *)
  __attribute__ ((__deprecated__));
extern int cygwin_conv_to_win32_path (const char *, char *)
  __attribute__ ((__deprecated__));
extern int cygwin_conv_to_full_win32_path (const char *, char *)
  __attribute__ ((__deprecated__));
extern int cygwin_conv_to_posix_path (const char *, char *)
  __attribute__ ((__deprecated__));
extern int cygwin_conv_to_full_posix_path (const char *, char *)
  __attribute__ ((__deprecated__));
#endif /* !__x86_64__ */

/* Use these interfaces in favor of the above. */

/* Possible 'what' values in calls to cygwin_conv_path/cygwin_create_path. */
enum
{
  CCP_POSIX_TO_WIN_A = 0, /* from is char*, to is char*       */
  CCP_POSIX_TO_WIN_W,	  /* from is char*, to is wchar_t*    */
  CCP_WIN_A_TO_POSIX,	  /* from is char*, to is char*       */
  CCP_WIN_W_TO_POSIX,	  /* from is wchar_t*, to is char*    */

  CCP_CONVTYPE_MASK = 3,

  /* Or these values to the above as needed. */
  CCP_ABSOLUTE = 0,	  	/* Request absolute path (default). 	*/
  CCP_RELATIVE = 0x100,    	/* Request to keep path relative.   	*/
  CCP_PROC_CYGDRIVE = 0x200,   	/* Request to return /proc/cygdrive
				   path (only with CCP_*_TO_POSIX).   */

  CCP_CONVFLAGS_MASK = 0x300,
};
typedef unsigned int cygwin_conv_path_t;

/* If size is 0, cygwin_conv_path returns the required buffer size in bytes.
   Otherwise, it returns 0 on success, or -1 on error and errno is set to
   one of the below values:

    EINVAL        what has an invalid value.
    EFAULT        from or to point into nirvana.
    ENAMETOOLONG  the resulting path is longer than 32K, or, in case
		  of what == CCP_POSIX_TO_WIN_A, longer than MAX_PATH.
    ENOSPC        size is less than required for the conversion.
*/
extern ssize_t cygwin_conv_path (cygwin_conv_path_t what, const void *from,
				 void *to, size_t size);
/* Same, but handles path lists separated by colon or semicolon. */
extern ssize_t cygwin_conv_path_list (cygwin_conv_path_t what, const void *from,
				 void *to, size_t size);
/* Allocate a buffer for the conversion result using malloc(3), and return
   a pointer to it.  Returns NULL if something goes wrong with errno set
   to one of the above values, or to ENOMEM if malloc fails. */
extern void *cygwin_create_path (cygwin_conv_path_t what, const void *from);

extern pid_t cygwin_winpid_to_pid (int);
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
    CW_CYGTLS_PADSIZE,
    CW_SET_DOS_FILE_WARNING,
    CW_SET_PRIV_KEY,
    CW_SETERRNO,
    CW_EXIT_PROCESS,
    CW_SET_EXTERNAL_TOKEN,
    CW_GET_INSTKEY,
    CW_INT_SETLOCALE,
    CW_CVT_MNT_OPTS,
    CW_LST_MNT_OPTS,
    CW_STRERROR,
    CW_CVT_ENV_TO_WINENV,
    CW_ALLOC_DRIVE_MAP,
    CW_MAP_DRIVE_MAP,
    CW_FREE_DRIVE_MAP,
    CW_SETENT,
    CW_GETENT,
    CW_ENDENT,
    CW_GETNSSSEP,
    CW_GETPWSID,
    CW_GETGRSID,
    CW_CYGNAME_FROM_WINNAME,
    CW_FIXED_ATEXIT,
    CW_GETNSS_PWD_SRC,
    CW_GETNSS_GRP_SRC,
    CW_EXCEPTION_RECORD_FROM_SIGINFO_T,
    CW_CYGHEAP_PROFTHR_ALL,
  } cygwin_getinfo_types;

#define CW_LOCK_PINFO CW_LOCK_PINFO
#define CW_UNLOCK_PINFO CW_UNLOCK_PINFO
#define CW_GETTHREADNAME CW_GETTHREADNAME
#define CW_GETPINFO CW_GETPINFO
#define CW_SETPINFO CW_SETPINFO
#define CW_SETTHREADNAME CW_SETTHREADNAME
#define CW_GETVERSIONINFO CW_GETVERSIONINFO
#define CW_READ_V1_MOUNT_TABLES CW_READ_V1_MOUNT_TABLES
#define CW_USER_DATA CW_USER_DATA
#define CW_PERFILE CW_PERFILE
#define CW_GET_CYGDRIVE_PREFIXES CW_GET_CYGDRIVE_PREFIXES
#define CW_GETPINFO_FULL CW_GETPINFO_FULL
#define CW_INIT_EXCEPTIONS CW_INIT_EXCEPTIONS
#define CW_GET_CYGDRIVE_INFO CW_GET_CYGDRIVE_INFO
#define CW_SET_CYGWIN_REGISTRY_NAME CW_SET_CYGWIN_REGISTRY_NAME
#define CW_GET_CYGWIN_REGISTRY_NAME CW_GET_CYGWIN_REGISTRY_NAME
#define CW_STRACE_TOGGLE CW_STRACE_TOGGLE
#define CW_STRACE_ACTIVE CW_STRACE_ACTIVE
#define CW_CYGWIN_PID_TO_WINPID CW_CYGWIN_PID_TO_WINPID
#define CW_EXTRACT_DOMAIN_AND_USER CW_EXTRACT_DOMAIN_AND_USER
#define CW_CMDLINE CW_CMDLINE
#define CW_CHECK_NTSEC CW_CHECK_NTSEC
#define CW_GET_ERRNO_FROM_WINERROR CW_GET_ERRNO_FROM_WINERROR
#define CW_GET_POSIX_SECURITY_ATTRIBUTE CW_GET_POSIX_SECURITY_ATTRIBUTE
#define CW_GET_SHMLBA CW_GET_SHMLBA
#define CW_GET_UID_FROM_SID CW_GET_UID_FROM_SID
#define CW_GET_GID_FROM_SID CW_GET_GID_FROM_SID
#define CW_GET_BINMODE CW_GET_BINMODE
#define CW_HOOK CW_HOOK
#define CW_ARGV CW_ARGV
#define CW_ENVP CW_ENVP
#define CW_DEBUG_SELF CW_DEBUG_SELF
#define CW_SYNC_WINENV CW_SYNC_WINENV
#define CW_CYGTLS_PADSIZE CW_CYGTLS_PADSIZE
#define CW_SET_DOS_FILE_WARNING CW_SET_DOS_FILE_WARNING
#define CW_SET_PRIV_KEY CW_SET_PRIV_KEY
#define CW_SETERRNO CW_SETERRNO
#define CW_EXIT_PROCESS CW_EXIT_PROCESS
#define CW_SET_EXTERNAL_TOKEN CW_SET_EXTERNAL_TOKEN
#define CW_GET_INSTKEY CW_GET_INSTKEY
#define CW_INT_SETLOCALE CW_INT_SETLOCALE
#define CW_CVT_MNT_OPTS CW_CVT_MNT_OPTS
#define CW_LST_MNT_OPTS CW_LST_MNT_OPTS
#define CW_STRERROR CW_STRERROR
#define CW_CVT_ENV_TO_WINENV CW_CVT_ENV_TO_WINENV
#define CW_ALLOC_DRIVE_MAP CW_ALLOC_DRIVE_MAP
#define CW_MAP_DRIVE_MAP CW_MAP_DRIVE_MAP
#define CW_FREE_DRIVE_MAP CW_FREE_DRIVE_MAP
#define CW_SETENT CW_SETENT
#define CW_GETENT CW_GETENT
#define CW_ENDENT CW_ENDENT
#define CW_GETNSSSEP CW_GETNSSSEP
#define CW_GETPWSID CW_GETPWSID
#define CW_GETGRSID CW_GETGRSID
#define CW_CYGNAME_FROM_WINNAME CW_CYGNAME_FROM_WINNAME
#define CW_FIXED_ATEXIT CW_FIXED_ATEXIT
#define CW_GETNSS_PWD_SRC CW_GETNSS_PWD_SRC
#define CW_GETNSS_GRP_SRC CW_GETNSS_GRP_SRC
#define CW_EXCEPTION_RECORD_FROM_SIGINFO_T CW_EXCEPTION_RECORD_FROM_SIGINFO_T
#define CW_CYGHEAP_PROFTHR_ALL CW_CYGHEAP_PROFTHR_ALL

/* Token type for CW_SET_EXTERNAL_TOKEN */
enum
{
  CW_TOKEN_IMPERSONATION = 0,
  CW_TOKEN_RESTRICTED    = 1
};

/* Source type for CW_GETNSS_PWD_SRC and CW_GETNSS_GRP_SRC. */
enum
{
  NSS_SRC_FILES = 1,
  NSS_SRC_DB = 2
};

/* Enumeration source constants for CW_SETENT called from mkpasswd/mkgroup. */
enum nss_enum_t
{
  ENUM_NONE = 0x00,
  ENUM_CACHE = 0x01,
  ENUM_FILES = 0x02,
  ENUM_BUILTIN = 0x04,
  ENUM_LOCAL = 0x08,
  ENUM_PRIMARY = 0x10,
  ENUM_TDOMS = 0x20,
  ENUM_TDOMS_ALL = 0x40,
  ENUM_ALL = 0x7f
};

#define CW_NEXTPID	0x80000000	/* or with pid to get next one */
uintptr_t cygwin_internal (cygwin_getinfo_types, ...);

/* Flags associated with process_state */
enum
{
  PID_IN_USE	       = 0x00001, /* Entry in use. */
  PID_UNUSED	       = 0x00002, /* Available. */
  PID_STOPPED	       = 0x00004, /* Waiting for SIGCONT. */
  PID_TTYIN	       = 0x00008, /* Waiting for terminal input. */
  PID_TTYOU	       = 0x00010, /* Waiting for terminal output. */
  PID_NOTCYGWIN	       = 0x00020, /* Set if process is not a cygwin app. */
  PID_ACTIVE	       = 0x00040, /* Pid accepts signals. */
  PID_CYGPARENT	       = 0x00080, /* Set if parent was a cygwin app. */
  PID_MAP_RW	       = 0x00100, /* Flag to open map rw. */
  PID_MYSELF	       = 0x00200, /* Flag that pid is me. */
  PID_NOCLDSTOP	       = 0x00400, /* Set if no SIGCHLD signal on stop. */
  PID_INITIALIZING     = 0x00800, /* Set until ready to receive signals. */
  PID_NEW	       = 0x01000, /* Available. */
  PID_ALLPIDS	       = 0x02000, /* used by pinfo scanner */
  PID_EXECED	       = 0x04000, /* redirect to original pid info block */
  PID_NOREDIR	       = 0x08000, /* don't redirect if execed */
  PID_EXITED	       = 0x40000000, /* Free entry. */
  PID_REAPED	       = 0x80000000  /* Reaped */
};

#ifdef WINVER

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

struct per_process_cxx_malloc;

struct per_process
{
  char *initial_sp;

  /* The offset of these 3 values can never change. */
  /* magic_biscuit is the size of this class and should never change. */
  uint32_t magic_biscuit;
  uint32_t dll_major;
  uint32_t dll_minor;

  struct _reent **impure_ptr_ptr;
#ifndef __x86_64__
  char ***envptr;
#endif

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

  /* non-zero if ctors have been run.  Inherited from parent. */
  int32_t run_ctors_p;

  DWORD_PTR unused[7];

  /* Pointers to real operator new/delete functions for forwarding.  */
  struct per_process_cxx_malloc *cxx_malloc;

  HMODULE hmodule;

  DWORD api_major;		/* API version that this program was */
  DWORD api_minor;		/*  linked with */
  /* For future expansion, so apps won't have to be relinked if we
     add an item. */
#ifdef __x86_64__
  DWORD_PTR unused2[4];
#else
  DWORD_PTR unused2[2];
#endif

  int (*posix_memalign)(void **, size_t, size_t);

  void *pseudo_reloc_start;
  void *pseudo_reloc_end;
  void *image_base;

#if defined (__INSIDE_CYGWIN__) && defined (__cplusplus)
  MTinterface *threadinterface;
#else
  void *threadinterface;
#endif
  struct _reent *impure_ptr;
};
#define per_process_overwrite offsetof (struct per_process, threadinterface)

#ifdef _PATH_PASSWD
extern HANDLE cygwin_logon_user (const struct passwd *, const char *);
#endif
extern void cygwin_set_impersonation_token (const HANDLE);

/* included if <windows.h> is included */
extern int cygwin_attach_handle_to_fd (char *, int, HANDLE, mode_t, DWORD);

extern void cygwin_premain0 (int, char **, struct per_process *);
extern void cygwin_premain1 (int, char **, struct per_process *);
extern void cygwin_premain2 (int, char **, struct per_process *);
extern void cygwin_premain3 (int, char **, struct per_process *);

#ifdef __CYGWIN__
#include <sys/resource.h>

#define TTY_CONSOLE	0x40000000

#define EXTERNAL_PINFO_VERSION_16_BIT 0
#define EXTERNAL_PINFO_VERSION_32_BIT 1
#define EXTERNAL_PINFO_VERSION_32_LP  2
#define EXTERNAL_PINFO_VERSION EXTERNAL_PINFO_VERSION_32_LP

#ifndef __INSIDE_CYGWIN__
typedef __uint16_t __uid16_t;
typedef __uint16_t __gid16_t;
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
  uid_t uid32;
  gid_t gid32;

  /* Only available if version >= EXTERNAL_PINFO_VERSION_32_LP */
  char *progname_long;
};
#endif /*__CYGWIN__*/
#endif /*WINVER*/

#ifdef __cplusplus
};
#endif
#endif /* _SYS_CYGWIN_H */
