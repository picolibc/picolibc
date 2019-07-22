/* ldap.h.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#pragma once

#pragma push_macro ("DECLSPEC_IMPORT")
#undef DECLSPEC_IMPORT
#define DECLSPEC_IMPORT
#include <winldap.h>
#include <ntldap.h>
#pragma pop_macro ("DECLSPEC_IMPORT")

class cyg_ldap {
  PLDAP lh;
  PWCHAR def_context;
  PLDAPMessage msg, entry;
  PWCHAR *val;
  PWCHAR *attr;
  bool isAD;
  PLDAPSearch srch_id;
  cygsid last_fetched_sid;

  inline int map_ldaperr_to_errno (ULONG lerr);
  inline int wait (cygthread *thr);
  inline int connect (PCWSTR domain);
  inline int search (PWCHAR base, ULONG scope, PWCHAR filter, PWCHAR *attrs);
  inline int next_page ();
  bool fetch_unix_sid_from_ad (uint32_t id, cygsid &sid, bool group);
  PWCHAR fetch_unix_name_from_rfc2307 (uint32_t id, bool group);

public:
  cyg_ldap () : lh (NULL), def_context (NULL), msg (NULL), entry (NULL),
		val (NULL), isAD (false), srch_id (NULL), 
		last_fetched_sid (NO_SID)
  {}
  ~cyg_ldap () { close (); }

  ULONG connect_ssl (PCWSTR domain);
  ULONG connect_non_ssl (PCWSTR domain);
  ULONG search_s (PWCHAR base, ULONG scope, PWCHAR filter, PWCHAR *attrs);
  ULONG next_page_s ();

  bool is_open () const { return !!lh; }
  operator PLDAP () const { return lh; }
  int open (PCWSTR in_domain);
  void close ();
  PWCHAR get_string_attribute (PCWSTR name);
  uint32_t get_num_attribute (PCWSTR name);
  bool fetch_ad_account (PSID sid, bool group, PCWSTR domain = NULL);
  int enumerate_ad_accounts (PCWSTR domain, bool group);
  int next_account (cygsid &sid);
  uint32_t fetch_posix_offset_for_domain (PCWSTR domain);
  uid_t remap_uid (uid_t uid);
  gid_t remap_gid (gid_t gid);
  /* User only */
  gid_t get_primary_gid () { return get_num_attribute (L"primaryGroupID"); }
  gid_t get_unix_uid () { return get_num_attribute (L"uidNumber"); }
  PWCHAR get_account_name ()
	    { return get_string_attribute (L"sAMAccountName"); }
  gid_t get_unix_gid () { return get_num_attribute (L"gidNumber"); }
  PWCHAR get_profile_path ()
	    { return get_string_attribute (L"profilePath"); }
};
