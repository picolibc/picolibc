/* security.h: security declarations

   Copyright 2000 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

/* File manipulation */
int __stdcall set_process_privileges ();
int __stdcall get_file_attribute (int, const char *, int *,
				  uid_t * = NULL, gid_t * = NULL);
int __stdcall set_file_attribute (int, const char *, int);
int __stdcall set_file_attribute (int, const char *, uid_t, gid_t, int, const char *);
extern BOOL allow_ntsec;

/* `lookup_name' should be called instead of LookupAccountName.
 * logsrv may be NULL, in this case only the local system is used for lookup.
 * The buffer for ret_sid (40 Bytes) has to be allocated by the caller! */
BOOL __stdcall lookup_name (const char *, const char *, PSID);
char *__stdcall convert_sid_to_string_sid (PSID, char *);
PSID __stdcall convert_string_sid_to_sid (PSID, const char *);
BOOL __stdcall get_pw_sid (PSID, struct passwd *);

/* Retrieve a security descriptor that allows all access */
SECURITY_DESCRIPTOR *__stdcall get_null_sd (void);

int __stdcall get_id_from_sid (PSID, BOOL);
extern inline int get_uid_from_sid (PSID psid) { return get_id_from_sid (psid, FALSE);}
extern inline int get_gid_from_sid (PSID psid) { return get_id_from_sid (psid, TRUE); }

/* Various types of security attributes for use in Create* functions. */
extern SECURITY_ATTRIBUTES sec_none, sec_none_nih, sec_all, sec_all_nih;
extern SECURITY_ATTRIBUTES *__stdcall sec_user (PVOID sa_buf, PSID sid2 = NULL, BOOL inherit = TRUE);
extern SECURITY_ATTRIBUTES *__stdcall sec_user_nih (PVOID sa_buf, PSID sid2 = NULL);

int __stdcall NTReadEA (const char *file, const char *attrname, char *buf, int len);
BOOL __stdcall NTWriteEA (const char *file, const char *attrname, char *buf, int len);
