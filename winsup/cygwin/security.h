/* security.h: security declarations

   Copyright 2000, 2001 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#define DONT_INHERIT (0)
#define INHERIT_ALL  (CONTAINER_INHERIT_ACE|OBJECT_INHERIT_ACE)
#define INHERIT_ONLY (INHERIT_ONLY_ACE|CONTAINER_INHERIT_ACE|OBJECT_INHERIT_ACE)

#define DEFAULT_UID DOMAIN_USER_RID_ADMIN
#define DEFAULT_GID DOMAIN_ALIAS_RID_ADMINS

#define MAX_SID_LEN 40

#define NO_SID ((PSID)NULL)

class cygsid {
  PSID psid;
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
  inline cygsid () : psid ((PSID) sbuf) {}
  inline cygsid (const PSID nsid) { *this = nsid; }
  inline cygsid (const char *nstrsid) { *this = nstrsid; }

  inline PSID set () { return psid = (PSID) sbuf; }

  BOOL getfrompw (struct passwd *pw);
  BOOL getfromgr (struct group *gr);

  int get_id (BOOL search_grp, int *type = NULL);
  inline int get_uid () { return get_id (FALSE); }
  inline int get_gid () { return get_id (TRUE); }

  char *string (char *nsidstr) const;

  inline const PSID operator= (cygsid &nsid)
    { return assign (nsid); }
  inline const PSID operator= (const PSID nsid)
    { return assign (nsid); }
  inline const PSID operator= (const char *nsidstr)
    { return getfromstr (nsidstr); }

  inline BOOL operator== (const PSID nsid) const
    {
      if (!psid || !nsid)
        return nsid == psid;
      return EqualSid (psid, nsid);
    }
  inline BOOL operator== (const char *nsidstr) const
    {
      cygsid nsid (nsidstr);
      return *this == nsid;
    }
  inline BOOL operator!= (const PSID nsid) const
    { return !(*this == nsid); }
  inline BOOL operator!= (const char *nsidstr) const
    { return !(*this == nsidstr); }

  inline operator const PSID () { return psid; }

  void debug_print (const char *prefix = NULL) const
    {
      char buf[256];
      debug_printf ("%s %s", prefix ?: "", string (buf) ?: "NULL");
    }
};

class cygsidlist {
public:
  int count;
  cygsid *sids;

  cygsidlist () : count (0), sids (NULL) {}
  ~cygsidlist () { delete [] sids; }

  BOOL add (cygsid &nsi)
    {
      cygsid *tmp = new cygsid [count + 1];
      if (!tmp)
        return FALSE;
      for (int i = 0; i < count; ++i)
        tmp[i] = sids[i];
      delete [] sids;
      sids = tmp;
      sids[count++] = nsi;
      return TRUE;
    }
  BOOL add (const PSID nsid) { return add (nsid); }
  BOOL add (const char *sidstr)
    { cygsid nsi (sidstr); return add (nsi); }
  
  BOOL operator+= (cygsid &si) { return add (si); }
  BOOL operator+= (const char *sidstr) { return add (sidstr); }

  BOOL contains (cygsid &sid) const
    {
      for (int i = 0; i < count; ++i)
        if (sids[i] == sid)
	  return TRUE;
      return FALSE;
    }
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

extern cygsid well_known_world_sid;
extern cygsid well_known_local_sid;
extern cygsid well_known_creator_owner_sid;
extern cygsid well_known_dialup_sid;
extern cygsid well_known_network_sid;
extern cygsid well_known_batch_sid;
extern cygsid well_known_interactive_sid;
extern cygsid well_known_service_sid;
extern cygsid well_known_authenticated_users_sid;
extern cygsid well_known_system_sid;
extern cygsid well_known_admin_sid;

inline BOOL
legal_sid_type (SID_NAME_USE type)
{
  return type == SidTypeUser  || type == SidTypeGroup
      || type == SidTypeAlias || type == SidTypeWellKnownGroup;
}

extern BOOL allow_ntsec;
extern BOOL allow_smbntsec;

/* These both functions are needed to allow walking through the passwd
   and group lists so they are somehow security related. Besides that
   I didn't find a better place to declare them. */
extern struct passwd *internal_getpwent (int);
extern struct group *internal_getgrent (int);

/* File manipulation */
int __stdcall set_process_privileges ();
int __stdcall get_file_attribute (int, const char *, int *,
				  uid_t * = NULL, gid_t * = NULL);
int __stdcall set_file_attribute (int, const char *, int);
int __stdcall set_file_attribute (int, const char *, uid_t, gid_t, int, const char *);
LONG __stdcall read_sd(const char *file, PSECURITY_DESCRIPTOR sd_buf, LPDWORD sd_size);
LONG __stdcall write_sd(const char *file, PSECURITY_DESCRIPTOR sd_buf, DWORD sd_size);
BOOL __stdcall add_access_allowed_ace (PACL acl, int offset, DWORD attributes, PSID sid, size_t &len_add, DWORD inherit);
BOOL __stdcall add_access_denied_ace (PACL acl, int offset, DWORD attributes, PSID sid, size_t &len_add, DWORD inherit);

/* Try a subauthentication. */
HANDLE subauth (struct passwd *pw);
/* Try creating a token directly. */
HANDLE create_token (cygsid &usersid, cygsid &pgrpsid);

/* Extract U-domain\user field from passwd entry. */
void extract_nt_dom_user (const struct passwd *pw, char *domain, char *user);
/* Get default logonserver and domain for this box. */
BOOL get_logon_server_and_user_domain (char *logonserver, char *domain);

/* sec_helper.cc: Security helper functions. */
BOOL __stdcall is_grp_member (uid_t uid, gid_t gid);
/* `lookup_name' should be called instead of LookupAccountName.
 * logsrv may be NULL, in this case only the local system is used for lookup.
 * The buffer for ret_sid (40 Bytes) has to be allocated by the caller! */
BOOL __stdcall lookup_name (const char *, const char *, PSID);
int set_process_privilege (const char *privilege, BOOL enable = TRUE);

/* shared.cc: */
/* Retrieve a security descriptor that allows all access */
SECURITY_DESCRIPTOR *__stdcall get_null_sd (void);

/* Various types of security attributes for use in Create* functions. */
extern SECURITY_ATTRIBUTES sec_none, sec_none_nih, sec_all, sec_all_nih;
extern SECURITY_ATTRIBUTES *__stdcall sec_user (PVOID sa_buf, PSID sid2 = NULL, BOOL inherit = TRUE);
extern SECURITY_ATTRIBUTES *__stdcall sec_user_nih (PVOID sa_buf, PSID sid2 = NULL);

int __stdcall NTReadEA (const char *file, const char *attrname, char *buf, int len);
BOOL __stdcall NTWriteEA (const char *file, const char *attrname, char *buf, int len);
