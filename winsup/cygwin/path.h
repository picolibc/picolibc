/* path.h: path data structures

   Copyright 1996, 1997, 1998, 2000, 2001, 2002, 2003, 2004, 2005 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "devices.h"

#include <sys/ioctl.h>
#include <fcntl.h>
#include <ntdef.h>

enum executable_states
{
  is_executable,
  dont_care_if_executable,
  not_executable = dont_care_if_executable,
  dont_know_if_executable
};

struct suffix_info
{
  const char *name;
  int addon;
  suffix_info (const char *s, int addit = 0): name (s), addon (addit) {}
};

enum pathconv_arg
{
  PC_SYM_FOLLOW		= 0x0001,
  PC_SYM_NOFOLLOW	= 0x0002,
  PC_SYM_IGNORE		= 0x0004,
  PC_SYM_CONTENTS	= 0x0008,
  PC_FULL		= 0x0010,
  PC_NULLEMPTY		= 0x0020,
  PC_CHECK_EA		= 0x0040,
  PC_POSIX		= 0x0080,
  PC_NO_ACCESS_CHECK	= 0x00800000
};

enum case_checking
{
  PCHECK_RELAXED	= 0,
  PCHECK_ADJUST		= 1,
  PCHECK_STRICT		= 2
};

#define PC_NONULLEMPTY -1

#include <sys/mount.h>

enum path_types
{
  PATH_NOTHING		= 0,
  PATH_SYMLINK		= MOUNT_SYMLINK,
  PATH_BINARY		= MOUNT_BINARY,
  PATH_EXEC		= MOUNT_EXEC,
  PATH_NOTEXEC		= MOUNT_NOTEXEC,
  PATH_CYGWIN_EXEC	= MOUNT_CYGWIN_EXEC,
  PATH_ENC		= MOUNT_ENC,
  PATH_ALL_EXEC		= (PATH_CYGWIN_EXEC | PATH_EXEC),
  PATH_NO_ACCESS_CHECK	= PC_NO_ACCESS_CHECK,
  PATH_LNK		= 0x01000000,
  PATH_TEXT		= 0x02000000,
  PATH_HAS_SYMLINKS	= 0x10000000,
  PATH_SOCKET		= 0x40000000
};

class symlink_info;
struct fs_info
{
 private:
  __ino64_t name_hash;
  struct status_flags
  {
    DWORD flags;  /* Volume flags */
    DWORD serial; /* Volume serial number */
    unsigned is_remote_drive : 1;
    unsigned has_buggy_open  : 1;
    unsigned has_ea          : 1;
    unsigned has_acls        : 1;
    unsigned is_fat          : 1;
    unsigned drive_type      : 3;
  } status;
 public:
  void clear ()
  {
    name_hash = 0;
    flags () = serial () = 0;
    is_remote_drive (false);
    has_buggy_open (false);
    has_ea (false);
    has_acls (false);
    is_fat (false);
    drive_type (false);
  }
  inline DWORD& flags () {return status.flags;};
  inline DWORD& serial () {return status.serial;};

  IMPLEMENT_STATUS_FLAG (bool, is_remote_drive)
  IMPLEMENT_STATUS_FLAG (bool, has_buggy_open)
  IMPLEMENT_STATUS_FLAG (bool, is_fat)
  IMPLEMENT_STATUS_FLAG (bool, has_ea)
  IMPLEMENT_STATUS_FLAG (bool, has_acls)
  IMPLEMENT_STATUS_FLAG (DWORD, drive_type)

  bool update (const char *);
};

class path_conv
{
  DWORD fileattr;
  fs_info fs;
  void add_ext_from_sym (symlink_info&);
 public:

  unsigned path_flags;
  char *known_suffix;
  int error;
  device dev;
  bool case_clash;

  bool isremote () {return fs.is_remote_drive ();}
  int has_acls () const {return fs.has_acls (); }
  int has_symlinks () const {return path_flags & PATH_HAS_SYMLINKS;}
  int hasgood_inode () const {return has_acls ();}  // Not strictly correct
  int has_buggy_open () const {return fs.has_buggy_open ();}
  bool isencoded () {return path_flags & PATH_ENC;}
  int binmode () const
  {
    if (path_flags & PATH_BINARY)
      return O_BINARY;
    if (path_flags & PATH_TEXT)
      return O_TEXT;
    return 0;
  }
  int issymlink () const {return path_flags & PATH_SYMLINK;}
  int is_lnk_symlink () const {return path_flags & PATH_LNK;}
  int isdevice () const {return dev.devn && dev.devn != FH_FS && dev.devn != FH_FIFO;}
  int isfifo () const {return dev == FH_FIFO;}
  int isspecial () const {return dev.devn && dev.devn != FH_FS;}
  int is_auto_device () const {return isdevice () && !is_fs_special ();}
  int is_fs_special () const {return isspecial () && dev.isfs ();}
  int issocket () const {return dev.devn == FH_UNIX;}
  int iscygexec () const {return path_flags & PATH_CYGWIN_EXEC;}
  bool exists () const {return fileattr != INVALID_FILE_ATTRIBUTES;}
  bool has_attribute (DWORD x) const {return exists () && (fileattr & x);}
  int isdir () const {return has_attribute (FILE_ATTRIBUTE_DIRECTORY);}
  executable_states exec_state ()
  {
    extern int _check_for_executable;
    if (path_flags & PATH_ALL_EXEC)
      return is_executable;
    if (path_flags & PATH_NOTEXEC)
      return not_executable;
    if (!_check_for_executable)
      return dont_care_if_executable;
    return dont_know_if_executable;
  }

  void set_binary () {path_flags |= PATH_BINARY;}
  void set_symlink (DWORD n) {path_flags |= PATH_SYMLINK; symlink_length = n;}
  void set_has_symlinks () {path_flags |= PATH_HAS_SYMLINKS;}
  void set_exec (int x = 1) {path_flags |= x ? PATH_EXEC : PATH_NOTEXEC;}

  void check (const char *src, unsigned opt = PC_SYM_FOLLOW,
	      const suffix_info *suffixes = NULL)  __attribute__ ((regparm(3)));

  path_conv (const device& in_dev): fileattr (INVALID_FILE_ATTRIBUTES),
     path_flags (0), known_suffix (NULL), error (0), dev (in_dev)
    {
      strcpy (path, in_dev.native);
    }

  path_conv (int, const char *src, unsigned opt = PC_SYM_FOLLOW,
	     const suffix_info *suffixes = NULL)
  {
    check (src, opt, suffixes);
  }

  path_conv (const char *src, unsigned opt = PC_SYM_FOLLOW,
	     const suffix_info *suffixes = NULL)
  {
    check (src, opt | PC_NULLEMPTY, suffixes);
  }

  path_conv (): fileattr (INVALID_FILE_ATTRIBUTES), path_flags (0),
  		known_suffix (NULL), error (0), normalized_path (NULL)
    {path[0] = '\0';}

  ~path_conv ();
  void set_name (const char *win32, const char *posix);
  inline char *get_win32 () { return path; }
  PUNICODE_STRING get_nt_native_path (UNICODE_STRING &upath);
  operator char *() {return path;}
  operator const char *() {return path;}
  operator DWORD &() {return fileattr;}
  operator int () {return fileattr; }
  char operator [](int i) const {return path[i];}
  DWORD get_devn () {return dev.devn;}
  short get_unitn () {return dev.minor;}
  DWORD file_attributes () {return fileattr;}
  void file_attributes (DWORD new_attr) {fileattr = new_attr;}
  DWORD drive_type () {return fs.drive_type ();}
  DWORD fs_flags () {return fs.flags ();}
  bool fs_has_ea () {return fs.has_ea ();}
  bool fs_is_fat () {return fs.is_fat ();}
  void set_path (const char *p) {strcpy (path, p);}
  DWORD volser () { return fs.serial (); }
  void fillin (HANDLE h);
  inline size_t size ()
  {
    return (sizeof (*this) - sizeof (path)) + strlen (path) + 1 + normalized_path_size;
  }

  unsigned __stdcall ndisk_links (DWORD);
  char *normalized_path;
  size_t normalized_path_size;
  void set_normalized_path (const char *, bool) __attribute__ ((regparm (3)));
  DWORD get_symlink_length () { return symlink_length; };
 private:
  DWORD symlink_length;
  char path[CYG_MAX_PATH];
};

/* Symlink marker */
#define SYMLINK_COOKIE "!<symlink>"

#define SYMLINK_EA_NAME ".CYGSYMLINK"

/* Socket marker */
#define SOCKET_COOKIE  "!<socket >"

/* The sizeof header written to a shortcut by Cygwin or U/WIN. */
#define SHORTCUT_HDR_SIZE       76

/* Maximum depth of symlinks (after which ELOOP is issued).  */
#define MAX_LINK_DEPTH 10
int __stdcall slash_unc_prefix_p (const char *path) __attribute__ ((regparm(1)));

enum fe_types
{
  FE_NADA = 0,		/* Nothing special */
  FE_NNF = 1,		/* Return NULL if not found */
  FE_NATIVE = 2,	/* Return native path in path_conv struct */
  FE_CWD = 4		/* Search CWD for program */
};
const char * __stdcall find_exec (const char *name, path_conv& buf,
				  const char *winenv = "PATH=",
				  unsigned opt = FE_NADA,
				  const char **known_suffix = NULL)
  __attribute__ ((regparm(3)));

/* Common macros for checking for invalid path names */
#define isdrive(s) (isalpha (*(s)) && (s)[1] == ':')

static inline bool
has_exec_chars (const char *buf, int len)
{
  return len >= 2 &&
	 ((buf[0] == '#' && buf[1] == '!') ||
	  (buf[0] == ':' && buf[1] == '\n') ||
	  (buf[0] == 'M' && buf[1] == 'Z'));
}

int pathmatch (const char *path1, const char *path2) __attribute__ ((regparm (2)));
int pathnmatch (const char *path1, const char *path2, int len) __attribute__ ((regparm (2)));

bool fnunmunge (char *, const char *) __attribute__ ((regparm (2)));

int path_prefix_p (const char *path1, const char *path2, int len1) __attribute__ ((regparm (3)));

/* FIXME: Move to own include file eventually */

#define MAX_ETC_FILES 2
class etc
{
  friend class dtable;
  static int curr_ix;
  static bool change_possible[MAX_ETC_FILES + 1];
  static const char *fn[MAX_ETC_FILES + 1];
  static FILETIME last_modified[MAX_ETC_FILES + 1];
  static bool dir_changed (int);
  static int init (int, const char *);
  static bool file_changed (int);
  static bool test_file_change (int);
  friend class pwdgrp;
};
