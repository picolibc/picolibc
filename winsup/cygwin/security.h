/* security.h: security declarations

   Copyright 2000, 2001 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#define DONT_INHERIT (0)
#define INHERIT_ALL  (CONTAINER_INHERIT_ACE|OBJECT_INHERIT_ACE)
#define INHERIT_ONLY (INHERIT_ONLY_ACE|CONTAINER_INHERIT_ACE|OBJECT_INHERIT_ACE)

extern BOOL allow_ntsec;
extern BOOL allow_smbntsec;

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


/* sec_helper.cc: Security helper functions. */
char *__stdcall convert_sid_to_string_sid (PSID psid, char *sid_str);
PSID __stdcall convert_string_sid_to_sid (PSID psid, const char *sid_str);
PSID __stdcall get_sid (PSID psid, DWORD s, DWORD cnt, DWORD *r);
BOOL __stdcall get_pw_sid (PSID sid, struct passwd *pw);
BOOL __stdcall get_gr_sid (PSID sid, struct group *gr);
PSID __stdcall get_admin_sid ();
PSID __stdcall get_system_sid ();
PSID __stdcall get_creator_owner_sid ();
PSID __stdcall get_world_sid ();
int get_id_from_sid (PSID psid, BOOL search_grp, int *type);
int __stdcall get_id_from_sid (PSID psid, BOOL search_grp);
BOOL __stdcall legal_sid_type (SID_NAME_USE type);
BOOL __stdcall is_grp_member (uid_t uid, gid_t gid);
/* `lookup_name' should be called instead of LookupAccountName.
 * logsrv may be NULL, in this case only the local system is used for lookup.
 * The buffer for ret_sid (40 Bytes) has to be allocated by the caller! */
BOOL __stdcall lookup_name (const char *, const char *, PSID);
int set_process_privilege (const char *privilege, BOOL enable = TRUE);

extern inline int get_uid_from_sid (PSID psid) { return get_id_from_sid (psid, FALSE);}
extern inline int get_gid_from_sid (PSID psid) { return get_id_from_sid (psid, TRUE); }

/* shared.cc: */
/* Retrieve a security descriptor that allows all access */
SECURITY_DESCRIPTOR *__stdcall get_null_sd (void);

/* Various types of security attributes for use in Create* functions. */
extern SECURITY_ATTRIBUTES sec_none, sec_none_nih, sec_all, sec_all_nih;
extern SECURITY_ATTRIBUTES *__stdcall sec_user (PVOID sa_buf, PSID sid2 = NULL, BOOL inherit = TRUE);
extern SECURITY_ATTRIBUTES *__stdcall sec_user_nih (PVOID sa_buf, PSID sid2 = NULL);

int __stdcall NTReadEA (const char *file, const char *attrname, char *buf, int len);
BOOL __stdcall NTWriteEA (const char *file, const char *attrname, char *buf, int len);
