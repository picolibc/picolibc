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
  suffix_info (const char *s, int addit = 0): name (s), addon (addit) {}
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
  PATH_NOTHING = 0,
  PATH_SYMLINK = MOUNT_SYMLINK,
  PATH_BINARY = MOUNT_BINARY,
  PATH_EXEC = MOUNT_EXEC,
  PATH_CYGWIN_EXEC = MOUNT_CYGWIN_EXEC,
  PATH_ALL_EXEC = (PATH_CYGWIN_EXEC | PATH_EXEC),
  PATH_ISDISK =	      0x04000000,
  PATH_NOTEXEC =      0x08000000,
  PATH_HAS_SYMLINKS = 0x10000000,
  PATH_HASBUGGYOPEN = 0x20000000,
  PATH_SOCKET =       0x40000000,
  PATH_HASACLS =      0x80000000
};

class symlink_info;
class path_conv
{
  char path[MAX_PATH];
  void add_ext_from_sym (symlink_info&);
  bool is_remote_drive;
 public:

  unsigned path_flags;
  DWORD vol_flags;
  DWORD drive_type;
  DWORD vol_serial;

  int isdisk () const { return path_flags & PATH_ISDISK;}
  int isremote () const {return is_remote_drive;}
  int has_acls () const {return path_flags & PATH_HASACLS;}
  int has_symlinks () const {return path_flags & PATH_HAS_SYMLINKS;}
  int hasgood_inode () const {return path_flags & PATH_HASACLS;}  // Not strictly correct
  int has_buggy_open () const {return path_flags & PATH_HASBUGGYOPEN;}
  int isbinary () const {return path_flags & PATH_BINARY;}
  int issymlink () const {return path_flags & PATH_SYMLINK;}
  int issocket () const {return path_flags & PATH_SOCKET;}
  int iscygexec () const {return path_flags & PATH_CYGWIN_EXEC;}
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
  void set_symlink () {path_flags |= PATH_SYMLINK;}
  void set_has_symlinks () {path_flags |= PATH_HAS_SYMLINKS;}
  void set_isdisk () {path_flags |= PATH_ISDISK;}
  void set_exec (int x = 1) {path_flags |= x ? PATH_EXEC : PATH_NOTEXEC;}
  void set_has_acls (int x = 1) {path_flags |= x ? PATH_HASACLS : PATH_NOTHING;}
  void set_has_buggy_open (int x = 1) {path_flags |= x ? PATH_HASBUGGYOPEN : PATH_NOTHING;}

  char *known_suffix;

  int error;
  DWORD devn;
  int unit;

  DWORD fileattr;

  BOOL case_clash;

  void check (const char *src, unsigned opt = PC_SYM_FOLLOW,
	      const suffix_info *suffixes = NULL)  __attribute__ ((regparm(3)));

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

int __stdcall get_device_number (const char *name, int &unit, BOOL from_conv = FALSE)  __attribute__ ((regparm(3)));
int __stdcall slash_unc_prefix_p (const char *path) __attribute__ ((regparm(1)));
int __stdcall check_null_empty_path (const char *name) __attribute__ ((regparm(1)));

const char * __stdcall find_exec (const char *name, path_conv& buf, const char *winenv = "PATH=",
			int null_if_notfound = 0, const char **known_suffix = NULL)  __attribute__ ((regparm(3)));

/* Common macros for checking for invalid path names */

#define check_null_empty_path_errno(src) \
({ \
  int __err; \
  if ((__err = check_null_empty_path(src))) \
    set_errno (__err); \
  __err; \
})

#define isdrive(s) (isalpha (*(s)) && (s)[1] == ':')

static inline bool
has_exec_chars (const char *buf, int len)
{
  return len >= 2 &&
	 ((buf[0] == '#' && buf[1] == '!') ||
	  (buf[0] == ':' && buf[1] == '\n') ||
	  (buf[0] == 'M' && buf[1] == 'Z'));
}

extern int pathmatch (const char *path1, const char *path2);
extern int pathnmatch (const char *path1, const char *path2, int len);
