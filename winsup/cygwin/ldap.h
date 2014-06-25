/* ldap.h.

   Copyright 2014 Red Hat, Inc.

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

#define LDAP_USER_PGRP_ATTR		0
#define LDAP_USER_GECOS_ATTR		1
#define LDAP_USER_HOME_ATTR		2
#define LDAP_USER_SHELL_ATTR		3
#define LDAP_USER_UID_ATTR		4

#define LDAP_GROUP_NAME_ATTR		0
#define LDAP_GROUP_GID_ATTR		1

class cyg_ldap {
  PLDAP lh;
  PWCHAR rootdse;
  PLDAPMessage msg, entry;
  PWCHAR *val;
  PWCHAR *attr;
  bool isAD;
  PLDAPSearch srch_id;
  PLDAPMessage srch_msg, srch_entry;

  inline int map_ldaperr_to_errno (ULONG lerr);
  inline int wait (cygthread *thr);
  inline int connect (PCWSTR domain);
  inline int search (PWCHAR base, PWCHAR filter, PWCHAR *attrs);
  inline int next_page ();
  bool fetch_unix_sid_from_ad (uint32_t id, cygsid &sid, bool group);
  PWCHAR fetch_unix_name_from_rfc2307 (uint32_t id, bool group);
  PWCHAR get_string_attribute (int idx);
  uint32_t get_num_attribute (int idx);

public:
  cyg_ldap () : lh (NULL), rootdse (NULL), msg (NULL), entry (NULL), val (NULL),
		isAD (false), srch_id (NULL), srch_msg (NULL), srch_entry (NULL)
  {}
  ~cyg_ldap () { close (); }

  ULONG connect_ssl (PCWSTR domain);
  ULONG connect_non_ssl (PCWSTR domain);
  ULONG search_s (PWCHAR base, PWCHAR filter, PWCHAR *attrs);
  ULONG next_page_s ();

  operator PLDAP () const { return lh; }
  int open (PCWSTR in_domain);
  void close ();
  bool fetch_ad_account (PSID sid, bool group, PCWSTR domain = NULL);
  int enumerate_ad_accounts (PCWSTR domain, bool group);
  int next_account (cygsid &sid);
  uint32_t fetch_posix_offset_for_domain (PCWSTR domain);
  uid_t remap_uid (uid_t uid);
  gid_t remap_gid (gid_t gid);
  /* User only */
  gid_t get_primary_gid () { return get_num_attribute (LDAP_USER_PGRP_ATTR); }
  PWCHAR get_gecos () { return get_string_attribute (LDAP_USER_GECOS_ATTR); }
  PWCHAR get_home ()
	    { return get_string_attribute (LDAP_USER_HOME_ATTR); }
  PWCHAR get_shell () { return get_string_attribute (LDAP_USER_SHELL_ATTR); }
  gid_t get_unix_uid () { return get_num_attribute (LDAP_USER_UID_ATTR); }
  /* group only */
  PWCHAR get_group_name ()
	    { return get_string_attribute (LDAP_GROUP_NAME_ATTR); }
  gid_t get_unix_gid () { return get_num_attribute (LDAP_GROUP_GID_ATTR); }
};
