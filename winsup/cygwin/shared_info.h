/* shared_info.h: shared info for cygwin

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "tty.h"
#include "security.h"
#include "mtinfo.h"
#include "limits.h"
#include "mount.h"
#include "loadavg.h"

#define CURR_USER_MAGIC 0xab1fcce8U

class user_info
{
  void initialize ();
public:
  LONG version;
  DWORD cb;
  bool warned_msdos;
  bool warned_notty;
  bool warned_nonativesyms;
  mount_info mountinfo;
  friend void dll_crt0_1 (void *);
  static void create (bool);
};

/******** Shared Info ********/
/* Data accessible to all tasks */


#define CURR_SHARED_MAGIC 0x9b1c0f25U

#define USER_VERSION   1

/* NOTE: Do not make gratuitous changes to the names or organization of the
   below class.  The layout is checksummed to determine compatibility between
   different cygwin versions. */
class shared_info
{
  LONG version;
  DWORD cb;
 public:
  tty_list tty;
  LONG last_used_bindresvport;
  DWORD obcaseinsensitive;
  mtinfo mt;
  loadavginfo loadavg;

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
  SH_MYSELF,
  SH_SHARED_CONSOLE,
  SH_TOTAL_SIZE,
  SH_JUSTCREATE,
  SH_JUSTOPEN

};

void memory_init ();
void __stdcall shared_destroy ();

#define shared_align_past(p) \
  ((char *) (system_info.dwAllocationGranularity * \
	     (((DWORD) ((p) + 1) + system_info.dwAllocationGranularity - 1) / \
	      system_info.dwAllocationGranularity)))

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
