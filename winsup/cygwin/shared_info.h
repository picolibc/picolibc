/* shared_info.h: shared info for cygwin

   Copyright 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2008, 2009 Red Hat, Inc.

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
public:
  DWORD version;
  DWORD cb;
  bool warned_msdos;
  mount_info mountinfo;
};
/******** Shared Info ********/
/* Data accessible to all tasks */

#define SHARED_VERSION (unsigned)(cygwin_version.api_major << 8 | \
				  cygwin_version.api_minor)
#define SHARED_VERSION_MAGIC CYGWIN_VERSION_MAGIC (SHARED_MAGIC, SHARED_VERSION)

#define SHARED_INFO_CB 31136

#define CURR_SHARED_MAGIC 0x18da899eU

#define USER_VERSION	1	// increment when mount table changes and
#define USER_VERSION_MAGIC CYGWIN_VERSION_MAGIC (USER_MAGIC, USER_VERSION)
#define CURR_USER_MAGIC 0xb2232e71U

/* NOTE: Do not make gratuitous changes to the names or organization of the
   below class.  The layout is checksummed to determine compatibility between
   different cygwin versions. */
class shared_info
{
  DWORD version;
  DWORD cb;
 public:
  unsigned heap_chunk;
  bool heap_slop_inited;
  unsigned heap_slop;
  DWORD sys_mount_table_counter;
  tty_list tty;
  LONG last_used_bindresvport;
  DWORD obcaseinsensitive;
  mtinfo mt;

  void initialize ();
  void init_obcaseinsensitive ();
  unsigned heap_chunk_size ();
  unsigned heap_slop_size ();
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
void *__stdcall open_shared (const WCHAR *name, int n, HANDLE &shared_h,
			     DWORD size, shared_locations&,
			     PSECURITY_ATTRIBUTES psa = &sec_all,
			     DWORD access = FILE_MAP_READ | FILE_MAP_WRITE);
extern void user_shared_create (bool reinit);
extern void user_shared_initialize ();
extern void init_installation_root ();
extern WCHAR installation_root[PATH_MAX];
extern UNICODE_STRING installation_key;
