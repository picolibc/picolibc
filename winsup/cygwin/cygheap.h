/* cygheap.h: Cygwin heap manager.

   Copyright 2000 Cygnus Solutions.

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
  HEAP_1_START,
  HEAP_1_STR,
  HEAP_1_ARGV,
  HEAP_1_BUF,
  HEAP_1_EXEC,
  HEAP_1_MAX = 100
};

#define CYGHEAPSIZE ((4000 * sizeof (fhandler_union)) + (2 * 65536))

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

class cygheap_root
{
  /* Root directory information.
     This is used after a chroot is called. */
  size_t rootlen;
  char *root;
public:
  cygheap_root (cygheap_root &nroot);
  ~cygheap_root ();
  char *operator =(const char *new_root);
  size_t length () const { return rootlen; }
  const char *path () const { return root; }
};

class cygheap_user {
  /* Extendend user information.
     The information is derived from the internal_getlogin call
     when on a NT system. */
  char  *pname;         /* user's name */
  char  *plogsrv;       /* Logon server, may be FQDN */
  char  *pdomain;       /* Logon domain of the user */
  PSID   psid;          /* buffer for user's SID */
public:
  uid_t orig_uid;      /* Remains intact even after impersonation */
  uid_t orig_gid;      /* Ditto */
  uid_t real_uid;      /* Remains intact on seteuid, replaced by setuid */
  gid_t real_gid;      /* Ditto */

  /* token is needed if set(e)uid should be called. It can be set by a call
     to `set_impersonation_token()'. */
  HANDLE token;
  BOOL   impersonated;

  cygheap_user () : pname (NULL), plogsrv (NULL), pdomain (NULL), psid (NULL) {}
  ~cygheap_user ();

  void set_name (const char *new_name);
  const char *name () const { return pname; }

  void set_logsrv (const char *new_logsrv);
  const char *logsrv () const { return plogsrv; }

  void set_domain (const char *new_domain);
  const char *domain () const { return pdomain; }

  BOOL set_sid (PSID new_sid);
  PSID sid () const { return psid; }

  void operator =(cygheap_user &user)
    {
      set_name (user.name ());
      set_logsrv (user.logsrv ());
      set_domain (user.domain ());
      set_sid (user.sid ());
    }
};

struct init_cygheap
{
  _cmalloc_entry *chain;
  cygheap_root root;
  cygheap_user user;
  mode_t umask;
};

extern init_cygheap *cygheap;
extern void *cygheap_max;

extern "C" {
void __stdcall cfree (void *) __attribute__ ((regparm(1)));
void __stdcall cygheap_fixup_in_child (HANDLE, bool);
void *__stdcall cmalloc (cygheap_types, DWORD) __attribute__ ((regparm(2)));
void *__stdcall crealloc (void *, DWORD) __attribute__ ((regparm(2)));
void *__stdcall ccalloc (cygheap_types, DWORD, DWORD) __attribute__ ((regparm(3)));
char *__stdcall cstrdup (const char *) __attribute__ ((regparm(1)));
char *__stdcall cstrdup1 (const char *) __attribute__ ((regparm(1)));
void __stdcall cygheap_init ();
}
