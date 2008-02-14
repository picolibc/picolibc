/* security.h: security declarations

   Copyright 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007 Red Hat, Inc.

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

#ifndef SE_CREATE_TOKEN_PRIVILEGE
#define SE_CREATE_TOKEN_PRIVILEGE            2UL
#define SE_ASSIGNPRIMARYTOKEN_PRIVILEGE      3UL
#define SE_LOCK_MEMORY_PRIVILEGE             4UL
#define SE_INCREASE_QUOTA_PRIVILEGE          5UL
#define SE_MACHINE_ACCOUNT_PRIVILEGE         6UL
#define SE_TCB_PRIVILEGE                     7UL
#define SE_SECURITY_PRIVILEGE                8UL
#define SE_TAKE_OWNERSHIP_PRIVILEGE          9UL
#define SE_LOAD_DRIVER_PRIVILEGE            10UL
#define SE_SYSTEM_PROFILE_PRIVILEGE         11UL
#define SE_SYSTEMTIME_PRIVILEGE             12UL
#define SE_PROF_SINGLE_PROCESS_PRIVILEGE    13UL
#define SE_INC_BASE_PRIORITY_PRIVILEGE      14UL
#define SE_CREATE_PAGEFILE_PRIVILEGE        15UL
#define SE_CREATE_PERMANENT_PRIVILEGE       16UL
#define SE_BACKUP_PRIVILEGE                 17UL
#define SE_RESTORE_PRIVILEGE                18UL
#define SE_SHUTDOWN_PRIVILEGE               19UL
#define SE_DEBUG_PRIVILEGE                  20UL
#define SE_AUDIT_PRIVILEGE                  21UL
#define SE_SYSTEM_ENVIRONMENT_PRIVILEGE     22UL
#define SE_CHANGE_NOTIFY_PRIVILEGE          23UL
#define SE_REMOTE_SHUTDOWN_PRIVILEGE        24UL
/* Starting with Windows 2000 */
#define SE_UNDOCK_PRIVILEGE                 25UL
#define SE_SYNC_AGENT_PRIVILEGE             26UL
#define SE_ENABLE_DELEGATION_PRIVILEGE      27UL
#define SE_MANAGE_VOLUME_PRIVILEGE          28UL
/* Starting with Windows 2000 SP4, XP SP2, 2003 Server */
#define SE_IMPERSONATE_PRIVILEGE            29UL
#define SE_CREATE_GLOBAL_PRIVILEGE          30UL
/* Starting with Vista */
#define SE_TRUSTED_CREDMAN_ACCESS_PRIVILEGE 31UL
#define SE_RELABEL_PRIVILEGE                32UL
#define SE_INCREASE_WORKING_SET_PRIVILEGE   33UL
#define SE_TIME_ZONE_PRIVILEGE              34UL
#define SE_CREATE_SYMBOLIC_LINK_PRIVILEGE   35UL

#define SE_MAX_WELL_KNOWN_PRIVILEGE SE_CREATE_SYMBOLIC_LINK_PRIVILEGE

#endif /* ! SE_CREATE_TOKEN_PRIVILEGE */

/* Added for debugging purposes. */
typedef struct {
  BYTE  Revision;
  BYTE  SubAuthorityCount;
  SID_IDENTIFIER_AUTHORITY IdentifierAuthority;
  DWORD SubAuthority[8];
} DBGSID, *PDBGSID;

/* Macro to define variable length SID structures */
#define MKSID(name, comment, authority, count, rid...) \
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
  operator PSID () const { return psid; }
  const PSID operator= (PSID nsid) { return psid = nsid;}
  __uid32_t get_id (BOOL search_grp, int *type = NULL);
  int get_uid () { return get_id (FALSE); }
  int get_gid () { return get_id (TRUE); }

  PWCHAR string (PWCHAR nsidstr) const;
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
  bool well_known_sid;

  const PSID getfromstr (const char *nsidstr, bool well_known);
  PSID get_sid (DWORD s, DWORD cnt, DWORD *r, bool well_known);

  inline const PSID assign (const PSID nsid, bool well_known)
    {
      if (!nsid)
	psid = NO_SID;
      else
	{
	  psid = (PSID) sbuf;
	  CopySid (MAX_SID_LEN, psid, nsid);
	  well_known_sid = well_known;
	}
      return psid;
    }

public:
  inline operator const PSID () { return psid; }
  inline bool is_well_known_sid () { return well_known_sid; }

  /* Both, = and *= are assignment operators.  = creates a "normal" SID,
     *= marks the SID as being a well-known SID.  This difference is
     important when creating a SID list for LSA authentication. */
  inline const PSID operator= (cygsid &nsid)
    { return assign (nsid, nsid.well_known_sid); }
  inline const PSID operator= (const PSID nsid)
    { return assign (nsid, false); }
  inline const PSID operator= (const char *nsidstr)
    { return getfromstr (nsidstr, false); }
  inline const PSID operator*= (cygsid &nsid)
    { return assign (nsid, true); }
  inline const PSID operator*= (const PSID nsid)
    { return assign (nsid, true); }
  inline const PSID operator*= (const char *nsidstr)
    { return getfromstr (nsidstr, true); }

  inline cygsid () : cygpsid ((PSID) sbuf), well_known_sid (false) {}
  inline cygsid (const PSID nsid) { *this = nsid; }
  inline cygsid (const char *nstrsid) { *this = nstrsid; }

  inline PSID set () { return psid = (PSID) sbuf; }

  BOOL getfrompw (const struct passwd *pw);
  BOOL getfromgr (const struct __group32 *gr);

  void debug_print (const char *prefix = NULL) const
    {
      char buf[256] __attribute__ ((unused));
      debug_printf ("%s %s%s", prefix ?: "", string (buf) ?: "NULL", well_known_sid ? " (*)" : " (+)");
    }
};

typedef enum { cygsidlist_empty, cygsidlist_alloc, cygsidlist_auto } cygsidlist_type;
class cygsidlist {
  int maxcnt;
  int cnt;

  BOOL add (const PSID nsi, bool well_known); /* Only with auto for now */

public:
  cygsid *sids;
  cygsidlist_type type;

  cygsidlist (cygsidlist_type t, int m)
  : maxcnt (m), cnt (0), type (t)
    {
      if (t == cygsidlist_alloc)
	sids = alloc_sids (m);
      else
	sids = new cygsid [m];
    }
  ~cygsidlist () { if (type == cygsidlist_auto) delete [] sids; }

  BOOL addfromgr (struct __group32 *gr) /* Only with alloc */
    { return sids[cnt].getfromgr (gr) && ++cnt; }

  /* += adds a "normal" SID, *= adds a well-known SID.  See comment in class
     cygsid above. */
  BOOL operator+= (cygsid &si) { return add ((PSID) si, false); }
  BOOL operator+= (const char *sidstr) { cygsid nsi (sidstr);
  					 return add ((PSID) nsi, false); }
  BOOL operator+= (const PSID psid) { return add (psid, false); }
  BOOL operator*= (cygsid &si) { return add ((PSID) si, true); }
  BOOL operator*= (const char *sidstr) { cygsid nsi (sidstr);
  					 return add ((PSID) nsi, true); }
  BOOL operator*= (const PSID psid) { return add (psid, true); }

  void count (int ncnt)
    { cnt = ncnt; }
  int count () const { return cnt; }
  int non_well_known_count () const
    {
      int wcnt = 0;
      for (int i = 0; i < cnt; ++i)
        if (!sids[i].is_well_known_sid ())
	  ++wcnt;
      return wcnt;
    }

  int position (const PSID sid) const
    {
      for (int i = 0; i < cnt; ++i)
	if (sids[i] == sid)
	  return i;
      return -1;
    }

  int next_non_well_known_sid (int idx)
    {
      while (++idx < cnt)
        if (!sids[idx].is_well_known_sid ())
	  return idx;
      return -1;
    }
  BOOL contains (const PSID sid) const { return position (sid) >= 0; }
  cygsid *alloc_sids (int n);
  void free_sids ();
  void debug_print (const char *prefix = NULL) const
    {
      debug_printf ("-- begin sidlist ---");
      if (!cnt)
	debug_printf ("No elements");
      for (int i = 0; i < cnt; ++i)
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
  inline DWORD copy (void *buf, DWORD buf_size) const {
    if (buf_size < size ())
      return sd_size;
    memcpy (buf, psd, sd_size);
    return 0;
  }
  inline operator const PSECURITY_DESCRIPTOR () { return psd; }
  inline operator PSECURITY_DESCRIPTOR *() { return &psd; }
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
extern cygpsid well_known_this_org_sid;
extern cygpsid well_known_system_sid;
extern cygpsid well_known_admins_sid;
extern cygpsid fake_logon_sid;

bool privilege_luid (const char *pname, LUID *luid);

inline BOOL
legal_sid_type (SID_NAME_USE type)
{
  return type == SidTypeUser  || type == SidTypeGroup
      || type == SidTypeAlias || type == SidTypeWellKnownGroup;
}

extern bool allow_ntsec;
extern bool allow_smbntsec;

/* File manipulation */
int __stdcall get_file_attribute (HANDLE, path_conv &, mode_t *,
				  __uid32_t *, __gid32_t *);
int __stdcall set_file_attribute (HANDLE, path_conv &,
				  __uid32_t, __gid32_t, int);
int __stdcall get_reg_attribute (HKEY hkey, mode_t *, __uid32_t *, __gid32_t *);
LONG __stdcall get_file_sd (HANDLE fh, path_conv &, security_descriptor &sd);
LONG __stdcall set_file_sd (HANDLE fh, path_conv &, security_descriptor &sd);
bool __stdcall add_access_allowed_ace (PACL acl, int offset, DWORD attributes, PSID sid, size_t &len_add, DWORD inherit);
bool __stdcall add_access_denied_ace (PACL acl, int offset, DWORD attributes, PSID sid, size_t &len_add, DWORD inherit);
int __stdcall check_file_access (path_conv &, int);
int __stdcall check_registry_access (HANDLE, int);

void set_security_attribute (int attribute, PSECURITY_ATTRIBUTES psa,
			     security_descriptor &sd_buf);

bool get_sids_info (cygpsid, cygpsid, __uid32_t * , __gid32_t *);

/* sec_acl.cc */
struct __acl32;
extern "C" int aclsort32 (int, int, __acl32 *);
extern "C" int acl32 (const char *, int, int, __acl32 *);
int getacl (HANDLE, path_conv &, int, __acl32 *);
int setacl (HANDLE, path_conv &, int, __acl32 *, bool &);

struct _UNICODE_STRING;
void __stdcall str2buf2uni (_UNICODE_STRING &, WCHAR *, const char *) __attribute__ ((regparm (3)));
void __stdcall str2uni_cat (_UNICODE_STRING &, const char *) __attribute__ ((regparm (2)));

/* Function creating a token by calling NtCreateToken. */
HANDLE create_token (cygsid &usersid, user_groups &groups, struct passwd * pw);
/* LSA authentication function. */
HANDLE lsaauth (cygsid &, user_groups &, struct passwd *);
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
int set_privilege (HANDLE token, DWORD privilege, bool enable);
void set_cygwin_privileges (HANDLE token);

#define _push_thread_privilege(_priv, _val, _check) { \
    HANDLE _dup_token = NULL; \
    HANDLE _token = (cygheap->user.issetuid () && (_check)) \
		    ? cygheap->user.primary_token () : hProcToken; \
    if (!DuplicateTokenEx (_token, MAXIMUM_ALLOWED, NULL, \
			   SecurityImpersonation, TokenImpersonation, \
			   &_dup_token)) \
      debug_printf ("DuplicateTokenEx: %E"); \
    else if (!ImpersonateLoggedOnUser (_dup_token)) \
      debug_printf ("ImpersonateLoggedOnUser: %E"); \
    else \
      set_privilege (_dup_token, (_priv), (_val));

#define push_thread_privilege(_priv, _val) _push_thread_privilege(_priv,_val,1)
#define push_self_privilege(_priv, _val)   _push_thread_privilege(_priv,_val,0)

#define pop_thread_privilege() \
    if (_dup_token) \
      { \
	if (!cygheap->user.issetuid ()) \
	  RevertToSelf (); \
	else \
	  cygheap->user.reimpersonate (); \
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

ssize_t __stdcall read_ea (HANDLE hdl, path_conv &pc, const char *name,
			   char *value, size_t size);
int __stdcall write_ea (HANDLE hdl, path_conv &pc, const char *name,
			const char *value, size_t size, int flags);

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
