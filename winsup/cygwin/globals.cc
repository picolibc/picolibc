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
#include "thread.h"
#include <malloc.h>
#include <cygwin/version.h>

HANDLE NO_COPY hMainProc = (HANDLE) -1;
HANDLE NO_COPY hMainThread;
HANDLE NO_COPY hProcToken;
HANDLE NO_COPY hProcImpToken;
HMODULE NO_COPY cygwin_hmodule;
HANDLE hExeced;

/* Codepage and multibyte string specific stuff. */
UINT active_codepage;

/* program exit the program */

enum exit_states
  {
    ES_NOT_EXITING = 0,
    ES_PROCESS_LOCKED,
    ES_GLOBAL_DTORS,
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
