/* security.h: security declarations

   Copyright 2000, 2001, 2002, 2003, 2004, 2005 Red Hat, Inc.

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
#define ACL_DEFAULT_SIZE 3072
#define NO_SID ((PSID)NULL)

/* Macro to define variable length SID structures */
#define SID(name, comment, authority, count, rid...) \
static NO_COPY struct  { \
  BYTE  Revision; \
  BYTE  SubAuthorityCount; \
  SID_IDENTIFIER_AUTHORITY IdentifierAuthority; \
  DWORD SubAuthority[count]; \
} name##_struct = { SID_REVISION, count, {authority}, {rid}}; \
cygpsid NO_COPY name = (PSID) &name##_struct;

#define FILE_READ_BITS   (FILE_READ_DATA | GENERIC_READ | GENERIC_ALL)
#define FILE_WRITE_BITS  (FILE_WRITE_DATA | GENERIC_WRITE | GENERIC_ALL)
#define FILE_EXEC_BITS   (FILE_EXECUTE | GENERIC_EXECUTE | GENERIC_ALL)

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
      char buf[256] __attribute__ ((unused));
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
    { return sids[count].getfromgr (gr) && ++count; }

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

/* Wrapper class to allow simple deleting of buffer space allocated
   by read_sd() */
class security_descriptor {
protected:
  PSECURITY_DESCRIPTOR psd;
  DWORD sd_size;
public:
  security_descriptor () : psd (NULL), sd_size (0) {}
  ~security_descriptor () { free (); }

  PSECURITY_DESCRIPTOR malloc (size_t nsize);
  PSECURITY_DESCRIPTOR realloc (size_t nsize);
  void free ();

  inline DWORD size () const { return sd_size; }
  inline operator const PSECURITY_DESCRIPTOR () { return psd; }
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

extern cygpsid well_known_null_sid;
extern cygpsid well_known_world_sid;
extern cygpsid well_known_local_sid;
extern cygpsid well_known_creator_owner_sid;
extern cygpsid well_known_creator_group_sid;
extern cygpsid well_known_dialup_sid;
extern cygpsid well_known_network_sid;
extern cygpsid well_known_batch_sid;
extern cygpsid well_known_interactive_sid;
extern cygpsid well_known_service_sid;
extern cygpsid well_known_authenticated_users_sid;
extern cygpsid well_known_system_sid;
extern cygpsid well_known_admins_sid;

/* Order must be same as cygpriv in sec_helper.cc. */
enum cygpriv_idx {
  SE_CREATE_TOKEN_PRIV = 0,
  SE_ASSIGNPRIMARYTOKEN_PRIV,
  SE_LOCK_MEMORY_PRIV,
  SE_INCREASE_QUOTA_PRIV,
  SE_UNSOLICITED_INPUT_PRIV,
  SE_MACHINE_ACCOUNT_PRIV,
  SE_TCB_PRIV,
  SE_SECURITY_PRIV,
  SE_TAKE_OWNERSHIP_PRIV,
  SE_LOAD_DRIVER_PRIV,
  SE_SYSTEM_PROFILE_PRIV,
  SE_SYSTEMTIME_PRIV,
  SE_PROF_SINGLE_PROCESS_PRIV,
  SE_INC_BASE_PRIORITY_PRIV,
  SE_CREATE_PAGEFILE_PRIV,
  SE_CREATE_PERMANENT_PRIV,
  SE_BACKUP_PRIV,
  SE_RESTORE_PRIV,
  SE_SHUTDOWN_PRIV,
  SE_DEBUG_PRIV,
  SE_AUDIT_PRIV,
  SE_SYSTEM_ENVIRONMENT_PRIV,
  SE_CHANGE_NOTIFY_PRIV,
  SE_REMOTE_SHUTDOWN_PRIV,
  SE_CREATE_GLOBAL_PRIV,
  SE_UNDOCK_PRIV,
  SE_MANAGE_VOLUME_PRIV,
  SE_IMPERSONATE_PRIV,
  SE_ENABLE_DELEGATION_PRIV,
  SE_SYNC_AGENT_PRIV,

  SE_NUM_PRIVS
};

const LUID *privilege_luid (enum cygpriv_idx idx);
const LUID *privilege_luid_by_name (const char *pname);
const char *privilege_name (enum cygpriv_idx idx);

inline BOOL
legal_sid_type (SID_NAME_USE type)
{
  return type == SidTypeUser  || type == SidTypeGroup
      || type == SidTypeAlias || type == SidTypeWellKnownGroup;
}

extern bool allow_ntea;
extern bool allow_ntsec;
extern bool allow_smbntsec;
extern bool allow_traverse;

/* File manipulation */
int __stdcall get_file_attribute (int, HANDLE, const char *, mode_t *,
				  __uid32_t * = NULL, __gid32_t * = NULL);
int __stdcall set_file_attribute (bool, HANDLE, const char *, int);
int __stdcall set_file_attribute (bool, HANDLE, const char *, __uid32_t, __gid32_t, int);
int __stdcall get_nt_object_security (HANDLE, SE_OBJECT_TYPE,
				      security_descriptor &);
int __stdcall get_object_attribute (HANDLE handle, SE_OBJECT_TYPE object_type, mode_t *,
				  __uid32_t * = NULL, __gid32_t * = NULL);
LONG __stdcall read_sd (const char *file, security_descriptor &sd);
LONG __stdcall write_sd (HANDLE fh, const char *file, security_descriptor &sd);
bool __stdcall add_access_allowed_ace (PACL acl, int offset, DWORD attributes, PSID sid, size_t &len_add, DWORD inherit);
bool __stdcall add_access_denied_ace (PACL acl, int offset, DWORD attributes, PSID sid, size_t &len_add, DWORD inherit);
int __stdcall check_file_access (const char *, int);

void set_security_attribute (int attribute, PSECURITY_ATTRIBUTES psa,
			     security_descriptor &sd_buf);

bool get_sids_info (cygpsid, cygpsid, __uid32_t * , __gid32_t *);

/* sec_acl.cc */
struct __acl32;
extern "C" int aclsort32 (int, int, __acl32 *);
extern "C" int acl32 (const char *, int, int, __acl32 *);
int getacl (HANDLE, const char *, DWORD, int, __acl32 *);
int setacl (HANDLE, const char *, int, __acl32 *);

struct _UNICODE_STRING;
void __stdcall str2buf2uni (_UNICODE_STRING &, WCHAR *, const char *) __attribute__ ((regparm (3)));
void __stdcall str2uni_cat (_UNICODE_STRING &, const char *) __attribute__ ((regparm (2)));

/* Try a subauthentication. */
HANDLE subauth (struct passwd *pw);
/* Try creating a token directly. */
HANDLE create_token (cygsid &usersid, user_groups &groups, struct passwd * pw,
		     HANDLE subauth_token);
/* Verify an existing token */
bool verify_token (HANDLE token, cygsid &usersid, user_groups &groups, bool *pintern = NULL);
/* Get groups of a user */
bool get_server_groups (cygsidlist &grp_list, PSID usersid, struct passwd *pw);

/* Extract U-domain\user field from passwd entry. */
void extract_nt_dom_user (const struct passwd *pw, char *domain, char *user);
/* Get default logonserver for a domain. */
bool get_logon_server (const char * domain, char * server, WCHAR *wserver,
		       bool rediscovery);

/* sec_helper.cc: Security helper functions. */
int set_privilege (HANDLE token, enum cygpriv_idx privilege, bool enable);
void set_cygwin_privileges (HANDLE token);

#define set_process_privilege(p,v) set_privilege (hProcImpToken, (p), (v))

#define _push_thread_privilege(_priv, _val, _check) { \
    HANDLE _token = NULL, _dup_token = NULL; \
    if (wincap.has_security ()) \
      { \
	_token = (cygheap->user.issetuid () && (_check)) \
		 ? cygheap->user.token () : hProcImpToken; \
	if (!DuplicateTokenEx (_token, MAXIMUM_ALLOWED, NULL, \
			       SecurityImpersonation, TokenImpersonation, \
			       &_dup_token)) \
	  debug_printf ("DuplicateTokenEx: %E"); \
	else if (!ImpersonateLoggedOnUser (_dup_token)) \
	  debug_printf ("ImpersonateLoggedOnUser: %E"); \
	else \
	  set_privilege (_dup_token, (_priv), (_val)); \
      }
#define push_thread_privilege(_priv, _val) _push_thread_privilege(_priv,_val,1)
#define push_self_privilege(_priv, _val)   _push_thread_privilege(_priv,_val,0)

#define pop_thread_privilege() \
    if (_dup_token) \
      { \
	ImpersonateLoggedOnUser (_token); \
	CloseHandle (_dup_token); \
      } \
  }
#define pop_self_privilege()		   pop_thread_privilege()

/* shared.cc: */
/* Retrieve a security descriptor that allows all access */
SECURITY_DESCRIPTOR *__stdcall get_null_sd ();

/* Various types of security attributes for use in Create* functions. */
extern SECURITY_ATTRIBUTES sec_none, sec_none_nih, sec_all, sec_all_nih;
extern SECURITY_ATTRIBUTES *__stdcall __sec_user (PVOID sa_buf, PSID sid1, PSID sid2,
						  DWORD access2, BOOL inherit)
  __attribute__ ((regparm (3)));
extern bool sec_acl (PACL acl, bool original, bool admins, PSID sid1 = NO_SID,
		     PSID sid2 = NO_SID, DWORD access2 = 0);

int __stdcall read_ea (HANDLE hdl, const char *file, const char *attrname,
		       char *buf, int len);
BOOL __stdcall write_ea (HANDLE hdl, const char *file, const char *attrname,
			 const char *buf, int len);

/* Note: sid1 is usually (read: currently always) the current user's
   effective sid (cygheap->user.sid ()). */
extern inline SECURITY_ATTRIBUTES *
sec_user_nih (SECURITY_ATTRIBUTES *sa_buf, PSID sid1, PSID sid2 = NULL,
	      DWORD access2 = 0)
{
  return __sec_user (sa_buf, sid1, sid2, access2, FALSE);
}

extern inline SECURITY_ATTRIBUTES *
sec_user (SECURITY_ATTRIBUTES *sa_buf, PSID sid1, PSID sid2 = NULL,
	  DWORD access2 = 0)
{
  return __sec_user (sa_buf, sid1, sid2, access2, TRUE);
}
#endif /*_SECURITY_H*/
