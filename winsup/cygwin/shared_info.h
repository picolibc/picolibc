/* shared_info.h: shared info for cygwin

   Copyright 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2008 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "tty.h"
#include "security.h"
#include "mtinfo.h"
#include "limits.h"

/* Mount table entry */

class mount_item
{
 public:
  /* FIXME: Nasty static allocation.  Need to have a heap in the shared
     area [with the user being able to configure at runtime the max size].  */
  /* Win32-style mounted partition source ("C:\foo\bar").
     native_path[0] == 0 for unused entries.  */
  char native_path[CYG_MAX_PATH];
  int native_pathlen;

  /* POSIX-style mount point ("/foo/bar") */
  char posix_path[CYG_MAX_PATH];
  int posix_pathlen;

  unsigned flags;

  void init (const char *dev, const char *path, unsigned flags);

  struct mntent *getmntent ();
  int build_win32 (char *, const char *, unsigned *, unsigned);
};

/* Warning: Decreasing this value will cause cygwin.dll to ignore existing
   higher numbered registry entries.  Don't change this number willy-nilly.
   What we need is to have a more dynamic allocation scheme, but the current
   scheme should be satisfactory for a long while yet.  */
#define MAX_MOUNTS 30

#define USER_VERSION	1	// increment when mount table changes and
#define USER_VERSION_MAGIC CYGWIN_VERSION_MAGIC (USER_MAGIC, USER_VERSION)
#define CURR_USER_MAGIC 0xb2232e71U

class reg_key;
struct device;

/* NOTE: Do not make gratuitous changes to the names or organization of the
   below class.  The layout is checksummed to determine compatibility between
   different cygwin versions. */
class mount_info
{
 public:
  DWORD sys_mount_table_counter;
  int nmounts;
  mount_item mount[MAX_MOUNTS];

  /* cygdrive_prefix is used as the root of the path automatically
     prepended to a path when the path has no associated mount.
     cygdrive_flags are the default flags for the cygdrives. */
  char cygdrive[CYG_MAX_PATH];
  size_t cygdrive_len;
  unsigned cygdrive_flags;
 private:
  int posix_sorted[MAX_MOUNTS];
  int native_sorted[MAX_MOUNTS];

 public:
  void init ();
  int add_item (const char *dev, const char *path, unsigned flags);
  int del_item (const char *path, unsigned flags);

  unsigned set_flags_from_win32_path (const char *path);
  int conv_to_win32_path (const char *src_path, char *dst, device&,
			  unsigned *flags = NULL);
  int conv_to_posix_path (PWCHAR src_path, char *posix_path,
			  int keep_rel_p);
  int conv_to_posix_path (const char *src_path, char *posix_path,
			  int keep_rel_p);
  struct mntent *getmntent (int x);

  int write_cygdrive_info (const char *cygdrive_prefix, unsigned flags);
  int get_cygdrive_info (char *user, char *system, char* user_flags,
			 char* system_flags);
  void cygdrive_posix_path (const char *src, char *dst, int trailing_slash_p);
  int get_mounts_here (const char *parent_dir, int,
		       PUNICODE_STRING mount_points,
		       PUNICODE_STRING cygd);

 private:
  void sort ();
  void mount_slash ();
  void create_root_entry (const PWCHAR root);

  bool from_fstab_line (char *line, bool user);
  bool from_fstab (bool user, WCHAR [], PWCHAR);

  int cygdrive_win32_path (const char *src, char *dst, int& unit);
};

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

#define SHARED_INFO_CB 39328

#define CURR_SHARED_MAGIC 0x398d8baU

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
  WCHAR installation_root[PATH_MAX];
  DWORD obcaseinsensitive;
  mtinfo mt;

  void initialize ();
  void init_installation_root ();
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

void __stdcall memory_init ();
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
void *__stdcall open_shared (const char *name, int n, HANDLE &shared_h, DWORD size,
			     shared_locations&, PSECURITY_ATTRIBUTES psa = &sec_all,
			     DWORD access = FILE_MAP_READ | FILE_MAP_WRITE);
extern void user_shared_create (bool reinit);
extern void user_shared_initialize ();

