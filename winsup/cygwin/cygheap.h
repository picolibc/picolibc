/* cygheap.h: Cygwin heap manager.

   Copyright 2000, 2001, 2002, 2003, 2004 Red Hat, Inc.

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
  HEAP_SIGS,
  HEAP_ARCHETYPES,
  HEAP_TLS,
  HEAP_1_START,
  HEAP_1_STR,
  HEAP_1_ARGV,
  HEAP_1_BUF,
  HEAP_1_EXEC,
  HEAP_1_MAX = 100,
  HEAP_MMAP = 200
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
  char posix_path[CYG_MAX_PATH];
  unsigned posix_pathlen;
  char native_path[CYG_MAX_PATH];
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
  cygsid effec_cygsid;  /* buffer for user's SID */
  cygsid saved_cygsid;  /* Remains intact even after impersonation */
public:
  __uid32_t saved_uid;     /* Remains intact even after impersonation */
  __gid32_t saved_gid;     /* Ditto */
  __uid32_t real_uid;      /* Remains intact on seteuid, replaced by setuid */
  __gid32_t real_gid;      /* Ditto */
  user_groups groups;      /* Primary and supp SIDs */

  /* token is needed if set(e)uid should be called. It can be set by a call
     to `set_impersonation_token()'. */
  HANDLE external_token;
  HANDLE internal_token;
  HANDLE current_token;

  /* CGF 2002-06-27.  I removed the initializaton from this constructor
     since this class is always allocated statically.  That means that everything
     is zero anyway so there is no need to initialize it to zero.  Since the
     token initialization is always handled during process startup as well,
     I've removed the constructor entirely.  Please reinstate this if this
     situation ever changes.
  cygheap_user () : pname (NULL), plogsrv (NULL), pdomain (NULL),
		    homedrive (NULL), homepath (NULL),
		    token (INVALID_HANDLE_VALUE) {}
  */

  ~cygheap_user ();

  void init ();
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
  BOOL set_sid (PSID new_sid) {return (BOOL) (effec_cygsid = new_sid);}
  BOOL set_saved_sid () { return (BOOL) (saved_cygsid = effec_cygsid); }
  PSID sid () { return effec_cygsid; }
  PSID saved_sid () { return saved_cygsid; }
  const char *ontherange (homebodies what, struct passwd * = NULL);
  bool issetuid () const { return current_token != INVALID_HANDLE_VALUE; }
  HANDLE token () { return current_token; }
  void deimpersonate ()
  {
    if (issetuid ())
      RevertToSelf ();
  }
  void reimpersonate ()
  {
    if (issetuid ()
	&& !ImpersonateLoggedOnUser (token ()))
      system_printf ("ImpersonateLoggedOnUser: %E");
  }
  bool has_impersonation_tokens ()
    { return external_token != INVALID_HANDLE_VALUE
             || internal_token != INVALID_HANDLE_VALUE
	     || current_token != INVALID_HANDLE_VALUE; }
  void close_impersonation_tokens ()
  {
    if (current_token != INVALID_HANDLE_VALUE)
      {
	if( current_token != external_token && current_token != internal_token)
	  CloseHandle (current_token);
	current_token = INVALID_HANDLE_VALUE;
      }
    if (external_token != INVALID_HANDLE_VALUE)
      {
	CloseHandle (external_token);
	external_token = INVALID_HANDLE_VALUE;
      }
    if (internal_token != INVALID_HANDLE_VALUE)
      {
	CloseHandle (internal_token);
	internal_token = INVALID_HANDLE_VALUE;
      }
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
  char *get (char *buf, int need_posix = 1, int with_chroot = 0, unsigned ulen = CYG_MAX_PATH);
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
  void *max;
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
  char *cygwin_regname;
  cwdstuff cwd;
  dtable fdtab;
  const char *shared_prefix;
#ifdef DEBUGGING
  cygheap_debug debug;
#endif
  struct sigaction *sigs;

  fhandler_tty_slave *ctty;	/* Current tty */
  fhandler_tty_slave *ctty_on_hold;
  struct _threadinfo **threadlist;
  size_t sthreads;
  int open_fhs;
  void close_ctty ();
};

#define CYGHEAPSIZE (sizeof (init_cygheap) + (20000 * sizeof (fhandler_union)) + (5 * 65536))

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
      cygheap->fdtab.unlock ();
  }
  void release ()
  {
    cygheap->fdtab.release (fd);
  }
  operator int &() {return fd;}
  operator fhandler_base* &() {return *fh;}
  operator fhandler_socket* () const {return reinterpret_cast<fhandler_socket *> (*fh);}
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
      cygheap->fdtab.lock ();
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
	  cygheap->fdtab.unlock ();
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
      cygheap->fdtab.lock ();
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
	  cygheap->fdtab.unlock ();
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
