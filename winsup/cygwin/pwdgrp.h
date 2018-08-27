/* pwdgrp.h

   Stuff common to pwd and grp handling.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#pragma once

#include "sync.h"
#include "ldap.h"
#include "miscfuncs.h"
#include "userinfo.h"

/* These functions are needed to allow searching and walking through
   the passwd and group lists */
extern struct passwd *internal_getpwsid (cygpsid &, cyg_ldap * = NULL);
extern struct passwd *internal_getpwsid_from_db (cygpsid &sid);
extern struct passwd *internal_getpwnam (const char *, cyg_ldap * = NULL);
extern struct passwd *internal_getpwuid (uid_t, cyg_ldap * = NULL);
extern struct group *internal_getgrsid (cygpsid &, cyg_ldap * = NULL);
extern struct group *internal_getgrsid_from_db (cygpsid &sid);
extern struct group *internal_getgrgid (gid_t, cyg_ldap * = NULL);
extern struct group *internal_getgrnam (const char *, cyg_ldap * = NULL);

extern int internal_getgroups (int, gid_t *, cyg_ldap *);

/* These functions are called from mkpasswd/mkgroup via cygwin_internal. */
void *setpwent_filtered (int enums, PCWSTR enum_tdoms);
void *getpwent_filtered (void *gr);
void endpwent_filtered (void *gr);
void *setgrent_filtered (int enums, PCWSTR enum_tdoms);
void *getgrent_filtered (void *gr);
void endgrent_filtered (void *gr);

/* NOTE: The below sid members were cygsid's originally.  Don't do that.
   cygsid's are pointer based.  When adding new entries to the passwd or
   group caches, a crealloc call potenitally moves the entries and then
   the cygsid pointers point into neverneverland. */
struct pg_pwd
{
  struct passwd p;
  BYTE sid[SECURITY_MAX_SID_SIZE];
  size_t len;
};

struct pg_grp
{
  struct group g;
  BYTE sid[SECURITY_MAX_SID_SIZE];
  size_t len;
};

class pwdgrp
{
  friend class pg_ent;
  friend class pw_ent;
  friend class gr_ent;

  unsigned pwdgrp_buf_elem_size;
  void *pwdgrp_buf;
  bool (pwdgrp::*parse) ();
  UNICODE_STRING path;
  OBJECT_ATTRIBUTES attr;
  LARGE_INTEGER last_modified;
  char *lptr;
  ULONG curr_lines;
  ULONG max_lines;
  static muto pglock;

  bool parse_passwd ();
  bool parse_group ();
  char *add_line (char *);
  char *raw_ptr () const {return lptr;}
  char *next_str (char);
  bool next_num (unsigned long&);
  bool next_num (unsigned int& i)
  {
    unsigned long x;
    bool res = next_num (x);
    i = (unsigned int) x;
    return res;
  }
  inline bool next_num (int& i)
  {
    unsigned long x;
    bool res = next_num (x);
    i = (int) x;
    return res;
  }
  void *add_account_post_fetch (char *line, bool lock);
  void *add_account_from_file (cygpsid &sid);
  void *add_account_from_file (const char *name);
  void *add_account_from_file (uint32_t id);
  void *add_account_from_windows (cygpsid &sid, cyg_ldap *pldap = NULL);
  void *add_account_from_windows (const char *name, cyg_ldap *pldap = NULL);
  void *add_account_from_windows (uint32_t id, cyg_ldap *pldap = NULL);
  void *add_account_from_cygserver (cygpsid &sid);
  void *add_account_from_cygserver (const char *name);
  void *add_account_from_cygserver (uint32_t id);
  bool construct_sid_from_name (cygsid &sid, wchar_t *name, wchar_t *sep);
  char *fetch_account_from_line (fetch_user_arg_t &arg, const char *line);
  char *fetch_account_from_file (fetch_user_arg_t &arg);
  char *fetch_account_from_windows (fetch_user_arg_t &arg,
				    cyg_ldap *pldap = NULL);
  char *fetch_account_from_cygserver (fetch_user_arg_t &arg);

public:
  ULONG cached_users () const { return curr_lines; }
  ULONG cached_groups () const { return curr_lines; }
  POBJECT_ATTRIBUTES file_attr () { return &attr; }
  bool check_file ();

  void init_pwd ();
  bool is_passwd () const { return pwdgrp_buf_elem_size == sizeof (pg_pwd); }
  pg_pwd *passwd () const { return (pg_pwd *) pwdgrp_buf; };
  struct passwd *add_user_from_cygserver (cygpsid &sid)
    { return (struct passwd *) add_account_from_cygserver (sid); }
  struct passwd *add_user_from_cygserver (const char *name)
    { return (struct passwd *) add_account_from_cygserver (name); }
  struct passwd *add_user_from_cygserver (uint32_t id)
    { return (struct passwd *) add_account_from_cygserver (id); }
  struct passwd *add_user_from_file (cygpsid &sid)
    { return (struct passwd *) add_account_from_file (sid); }
  struct passwd *add_user_from_file (const char *name)
    { return (struct passwd *) add_account_from_file (name); }
  struct passwd *add_user_from_file (uint32_t id)
    { return (struct passwd *) add_account_from_file (id); }
  struct passwd *add_user_from_windows (cygpsid &sid, cyg_ldap *pldap = NULL)
    { return (struct passwd *) add_account_from_windows (sid, pldap); }
  struct passwd *add_user_from_windows (const char *name,
  					cyg_ldap* pldap = NULL)
    { return (struct passwd *) add_account_from_windows (name, pldap); }
  struct passwd *add_user_from_windows (uint32_t id, cyg_ldap *pldap = NULL)
    { return (struct passwd *) add_account_from_windows (id, pldap); }
  struct passwd *find_user (cygpsid &sid);
  struct passwd *find_user (const char *name);
  struct passwd *find_user (uid_t uid);

  void init_grp ();
  bool is_group () const { return pwdgrp_buf_elem_size == sizeof (pg_grp); }
  pg_grp *group () const { return (pg_grp *) pwdgrp_buf; };
  struct group *add_group_from_cygserver (cygpsid &sid)
    { return (struct group *) add_account_from_cygserver (sid); }
  struct group *add_group_from_cygserver (const char *name)
    { return (struct group *) add_account_from_cygserver (name); }
  struct group *add_group_from_cygserver (uint32_t id)
    { return (struct group *) add_account_from_cygserver (id); }
  struct group *add_group_from_file (cygpsid &sid)
    { return (struct group *) add_account_from_file (sid); }
  struct group *add_group_from_file (const char *name)
    { return (struct group *) add_account_from_file (name); }
  struct group *add_group_from_file (uint32_t id)
    { return (struct group *) add_account_from_file (id); }
  struct group *add_group_from_windows (cygpsid &sid, cyg_ldap *pldap = NULL)
    { return (struct group *) add_account_from_windows (sid, pldap); }
  struct group *add_group_from_windows (const char *name,
					cyg_ldap *pldap = NULL)
    { return (struct group *) add_account_from_windows (name, pldap); }
  struct group *add_group_from_windows (uint32_t id, cyg_ldap *pldap = NULL)
    { return (struct group *) add_account_from_windows (id, pldap); }
  struct group *add_group_from_windows (fetch_acc_t &full_acc,
  					cyg_ldap *pldap = NULL);
  struct group *find_group (cygpsid &sid);
  struct group *find_group (const char *name);
  struct group *find_group (gid_t gid);
};

class pg_ent
{
protected:
  pwdgrp         pg;
  bool           group;
  pg_pwd         pwd;
  pg_grp         grp;
  NT_readline    rl;
  cyg_ldap       cldap;
  PCHAR          buf;
  ULONG          cnt;
  ULONG          max;
  ULONG_PTR      resume;
  int            enums;		/* ENUM_xxx values defined in sys/cygwin.h. */
  PCWSTR         enum_tdoms;
  bool           from_files;
  bool           from_db;
  UNICODE_STRING dom;
  enum {
    rewound = 0,
    from_cache,
    from_file,
    from_builtin,
    from_local,
    from_sam,
    from_ad,
    finished
  } state;

  void clear_cache ();
  inline bool nss_db_enum_caches () const { return !!(enums & ENUM_CACHE); }
  inline bool nss_db_enum_files () const { return !!(enums & ENUM_FILES); }
  inline bool nss_db_enum_builtin () const { return !!(enums & ENUM_BUILTIN); }
  inline bool nss_db_enum_local () const { return !!(enums & ENUM_LOCAL); }
  inline bool nss_db_enum_primary () const { return !!(enums & ENUM_PRIMARY); }
  inline bool nss_db_enum_tdom (PWCHAR domain)
    {
      if (enums & ENUM_TDOMS_ALL)
        return true;
      if (!(enums & ENUM_TDOMS) || !enum_tdoms || !domain)
        return false;
      for (PCWSTR td = enum_tdoms; td && *td; td = wcschr (td, L'\0'))
        if (!wcscasecmp (td, domain))
          return true;
      return false;
    }
  virtual void *enumerate_caches () = 0;
  virtual void *enumerate_file ();
  virtual void *enumerate_builtin ();
  virtual void *enumerate_local () = 0;
  virtual void *enumerate_sam ();
  virtual void *enumerate_ad ();

public:
  void setent (bool _group, int _enums = 0, PCWSTR _enum_tdoms = NULL);
  void *getent ();
  void endent (bool _group);
};

class pw_ent : public pg_ent
{
  void *enumerate_caches ();
  void *enumerate_local ();
public:
  inline void setpwent (int _enums = 0, PCWSTR _enum_tdoms = NULL)
    { setent (false, _enums, _enum_tdoms); }
  struct passwd *getpwent ();
  inline void endpwent () { endent (false); }
};

class gr_ent : public pg_ent
{
  void *enumerate_caches ();
  void *enumerate_local ();
public:
  inline void setgrent (int _enums = 0, PCWSTR _enum_tdoms = NULL)
    { setent (true, _enums, _enum_tdoms); }
  struct group *getgrent ();
  inline void endgrent () { endent (true); }
};

/* These inline methods have to be defined here so that pg_pwd and pg_grp
   are defined. */
inline BOOL cygsid::getfrompw (const struct passwd *pw)
  { return (*this = pw ? (PSID) ((pg_pwd *) pw)->sid : NO_SID) != NO_SID; }

inline BOOL cygsid::getfromgr (const struct group *gr)
  { return (*this = gr ? (PSID) ((pg_grp *) gr)->sid : NO_SID) != NO_SID; }

/* Use these functions if you just need the PSID. */
inline PSID sidfromuid (uid_t uid, cyg_ldap *pldap)
  {
    struct passwd *pw = internal_getpwuid (uid, pldap);
    if (pw)
      return (PSID) ((pg_pwd *) pw)->sid;
    return NO_SID;
  }
inline PSID sidfromgid (gid_t gid, cyg_ldap *pldap)
  {
    struct group *gr = internal_getgrgid (gid, pldap);
    if (gr)
      return (PSID) ((pg_grp *) gr)->sid;
    return NO_SID;
  }
