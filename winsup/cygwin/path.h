/* path.h: path data structures

   Copyright 1996, 1997, 1998, 2000 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

struct suffix_info
{
  const char *name;
  int addon;
  suffix_info (const char *s, int addit = 0) {name = s, addon = addit;}
};

enum pathconv_arg
{
  PC_SYM_FOLLOW		= 0x0001,
  PC_SYM_NOFOLLOW	= 0x0002,
  PC_SYM_IGNORE		= 0x0004,
  PC_SYM_CONTENTS	= 0x0008,
  PC_FULL		= 0x0010,
  PC_NULLEMPTY		= 0x0020
};

#define PC_NONULLEMPTY -1

#include <sys/mount.h>

enum path_types
{
  PATH_NOTHING = 0,
  PATH_SYMLINK = MOUNT_SYMLINK,
  PATH_BINARY = MOUNT_BINARY,
  PATH_EXEC = MOUNT_EXEC,
  PATH_CYGWIN_EXEC = MOUNT_CYGWIN_EXEC,
  PATH_SOCKET =  0x40000000,
  PATH_HASACLS = 0x80000000
};

class path_conv
{
  char path[MAX_PATH];
 public:

  unsigned path_flags;

  int has_acls () {return path_flags & PATH_HASACLS;}
  int hasgood_inode () {return path_flags & PATH_HASACLS;}  // Not strictly correct
  int isbinary () {return path_flags & PATH_BINARY;}
  int issymlink () {return path_flags & PATH_SYMLINK;}
  int issocket () {return path_flags & PATH_SOCKET;}
  int isexec () {return path_flags & PATH_EXEC;}
  int iscygexec () {return path_flags & PATH_CYGWIN_EXEC;}

  void set_binary () {path_flags |= PATH_BINARY;}
  void set_symlink () {path_flags |= PATH_SYMLINK;}
  void set_exec (int x = 1) {path_flags |= x ? PATH_EXEC : PATH_NOTHING;}
  void set_has_acls (int x = 1) {path_flags |= x ? PATH_HASACLS : PATH_NOTHING;}

  char *known_suffix;

  int error;
  DWORD devn;
  int unit;

  DWORD fileattr;

  void check (const char *src, unsigned opt = PC_SYM_FOLLOW,
	      const suffix_info *suffixes = NULL);

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

  path_conv (): path_flags (0), known_suffix (NULL), error (0), devn (0), unit (0), fileattr (0xffffffff) {path[0] = '\0';}

  inline char *get_win32 () { return path; }
  operator char *() {return path; }
  BOOL is_device () {return devn != FH_BAD;}
  DWORD get_devn () {return devn == FH_BAD ? (DWORD) FH_DISK : devn;}
  short get_unitn () {return devn == FH_BAD ? 0 : unit;}
  DWORD file_attributes () {return fileattr;}
};

/* Symlink marker */
#define SYMLINK_COOKIE "!<symlink>"

/* Socket marker */
#define SOCKET_COOKIE  "!<socket >"

/* Maximum depth of symlinks (after which ELOOP is issued).  */
#define MAX_LINK_DEPTH 10

extern suffix_info std_suffixes[];

int __stdcall get_device_number (const char *name, int &unit, BOOL from_conv = FALSE);
int __stdcall slash_unc_prefix_p (const char *path);
int __stdcall check_null_empty_path (const char *name);

const char * __stdcall find_exec (const char *name, path_conv& buf, const char *winenv = "PATH=",
			int null_if_notfound = 0, const char **known_suffix = NULL);

/* Common macros for checking for invalid path names */

#define check_null_empty_path_errno(src) \
({ \
  int __err; \
  if ((__err = check_null_empty_path(src))) \
    set_errno (__err); \
  __err; \
})

#define isdrive(s) (isalpha (*(s)) && (s)[1] == ':')

/* cwd cache stuff.  */

class muto;

struct cwdstuff
{
  char *posix;
  char *win32;
  DWORD hash;
  muto *lock;
  char *get (char *buf, int need_posix = 1, int with_chroot = 0, unsigned ulen = MAX_PATH);
  DWORD get_hash ();
  void init ();
  void fixup_after_exec (char *win32, char *posix, DWORD hash);
  bool get_initial ();
  void copy (char * &posix_cwd, char * &win32_cwd, DWORD hash_cwd);
  void set (const char *win32_cwd, const char *posix_cwd = NULL);
};

extern cwdstuff cygcwd;
