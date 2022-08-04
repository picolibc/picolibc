/* path.h: path data structures

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "devices.h"
#include "mount.h"
#include "cygheap_malloc.h"
#include "nfs.h"

#include <sys/mount.h>
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

/* DO NOT copy any of these files into the same set of flags as the
   below path_types.  Ever. */
enum pathconv_arg
{
  PC_SYM_FOLLOW		 = _BIT ( 0),	/* follow symlinks */
  PC_SYM_NOFOLLOW	 = _BIT ( 1),	/* don't follow symlinks (but honor
					   trailing slashes) */
  PC_SYM_NOFOLLOW_REP	 = _BIT ( 2),	/* don't follow dir reparse point */
  PC_SYM_CONTENTS	 = _BIT ( 3),	/* don't follow, return content only */
  PC_NOFULL		 = _BIT ( 4),	/* keep relative path */
  PC_NULLEMPTY		 = _BIT ( 5),	/* empty path is no error */
  PC_NONULLEMPTY	 = _BIT ( 6),	/* override PC_NULLEMPTY default */
  PC_POSIX		 = _BIT ( 7),	/* return normalized posix path */
  PC_OPEN		 = _BIT ( 9),	/* use open semantics */
  PC_CTTY		 = _BIT (10),	/* could later be used as ctty */
  PC_SYM_NOFOLLOW_PROCFD = _BIT (11),	/* allow /proc/PID/fd redirection */
  PC_KEEP_HANDLE	 = _BIT (12),	/* keep handle for later stat calls */
  PC_NO_ACCESS_CHECK	 = _BIT (13),	/* helper flag for error check */
  PC_SYM_NOFOLLOW_DIR	 = _BIT (14),	/* don't follow a trailing slash */
  PC_DONT_USE		 = _BIT (31)	/* conversion to signed happens. */
};

enum path_types
{
  PATH_CTTY		= _BIT ( 0),	/* could later be used as ctty */
  PATH_OPEN		= _BIT ( 1),	/* use open semantics */
  PATH_LNK		= _BIT ( 2),	/* *.lnk-type symlink */
  PATH_REP		= _BIT ( 3),	/* reparse point known to Cygwin */
  PATH_SYMLINK		= _BIT ( 4),	/* symlink understood by Cygwin */
  PATH_SOCKET		= _BIT ( 5),	/* AF_UNIX socket file */
  PATH_RESOLVE_PROCFD	= _BIT ( 6),	/* fd symlink via /proc */
  PATH_REP_NOAPI	= _BIT ( 7),	/* rep. point unknown to WinAPI */
  PATH_DONT_USE		= _BIT (31)	/* conversion to signed happens. */
};

enum fetch_fh_flags
{
  FFH_LINKAT	= (1 <<  0),
};

NTSTATUS file_get_fai (HANDLE, PFILE_ALL_INFORMATION);
int check_reparse_point_target (HANDLE, bool, PREPARSE_DATA_BUFFER,
				PUNICODE_STRING);

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
    if (pch.handle ()
	&& !DuplicateHandle (GetCurrentProcess (), pch.handle (),
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
  DWORD symlink_length;
  const char *path;
  uint32_t mount_flags;
  uint32_t path_flags;
  const char *suffix;
  const char *posix_path;
  path_conv_handle conv_handle;

  void add_ext_from_sym (symlink_info&);
  char *modifiable_path () {return (char *) path;}

 public:
  int error;
  device dev;

  void *serialize (HANDLE, unsigned int &) const;
  HANDLE deserialize (void *);

  const char *known_suffix () { return suffix; }
  bool isremote () const {return fs.is_remote_drive ();}
  ULONG objcaseinsensitive () const {return caseinsensitive;}
  bool has_acls () const {return !(mount_flags & MOUNT_NOACL)
				 && fs.has_acls (); }
  bool hasgood_inode () const {return !(mount_flags & MOUNT_IHASH); }
  bool isgood_inode (ino_t ino) const;
  bool support_sparse () const
  {
    return (mount_flags & MOUNT_SPARSE)
	   && (fs_flags () & FILE_SUPPORTS_SPARSE_FILES);
  }
  int has_dos_filenames_only () const {return mount_flags & MOUNT_DOS;}
  int has_buggy_reopen () const {return fs.has_buggy_reopen ();}
  int has_buggy_fileid_dirinfo () const {return fs.has_buggy_fileid_dirinfo ();}
  int has_buggy_basic_info () const {return fs.has_buggy_basic_info ();}
  int binmode () const
  {
    return (mount_flags & MOUNT_TEXT) ? O_TEXT : O_BINARY;
  }
  int issymlink () const {return path_flags & PATH_SYMLINK;}
  int is_lnk_symlink () const {return path_flags & PATH_LNK;}
  /* This indicates any known reparse point */
  int is_known_reparse_point () const {return path_flags & PATH_REP;}
  /* This indicates any known reparse point, handled sanely by WinAPI.
     The difference is crucial: WSL symlinks, for instance, are known
     reparse points, so we want to open them as reparse points usually.
     However they are foreign to WinAPI and not handled sanely.  If one
     is part of $PATH, WinAPI functions may fail under the hood with
     STATUS_IO_REPARSE_TAG_NOT_HANDLED. */
  int is_winapi_reparse_point () const
  {
    return (path_flags & (PATH_REP | PATH_REP_NOAPI)) == PATH_REP;
  }
  int isdevice () const {return dev.not_device (FH_FS) && dev.not_device (FH_FIFO);}
  int isfifo () const {return dev.is_device (FH_FIFO);}
  int isspecial () const {return dev.not_device (FH_FS);}
  int iscygdrive () const {return dev.is_device (FH_CYGDRIVE);}
  int is_fs_special () const {return dev.is_fs_special ();}

  int is_lnk_special () const {return (isdevice () && is_fs_special ()
				       && !issocket ())
      || isfifo () || is_lnk_symlink ();}
#ifdef __WITH_AF_UNIX
  int issocket () const {return dev.is_device (FH_LOCAL)
				|| dev.is_device (FH_UNIX);}
#else
  int issocket () const {return dev.is_device (FH_LOCAL);}
#endif /* __WITH_AF_UNIX */
  int iscygexec () const {return mount_flags & MOUNT_CYGWIN_EXEC;}
  int isopen () const {return path_flags & PATH_OPEN;}
  int isctty_capable () const {return path_flags & PATH_CTTY;}
  int follow_fd_symlink () const {return path_flags & PATH_RESOLVE_PROCFD;}
  void set_cygexec (bool isset)
  {
    if (isset)
      mount_flags |= MOUNT_CYGWIN_EXEC;
    else
      mount_flags &= ~MOUNT_CYGWIN_EXEC;
  }
  void set_cygexec (void *target)
  {
    if (target)
      mount_flags |= MOUNT_CYGWIN_EXEC;
    else
      mount_flags &= ~MOUNT_CYGWIN_EXEC;
  }
  bool isro () const {return !!(mount_flags & MOUNT_RO);}
  bool exists () const {return fileattr != INVALID_FILE_ATTRIBUTES;}
  bool has_attribute (DWORD x) const {return exists () && (fileattr & x);}
  int isdir () const {return has_attribute (FILE_ATTRIBUTE_DIRECTORY);}
  executable_states exec_state ()
  {
    extern int _check_for_executable;
    if (mount_flags & (MOUNT_CYGWIN_EXEC | MOUNT_EXEC))
      return is_executable;
    if (mount_flags & MOUNT_NOTEXEC)
      return not_executable;
    if (!_check_for_executable)
      return dont_care_if_executable;
    return dont_know_if_executable;
  }

  void set_symlink (DWORD n) {path_flags |= PATH_SYMLINK; symlink_length = n;}
  void set_exec (int x = 1) {mount_flags |= x ? MOUNT_EXEC : MOUNT_NOTEXEC;}

  void check (const UNICODE_STRING *upath, uint32_t opt = PC_SYM_FOLLOW,
	      const suffix_info *suffixes = NULL);
  void check (const char *src, uint32_t opt = PC_SYM_FOLLOW,
	      const suffix_info *suffixes = NULL);

  path_conv (const device& in_dev)
  : fileattr (INVALID_FILE_ATTRIBUTES), wide_path (NULL), path (NULL),
    mount_flags (0), path_flags (0), suffix (NULL), posix_path (NULL),
    error (0), dev (in_dev)
  {
    set_path (in_dev.native ());
  }

  path_conv (const UNICODE_STRING *src, uint32_t opt = PC_SYM_FOLLOW,
	     const suffix_info *suffixes = NULL)
  : fileattr (INVALID_FILE_ATTRIBUTES), wide_path (NULL), path (NULL),
    mount_flags (0), path_flags (0), suffix (NULL), posix_path (NULL), error (0)
  {
    check (src, opt | ((opt & PC_NONULLEMPTY) ? 0 : PC_NULLEMPTY), suffixes);
  }

  path_conv (const char *src, uint32_t opt = PC_SYM_FOLLOW,
	     const suffix_info *suffixes = NULL)
  : fileattr (INVALID_FILE_ATTRIBUTES), wide_path (NULL), path (NULL),
    mount_flags (0), path_flags (0), suffix (NULL), posix_path (NULL), error (0)
  {
    check (src, opt | ((opt & PC_NONULLEMPTY) ? 0 : PC_NULLEMPTY), suffixes);
  }

  path_conv ()
  : fileattr (INVALID_FILE_ATTRIBUTES), wide_path (NULL), path (NULL),
    mount_flags (0), path_flags (0), suffix (NULL), posix_path (NULL), error (0)
  {}

  ~path_conv ();
  inline const char *get_win32 () const { return path; }
  void set_nt_native_path (PUNICODE_STRING);
  PUNICODE_STRING get_nt_native_path (PUNICODE_STRING = NULL);
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
    memcpy ((void *) this, &pc, sizeof pc);
    /* The device info might contain pointers to allocated strings, in
       contrast to statically allocated strings.  Calling device::dup()
       will duplicate the string if the source was allocated. */
    dev.dup ();
    if (in_path)
      path = cstrdup (in_path);
    conv_handle.dup (pc.conv_handle);
    if (pc.posix_path)
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
  bool fs_is_exfat () const {return fs.is_exfat ();}
  bool fs_is_any_fat () const {return fs.is_fat () || fs.is_exfat ();}
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
  void close_conv_handle () { conv_handle.close (); }

  ino_t get_ino_by_handle (HANDLE h);
  inline const char *get_posix () const { return posix_path; }
  void set_posix (const char *);
  DWORD get_symlink_length () { return symlink_length; };
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
const char *find_exec (const char *name, path_conv& buf,
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

int pathmatch (const char *path1, const char *path2, bool caseinsensitive);
int pathnmatch (const char *path1, const char *path2, int len, bool caseinsensitive);
bool has_dot_last_component (const char *dir, bool test_dot_dot);

int path_prefix_p (const char *path1, const char *path2, int len1,
		   bool caseinsensitive);

int normalize_win32_path (const char *, char *, char *&);
int normalize_posix_path (const char *, char *, char *&);
PUNICODE_STRING get_nt_native_path (const char *, UNICODE_STRING&, bool);

int symlink_worker (const char *, path_conv &, bool);
