/* mount.h: mount definitions.

   Copyright 1996, 1997, 1998, 2000, 2001, 2002, 2003, 2004, 2005,
   2006, 2007, 2008, 2009 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _MOUNT_H
#define _MOUNT_H

class fs_info
{
  struct status_flags
  {
    ULONG flags;		  /* Volume flags */
    ULONG samba_version;	  /* Samba version if available */
    ULONG name_len;		  /* MaximumComponentNameLength */
    unsigned is_remote_drive		: 1;
    unsigned has_acls			: 1;
    unsigned hasgood_inode		: 1;
    unsigned caseinsensitive		: 1;
    union
    {
      struct
      {
	unsigned is_fat			: 1;
	unsigned is_ntfs		: 1;
	unsigned is_samba		: 1;
	unsigned is_nfs			: 1;
	unsigned is_netapp 		: 1;
	unsigned is_cdrom		: 1;
	unsigned is_udf			: 1;
	unsigned is_csc_cache		: 1;
	unsigned is_sunwnfs		: 1;
	unsigned is_unixfs		: 1;
	unsigned is_mvfs		: 1;
      };
      unsigned long fs_flags;
    };
  } status;
  ULONG sernum;
  char fsn[80];
  unsigned long got_fs () { return status.fs_flags; }

 public:
  void clear ()
  {
    memset (&status, 0 , sizeof status);
    sernum = 0UL; 
    fsn[0] = '\0';
  }
  fs_info () { clear (); }

  IMPLEMENT_STATUS_FLAG (ULONG, flags)
  IMPLEMENT_STATUS_FLAG (ULONG, samba_version)
  IMPLEMENT_STATUS_FLAG (ULONG, name_len)
  IMPLEMENT_STATUS_FLAG (bool, is_remote_drive)
  IMPLEMENT_STATUS_FLAG (bool, has_acls)
  IMPLEMENT_STATUS_FLAG (bool, hasgood_inode)
  IMPLEMENT_STATUS_FLAG (bool, caseinsensitive)
  IMPLEMENT_STATUS_FLAG (bool, is_fat)
  IMPLEMENT_STATUS_FLAG (bool, is_ntfs)
  IMPLEMENT_STATUS_FLAG (bool, is_samba)
  IMPLEMENT_STATUS_FLAG (bool, is_nfs)
  IMPLEMENT_STATUS_FLAG (bool, is_netapp)
  IMPLEMENT_STATUS_FLAG (bool, is_cdrom)
  IMPLEMENT_STATUS_FLAG (bool, is_udf)
  IMPLEMENT_STATUS_FLAG (bool, is_csc_cache)
  IMPLEMENT_STATUS_FLAG (bool, is_sunwnfs)
  IMPLEMENT_STATUS_FLAG (bool, is_unixfs)
  IMPLEMENT_STATUS_FLAG (bool, is_mvfs)
  ULONG serial_number () const { return sernum; }

  int has_buggy_open () const {return is_sunwnfs ();}
  int has_buggy_fileid_dirinfo () const {return is_unixfs ();}
  const char *fsname () const { return fsn[0] ? fsn : "unknown"; }

  bool update (PUNICODE_STRING, HANDLE) __attribute__ ((regparm (3)));
};

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

  static bool got_usr_bin;
  static bool got_usr_lib;
  static int root_idx;

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
#endif
