/* cygheap.h: Cygwin heap manager.

   Copyright 2000, 2001, 2002 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#undef cfree

enum cygheap_types
{
  HEAP_FHANDLER,
  HEAP_STR,
  HEAP_ARGV,
  HEAP_BUF,
  HEAP_MOUNT,
  HEAP_1_START,
  HEAP_1_STR,
  HEAP_1_ARGV,
  HEAP_1_BUF,
  HEAP_1_EXEC,
  HEAP_1_MAX = 100
};

#define incygheap(s) (cygheap && ((char *) (s) >= (char *) cygheap) && ((char *) (s) <= ((char *) cygheap_max)))

struct _cmalloc_entry
{
  union
  {
    DWORD b;
    char *ptr;
  };
  struct _cmalloc_entry *prev;
  char data[0];
};

struct cygheap_root_mount_info
{
  char posix_path[MAX_PATH];
  unsigned posix_pathlen;
  char native_path[MAX_PATH];
  unsigned native_pathlen;
};

/* CGF: FIXME This doesn't belong here */

class cygheap_root
{
  /* Root directory information.
     This is used after a chroot is called. */
  struct cygheap_root_mount_info *m;

public:
  bool posix_ok (const char *path)
  {
    if (!m)
      return 1;
    return path_prefix_p (m->posix_path, path, m->posix_pathlen);
  }
  bool ischroot_native (const char *path)
  {
    if (!m)
      return 1;
    return strncasematch (m->native_path, path, m->native_pathlen)
	    && (path[m->native_pathlen] == '\\' || !path[m->native_pathlen]);
  }
  const char *unchroot (const char *path)
  {
    if (!m)
      return path;
    const char *p = path + m->posix_pathlen;
    if (!*p)
      p = "/";
    return p;
  }
  bool exists () {return !!m;}
  void set (const char *posix, const char *native);
  size_t posix_length () const { return m->posix_pathlen; }
  const char *posix_path () const { return m->posix_path; }
  size_t native_length () const { return m->native_pathlen; }
  const char *native_path () const { return m->native_path; }
};

enum homebodies
{
  CH_HOMEDRIVE,
  CH_HOMEPATH,
  CH_HOME
};

struct passwd;

class cygheap_user
{
  /* Extendend user information.
     The information is derived from the internal_getlogin call
     when on a NT system. */
  char  *pname;         /* user's name */
  char  *plogsrv;       /* Logon server, may be FQDN */
  char  *pdomain;       /* Logon domain of the user */
  char  *homedrive;	/* User's home drive */
  char  *homepath;	/* User's home path */
  char  *pwinname;	/* User's name as far as Windows knows it */
  char  *puserprof;	/* User profile */
  PSID   psid;          /* buffer for user's SID */
  PSID   orig_psid;     /* Remains intact even after impersonation */
public:
  __uid32_t orig_uid;      /* Remains intact even after impersonation */
  __gid32_t orig_gid;      /* Ditto */
  __uid32_t real_uid;      /* Remains intact on seteuid, replaced by setuid */
  __gid32_t real_gid;      /* Ditto */
  user_groups groups;      /* Primary and supp SIDs */

  /* token is needed if set(e)uid should be called. It can be set by a call
     to `set_impersonation_token()'. */
  HANDLE token;
  BOOL   impersonated;

  /* CGF 2002-06-27.  I removed the initializaton from this constructor
     since this class is always allocated statically.  That means that everything
     is zero anyway so there is no need to initialize it to zero.  Since the
     token initialization is always handled during process startup as well,
     I've removed the constructor entirely.  Please reinstate this f this
     situation ever changes.
  cygheap_user () : pname (NULL), plogsrv (NULL), pdomain (NULL),
		    homedrive (NULL), homepath (NULL), psid (NULL),
		    token (INVALID_HANDLE_VALUE) {}
  */

  ~cygheap_user ();

  void set_name (const char *new_name);
  const char *name () const { return pname; }

  const char *env_logsrv (const char *, size_t);
  const char *env_homepath (const char *, size_t);
  const char *env_homedrive (const char *, size_t);
  const char *env_userprofile (const char *, size_t);
  const char *env_domain (const char *, size_t);
  const char *env_name (const char *, size_t);

  const char *logsrv ()
  {
    const char *p = env_logsrv ("LOGONSERVER=", sizeof ("LOGONSERVER=") - 1);
    return (p == almost_null) ? NULL : p;
  }
  const char *winname ()
  {
    const char *p = env_name ("USERNAME=", sizeof ("USERNAME=") - 1);
    return (p == almost_null) ? NULL : p;
  }
  const char *domain ()
  {
    const char *p = env_domain ("USERDOMAIN=", sizeof ("USERDOMAIN=") - 1);
    return (p == almost_null) ? NULL : p;
  }
  BOOL set_sid (PSID new_sid);
  BOOL set_orig_sid ();
  PSID sid () const { return psid; }
  PSID orig_sid () const { return orig_psid; }
  const char *ontherange (homebodies what, struct passwd * = NULL);
  bool issetuid () const
  {
    return impersonated && token != INVALID_HANDLE_VALUE;
  }
  const char *cygheap_user::test_uid (char *&, const char *, size_t)
    __attribute__ ((regparm (3)));
};

/* cwd cache stuff.  */

class muto;

struct cwdstuff
{
  char *posix;
  char *win32;
  DWORD hash;
  muto *cwd_lock;
  char *get (char *buf, int need_posix = 1, int with_chroot = 0, unsigned ulen = MAX_PATH);
  DWORD get_hash ();
  void init ();
  void fixup_after_exec (char *win32, char *posix, DWORD hash);
  bool get_initial ();
  void set (const char *win32_cwd, const char *posix_cwd = NULL);
};

#ifdef DEBUGGING
struct cygheap_debug
{
  handle_list starth;
  handle_list *endh;
  handle_list freeh[500];
};
#endif

struct user_heap_info
{
  void *base;
  void *ptr;
  void *top;
  unsigned chunk;
};

struct init_cygheap
{
  _cmalloc_entry *chain;
  char *buckets[32];
  cygheap_root root;
  cygheap_user user;
  user_heap_info user_heap;
  mode_t umask;
  HANDLE shared_h;
  HANDLE console_h;
  HANDLE etc_changed_h;
  char *cygwin_regname;
  cwdstuff cwd;
  dtable fdtab;
#ifdef DEBUGGING
  cygheap_debug debug;
#endif

  bool etc_changed ();
};

#define CYGHEAPSIZE (sizeof (init_cygheap) + (16000 * sizeof (fhandler_union)) + (4 * 65536))

extern init_cygheap *cygheap;
extern void *cygheap_max;

class cygheap_fdmanip
{
 protected:
  int fd;
  fhandler_base **fh;
  bool locked;
 public:
  cygheap_fdmanip (): fh (NULL) {}
  virtual ~cygheap_fdmanip ()
  {
    if (locked)
      ReleaseResourceLock (LOCK_FD_LIST, WRITE_LOCK | READ_LOCK, "cygheap_fdmanip");
  }
  void release ()
  {
    cygheap->fdtab.release (fd);
  }
  operator int &() {return fd;}
  operator fhandler_base* &() {return *fh;}
  void operator = (fhandler_base *fh) {*this->fh = fh;}
  fhandler_base *operator -> () const {return *fh;}
  bool isopen () const
  {
    if (*fh)
      return true;
    set_errno (EBADF);
    return false;
  }
};

class cygheap_fdnew : public cygheap_fdmanip
{
 public:
  cygheap_fdnew (int seed_fd = -1, bool lockit = true)
  {
    if (lockit)
      SetResourceLock (LOCK_FD_LIST, WRITE_LOCK | READ_LOCK, "cygheap_fdnew");
    if (seed_fd < 0)
      fd = cygheap->fdtab.find_unused_handle ();
    else
      fd = cygheap->fdtab.find_unused_handle (seed_fd + 1);
    if (fd >= 0)
      {
	locked = lockit;
	fh = cygheap->fdtab + fd;
      }
    else
      {
	set_errno (EMFILE);
	if (lockit)
	  ReleaseResourceLock (LOCK_FD_LIST, WRITE_LOCK | READ_LOCK, "cygheap_fdnew");
	locked = false;
      }
  }
  void operator = (fhandler_base *fh) {*this->fh = fh;}
};

class cygheap_fdget : public cygheap_fdmanip
{
 public:
  cygheap_fdget (int fd, bool lockit = false, bool do_set_errno = true)
  {
    if (lockit)
      SetResourceLock (LOCK_FD_LIST, READ_LOCK, "cygheap_fdget");
    if (fd >= 0 && fd < (int) cygheap->fdtab.size
	&& *(fh = cygheap->fdtab + fd) != NULL)
      {
	this->fd = fd;
	locked = lockit;
      }
    else
      {
	this->fd = -1;
	if (do_set_errno)
	  set_errno (EBADF);
	if (lockit)
	  ReleaseResourceLock (LOCK_FD_LIST, READ_LOCK, "cygheap_fdget");
	locked = false;
      }
  }
};

class child_info;
void *__stdcall cygheap_setup_for_child (child_info *ci, bool dup_later) __attribute__ ((regparm(2)));
void __stdcall cygheap_setup_for_child_cleanup (void *, child_info *, bool) __attribute__ ((regparm(3)));
void __stdcall cygheap_fixup_in_child (bool);
extern "C" {
void __stdcall cfree (void *) __attribute__ ((regparm(1)));
void *__stdcall cmalloc (cygheap_types, DWORD) __attribute__ ((regparm(2)));
void *__stdcall crealloc (void *, DWORD) __attribute__ ((regparm(2)));
void *__stdcall ccalloc (cygheap_types, DWORD, DWORD) __attribute__ ((regparm(3)));
char *__stdcall cstrdup (const char *) __attribute__ ((regparm(1)));
char *__stdcall cstrdup1 (const char *) __attribute__ ((regparm(1)));
void __stdcall cfree_and_set (char *&, char * = NULL) __attribute__ ((regparm(2)));
void __stdcall cygheap_init ();
extern DWORD _cygheap_start;
}
