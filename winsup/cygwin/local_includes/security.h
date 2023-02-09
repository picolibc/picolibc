/* security.h: security declarations

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#pragma once

#include <accctrl.h>
#include <dsgetdc.h>

/* Special file attribute set in set_created_file_access to flag that a file
   has just been created.  Used in get_posix_access. */
#define S_JUSTCREATED	0x80000000

/* UID/GID */
void uinfo_init ();
bool check_token_membership (PSID);

#define ILLEGAL_UID ((uid_t)-1)
#define ILLEGAL_GID ((gid_t)-1)

/* Offset for accounts in the primary domain of the machine. */
#define PRIMARY_POSIX_OFFSET		(0x00100000)

/* Fake POSIX offsets used in scenarios in which the account has no permission
   to fetch the POSIX offset, or when the admins have set the offset to an
   unreasonable low value.  The values are chosen in the hope that they won't
   collide with "real" offsets. */
#define NOACCESS_POSIX_OFFSET		(0xfe500000)
#define UNUSABLE_POSIX_OFFSET		(0xfea00000)

/* For UNIX accounts not mapped to Windows accounts via winbind, Samba returns
   SIDs of the form S-1-22-x-y, with x == 1 for users and x == 2 for groups,
   and y == UNIX uid/gid.  NFS returns no SIDs at all, but the plain UNIX
   uid/gid values.

   UNIX uid/gid values are mapped to Cygwin uid/gid values 0xff000000 +
   unix uid/gid.  This *might* collide with a posix_offset of some trusted
   domain, but it's *very* unlikely.  Define the mapping as macro. */
#define UNIX_POSIX_OFFSET		(0xff000000)
#define UNIX_POSIX_MASK			(0x00ffffff)
#define MAP_UNIX_TO_CYGWIN_ID(id)	(UNIX_POSIX_OFFSET \
					 | ((id) & UNIX_POSIX_MASK))

#define MAX_DACL_LEN(n) (sizeof (ACL) \
		   + (n) * (sizeof (ACCESS_ALLOWED_ACE) - sizeof (DWORD) \
			    + SECURITY_MAX_SID_SIZE))
#define SD_MIN_SIZE (sizeof (SECURITY_DESCRIPTOR) + MAX_DACL_LEN (1))
#define ACL_MAXIMUM_SIZE 65532	/* Yeah, right.  64K - sizeof (DWORD). */
#define SD_MAXIMUM_SIZE 65536
#define NO_SID ((PSID)NULL)

#ifndef SE_CREATE_TOKEN_PRIVILEGE
#define SE_CREATE_TOKEN_PRIVILEGE            2U
#define SE_ASSIGNPRIMARYTOKEN_PRIVILEGE      3U
#define SE_LOCK_MEMORY_PRIVILEGE             4U
#define SE_INCREASE_QUOTA_PRIVILEGE          5U
#define SE_MACHINE_ACCOUNT_PRIVILEGE         6U
#define SE_TCB_PRIVILEGE                     7U
#define SE_SECURITY_PRIVILEGE                8U
#define SE_TAKE_OWNERSHIP_PRIVILEGE          9U
#define SE_LOAD_DRIVER_PRIVILEGE            10U
#define SE_SYSTEM_PROFILE_PRIVILEGE         11U
#define SE_SYSTEMTIME_PRIVILEGE             12U
#define SE_PROF_SINGLE_PROCESS_PRIVILEGE    13U
#define SE_INC_BASE_PRIORITY_PRIVILEGE      14U
#define SE_CREATE_PAGEFILE_PRIVILEGE        15U
#define SE_CREATE_PERMANENT_PRIVILEGE       16U
#define SE_BACKUP_PRIVILEGE                 17U
#define SE_RESTORE_PRIVILEGE                18U
#define SE_SHUTDOWN_PRIVILEGE               19U
#define SE_DEBUG_PRIVILEGE                  20U
#define SE_AUDIT_PRIVILEGE                  21U
#define SE_SYSTEM_ENVIRONMENT_PRIVILEGE     22U
#define SE_CHANGE_NOTIFY_PRIVILEGE          23U
#define SE_REMOTE_SHUTDOWN_PRIVILEGE        24U
#define SE_UNDOCK_PRIVILEGE                 25U
#define SE_SYNC_AGENT_PRIVILEGE             26U
#define SE_ENABLE_DELEGATION_PRIVILEGE      27U
#define SE_MANAGE_VOLUME_PRIVILEGE          28U
#define SE_IMPERSONATE_PRIVILEGE            29U
#define SE_CREATE_GLOBAL_PRIVILEGE          30U
#define SE_TRUSTED_CREDMAN_ACCESS_PRIVILEGE 31U
#define SE_RELABEL_PRIVILEGE                32U
#define SE_INCREASE_WORKING_SET_PRIVILEGE   33U
#define SE_TIME_ZONE_PRIVILEGE              34U
#define SE_CREATE_SYMBOLIC_LINK_PRIVILEGE   35U

#define SE_MAX_WELL_KNOWN_PRIVILEGE SE_CREATE_SYMBOLIC_LINK_PRIVILEGE

#endif /* ! SE_CREATE_TOKEN_PRIVILEGE */

/* Added for debugging purposes. */
typedef struct {
  BYTE  Revision;
  BYTE  SubAuthorityCount;
  SID_IDENTIFIER_AUTHORITY IdentifierAuthority;
  DWORD SubAuthority[SID_MAX_SUB_AUTHORITIES];
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

/* Convenience macros.  The Windows SID access functions are crude. */
#define sid_id_auth(s)		\
	(RtlIdentifierAuthoritySid (s)->Value[5])
#define sid_sub_auth_count(s)	\
	(*RtlSubAuthorityCountSid ((s)))
#define sid_sub_auth(s,i)	\
	(*RtlSubAuthoritySid ((s),(i)))
#define sid_sub_auth_rid(s)	\
	(*RtlSubAuthoritySid ((s), (*RtlSubAuthorityCountSid ((s)) - 1)))

#ifdef __cplusplus
extern "C"
{
#endif
  /* We need these declarations, otherwise g++ complains that the below
     inline methods use an undefined function, if ntdll.h isn't included. */
  BOOLEAN RtlEqualSid (PSID, PSID);
  NTSTATUS RtlCopySid (ULONG, PSID, PSID);
#ifdef __cplusplus
}
#endif

class cyg_ldap;

class cygpsid {
protected:
  PSID psid;
public:
  cygpsid () {}
  cygpsid (PSID nsid) { psid = nsid; }
  operator PSID () const { return psid; }
  const PSID operator= (PSID nsid) { return psid = nsid;}
  uid_t get_id (BOOL search_grp, int *type, cyg_ldap *pldap);
  int get_uid (cyg_ldap *pldap) { return get_id (FALSE, NULL, pldap); }
  int get_gid (cyg_ldap *pldap) { return get_id (TRUE, NULL, pldap); }

  PWCHAR pstring (PWCHAR nsidstr) const;
  PWCHAR string (PWCHAR nsidstr) const;
  char *pstring (char *nsidstr) const;
  char *string (char *nsidstr) const;

  bool operator== (const PSID nsid) const
    {
      if (!psid || !nsid)
	return nsid == psid;
      return RtlEqualSid (psid, nsid);
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
  char sbuf[SECURITY_MAX_SID_SIZE];
  bool well_known_sid;

  const PSID getfromstr (PCWSTR nsidstr, bool well_known);
  const PSID getfromstr (const char *nsidstr, bool well_known);
  PSID get_sid (DWORD s, DWORD cnt, DWORD *r, bool well_known);

  inline const PSID assign (const PSID nsid, bool well_known)
    {
      if (!nsid)
	psid = NO_SID;
      else
	{
	  psid = (PSID) sbuf;
	  RtlCopySid (SECURITY_MAX_SID_SIZE, psid, nsid);
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
  inline const PSID operator= (PCWSTR nsidstr)
    { return getfromstr (nsidstr, false); }
  inline const PSID operator= (const char *nsidstr)
    { return getfromstr (nsidstr, false); }
  inline const PSID operator*= (cygsid &nsid)
    { return assign (nsid, true); }
  inline const PSID operator*= (const PSID nsid)
    { return assign (nsid, true); }
  inline const PSID operator*= (PCWSTR nsidstr)
    { return getfromstr (nsidstr, true); }
  inline const PSID operator*= (const char *nsidstr)
    { return getfromstr (nsidstr, true); }

  inline cygsid () : cygpsid ((PSID) sbuf), well_known_sid (false) {}
  inline cygsid (const PSID nsid) { *this = nsid; }
  inline cygsid (const char *nstrsid) { *this = nstrsid; }
  inline cygsid (cygsid &nsid) { *this = nsid; }

  inline PSID set () { return psid = (PSID) sbuf; }

  inline BOOL getfrompw_gecos (const struct passwd *pw)
    {
      char *sp = (pw && pw->pw_gecos) ? strrchr (pw->pw_gecos, ',') : NULL;
      return (*this = sp ? sp + 1 : sp) != NO_SID;
    }
  inline BOOL getfromgr_passwd (const struct group *gr)
    {
      char *sp = (gr && gr->gr_passwd) ? gr->gr_passwd : NULL;
      return (*this = sp) != NO_SID;
    }

  const PSID create (DWORD auth, DWORD subauth_cnt, ...);
  bool append (DWORD rid);

  /* Implemented in pwdgrp.h. */
  BOOL getfrompw (const struct passwd *pw);
  BOOL getfromgr (const struct group *gr);

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

  BOOL addfromgr (struct group *gr) /* Only with alloc */
    { return sids[cnt].getfromgr (gr) && ++cnt; }

  /* += adds a "normal" SID, *= adds a well-known SID.  See comment in class
     cygsid above. */
  BOOL operator+= (cygsid &si) { return add ((PSID) si,
					     si.is_well_known_sid ()); }
  BOOL operator+= (const char *sidstr) { cygsid nsi (sidstr);
					 return add ((PSID) nsi,
						     nsi.is_well_known_sid ());
				       }
  BOOL operator+= (const PSID psid) { return add (psid, false); }
  BOOL operator*= (cygsid &si) { return add ((PSID) si, true); }
  BOOL operator*= (const char *sidstr) { cygsid nsi (sidstr);
					 return add ((PSID) nsi, true); }
  BOOL operator*= (const PSID psid) { return add (psid, true); }

  void count (int ncnt)
    { cnt = ncnt; }
  int count () const { return cnt; }
  int position (const PSID sid) const
    {
      for (int i = 0; i < cnt; ++i)
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
  inline void operator =(PSECURITY_DESCRIPTOR nsd) { psd = nsd; }
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
extern cygpsid well_known_console_logon_sid;
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
extern cygpsid well_known_local_service_sid;
extern cygpsid well_known_network_service_sid;
extern cygpsid well_known_builtin_sid;
extern cygpsid well_known_admins_sid;
extern cygpsid well_known_users_sid;
extern cygpsid trusted_installer_sid;
extern cygpsid mandatory_medium_integrity_sid;
extern cygpsid mandatory_high_integrity_sid;
extern cygpsid mandatory_system_integrity_sid;
extern cygpsid well_known_samba_unix_user_fake_sid;

bool privilege_luid (const PWCHAR pname, LUID &luid, bool &high_integrity);

extern inline BOOL
well_known_sid_type (SID_NAME_USE type)
{
  return type == SidTypeAlias || type == SidTypeWellKnownGroup;
}

extern inline BOOL
legal_sid_type (SID_NAME_USE type)
{
  return type == SidTypeUser  || type == SidTypeGroup
      || type == SidTypeAlias || type == SidTypeWellKnownGroup;
}

class path_conv;
/* File manipulation */
int get_file_attribute (HANDLE, path_conv &, mode_t &,
				  uid_t *, gid_t *);
int set_created_file_access (HANDLE, path_conv &, mode_t);
int get_object_sd (HANDLE, security_descriptor &);
int get_object_attribute (HANDLE, uid_t *, gid_t *, mode_t &);
int set_object_attribute (HANDLE, uid_t, gid_t, mode_t);
int create_object_sd_from_attribute (uid_t, gid_t, mode_t,
					    security_descriptor &);
int set_object_sd (HANDLE, security_descriptor &, bool);

int get_reg_attribute (HKEY hkey, mode_t &, uid_t *, gid_t *);
LONG get_file_sd (HANDLE fh, path_conv &, security_descriptor &, bool);
LONG set_file_sd (HANDLE fh, path_conv &, security_descriptor &, bool);
bool add_access_allowed_ace (PACL, DWORD, PSID, size_t &, DWORD);
bool add_access_denied_ace (PACL, DWORD, PSID, size_t &, DWORD);
int check_file_access (path_conv &, int, bool);
int check_registry_access (HANDLE, int, bool);

void set_security_attribute (path_conv &pc, int attribute,
			     PSECURITY_ATTRIBUTES psa,
			     security_descriptor &sd_buf);

bool authz_get_user_attribute (mode_t *attribute, PSECURITY_DESCRIPTOR psd,
			       PSID user_sid);

/* sec_acl.cc */
struct acl;
int searchace (struct acl *, int, int, uid_t id = ILLEGAL_UID);
PSECURITY_DESCRIPTOR set_posix_access (mode_t, uid_t, gid_t, struct acl *, int,
				       security_descriptor &, bool);
int get_posix_access (PSECURITY_DESCRIPTOR, mode_t &, uid_t *, gid_t *,
		      struct acl *, int, bool * = NULL);
int getacl (HANDLE, path_conv &, int, struct acl *);
int setacl (HANDLE, path_conv &, int, struct acl *, bool &);

/* Set impersonation or restricted token.  */
void set_imp_token (HANDLE token, int type);
/* LSA private key storage authentication, same as when using service logons. */
HANDLE lsaprivkeyauth (struct passwd *pw);
/* Kerberos or MsV1 S4U logon. */
HANDLE s4uauth (bool logon, PCWSTR domain, PCWSTR user, NTSTATUS &ret_status);
/* Verify an existing token */
bool verify_token (HANDLE token, cygsid &usersid, user_groups &groups, bool *pintern = NULL);
/* Get groups of a user */
enum acct_disabled_chk_t {
  NO_CHK_DISABLED = 0,
  CHK_DISABLED = 1
};

bool get_server_groups (cygsidlist &grp_list, PSID usersid,
			acct_disabled_chk_t check_account_disabled);

/* Extract U-domain\user field from passwd entry. */
void extract_nt_dom_user (const struct passwd *pw, PWCHAR domain, PWCHAR user);
/* Get default logonserver for a domain. */
bool get_logon_server (PCWSTR domain, PWCHAR wserver, ULONG flags);

/* Fetch user profile path from registry, if it already exists. */
PWCHAR get_user_profile_directory (PCWSTR sidstr, PWCHAR path, SIZE_T path_len);

/* Load user profile if it's not already loaded. */
HANDLE load_user_profile (HANDLE token, struct passwd *pw, cygpsid &sid);

HANDLE lsa_open_policy (PWCHAR server, ACCESS_MASK access);
void lsa_close_policy (HANDLE lsa);

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

/* Various types of security attributes for use in Create* functions. */
extern SECURITY_ATTRIBUTES sec_none, sec_none_nih, sec_all, sec_all_nih;
extern SECURITY_ATTRIBUTES *__sec_user (PVOID, PSID, PSID,
						  DWORD, BOOL);

extern PSECURITY_DESCRIPTOR _recycler_sd (void *buf, bool users, bool dir);
#define recycler_sd(users,dir) \
  (_recycler_sd (alloca (sizeof (SECURITY_DESCRIPTOR) + MAX_DACL_LEN (3)), \
		 (users), \
		 (dir)))

extern PSECURITY_DESCRIPTOR _everyone_sd (void *buf, ACCESS_MASK access);
#define everyone_sd(access)	(_everyone_sd (alloca (SD_MIN_SIZE), (access)))

#define sec_none_cloexec(f) (((f) & O_CLOEXEC ? &sec_none_nih : &sec_none))

extern bool sec_acl (PACL acl, bool original, bool admins, PSID sid1 = NO_SID,
		     PSID sid2 = NO_SID, DWORD access2 = 0);

ssize_t read_ea (HANDLE, path_conv &, const char *,
			   char *, size_t);
int write_ea (HANDLE, path_conv &, const char *, const char *,
			size_t, int);

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
