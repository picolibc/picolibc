/* shared_info.h: shared info for cygwin

   Copyright 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2008, 2009,
   2010, 2011 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "tty.h"
#include "security.h"
#include "mtinfo.h"
#include "limits.h"
#include "mount.h"

class user_info
{
  void initialize ();
public:
  LONG version;
  DWORD cb;
  bool warned_msdos;
  mount_info mountinfo;
  friend void dll_crt0_1 (void *);
  static void create (bool);
};
/******** Shared Info ********/
/* Data accessible to all tasks */


#define CURR_SHARED_MAGIC 0xb41ae342U

#define USER_VERSION   1
#define CURR_USER_MAGIC 0x6112afb3U

/* NOTE: Do not make gratuitous changes to the names or organization of the
   below class.  The layout is checksummed to determine compatibility between
   different cygwin versions. */
class shared_info
{
  LONG version;
  DWORD cb;
 public:
  DWORD heap_chunk;
  DWORD sys_mount_table_counter;
  tty_list tty;
  LONG last_used_bindresvport;
  DWORD obcaseinsensitive;
  mtinfo mt;

  void initialize ();
  void init_obcaseinsensitive ();
  unsigned heap_chunk_size ();
  static void create ();
};

extern shared_info *cygwin_shared;
extern user_info *user_shared;
#define mount_table (&(user_shared->mountinfo))
extern HANDLE cygwin_user_h;

enum shared_locations
{
  SH_CYGWIN_SHARED,
  SH_USER_SHARED,
  SH_SHARED_CONSOLE,
  SH_MYSELF,
  SH_TOTAL_SIZE,
  SH_JUSTCREATE,
  SH_JUSTOPEN

};

void memory_init (bool) __attribute__ ((regparm(1)));
void __stdcall shared_destroy ();

#define shared_align_past(p) \
  ((char *) (system_info.dwAllocationGranularity * \
	     (((DWORD) ((p) + 1) + system_info.dwAllocationGranularity - 1) / \
	      system_info.dwAllocationGranularity)))

#ifdef _FHANDLER_H_
struct console_state
{
  tty_min tty_min_state;
  dev_console dev_state;
};
#endif

HANDLE get_shared_parent_dir ();
HANDLE get_session_parent_dir ();
char *__stdcall shared_name (char *, const char *, int);
WCHAR *__stdcall shared_name (WCHAR *, const WCHAR *, int);
void *__stdcall open_shared (const WCHAR *, int, HANDLE&, DWORD,
			     shared_locations, PSECURITY_ATTRIBUTES = &sec_all,
			     DWORD = FILE_MAP_READ | FILE_MAP_WRITE);
void *__stdcall open_shared (const WCHAR *, int, HANDLE&, DWORD,
			     shared_locations *, PSECURITY_ATTRIBUTES = &sec_all,
			     DWORD = FILE_MAP_READ | FILE_MAP_WRITE);
extern void user_shared_create (bool reinit);
extern void init_installation_root ();
extern WCHAR installation_root[PATH_MAX];
extern UNICODE_STRING installation_key;
