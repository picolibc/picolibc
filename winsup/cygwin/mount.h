/* mount.h: mount definitions.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _MOUNT_H
#define _MOUNT_H

#define __CCP_APP_SLASH	0x10000000	/* Internal flag for conv_to_posix_path.
					   always append slash, even if path
					   is "X:\\" only. */

enum disk_type
{
  DT_NODISK,
  DT_CDROM,
  DT_FLOPPY,
  DT_HARDDISK,
  DT_SHARE_SMB,
  DT_SHARE_NFS
};

disk_type get_disk_type (LPCWSTR);

/* Don't add new fs types without adding them to fs_names in mount.cc!
   Don't reorder without reordering fs_names in mount.cc! */
enum fs_info_type
{
  none = 0,
  fat,
  exfat,
  ntfs,
  refs,
  samba,
  nfs,
  netapp,
  cdrom,
  udf,
  csc_cache,
  unixfs,
  mvfs,
  cifs,
  nwfs,
  ncfsd,
  afs,
  prlfs,
  /* Always last. */
  max_fs_type
};

extern struct fs_names_t {
    const char *name;
    bool block_device;
} fs_names[];

#define IMPLEMENT_FS_FLAG(type) \
  bool is_##type (bool val) { if (val) status.fs_type = type; return val; } \
  bool is_##type () const   { return status.fs_type == type; }

class fs_info
{
  struct status_flags
  {
    ULONG flags;		/* Volume flags */
    ULONG samba_version;	/* Samba version if available */
    ULONG name_len;		/* MaximumComponentNameLength */
    fs_info_type fs_type;	/* Filesystem type */
    unsigned is_remote_drive		: 1;
    unsigned has_acls			: 1;
    unsigned hasgood_inode		: 1;
    unsigned caseinsensitive		: 1;
    unsigned has_buggy_reopen		: 1;
    unsigned has_buggy_fileid_dirinfo	: 1;
    unsigned has_buggy_basic_info	: 1;
    unsigned has_dos_filenames_only	: 1;
  } status;
  ULONG sernum;			/* Volume Serial Number */
  char fsn[80];			/* Windows filesystem name */

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
  IMPLEMENT_STATUS_FLAG (bool, has_buggy_reopen)
  IMPLEMENT_STATUS_FLAG (bool, has_buggy_fileid_dirinfo)
  IMPLEMENT_STATUS_FLAG (bool, has_buggy_basic_info)
  IMPLEMENT_STATUS_FLAG (bool, has_dos_filenames_only)
  IMPLEMENT_FS_FLAG (fat)
  IMPLEMENT_FS_FLAG (exfat)
  IMPLEMENT_FS_FLAG (ntfs)
  IMPLEMENT_FS_FLAG (refs)
  IMPLEMENT_FS_FLAG (samba)
  IMPLEMENT_FS_FLAG (nfs)
  IMPLEMENT_FS_FLAG (netapp)
  IMPLEMENT_FS_FLAG (cdrom)
  IMPLEMENT_FS_FLAG (udf)
  IMPLEMENT_FS_FLAG (csc_cache)
  IMPLEMENT_FS_FLAG (unixfs)
  IMPLEMENT_FS_FLAG (mvfs)
  IMPLEMENT_FS_FLAG (cifs)
  IMPLEMENT_FS_FLAG (nwfs)
  IMPLEMENT_FS_FLAG (ncfsd)
  IMPLEMENT_FS_FLAG (afs)
  IMPLEMENT_FS_FLAG (prlfs)
  fs_info_type what_fs () const { return status.fs_type; }
  bool got_fs () const { return status.fs_type != none; }

  ULONG serial_number () const { return sernum; }

  const char *fsname () const { return fsn[0] ? fsn : "unknown"; }

  bool __reg3 update (PUNICODE_STRING, HANDLE);
  bool inited () const { return !!status.flags; }
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
#define MAX_MOUNTS 64

class reg_key;
struct device;

/* NOTE: Do not make gratuitous changes to the names or organization of the
   below class.  The layout is checksummed to determine compatibility between
   different cygwin versions. */
class mount_info
{
 public:
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
  void init (bool);
  int add_item (const char *dev, const char *path, unsigned flags);
  int del_item (const char *path, unsigned flags);

  int conv_to_win32_path (const char *src_path, char *dst, device&,
			  unsigned *flags = NULL);
  int conv_to_posix_path (PWCHAR src_path, char *posix_path, int ccp_flags);
  int conv_to_posix_path (const char *src_path, char *posix_path,
			  int ccp_flags);
  struct mntent *getmntent (int x);

  int write_cygdrive_info (const char *cygdrive_prefix, unsigned flags);
  int get_cygdrive_info (char *user, char *system, char* user_flags,
			 char* system_flags);
  void cygdrive_posix_path (const char *src, char *dst, int flags);
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

class dos_drive_mappings
{
  struct mapping
  {
    mapping *next;
    size_t doslen;
    size_t ntlen;
    wchar_t *dospath;
    wchar_t *ntdevpath;
  };
  mapping *mappings;

public:
  dos_drive_mappings ();
  ~dos_drive_mappings ();
  wchar_t *fixup_if_match (wchar_t *path);
};
#endif
