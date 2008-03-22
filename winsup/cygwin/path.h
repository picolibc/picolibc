/* path.h: path data structures

   Copyright 1996, 1997, 1998, 2000, 2001, 2002, 2003, 2004, 2005,
   2006, 2007 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "devices.h"

#include <sys/ioctl.h>
#include <fcntl.h>

#define isproc_dev(devn) \
  (devn == FH_PROC || devn == FH_REGISTRY || devn == FH_PROCESS || \
   devn == FH_PROCNET)

#define isvirtual_dev(devn) \
  (isproc_dev (devn) || devn == FH_CYGDRIVE || devn == FH_NETDRIVE)

inline bool
has_attribute (DWORD attributes, DWORD attribs_to_test)
{
  return attributes != INVALID_FILE_ATTRIBUTES
	 && (attributes & attribs_to_test);
}

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

extern suffix_info stat_suffixes[];

enum pathconv_arg
{
  PC_SYM_FOLLOW		= 0x0001,
  PC_SYM_NOFOLLOW	= 0x0002,
  PC_SYM_IGNORE		= 0x0004,
  PC_SYM_CONTENTS	= 0x0008,
  PC_NOFULL		= 0x0010,
  PC_NULLEMPTY		= 0x0020,
  PC_CHECK_EA		= 0x0040,
  PC_POSIX		= 0x0080,
  PC_NOWARN		= 0x0100,
  PC_NO_ACCESS_CHECK	= 0x00800000
};

enum case_checking
{
  PCHECK_RELAXED	= 0,
  PCHECK_ADJUST		= 1,
  PCHECK_STRICT		= 2
};

#define PC_NONULLEMPTY -1

#include "sys/mount.h"

enum path_types
{
  PATH_NOTHING		= 0,
  PATH_SYMLINK		= MOUNT_SYMLINK,
  PATH_BINARY		= MOUNT_BINARY,
  PATH_EXEC		= MOUNT_EXEC,
  PATH_NOTEXEC		= MOUNT_NOTEXEC,
  PATH_CYGWIN_EXEC	= MOUNT_CYGWIN_EXEC,
  PATH_ENC		= MOUNT_ENC,
  PATH_RO		= MOUNT_RO,
  PATH_ALL_EXEC		= (PATH_CYGWIN_EXEC | PATH_EXEC),
  PATH_NO_ACCESS_CHECK	= PC_NO_ACCESS_CHECK,
  PATH_LNK		= 0x01000000,
  PATH_TEXT		= 0x02000000,
  PATH_REP		= 0x04000000,
  PATH_HAS_SYMLINKS	= 0x10000000,
  PATH_SOCKET		= 0x40000000
};

class symlink_info;
struct fs_info
{
 private:
  struct status_flags
  {
    DWORD flags;                  /* Volume flags */
    DWORD samba_version;          /* Samba version if available */
    unsigned is_remote_drive : 1;
    unsigned has_buggy_open  : 1;
    unsigned has_acls	     : 1;
    unsigned hasgood_inode   : 1;
    unsigned is_fat	     : 1;
    unsigned is_ntfs	     : 1;
    unsigned is_samba	     : 1;
    unsigned is_nfs	     : 1;
    unsigned is_netapp       : 1;
    unsigned is_cdrom	     : 1;
  } status;
 public:
  void clear () { memset (&status, 0 , sizeof status); }
  fs_info () { clear (); }

  IMPLEMENT_STATUS_FLAG (DWORD, flags)
  IMPLEMENT_STATUS_FLAG (DWORD, samba_version)
  IMPLEMENT_STATUS_FLAG (bool, is_remote_drive)
  IMPLEMENT_STATUS_FLAG (bool, has_buggy_open)
  IMPLEMENT_STATUS_FLAG (bool, has_acls)
  IMPLEMENT_STATUS_FLAG (bool, hasgood_inode)
  IMPLEMENT_STATUS_FLAG (bool, is_fat)
  IMPLEMENT_STATUS_FLAG (bool, is_ntfs)
  IMPLEMENT_STATUS_FLAG (bool, is_samba)
  IMPLEMENT_STATUS_FLAG (bool, is_nfs)
  IMPLEMENT_STATUS_FLAG (bool, is_netapp)
  IMPLEMENT_STATUS_FLAG (bool, is_cdrom)

  bool update (PUNICODE_STRING, bool) __attribute__ ((regparm (3)));
};

class path_conv
{
  DWORD fileattr;
  fs_info fs;
  PWCHAR wide_path;
  UNICODE_STRING uni_path;
  void add_ext_from_sym (symlink_info&);
 public:

  unsigned path_flags;
  char *known_suffix;
  int error;
  device dev;
  bool case_clash;

  bool isremote () const {return fs.is_remote_drive ();}
  bool has_acls () const {return fs.has_acls (); }
  bool hasgood_inode () const {return fs.hasgood_inode (); }
  bool isgood_inode (__ino64_t ino) const;
  int has_symlinks () const {return path_flags & PATH_HAS_SYMLINKS;}
  int has_buggy_open () const {return fs.has_buggy_open ();}
  bool isencoded () const {return path_flags & PATH_ENC;}
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
  int is_rep_symlink () const {return path_flags & PATH_REP;}
  int isdevice () const {return dev.devn && dev.devn != FH_FS && dev.devn != FH_FIFO;}
  int isfifo () const {return dev == FH_FIFO;}
  int isspecial () const {return dev.devn && dev.devn != FH_FS;}
  int iscygdrive () const {return dev.devn == FH_CYGDRIVE;}
  int is_auto_device () const {return isdevice () && !is_fs_special ();}
  int is_fs_device () const {return isdevice () && is_fs_special ();}
  int is_fs_special () const {return dev.is_fs_special ();}
  int is_lnk_special () const {return is_fs_device () || isfifo () || is_lnk_symlink ();}
  int issocket () const {return dev.devn == FH_UNIX;}
  int iscygexec () const {return path_flags & PATH_CYGWIN_EXEC;}
  void set_cygexec (bool isset)
  {
    if (isset)
      path_flags |= PATH_CYGWIN_EXEC;
    else
      path_flags &= ~PATH_CYGWIN_EXEC;
  }
  bool isro () const {return !!(path_flags & PATH_RO);}
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

  void check (const UNICODE_STRING *upath, unsigned opt = PC_SYM_FOLLOW,
	      const suffix_info *suffixes = NULL) __attribute__ ((regparm(3)));
  void check (const char *src, unsigned opt = PC_SYM_FOLLOW,
	      const suffix_info *suffixes = NULL) __attribute__ ((regparm(3)));

  path_conv (const device& in_dev): fileattr (INVALID_FILE_ATTRIBUTES),
     wide_path (NULL), path_flags (0), known_suffix (NULL), error (0),
     dev (in_dev)
    {
      strcpy (path, in_dev.native);
    }

  path_conv (int, const char *src, unsigned opt = PC_SYM_FOLLOW,
	     const suffix_info *suffixes = NULL)
  {
    check (src, opt, suffixes);
  }

  path_conv (const UNICODE_STRING *src, unsigned opt = PC_SYM_FOLLOW,
	     const suffix_info *suffixes = NULL)
  {
    check (src, opt | PC_NULLEMPTY, suffixes);
  }

  path_conv (const char *src, unsigned opt = PC_SYM_FOLLOW,
	     const suffix_info *suffixes = NULL)
  {
    check (src, opt | PC_NULLEMPTY, suffixes);
  }

  path_conv (): fileattr (INVALID_FILE_ATTRIBUTES), wide_path (NULL),
  		path_flags (0), known_suffix (NULL), error (0),
		normalized_path (NULL)
    {path[0] = '\0';}

  ~path_conv ();
  inline char *get_win32 () { return path; }
  PUNICODE_STRING get_nt_native_path ();
  POBJECT_ATTRIBUTES get_object_attr (OBJECT_ATTRIBUTES &attr,
				      SECURITY_ATTRIBUTES &sa);
  inline size_t get_wide_win32_path_len ()
  {
    get_nt_native_path ();
    return uni_path.Length / sizeof (WCHAR);
  }

  PWCHAR get_wide_win32_path (PWCHAR wc);
  operator DWORD &() {return fileattr;}
  operator int () {return fileattr; }
  path_conv &operator =(path_conv &pc)
  {
    memcpy (this, &pc, pc.size ());
    set_normalized_path (pc.normalized_path, false);
    wide_path = NULL;
    return *this;
  }
  DWORD get_devn () const {return dev.devn;}
  short get_unitn () const {return dev.minor;}
  DWORD file_attributes () const {return fileattr;}
  void file_attributes (DWORD new_attr) {fileattr = new_attr;}
  DWORD fs_flags () {return fs.flags ();}
  bool fs_is_fat () const {return fs.is_fat ();}
  bool fs_is_ntfs () const {return fs.is_ntfs ();}
  bool fs_is_samba () const {return fs.is_samba ();}
  bool fs_is_nfs () const {return fs.is_nfs ();}
  bool fs_is_netapp () const {return fs.is_netapp ();}
  bool fs_is_cdrom () const {return fs.is_cdrom ();}
  void set_path (const char *p) {strcpy (path, p);}
  void fillin (HANDLE h);
  inline size_t size ()
  {
    return (sizeof (*this) - sizeof (path)) + strlen (path) + 1 + normalized_path_size;
  }
  bool is_binary ();

  unsigned __stdcall ndisk_links (DWORD);
  char *normalized_path;
  size_t normalized_path_size;
  void set_normalized_path (const char *, bool) __attribute__ ((regparm (3)));
  DWORD get_symlink_length () { return symlink_length; };
 private:
  DWORD symlink_length;
  char path[NT_MAX_PATH];
};

/* Symlink marker */
#define SYMLINK_COOKIE "!<symlink>"

/* Socket marker */
#define SOCKET_COOKIE  "!<socket >"

int __stdcall slash_unc_prefix_p (const char *path) __attribute__ ((regparm(1)));

enum fe_types
{
  FE_NADA = 0,		/* Nothing special */
  FE_NNF = 1,		/* Return NULL if not found */
  FE_NATIVE = 2,	/* Return native path in path_conv struct */
  FE_CWD = 4,		/* Search CWD for program */
  FE_DLL = 8		/* Search for DLLs, not executables. */
};
const char *__stdcall find_exec (const char *name, path_conv& buf,
				 const char *winenv = "PATH=",
				 unsigned opt = FE_NADA,
				 const char **known_suffix = NULL)
  __attribute__ ((regparm(3)));

/* Common macros for checking for invalid path names */
#define isdrive(s) (isalpha (*(s)) && (s)[1] == ':')
#define iswdrive(s) (iswalpha (*(s)) && (s)[1] == L':')

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
bool has_dot_last_component (const char *dir, bool test_dot_dot) __attribute__ ((regparm (2)));

bool fnunmunge (char *, const char *) __attribute__ ((regparm (2)));

int path_prefix_p (const char *path1, const char *path2, int len1) __attribute__ ((regparm (3)));

bool is_floppy (const char *);

/* FIXME: Move to own include file eventually */

#define MAX_ETC_FILES 2
class etc
{
  friend class dtable;
  static int curr_ix;
  static HANDLE changed_h;
  static bool change_possible[MAX_ETC_FILES + 1];
  static OBJECT_ATTRIBUTES fn[MAX_ETC_FILES + 1];
  static LARGE_INTEGER last_modified[MAX_ETC_FILES + 1];
  static bool dir_changed (int);
  static int init (int, PUNICODE_STRING);
  static bool file_changed (int);
  static bool test_file_change (int);
  friend class pwdgrp;
};
