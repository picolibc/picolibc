/* globals.cc - Define global variables here.

   Copyright 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005,
   2006, 2007, 2008, 2009 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#define _GLOBALS_H 1
#include "winsup.h"
#include "cygtls.h"
#include "perprocess.h"
#include "cygprops.h"
#include "thread.h"
#include <malloc.h>
#include <cygwin/version.h>

HANDLE NO_COPY hMainThread;
HANDLE NO_COPY hProcToken;
HANDLE NO_COPY hProcImpToken;
HMODULE NO_COPY cygwin_hmodule;
HANDLE hExeced;
int NO_COPY sigExeced;

/* program exit the program */

enum exit_states
  {
    ES_NOT_EXITING = 0,
    ES_PROCESS_LOCKED,
    ES_EVENTS_TERMINATE,
    ES_THREADTERM,
    ES_SIGNAL,
    ES_CLOSEALL,
    ES_HUP_PGRP,
    ES_HUP_SID,
    ES_EXEC_EXIT,
    ES_TITLE,
    ES_TTY_TERMINATE,
    ES_FINAL
  };

exit_states NO_COPY exit_state;

SYSTEM_INFO system_info;

/* Set in init.cc.  Used to check if Cygwin DLL is dynamically loaded. */
int NO_COPY dynamically_loaded;

bool display_title;
bool strip_title_path;
bool allow_glob = true;
bool NO_COPY in_forkee;

int __argc_safe;
int __argc;
char **__argv;
#ifdef NEWVFORK
vfork_save NO_COPY *main_vfork;
#endif

_cygtls NO_COPY *_main_tls /* !globals.h */;

bool NO_COPY cygwin_finished_initializing;

bool NO_COPY _cygwin_testing;

char NO_COPY almost_null[1];

char *old_title;

/* Define globally used, but readonly variables using the _RDATA attribute. */
#define _RDATA __attribute__ ((section(".rdata")))

/* Heavily-used const UNICODE_STRINGs are defined here once.  The idea is a
   speed improvement by not having to initialize a UNICODE_STRING every time
   we make a string comparison.  The strings are not defined as const,
   because the respective NT functions are not taking const arguments
   and doing so here results in lots of extra casts for no good reason.
   Rather, the strings are placed in the R/O section .rdata, so we get
   a SEGV if some code erroneously tries to overwrite these strings. */
#define _ROU(_s) \
        { Length: sizeof (_s) - sizeof (WCHAR), \
          MaximumLength: sizeof (_s), \
          Buffer: (PWSTR) (_s) }
UNICODE_STRING _RDATA ro_u_empty = _ROU (L"");
UNICODE_STRING _RDATA ro_u_lnk = _ROU (L".lnk");
UNICODE_STRING _RDATA ro_u_exe = _ROU (L".exe");
UNICODE_STRING _RDATA ro_u_dll = _ROU (L".dll");
UNICODE_STRING _RDATA ro_u_com = _ROU (L".com");
UNICODE_STRING _RDATA ro_u_scr = _ROU (L".scr");
UNICODE_STRING _RDATA ro_u_sys = _ROU (L".sys");
UNICODE_STRING _RDATA ro_u_proc = _ROU (L"proc");
UNICODE_STRING _RDATA ro_u_pmem = _ROU (L"\\device\\physicalmemory");
UNICODE_STRING _RDATA ro_u_natp = _ROU (L"\\??\\");
UNICODE_STRING _RDATA ro_u_uncp = _ROU (L"\\??\\UNC\\");
UNICODE_STRING _RDATA ro_u_mtx = _ROU (L"mtx");
UNICODE_STRING _RDATA ro_u_csc = _ROU (L"CSC-CACHE");
UNICODE_STRING _RDATA ro_u_fat = _ROU (L"FAT");
UNICODE_STRING _RDATA ro_u_mvfs = _ROU (L"MVFS");
UNICODE_STRING _RDATA ro_u_nfs = _ROU (L"NFS");
UNICODE_STRING _RDATA ro_u_ntfs = _ROU (L"NTFS");
UNICODE_STRING _RDATA ro_u_sunwnfs = _ROU (L"SUNWNFS");
UNICODE_STRING _RDATA ro_u_udf = _ROU (L"UDF");
UNICODE_STRING _RDATA ro_u_unixfs = _ROU (L"UNIXFS");
UNICODE_STRING _RDATA ro_u_volume = _ROU (L"\\??\\Volume{");
#undef _ROU

/* Cygwin properties are meant to be readonly data placed in the DLL, but
   which can be changed by external tools to make adjustments to the
   behaviour of a DLL based on the binary of the DLL itself.  This is
   different from $CYGWIN since it only affects that very DLL, not all
   DLLs which have access to the $CYGWIN environment variable. */
cygwin_props_t _RDATA cygwin_props =
{
  CYGWIN_PROPS_MAGIC,
  sizeof (cygwin_props_t),
  0
};

#undef _RDATA

extern "C"
{
  /* This is an exported copy of environ which can be used by DLLs
     which use cygwin.dll.  */
  char **__cygwin_environ;
  char ***main_environ = &__cygwin_environ;
  /* __progname used in getopt error message */
  char *__progname;
  static MTinterface _mtinterf;
  struct per_process __cygwin_user_data =
  {/* initial_sp */ 0, /* magic_biscuit */ 0,
   /* dll_major */ CYGWIN_VERSION_DLL_MAJOR,
   /* dll_major */ CYGWIN_VERSION_DLL_MINOR,
   /* impure_ptr_ptr */ NULL, /* envptr */ NULL,
   /* malloc */ malloc, /* free */ free,
   /* realloc */ realloc,
   /* fmode_ptr */ NULL, /* main */ NULL, /* ctors */ NULL,
   /* dtors */ NULL, /* data_start */ NULL, /* data_end */ NULL,
   /* bss_start */ NULL, /* bss_end */ NULL,
   /* calloc */ calloc,
   /* premain */ {NULL, NULL, NULL, NULL},
   /* run_ctors_p */ 0,
   /* unused */ {0, 0, 0, 0, 0, 0, 0},
   /* cxx_malloc */ &default_cygwin_cxx_malloc,
   /* hmodule */ NULL,
   /* api_major */ CYGWIN_VERSION_API_MAJOR,
   /* api_minor */ CYGWIN_VERSION_API_MINOR,
   /* unused2 */ {0, 0, 0, 0, 0, 0},
   /* threadinterface */ &_mtinterf,
   /* impure_ptr */ _GLOBAL_REENT,
  };
  bool ignore_case_with_glob;
  int _check_for_executable = true;
};

int NO_COPY __api_fatal_exit_val = 1;
