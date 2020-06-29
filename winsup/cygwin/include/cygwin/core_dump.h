/* core_dump.h

   Written by Egor Duda <deo@logos-m.ru>

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _CYGWIN_CORE_DUMP_H
#define _CYGWIN_CORE_DUMP_H

/*
  Note that elfcore_grok_win32pstatus() in libbfd relies on the precise layout
  of these structures.
*/

#include <windows.h>

#define	NOTE_INFO_PROCESS	1
#define	NOTE_INFO_THREAD	2
#define	NOTE_INFO_MODULE	3
#define	NOTE_INFO_MODULE64	4

struct win32_core_process_info
{
  DWORD pid;
  DWORD signal;
  DWORD command_line_size;
  char command_line[1];
}
#ifdef __GNUC__
  __attribute__ ((__packed__))
#endif
;

struct win32_core_thread_info
{
  DWORD tid;
  BOOL is_active_thread;
  CONTEXT thread_context;
}
#ifdef __GNUC__
  __attribute__ ((__packed__))
#endif
;

/* Used with data_type NOTE_INFO_MODULE or NOTE_INFO_MODULE64, depending on
   arch */
struct win32_core_module_info
{
  void* base_address;
  DWORD module_name_size;
  char module_name[1];
}
#ifdef __GNUC__
  __attribute__ ((__packed__))
#endif
;

struct win32_pstatus
{
  DWORD data_type;
  union
    {
      struct win32_core_process_info process_info;
      struct win32_core_thread_info thread_info;
      struct win32_core_module_info module_info;
    } data ;
}
#ifdef __GNUC__
  __attribute__ ((__packed__))
#endif
;

typedef struct win32_pstatus win32_pstatus_t ;

#endif /* _CYGWIN_CORE_DUMP_H */
