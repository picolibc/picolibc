/* shared_sec.h: shared info for cygwin

   Copyright 1998, 1999, 2000 Cygnus Solutions.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

/* Mount table entry */

class mount_item
{
public:
  /* FIXME: Nasty static allocation.  Need to have a heap in the shared
     area [with the user being able to configure at runtime the max size].  */

  /* Win32-style mounted partition source ("C:\foo\bar").
     native_path[0] == 0 for unused entries.  */
  char native_path[MAX_PATH];
  int native_pathlen;

  /* POSIX-style mount point ("/foo/bar") */
  char posix_path[MAX_PATH];
  int posix_pathlen;

  unsigned flags;

  void init (const char *dev, const char *path, unsigned flags);

  struct mntent *getmntent ();
};

/* Warning: Decreasing this value will cause cygwin.dll to ignore existing
   higher numbered registry entries.  Don't change this number willy-nilly.
   What we need is to have a more dynamic allocation scheme, but the current
   scheme should be satisfactory for a long while yet.  */
#define MAX_MOUNTS 30

class mount_info
{
  int posix_sorted[MAX_MOUNTS];
  int native_sorted[MAX_MOUNTS];
public:
  int nmounts;
  mount_item mount[MAX_MOUNTS];

  /* Strings used by getmntent(). */
  char mnt_type[20];
  char mnt_opts[20];
  char mnt_fsname[MAX_PATH];
  char mnt_dir[MAX_PATH];

  /* cygdrive_prefix is used as the root of the path automatically
     prepended to a path when the path has no associated mount.
     cygdrive_flags are the default flags for the cygdrives. */
  char cygdrive[MAX_PATH];
  size_t cygdrive_len;
  unsigned cygdrive_flags;

  /* Increment when setting up a reg_key if mounts area had to be
     created so we know when we need to import old mount tables. */
  int had_to_create_mount_areas;

  void init ();
  int add_item (const char *dev, const char *path, unsigned flags, int reg_p);
  int del_item (const char *path, unsigned flags, int reg_p);

  void from_registry ();
  int add_reg_mount (const char * native_path, const char * posix_path,
		      unsigned mountflags);
  int del_reg_mount (const char * posix_path, unsigned mountflags);

  unsigned set_flags_from_win32_path (const char *path);
  int conv_to_win32_path (const char *src_path, char *win32_path,
			  char *full_win32_path, DWORD &devn, int &unit,
			  unsigned *flags = NULL);
  int conv_to_posix_path (const char *src_path, char *posix_path,
			  int keep_rel_p);
  struct mntent *getmntent (int x);

  int write_cygdrive_info_to_registry (const char *cygdrive_prefix, unsigned flags);
  int remove_cygdrive_info_from_registry (const char *cygdrive_prefix, unsigned flags);
  int get_cygdrive_prefixes (char *user, char *system);

  void import_v1_mounts ();

private:

  void sort ();
  void read_mounts (reg_key& r);
  void read_v1_mounts (reg_key r, unsigned which);
  void mount_slash ();
  void to_registry ();

  int cygdrive_win32_path (const char *src, char *dst, int trailing_slash_p);
  void cygdrive_posix_path (const char *src, char *dst, int trailing_slash_p);
  void slash_drive_to_win32_path (const char *path, char *buf, int trailing_slash_p);
  void read_cygdrive_info_from_registry ();
};

/******** Shared Info ********/
/* Data accessible to all tasks */

class shared_info
{
  DWORD inited;

public:
  /* FIXME: Doesn't work if more than one user on system. */
  mount_info mount;

  int heap_chunk_in_mb;
  unsigned heap_chunk_size (void);

  tty_list tty;
  delqueue_list delqueue;
  void initialize (void);
};

extern shared_info *cygwin_shared;
extern HANDLE cygwin_shared_h;
extern HANDLE console_shared_h;

void __stdcall shared_init (void);
void __stdcall shared_terminate (void);

char *__stdcall shared_name (const char *, int);
void *__stdcall open_shared (const char *name, HANDLE &shared_h, DWORD size, void *addr);
