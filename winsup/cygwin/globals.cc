/* globals.cc - Define global variables here.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#define NO_GLOBALS_H
#include "winsup.h"
#include "cygtls.h"
#include "perprocess.h"
#include "thread.h"
#include <malloc.h>
#include <cygwin/version.h>

HANDLE NO_COPY hMainThread;
HANDLE NO_COPY hProcToken;
HANDLE NO_COPY hProcImpToken;
HANDLE my_wr_proc_pipe;
HMODULE NO_COPY cygwin_hmodule;
HMODULE NO_COPY hntdll;
int NO_COPY sigExeced;
WCHAR windows_system_directory[MAX_PATH];
UINT windows_system_directory_length;
#ifndef __x86_64__
WCHAR system_wow64_directory[MAX_PATH];
UINT system_wow64_directory_length;
#endif /* !__x86_64__ */
WCHAR global_progname[NT_MAX_PATH];

/* program exit the program */

enum exit_states
{
  ES_NOT_EXITING = 0,
  ES_EXIT_STARTING,
  ES_SIGNAL_EXIT,
  ES_PROCESS_LOCKED,
  ES_EVENTS_TERMINATE,
  ES_SIGNAL,
  ES_CLOSEALL,
  ES_THREADTERM,
  ES_HUP_PGRP,
  ES_HUP_SID,
  ES_TTY_TERMINATE,
  ES_FINAL
};

/* The type of symlink to create.  The value is set depending on the
   "winsymlinks" setting of the CYGWIN environment variable. */
enum winsym_t
{
  WSYM_sysfile = 0,
  WSYM_lnk,
  WSYM_native,
  WSYM_nativestrict,
  WSYM_nfs
};

exit_states NO_COPY exit_state;

/* Set in init.cc.  Used to check if Cygwin DLL is dynamically loaded. */
int NO_COPY dynamically_loaded;

/* Some CYGWIN environment variable variables. */
bool allow_glob = true;
bool dos_file_warning;
bool ignore_case_with_glob;
bool pipe_byte;
bool reset_com;
bool wincmdln;
winsym_t allow_winsymlinks = WSYM_sysfile;

bool NO_COPY in_forkee;

/* Taken from BSD libc:
   This variable is zero until a process has created a pthread.  It is used
   to avoid calling locking functions in libc when they are not required.
   Note that this is moderately dangerous.  Do not rely on it if the public
   API is also used from a non-pthread thread like the signal thread. */
int NO_COPY __isthreaded = 0;

int __argc_safe;
int __argc;
char **__argv;

_cygtls NO_COPY *_main_tls /* !globals.h */;

bool NO_COPY cygwin_finished_initializing;

bool NO_COPY _cygwin_testing;

char NO_COPY almost_null[1];

extern "C" {

/* We never have a collate load error. */
const int __collate_load_error = 0;

  /* Heavily-used const UNICODE_STRINGs are defined here once.  The idea is a
     speed improvement by not having to initialize a UNICODE_STRING every time
     we make a string comparison.  The _RDATA trick allows defining the strings
     as const (so we get a SEGV if some code erroneously tries to overwrite
     them), while declaring them as non-const in the auto-generated globals.h.
     The strings are usually used in NT functions which don't take const
     arguments.  We avoid a lot of extra casts here...
     Note:  The "extern" is required, otherwise either the variables are dropped
            entirely, or C++ name mangling is applied despite the extern "C"
	    bracket, depending on the compiler version */
#ifndef _RDATA
#  define _RDATA const
#endif

#define _ROU(_s) \
	  { Length: sizeof (_s) - sizeof (WCHAR), \
	    MaximumLength: sizeof (_s), \
	    Buffer: (PWSTR) (_s) }
  extern UNICODE_STRING _RDATA ro_u_empty = _ROU (L"");
  extern UNICODE_STRING _RDATA ro_u_lnk = _ROU (L".lnk");
  extern UNICODE_STRING _RDATA ro_u_exe = _ROU (L".exe");
  extern UNICODE_STRING _RDATA ro_u_dll = _ROU (L".dll");
  extern UNICODE_STRING _RDATA ro_u_com = _ROU (L".com");
  extern UNICODE_STRING _RDATA ro_u_scr = _ROU (L".scr");
  extern UNICODE_STRING _RDATA ro_u_sys = _ROU (L".sys");
  extern UNICODE_STRING _RDATA ro_u_proc = _ROU (L"proc");
  extern UNICODE_STRING _RDATA ro_u_dev = _ROU (L"dev");
  extern UNICODE_STRING _RDATA ro_u_natp = _ROU (L"\\??\\");
  extern UNICODE_STRING _RDATA ro_u_uncp = _ROU (L"\\??\\UNC\\");
  extern UNICODE_STRING _RDATA ro_u_mtx = _ROU (L"mtx");
  extern UNICODE_STRING _RDATA ro_u_csc = _ROU (L"CSC-CACHE");
  extern UNICODE_STRING _RDATA ro_u_fat = _ROU (L"FAT");
  extern UNICODE_STRING _RDATA ro_u_mvfs = _ROU (L"MVFS");
  extern UNICODE_STRING _RDATA ro_u_nfs = _ROU (L"NFS");
  extern UNICODE_STRING _RDATA ro_u_ntfs = _ROU (L"NTFS");
  /* No typo!  It's actually "SF", not "FS", and the trailing NUL is counted
     in the reply from the filesystem. */
  extern UNICODE_STRING _RDATA ro_u_prlfs = _ROU (L"PrlSF\0");
  extern UNICODE_STRING _RDATA ro_u_refs = _ROU (L"ReFS");
  extern UNICODE_STRING _RDATA ro_u_udf = _ROU (L"UDF");
  extern UNICODE_STRING _RDATA ro_u_unixfs = _ROU (L"UNIXFS");
  extern UNICODE_STRING _RDATA ro_u_nwfs = _ROU (L"NWFS");
  extern UNICODE_STRING _RDATA ro_u_ncfsd = _ROU (L"NcFsd");
  extern UNICODE_STRING _RDATA ro_u_afs = _ROU (L"AFSRDRFsd");
  extern UNICODE_STRING _RDATA ro_u_volume = _ROU (L"\\??\\Volume{");
  extern UNICODE_STRING _RDATA ro_u_pipedir = _ROU (L"\\\\?\\PIPE\\");
  extern UNICODE_STRING _RDATA ro_u_globalroot = _ROU (L"\\\\.\\GLOBALROOT");
  extern UNICODE_STRING _RDATA ro_u_null = _ROU (L"\\Device\\Null");
  extern UNICODE_STRING _RDATA ro_u_natdir = _ROU (L"Directory");
  extern UNICODE_STRING _RDATA ro_u_natsyml = _ROU (L"SymbolicLink");
  extern UNICODE_STRING _RDATA ro_u_natdev = _ROU (L"Device");
  #undef _ROU

  /* This is an exported copy of environ which can be used by DLLs
     which use cygwin.dll.  */
  char **__cygwin_environ;
#ifndef __x86_64__
  char ***main_environ = &__cygwin_environ;
#endif
  /* __progname used in getopt error message */
  char *__progname;
  char *program_invocation_name;
  char *program_invocation_short_name;
  static MTinterface _mtinterf;
  struct per_process __cygwin_user_data =
  {/* initial_sp */ 0, /* magic_biscuit */ 0,
   /* dll_major */ CYGWIN_VERSION_DLL_MAJOR,
   /* dll_major */ CYGWIN_VERSION_DLL_MINOR,
   /* impure_ptr_ptr */ NULL,
#ifndef __x86_64__
   /* envptr */ NULL,
#endif
   /* malloc */ malloc, /* free */ free,
   /* realloc */ realloc,
   /* fmode_ptr */ NULL, /* main */ NULL, /* ctors */ NULL,
   /* dtors */ NULL, /* data_start */ NULL, /* data_end */ NULL,
   /* bss_start */ NULL, /* bss_end */ NULL,
   /* calloc */ calloc,
   /* premain */ {NULL, NULL, NULL, NULL},
   /* run_ctors_p */ 0,
   /* unused */ {},
   /* cxx_malloc */ &default_cygwin_cxx_malloc,
   /* hmodule */ NULL,
   /* api_major */ 0,
   /* api_minor */ 0,
   /* unused2 */ {},
   /* posix_memalign */ posix_memalign,
   /* pseudo_reloc_start */ NULL,
   /* pseudo_reloc_end */ NULL,
   /* image_base */ NULL,
   /* threadinterface */ &_mtinterf,
   /* impure_ptr */ _GLOBAL_REENT,
  };
  int _check_for_executable = true;
};

int NO_COPY __api_fatal_exit_val = 1;
