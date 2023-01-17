/* cygheap.h: Cygwin heap manager.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "clock.h"
#include "cygheap_malloc.h"
#include "pwdgrp.h"

#define incygheap(s) (cygheap && ((char *) (s) >= (char *) cygheap) && ((char *) (s) <= ((char *) cygheap_max)))

struct _cmalloc_entry
{
  union
  {
    unsigned b;
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
  bool caseinsensitive;
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
    return path_prefix_p (m->posix_path, path, m->posix_pathlen,
			  m->caseinsensitive);
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
  void set (const char *, const char *, bool);
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
  char  *pname;		/* user's name */
  char  *plogsrv;       /* Logon server, may be FQDN */
  char  *pdomain;       /* Logon domain of the user */
  char  *homedrive;	/* User's home drive */
  char  *homepath;	/* User's home path */
  char  *psystemroot;	/* Value of SYSTEMROOT */
  char  *pwinname;	/* User's name as far as Windows knows it */
  char  *puserprof;	/* User profile */
  cygsid effec_cygsid;  /* buffer for user's SID */
  cygsid saved_cygsid;  /* Remains intact even after impersonation */
public:
  uid_t saved_uid;      /* Remains intact even after impersonation */
  gid_t saved_gid;      /* Ditto */
  uid_t real_uid;       /* Remains intact on seteuid, replaced by setuid */
  gid_t real_gid;       /* Ditto */
  user_groups groups;   /* Primary and supp SIDs */

  HANDLE external_token;	 /* token from set_impersonation_token call */
  HANDLE internal_token;	 /* password-less token fetched in seteuid */
  HANDLE curr_primary_token;	 /* Just a copy of external or internal token */
  HANDLE curr_imp_token;	 /* impersonation token derived from primary
				    token */
  bool ext_token_is_restricted;  /* external_token is restricted token */
  bool curr_token_is_restricted; /* curr_primary_token is restricted token */
  bool setuid_to_restricted;     /* switch to restricted token by setuid () */

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
  const char *env_systemroot (const char *, size_t);

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
  void set_sid (PSID new_sid) { effec_cygsid = new_sid;}
  void set_saved_sid () { saved_cygsid = effec_cygsid; }
  cygpsid &sid () { return effec_cygsid; }
  cygpsid &saved_sid () { return saved_cygsid; }
  const char *ontherange (homebodies what, struct passwd * = NULL);
#define NO_IMPERSONATION NULL
  bool issetuid () const { return curr_imp_token != NO_IMPERSONATION; }
  HANDLE primary_token () { return curr_primary_token; }
  HANDLE imp_token () { return curr_imp_token; }
  void deimpersonate ()
  {
    RevertToSelf ();
  }
  bool reimpersonate ()
  {
    if (issetuid ())
      return ImpersonateLoggedOnUser (primary_token ());
    return true;
  }
  bool has_impersonation_tokens ()
    { return external_token != NO_IMPERSONATION
	     || internal_token != NO_IMPERSONATION
	     || curr_primary_token != NO_IMPERSONATION; }
  void close_impersonation_tokens ()
  {
    if (curr_imp_token != NO_IMPERSONATION)
      CloseHandle (curr_imp_token);
    if (external_token != NO_IMPERSONATION)
      CloseHandle (external_token);
    if (internal_token != NO_IMPERSONATION)
      CloseHandle (internal_token);
  }
  PWCHAR get_windows_id (PWCHAR buf)
  {
    return effec_cygsid.string (buf);
  }
  char *get_windows_id (char *buf)
  {
    return effec_cygsid.string (buf);
  }

  const char *test_uid (char *&, const char *, size_t);
};

/* cwd cache stuff.  */

/* This class is used to store the CWD.  The CWD storage in the
   RTL_USER_PROCESS_PARAMETERS block is only an afterthought now.  The actual
   CWD storage is a FAST_CWD structure which is allocated on the process heap.
   The new method only requires minimal locking and it's much more multi-thread
   friendly.  Presumably it minimizes contention when accessing the CWD.
   The class fcwd_access_t is supposed to encapsulate the gory implementation
   details depending on OS version from the calling functions.
   The layout of all structures has been tested on 32 and 64 bit. */
class fcwd_access_t {
  LONG           ReferenceCount;	/* Only release when this is 0. */
  HANDLE         DirectoryHandle;
  ULONG          OldDismountCount;	/* Reflects the system DismountCount
					   at the time the CWD has been set. */
  UNICODE_STRING Path;			/* Path's Buffer member always refers
					   to the following Buffer array. */
  LONG           FSCharacteristics;	/* Taken from FileFsDeviceInformation */
  /* fcwd_access_t is dynamically allocated with a size of
     sizeof(fcwd_access_t) + cwd.Length.  Preallocating 2 chars
     here allows to append a trailing backslash and NUL. */
  WCHAR          Buffer[2] __attribute ((aligned (8)));

public:
  void CopyPath (UNICODE_STRING &target);
  void Free (PVOID heap);
  void FillIn (HANDLE dir, PUNICODE_STRING name, ULONG old_dismount_count);
  static void SetDirHandleFromBufferPointer (PWCHAR buf_p, HANDLE dir);
  static void SetVersionFromPointer (PBYTE buf_p, bool is_buffer);
};

class cwdstuff
{
private:
  char *posix;
  HANDLE dir;
  DWORD drive_length;
  int error;		/* This contains an errno number which corresponds
			   to the problem with this path when trying to start
			   a native Win32 application.  See cwdstuff::set for
			   how it gets set.  See child_info_spawn::worker for how
			   it's evaluated. */

  friend class fcwd_access_t;
  /* fast_cwd_ptr is a pointer to the global RtlpCurDirRef pointer in
     ntdll.dll pointing to the FAST_CWD structure which constitutes the CWD.
     Unfortunately RtlpCurDirRef is not exported from ntdll.dll. */
  fcwd_access_t **fast_cwd_ptr;
  void override_win32_cwd (bool init, ULONG old_dismount_count);

public:
  UNICODE_STRING win32;
  static SRWLOCK NO_COPY cwd_lock;

  static void acquire_read () { AcquireSRWLockShared (&cwd_lock); }
  static void release_read () { ReleaseSRWLockShared (&cwd_lock); }
  static void acquire_write () { AcquireSRWLockExclusive (&cwd_lock); }
  static void release_write () { ReleaseSRWLockExclusive (&cwd_lock); }
  const char *get_posix () const { return posix; };
  void reset_posix (wchar_t *w_cwd);
  char *get (char *buf, int need_posix = 1, int with_chroot = 0,
	     unsigned ulen = NT_MAX_PATH);
  PWCHAR get (PWCHAR buf, unsigned buflen = NT_MAX_PATH)
  {
    acquire_read ();
    buf[0] = L'\0';
    wcsncat (buf, win32.Buffer, buflen - 1);
    release_read ();
    return buf;
  }
  HANDLE get_handle () { return dir; }
  DWORD get_drive (char * dst)
  {
    acquire_read ();
    DWORD ret = sys_wcstombs (dst, NT_MAX_PATH, win32.Buffer, drive_length);
    release_read ();
    return ret;
  }
  int get_error () const { return error; }
  const char *get_error_desc () const;
  void init ();
  int set (path_conv *, const char *);
};

#ifdef DEBUGGING
struct cygheap_debug
{
  handle_list starth;
  handle_list freeh[500];
};
#endif

struct cygheap_locale
{
  mbtowc_p mbtowc;
};

struct user_heap_info
{
  void *base;
  void *ptr;
  void *top;
  void *max;
  SIZE_T chunk;
  void *sbrk (ptrdiff_t);
  void init ();
};

/* This info is maintained for /proc/<PID>/maps ONLY! */
struct shared_region_info
{
  void *cygwin_shared_addr;
  void *user_shared_addr;
  void *myself_shared_addr;
  void *console_shared_addr;
};

class cygheap_domain_info
{
  PWCHAR pdom_name;
  PWCHAR pdom_dns_name;
  cygsid pdom_sid;

  PWCHAR adom_name;
  cygsid adom_sid;

  PDS_DOMAIN_TRUSTSW tdom;
  ULONG tdom_count;

  PWCHAR rfc2307_domain_buf;

public:
  bool init ();

  inline PCWSTR primary_flat_name () const { return pdom_name; }
  inline PCWSTR primary_dns_name () const { return pdom_dns_name; }
  inline cygsid &primary_sid () { return pdom_sid; }

  inline bool member_machine () const { return pdom_sid != NO_SID; }

  inline PCWSTR account_flat_name () const { return adom_name; }
  inline cygsid &account_sid () { return adom_sid; }

  inline PDS_DOMAIN_TRUSTSW trusted_domain (ULONG idx) const
    { return (idx < tdom_count) ? tdom + idx : NULL; }
  PDS_DOMAIN_TRUSTSW add_domain (PCWSTR, PSID);

  inline PWCHAR get_rfc2307_domain () const
    { return rfc2307_domain_buf ?: NULL; }
};

#define NSS_SEPARATOR_STRING	L"+"
#define NSS_SEPARATOR_CHAR	(NSS_SEPARATOR_STRING[0])

class cygheap_pwdgrp
{
public:
  enum nss_scheme_method {
    NSS_SCHEME_FALLBACK = 0,
    NSS_SCHEME_WINDOWS,
    NSS_SCHEME_CYGWIN,
    NSS_SCHEME_UNIX,
    NSS_SCHEME_DESC,
    NSS_SCHEME_PATH,
    NSS_SCHEME_FREEATTR
  };
  struct nss_scheme_t {
    nss_scheme_method	method;
    PWCHAR		attrib;
  };
private:
  bool		nss_inited;
  uint32_t	pwd_src;
  uint32_t	grp_src;
  bool		caching;

#define NSS_SCHEME_MAX	4
  nss_scheme_t	home_scheme[NSS_SCHEME_MAX];
  nss_scheme_t	shell_scheme[NSS_SCHEME_MAX];
  nss_scheme_t	gecos_scheme[NSS_SCHEME_MAX];

  uint32_t	enums;
  PWCHAR	enum_tdoms;

  void nss_init_line (const char *line);
  void _nss_init ();

public:
  struct {
    pwdgrp cygserver;
    pwdgrp file;
    pwdgrp win;
  } pwd_cache;
  struct {
    pwdgrp cygserver;
    pwdgrp file;
    pwdgrp win;
  } grp_cache;

  void init ();

  /* Implemented in ldap.cc */
  PWCHAR *ldap_user_attr;
  void init_ldap_user_attr ();

  inline void nss_init () { if (!nss_inited) _nss_init (); }
  inline bool nss_pwd_files () const { return !!(pwd_src & NSS_SRC_FILES); }
  inline bool nss_pwd_db () const { return !!(pwd_src & NSS_SRC_DB); }
  inline int  nss_pwd_src () const { return pwd_src; } /* CW_GETNSS_PWD_SRC */
  inline bool nss_grp_files () const { return !!(grp_src & NSS_SRC_FILES); }
  inline bool nss_grp_db () const { return !!(grp_src & NSS_SRC_DB); }
  inline int  nss_grp_src () const { return grp_src; } /* CW_GETNSS_GRP_SRC */
  inline bool nss_cygserver_caching () const { return caching; }
  inline void nss_disable_cygserver_caching () { caching = false; }

  char *get_home (cyg_ldap *pldap, cygpsid &sid, PCWSTR dom, PCWSTR dnsdomain,
		  PCWSTR name, bool fq);
  char *get_home (struct _USER_INFO_3 *ui, cygpsid &sid, PCWSTR dom,
		  PCWSTR name, bool fq);

  char *get_shell (cyg_ldap *pldap, cygpsid &sid, PCWSTR dom, PCWSTR dnsdomain,
		   PCWSTR name, bool fq);
  char *get_shell (struct _USER_INFO_3 *ui, cygpsid &sid, PCWSTR dom,
		   PCWSTR name, bool fq);

  char *get_gecos (cyg_ldap *pldap, cygpsid &sid, PCWSTR dom, PCWSTR dnsdomain,
		   PCWSTR name, bool fq);
  char *get_gecos (struct _USER_INFO_3 *ui, cygpsid &sid, PCWSTR dom,
		   PCWSTR name, bool fq);

  inline int nss_db_enums () const { return enums; }
  inline PCWSTR nss_db_enum_tdoms () const { return enum_tdoms; }
};

class cygheap_ugid_cache
{
  struct idmap {
    uint32_t nfs_id;
    uint32_t cyg_id;
  };
  class idmaps {
    uint32_t _cnt;
    uint32_t _max;
    idmap *_map;
  public:
    uint32_t get (uint32_t id) const
    {
      for (uint32_t i = 0; i < _cnt; ++i)
	if (_map[i].nfs_id == id)
	  return _map[i].cyg_id;
      return (uint32_t) -1;
    }
    uint32_t reverse_get (uint32_t id) const
    {
      for (uint32_t i = 0; i < _cnt; ++i)
	if (_map[i].cyg_id == id)
	  return _map[i].nfs_id;
      return (uint32_t) -1;
    }
    void add (uint32_t nfs_id, uint32_t cyg_id)
    {
      if (_cnt >= _max)
	_map = (idmap *) crealloc (_map, (_max += 10) * sizeof (*_map));
      _map[_cnt].nfs_id = nfs_id;
      _map[_cnt].cyg_id = cyg_id;
      ++_cnt;
    }
  };
  idmaps uids;
  idmaps gids;

public:
  uid_t get_uid (uid_t uid) const { return uids.get (uid); }
  gid_t get_gid (gid_t gid) const { return gids.get (gid); }
  uid_t reverse_get_uid (uid_t uid) const { return uids.reverse_get (uid); }
  gid_t reverse_get_gid (gid_t gid) const { return gids.reverse_get (gid); }
  void add_uid (uid_t nfs_uid, uid_t cyg_uid) { uids.add (nfs_uid, cyg_uid); }
  void add_gid (gid_t nfs_gid, gid_t cyg_gid) { gids.add (nfs_gid, cyg_gid); }
};

struct hook_chain
{
  void **loc;
  const void *func;
  struct hook_chain *next;
};

struct mini_cygheap
{
  cygheap_locale locale;
};

#define NBUCKETS 40

struct threadlist_t
{
  struct _cygtls *thread;
  HANDLE mutex;			/* Used to avoid accessing tls area of
				   deleted thread.  See comment in
				   cygheap::remove_tls for a description. */
};

struct init_cygheap: public mini_cygheap
{
  _cmalloc_entry *chain;
  char *buckets[NBUCKETS];
  UNICODE_STRING installation_root;
  WCHAR installation_root_buf[PATH_MAX];
  UNICODE_STRING installation_dir;
  WCHAR installation_dir_buf[PATH_MAX];
  UNICODE_STRING installation_key;
  WCHAR installation_key_buf[18];
  cygheap_root root;
  cygheap_domain_info dom;
  cygheap_pwdgrp pg;
  cygheap_ugid_cache ugid_cache;
  cygheap_user user;
  user_heap_info user_heap;
  shared_region_info shared_regions;
  mode_t umask;
  LONG rlim_as_id;
  unsigned long rlim_core;
  HANDLE console_h;
  cwdstuff cwd;
  dtable fdtab;
#ifdef DEBUGGING
  cygheap_debug debug;
#endif
  struct sigaction *sigs;

  fhandler_termios *ctty;	/* Current tty */
  threadlist_t *threadlist;
  uint32_t sthreads;
  pid_t pid;			/* my pid */
  struct {			/* Equivalent to using LIST_HEAD. */
    struct inode_t *lh_first;
  } inode_list;			/* Global inode pointer for adv. locking. */
  hook_chain hooks;
  void close_ctty ();
  void init_installation_root ();
  void init_tls_list ();;
  void add_tls (_cygtls *);
  HANDLE remove_tls (_cygtls *);
  threadlist_t *find_tls (_cygtls *);
  threadlist_t *find_tls (int, bool&);
  sigset_t compute_sigblkmask ();
  void unlock_tls (threadlist_t *t) { if (t) ReleaseMutex (t->mutex); }
};


#define _CYGHEAPSIZE_SLOP (128 * 1024)
#define CYGHEAPSIZE (sizeof (init_cygheap) + (20000 * sizeof (fhandler_union)) + _CYGHEAPSIZE_SLOP)
#define CYGHEAPSIZE_MIN (sizeof (init_cygheap) + (10000 * sizeof (fhandler_union)))

extern init_cygheap *cygheap;
extern void *cygheap_max;

class cygheap_fdmanip
{
 protected:
  int fd;
  bool locked;
 public:
  cygheap_fdmanip (): fd (-1), locked (false) {}
  virtual ~cygheap_fdmanip ()
  {
    if (locked)
      cygheap->fdtab.unlock ();
  }
  virtual void release () { cygheap->fdtab.release (fd); }
  operator int &() {return fd;}
  operator fhandler_base* &() {return cygheap->fdtab[fd];}
  operator fhandler_socket* () const {return reinterpret_cast<fhandler_socket *> (cygheap->fdtab[fd]);}
  operator fhandler_pipe* () const {return reinterpret_cast<fhandler_pipe *> (cygheap->fdtab[fd]);}
  void operator = (fhandler_base *fh) {cygheap->fdtab[fd] = fh;}
  fhandler_base *operator -> () const {return cygheap->fdtab[fd];}
  bool isopen () const
  {
    if (cygheap->fdtab[fd])
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
      locked = lockit;
    else
      {
	/* errno set by find_unused_handle */
	if (lockit)
	  cygheap->fdtab.unlock ();
	locked = false;
      }
  }
  ~cygheap_fdnew ()
  {
    if (cygheap->fdtab[fd])
      cygheap->fdtab[fd]->inc_refcnt ();
  }
  void operator = (fhandler_base *fh) {cygheap->fdtab[fd] = fh;}
};

class cygheap_fdget : public cygheap_fdmanip
{
  fhandler_base *fh;
public:
  cygheap_fdget (int fd, bool lockit = false, bool do_set_errno = true)
  {
    if (lockit)
      cygheap->fdtab.lock ();
    if (fd >= 0 && fd < (int) cygheap->fdtab.size && cygheap->fdtab[fd] != NULL)
      {
	this->fd = fd;
	locked = lockit;
	fh = cygheap->fdtab[fd];
	fh->inc_refcnt ();
      }
    else
      {
	this->fd = -1;
	if (do_set_errno)
	  set_errno (EBADF);
	if (lockit)
	  cygheap->fdtab.unlock ();
	locked = false;
	fh = NULL;
      }
  }
  ~cygheap_fdget ()
  {
    if (fh && fh->dec_refcnt () <= 0)
      {
	debug_only_printf ("deleting fh %p", fh);
	delete fh;
      }
  }
  void release () { cygheap->fdtab.release (fd); }
};

class cygheap_fdenum : public cygheap_fdmanip
{
 public:
  cygheap_fdenum (bool lockit = false)
  {
    locked = lockit;
    if (lockit)
      cygheap->fdtab.lock ();
    fd = -1;
  }
  int next ()
  {
    while (++fd < (int) cygheap->fdtab.size)
      if (cygheap->fdtab[fd] != NULL)
	return fd;
    return -1;
  }
  void rewind ()
  {
    fd = -1;
  }
};

void cygheap_fixup_in_child (bool);
void cygheap_init ();
void setup_cygheap ();
