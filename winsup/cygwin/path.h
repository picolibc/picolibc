/* path.h: path data structures

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "devices.h"
#include "mount.h"
#include "cygheap_malloc.h"
#include "nfs.h"

#include <sys/ioctl.h>
#include <fcntl.h>
#include <alloca.h>

extern inline bool
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
  PC_SYM_NOFOLLOW_REP	= 0x0004,
  PC_SYM_CONTENTS	= 0x0008,
  PC_NOFULL		= 0x0010,
  PC_NULLEMPTY		= 0x0020,
  PC_POSIX		= 0x0080,
  PC_NOWARN		= 0x0100,
  PC_OPEN		= 0x0200,	/* use open semantics */
  PC_CTTY		= 0x0400,	/* could later be used as ctty */
  PC_KEEP_HANDLE	= 0x00400000,
  PC_NO_ACCESS_CHECK	= 0x00800000
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
  PATH_SPARSE		= MOUNT_SPARSE,
  PATH_RO		= MOUNT_RO,
  PATH_NOACL		= MOUNT_NOACL,
  PATH_NOPOSIX		= MOUNT_NOPOSIX,
  PATH_DOS		= MOUNT_DOS,
  PATH_IHASH		= MOUNT_IHASH,
  PATH_ALL_EXEC		= (PATH_CYGWIN_EXEC | PATH_EXEC),
  PATH_NO_ACCESS_CHECK	= PC_NO_ACCESS_CHECK,
  PATH_CTTY		= 0x00400000,	/* could later be used as ctty */
  PATH_OPEN		= 0x00800000,	/* use open semantics */
  					/* FIXME?  PATH_OPEN collides with
					   PATH_NO_ACCESS_CHECK, but it looks
					   like they are never used together. */
  PATH_LNK		= 0x01000000,
  PATH_TEXT		= 0x02000000,
  PATH_REP		= 0x04000000,
  PATH_HAS_SYMLINKS	= 0x10000000,
  PATH_SOCKET		= 0x40000000
};

NTSTATUS file_get_fai (HANDLE, PFILE_ALL_INFORMATION);

class symlink_info;

class path_conv_handle
{
  HANDLE      hdl;
  union {
    FILE_ALL_INFORMATION _fai;
    /* For NFS. */
    fattr3 _fattr3;
  } attribs;
public:
  path_conv_handle () : hdl (NULL) {}
  inline void set (HANDLE h) { hdl = h; }
  inline void close ()
  {
    if (hdl)
      CloseHandle (hdl);
    set (NULL);
  }
  inline void dup (const path_conv_handle &pch)
  {
    if (!DuplicateHandle (GetCurrentProcess (), pch.handle (),
			  GetCurrentProcess (), &hdl,
			  0, TRUE, DUPLICATE_SAME_ACCESS))
      hdl = NULL;
  }
  inline HANDLE handle () const { return hdl; }
  inline PFILE_ALL_INFORMATION fai () const
  { return (PFILE_ALL_INFORMATION) &attribs._fai; }
  inline struct fattr3 *nfsattr () const
  { return (struct fattr3 *) &attribs._fattr3; }
  inline NTSTATUS get_finfo (HANDLE h, bool nfs)
  {
    return nfs ? nfs_fetch_fattr3 (h, nfsattr ()) : file_get_fai (h, fai ());
  }
  inline ino_t get_ino (bool nfs) const
  {
    return nfs ? nfsattr ()->fileid
	       : fai ()->InternalInformation.IndexNumber.QuadPart;
  }
  inline DWORD get_dosattr (bool nfs) const
  {
    if (nfs)
      return (nfsattr ()->type & 7) == NF3DIR ? FILE_ATTRIBUTE_DIRECTORY : 0;
    return fai ()->BasicInformation.FileAttributes;
  }
};

class path_conv
{
  DWORD fileattr;
  ULONG caseinsensitive;
  fs_info fs;
  PWCHAR wide_path;
  UNICODE_STRING uni_path;
  void add_ext_from_sym (symlink_info&);
  DWORD symlink_length;
  const char *path;
  unsigned path_flags;
  const char *suffix;
  const char *posix_path;
  path_conv_handle conv_handle;
 public:
  int error;
  device dev;

  const char *known_suffix () { return suffix; }
  bool isremote () const {return fs.is_remote_drive ();}
  ULONG objcaseinsensitive () const {return caseinsensitive;}
  bool has_acls () const {return !(path_flags & PATH_NOACL) && fs.has_acls (); }
  bool hasgood_inode () const {return !(path_flags & PATH_IHASH); }
  bool isgood_inode (ino_t ino) const;
  bool support_sparse () const
  {
    return (path_flags & PATH_SPARSE)
	   && (fs_flags () & FILE_SUPPORTS_SPARSE_FILES);
  }
  int has_symlinks () const {return path_flags & PATH_HAS_SYMLINKS;}
  int has_dos_filenames_only () const {return path_flags & PATH_DOS;}
  int has_buggy_reopen () const {return fs.has_buggy_reopen ();}
  int has_buggy_fileid_dirinfo () const {return fs.has_buggy_fileid_dirinfo ();}
  int has_buggy_basic_info () const {return fs.has_buggy_basic_info ();}
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
  int isdevice () const {return dev.not_device (FH_FS) && dev.not_device (FH_FIFO);}
  int isfifo () const {return dev.is_device (FH_FIFO);}
  int isspecial () const {return dev.not_device (FH_FS);}
  int iscygdrive () const {return dev.is_device (FH_CYGDRIVE);}
  int is_auto_device () const {return isdevice () && !is_fs_special ();}
  int is_fs_device () const {return isdevice () && is_fs_special ();}
  int is_fs_special () const {return dev.is_fs_special ();}
  int is_lnk_special () const {return is_fs_device () || isfifo () || is_lnk_symlink ();}
  int issocket () const {return dev.is_device (FH_UNIX);}
  int iscygexec () const {return path_flags & PATH_CYGWIN_EXEC;}
  int isopen () const {return path_flags & PATH_OPEN;}
  int isctty_capable () const {return path_flags & PATH_CTTY;}
  void set_cygexec (bool isset)
  {
    if (isset)
      path_flags |= PATH_CYGWIN_EXEC;
    else
      path_flags &= ~PATH_CYGWIN_EXEC;
  }
  void set_cygexec (void *target)
  {
    if (target)
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

  void set_symlink (DWORD n) {path_flags |= PATH_SYMLINK; symlink_length = n;}
  void set_has_symlinks () {path_flags |= PATH_HAS_SYMLINKS;}
  void set_exec (int x = 1) {path_flags |= x ? PATH_EXEC : PATH_NOTEXEC;}

  void __reg3 check (const UNICODE_STRING *upath, unsigned opt = PC_SYM_FOLLOW,
	      const suffix_info *suffixes = NULL);
  void __reg3 check (const char *src, unsigned opt = PC_SYM_FOLLOW,
	      const suffix_info *suffixes = NULL);

  path_conv (const device& in_dev)
  : fileattr (INVALID_FILE_ATTRIBUTES), wide_path (NULL), path (NULL),
    path_flags (0), suffix (NULL), posix_path (NULL), error (0),
    dev (in_dev)
  {
    set_path (in_dev.native ());
  }

  path_conv (int, const char *src, unsigned opt = PC_SYM_FOLLOW,
	     const suffix_info *suffixes = NULL)
  : fileattr (INVALID_FILE_ATTRIBUTES), wide_path (NULL), path (NULL),
    path_flags (0), suffix (NULL), posix_path (NULL), error (0)
  {
    check (src, opt, suffixes);
  }

  path_conv (const UNICODE_STRING *src, unsigned opt = PC_SYM_FOLLOW,
	     const suffix_info *suffixes = NULL)
  : fileattr (INVALID_FILE_ATTRIBUTES), wide_path (NULL), path (NULL),
    path_flags (0), suffix (NULL), posix_path (NULL), error (0)
  {
    check (src, opt | PC_NULLEMPTY, suffixes);
  }

  path_conv (const char *src, unsigned opt = PC_SYM_FOLLOW,
	     const suffix_info *suffixes = NULL)
  : fileattr (INVALID_FILE_ATTRIBUTES), wide_path (NULL), path (NULL),
    path_flags (0), suffix (NULL), posix_path (NULL), error (0)
  {
    check (src, opt | PC_NULLEMPTY, suffixes);
  }

  path_conv ()
  : fileattr (INVALID_FILE_ATTRIBUTES), wide_path (NULL), path (NULL),
    path_flags (0), suffix (NULL), posix_path (NULL), error (0)
  {}

  ~path_conv ();
  inline const char *get_win32 () const { return path; }
  void set_nt_native_path (PUNICODE_STRING);
  PUNICODE_STRING get_nt_native_path ();
  inline POBJECT_ATTRIBUTES get_object_attr (OBJECT_ATTRIBUTES &attr,
					     SECURITY_ATTRIBUTES &sa)
  {
    if (!get_nt_native_path ())
      return NULL;
    InitializeObjectAttributes (&attr, &uni_path,
				objcaseinsensitive ()
				| (sa.bInheritHandle ? OBJ_INHERIT : 0),
				NULL, sa.lpSecurityDescriptor);
    return &attr;
  }
  inline POBJECT_ATTRIBUTES init_reopen_attr (OBJECT_ATTRIBUTES &attr, HANDLE h)
  {
    if (has_buggy_reopen ())
      InitializeObjectAttributes (&attr, get_nt_native_path (),
				  objcaseinsensitive (), NULL, NULL)
    else
      InitializeObjectAttributes (&attr, &ro_u_empty, objcaseinsensitive (),
				  h, NULL);
    return &attr;
  }
  inline size_t get_wide_win32_path_len ()
  {
    get_nt_native_path ();
    return uni_path.Length / sizeof (WCHAR);
  }

  PWCHAR get_wide_win32_path (PWCHAR wc);
  operator DWORD &() {return fileattr;}
  operator int () {return fileattr; }
# define cfree_and_null(x) \
  if (x) \
    { \
      cfree ((void *) (x)); \
      (x) = NULL; \
    }
  void free_strings ()
  {
    cfree_and_null (path);
    cfree_and_null (posix_path);
    cfree_and_null (wide_path);
  }
  path_conv& eq_worker (const path_conv& pc, const char *in_path)
  {
    free_strings ();
    memcpy (this, &pc, sizeof pc);
    /* The device info might contain pointers to allocated strings, in
       contrast to statically allocated strings.  Calling device::dup()
       will duplicate the string if the source was allocated. */
    dev.dup ();
    path = cstrdup (in_path);
    conv_handle.dup (pc.conv_handle);
    posix_path = cstrdup(pc.posix_path);
    if (pc.wide_path)
      {
	wide_path = cwcsdup (uni_path.Buffer);
	if (!wide_path)
	  api_fatal ("cwcsdup would have returned NULL");
	uni_path.Buffer = wide_path;
      }
    return *this;
  }

  path_conv &operator << (const path_conv& pc)
  {
    const char *save_path;

    if (!path)
      save_path = pc.path;
    else
      {
	save_path = (char *) alloca (strlen (path) + 1);
	strcpy ((char *) save_path, path);
      }
    return eq_worker (pc, save_path);
  }

  path_conv &operator =(const path_conv& pc)
  {
    return eq_worker (pc, pc.path);
  }
  dev_t get_device () {return dev.get_device ();}
  DWORD file_attributes () const {return fileattr;}
  void file_attributes (DWORD new_attr) {fileattr = new_attr;}
  DWORD fs_flags () const {return fs.flags ();}
  DWORD fs_name_len () const {return fs.name_len ();}
  bool fs_got_fs () const { return fs.got_fs (); }
  bool fs_is_fat () const {return fs.is_fat ();}
  bool fs_is_ntfs () const {return fs.is_ntfs ();}
  bool fs_is_refs () const {return fs.is_refs ();}
  bool fs_is_samba () const {return fs.is_samba ();}
  bool fs_is_nfs () const {return fs.is_nfs ();}
  bool fs_is_netapp () const {return fs.is_netapp ();}
  bool fs_is_cdrom () const {return fs.is_cdrom ();}
  bool fs_is_mvfs () const {return fs.is_mvfs ();}
  bool fs_is_cifs () const {return fs.is_cifs ();}
  bool fs_is_nwfs () const {return fs.is_nwfs ();}
  bool fs_is_ncfsd () const {return fs.is_ncfsd ();}
  bool fs_is_afs () const {return fs.is_afs ();}
  bool fs_is_prlfs () const {return fs.is_prlfs ();}
  fs_info_type fs_type () const {return fs.what_fs ();}
  ULONG fs_serial_number () const {return fs.serial_number ();}
  inline const char *set_path (const char *p)
  {
    if (path)
      cfree (modifiable_path ());
    char *new_path = (char *) cmalloc_abort (HEAP_STR, strlen (p) + 7);
    strcpy (new_path, p);
    return path = new_path;
  }
  bool is_binary ();

  HANDLE handle () const { return conv_handle.handle (); }
  PFILE_ALL_INFORMATION fai () { return conv_handle.fai (); }
  struct fattr3 *nfsattr () { return conv_handle.nfsattr (); }
  inline NTSTATUS get_finfo (HANDLE h)
  {
    return conv_handle.get_finfo (h, fs.is_nfs ());
  }
  inline ino_t get_ino () const { return conv_handle.get_ino (fs.is_nfs ()); }
  void reset_conv_handle () { conv_handle.set (NULL); }
  void close_conv_handle () { conv_handle.close (); }

  ino_t get_ino_by_handle (HANDLE h);
  inline const char *get_posix () const { return posix_path; }
  void __reg2 set_posix (const char *);
  DWORD get_symlink_length () { return symlink_length; };
 private:
  char *modifiable_path () {return (char *) path;}
};

/* Symlink marker */
#define SYMLINK_COOKIE "!<symlink>"

/* Socket marker */
#define SOCKET_COOKIE  "!<socket >"

/* Interix symlink marker */
#define INTERIX_SYMLINK_COOKIE  "IntxLNK\1"

enum fe_types
{
  FE_NADA = 0,		/* Nothing special */
  FE_NNF = 1,		/* Return NULL if not found */
  FE_CWD = 4,		/* Search CWD for program */
  FE_DLL = 8		/* Search for DLLs, not executables. */
};
const char *__reg3 find_exec (const char *name, path_conv& buf,
				 const char *search = "PATH",
				 unsigned opt = FE_NADA,
				 const char **known_suffix = NULL);

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

int __reg3 pathmatch (const char *path1, const char *path2, bool caseinsensitive);
int __reg3 pathnmatch (const char *path1, const char *path2, int len, bool caseinsensitive);
bool __reg2 has_dot_last_component (const char *dir, bool test_dot_dot);

int __reg3 path_prefix_p (const char *path1, const char *path2, int len1,
		   bool caseinsensitive);

int normalize_win32_path (const char *, char *, char *&);
int normalize_posix_path (const char *, char *, char *&);
PUNICODE_STRING __reg3 get_nt_native_path (const char *, UNICODE_STRING&, bool);

int __reg3 symlink_worker (const char *, const char *, bool);
