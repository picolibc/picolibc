/* security.h: security declarations

   Copyright 2000, 2001, 2002 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _SECURITY_H
#define _SECURITY_H

#include <accctrl.h>

#define DEFAULT_UID DOMAIN_USER_RID_ADMIN
#define UNKNOWN_UID 400 /* Non conflicting number */
#define UNKNOWN_GID 401

#define MAX_SID_LEN 40
#define MAX_DACL_LEN(n) (sizeof (ACL) \
		   + (n) * (sizeof (ACCESS_ALLOWED_ACE) - sizeof (DWORD) + MAX_SID_LEN))

#define NO_SID ((PSID)NULL)

class cygpsid {
protected:
  PSID psid;
public:
  cygpsid () {}
  cygpsid (PSID nsid) { psid = nsid; }
  operator const PSID () { return psid; }
  const PSID operator= (PSID nsid) { return psid = nsid;}
  __uid32_t get_id (BOOL search_grp, int *type = NULL);
  int get_uid () { return get_id (FALSE); }
  int get_gid () { return get_id (TRUE); }

  char *string (char *nsidstr) const;

  bool operator== (const PSID nsid) const
    {
      if (!psid || !nsid)
	return nsid == psid;
      return EqualSid (psid, nsid);
    }
  bool operator!= (const PSID nsid) const
    { return !(*this == nsid); }
  bool operator== (const char *nsidstr) const;
  bool operator!= (const char *nsidstr) const
    { return !(*this == nsidstr); }

  void debug_print (const char *prefix = NULL) const
    {
      char buf[256];
      debug_printf ("%s %s", prefix ?: "", string (buf) ?: "NULL");
    }
};

class cygsid : public cygpsid {
  char sbuf[MAX_SID_LEN];

  const PSID getfromstr (const char *nsidstr);
  PSID get_sid (DWORD s, DWORD cnt, DWORD *r);

  inline const PSID assign (const PSID nsid)
    {
      if (!nsid)
	psid = NO_SID;
      else
	{
	  psid = (PSID) sbuf;
	  CopySid (MAX_SID_LEN, psid, nsid);
	}
      return psid;
    }

public:
  static void init();
  inline operator const PSID () { return psid; }

  inline const PSID operator= (cygsid &nsid)
    { return assign (nsid); }
  inline const PSID operator= (const PSID nsid)
    { return assign (nsid); }
  inline const PSID operator= (const char *nsidstr)
    { return getfromstr (nsidstr); }

  inline cygsid () : cygpsid ((PSID) sbuf) {}
  inline cygsid (const PSID nsid) { *this = nsid; }
  inline cygsid (const char *nstrsid) { *this = nstrsid; }

  inline PSID set () { return psid = (PSID) sbuf; }

  BOOL getfrompw (const struct passwd *pw);
  BOOL getfromgr (const struct __group32 *gr);
};

typedef enum { cygsidlist_empty, cygsidlist_alloc, cygsidlist_auto } cygsidlist_type;
class cygsidlist {
  int maxcount;
public:
  int count;
  cygsid *sids;
  cygsidlist_type type;

  cygsidlist (cygsidlist_type t, int m)
    {
      type = t;
      count = 0;
      maxcount = m;
      if (t == cygsidlist_alloc)
	sids = alloc_sids (m);
      else
	sids = new cygsid [m];
    }
  ~cygsidlist () { if (type == cygsidlist_auto) delete [] sids; }

  BOOL add (const PSID nsi) /* Only with auto for now */
    {
      if (count >= maxcount)
	{
	  cygsid *tmp = new cygsid [ 2 * maxcount];
	  if (!tmp)
	    return FALSE;
	  maxcount *= 2;
	  for (int i = 0; i < count; ++i)
	    tmp[i] = sids[i];
	  delete [] sids;
	  sids = tmp;
	}
      sids[count++] = nsi;
      return TRUE;
    }
  BOOL add (cygsid &nsi) { return add ((PSID) nsi); }
  BOOL add (const char *sidstr)
    { cygsid nsi (sidstr); return add (nsi); }
  BOOL addfromgr (struct __group32 *gr) /* Only with alloc */
    { return sids[count++].getfromgr (gr); }

  BOOL operator+= (cygsid &si) { return add (si); }
  BOOL operator+= (const char *sidstr) { return add (sidstr); }
  BOOL operator+= (const PSID psid) { return add (psid); }

  int position (const PSID sid) const
    {
      for (int i = 0; i < count; ++i)
	if (sids[i] == sid)
	  return i;
      return -1;
    }

  BOOL contains (const PSID sid) const { return position (sid) >= 0; }
  cygsid *alloc_sids (int n);
  void free_sids ();
  void debug_print (const char *prefix = NULL) const
    {
      debug_printf ("-- begin sidlist ---");
      if (!count)
	debug_printf ("No elements");
      for (int i = 0; i < count; ++i)
	sids[i].debug_print (prefix);
      debug_printf ("-- ende sidlist ---");
    }
};

class user_groups {
public:
  cygsid pgsid;
  cygsidlist sgsids;
  BOOL ischanged;

  BOOL issetgroups () const { return (sgsids.type == cygsidlist_alloc); }
  void update_supp (const cygsidlist &newsids)
    {
      sgsids.free_sids ();
      sgsids = newsids;
      ischanged = TRUE;
    }
  void clear_supp ()
    {
      if (issetgroups ())
	{
	  sgsids.free_sids ();
	  ischanged = TRUE;
	}
    }
  void update_pgrp (const PSID sid)
    {
      pgsid = sid;
      ischanged = TRUE;
    }
};

extern cygsid well_known_null_sid;
extern cygsid well_known_world_sid;
extern cygsid well_known_local_sid;
extern cygsid well_known_creator_owner_sid;
extern cygsid well_known_creator_group_sid;
extern cygsid well_known_dialup_sid;
extern cygsid well_known_network_sid;
extern cygsid well_known_batch_sid;
extern cygsid well_known_interactive_sid;
extern cygsid well_known_service_sid;
extern cygsid well_known_authenticated_users_sid;
extern cygsid well_known_system_sid;
extern cygsid well_known_admins_sid;

inline BOOL
legal_sid_type (SID_NAME_USE type)
{
  return type == SidTypeUser  || type == SidTypeGroup
      || type == SidTypeAlias || type == SidTypeWellKnownGroup;
}

extern bool allow_ntea;
extern bool allow_ntsec;
extern bool allow_smbntsec;

/* File manipulation */
int __stdcall set_process_privileges ();
int __stdcall get_file_attribute (int, const char *, mode_t *,
				  __uid32_t * = NULL, __gid32_t * = NULL);
int __stdcall set_file_attribute (int, const char *, int);
int __stdcall set_file_attribute (int, const char *, __uid32_t, __gid32_t, int);
int __stdcall get_object_attribute (HANDLE handle, SE_OBJECT_TYPE object_type, mode_t *,
				  __uid32_t * = NULL, __gid32_t * = NULL);
LONG __stdcall read_sd(const char *file, PSECURITY_DESCRIPTOR sd_buf, LPDWORD sd_size);
LONG __stdcall write_sd(const char *file, PSECURITY_DESCRIPTOR sd_buf, DWORD sd_size);
BOOL __stdcall add_access_allowed_ace (PACL acl, int offset, DWORD attributes, PSID sid, size_t &len_add, DWORD inherit);
BOOL __stdcall add_access_denied_ace (PACL acl, int offset, DWORD attributes, PSID sid, size_t &len_add, DWORD inherit);
int __stdcall check_file_access (const char *, int);

void set_security_attribute (int attribute, PSECURITY_ATTRIBUTES psa,
			     void *sd_buf, DWORD sd_buf_size);

bool get_sids_info (cygpsid, cygpsid, __uid32_t * , __gid32_t *);

/* Try a subauthentication. */
HANDLE subauth (struct passwd *pw);
/* Try creating a token directly. */
HANDLE create_token (cygsid &usersid, user_groups &groups, struct passwd * pw);
/* Verify an existing token */
BOOL verify_token (HANDLE token, cygsid &usersid, user_groups &groups, BOOL * pintern = NULL);

/* Extract U-domain\user field from passwd entry. */
void extract_nt_dom_user (const struct passwd *pw, char *domain, char *user);
/* Get default logonserver for a domain. */
BOOL get_logon_server (const char * domain, char * server, WCHAR *wserver = NULL);

/* sec_helper.cc: Security helper functions. */
int set_process_privilege (const char *privilege, bool enable = true, bool use_thread = false);

/* shared.cc: */
/* Retrieve a security descriptor that allows all access */
SECURITY_DESCRIPTOR *__stdcall get_null_sd (void);

/* Various types of security attributes for use in Create* functions. */
extern SECURITY_ATTRIBUTES sec_none, sec_none_nih, sec_all, sec_all_nih;
extern SECURITY_ATTRIBUTES *__stdcall __sec_user (PVOID sa_buf, PSID sid2, BOOL inherit)
  __attribute__ ((regparm (3)));
extern BOOL sec_acl (PACL acl, BOOL admins, PSID sid1 = NO_SID, PSID sid2 = NO_SID);

int __stdcall NTReadEA (const char *file, const char *attrname, char *buf, int len);
BOOL __stdcall NTWriteEA (const char *file, const char *attrname, const char *buf, int len);
PSECURITY_DESCRIPTOR alloc_sd (__uid32_t uid, __gid32_t gid, int attribute,
	  PSECURITY_DESCRIPTOR sd_ret, DWORD *sd_size_ret);

extern inline SECURITY_ATTRIBUTES *
sec_user_nih (char sa_buf[], PSID sid = NULL)
{
  return allow_ntsec ? __sec_user (sa_buf, sid, FALSE) : &sec_none_nih;
}

extern inline SECURITY_ATTRIBUTES *
sec_user (char sa_buf[], PSID sid = NULL)
{
  return allow_ntsec ? __sec_user (sa_buf, sid, TRUE) : &sec_none;
}
#endif /*_SECURITY_H*/
