/* ldap.cc: Helper functions for ldap access to Active Directory.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <lm.h>
#include <dsgetdc.h>
#include <iptypes.h>
#include <sys/param.h>
#include "ldap.h"
#include "cygerrno.h"
#include "security.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include "registry.h"
#include "pinfo.h"
#include "tls_pbuf.h"

#define CYG_LDAP_ENUM_PAGESIZE	100	/* entries per page */

static PWCHAR rootdse_attr[] =
{
  (PWCHAR) L"defaultNamingContext",
  (PWCHAR) L"supportedCapabilities",
  NULL
};

static const PCWSTR std_user_attr[] =
{
  L"sAMAccountName",
  L"objectSid",
  L"primaryGroupID",
  L"uidNumber",
  L"profilePath",
  L"cygwinUnixUid",		/* TODO */
  /* windows scheme */
  L"displayName",
  L"homeDrive",
  L"homeDirectory",
  /* cygwin scheme */
  L"cygwinGecos",
  L"cygwinHome",
  L"cygwinShell",
  /* unix scheme */
  L"gecos",
  L"unixHomeDirectory",
  L"loginShell",
  /* desc scheme */
  L"description"
};

static PWCHAR group_attr[] =
{
  (PWCHAR) L"sAMAccountName",
  (PWCHAR) L"objectSid",
  (PWCHAR) L"gidNumber",
  (PWCHAR) L"cygwinUnixGid",	/* TODO */
  NULL
};

PWCHAR tdom_attr[] =
{
  (PWCHAR) L"trustPosixOffset",
  NULL
};

PWCHAR sid_attr[] =
{
  (PWCHAR) L"objectSid",
  NULL
};

PWCHAR rfc2307_uid_attr[] =
{
  (PWCHAR) L"uid",
  NULL
};

PWCHAR rfc2307_gid_attr[] =
{
  (PWCHAR) L"cn",
  NULL
};

/* ================================================================= */
/* Helper method of cygheap_pwdgrp class.  It sets the user attribs  */
/* from the settings in nsswitch.conf.				     */
/* ================================================================= */

#define user_attr	(cygheap->pg.ldap_user_attr)

void
cygheap_pwdgrp::init_ldap_user_attr ()
{
  ldap_user_attr = (PWCHAR *)
    ccalloc_abort (HEAP_BUF, sizeof (std_user_attr) / sizeof (*std_user_attr)
			     + 3 * NSS_SCHEME_MAX + 1, sizeof (PWCHAR));
  memcpy (ldap_user_attr, std_user_attr, sizeof (std_user_attr));
  uint16_t freeattr_idx = sizeof (std_user_attr) / sizeof (*std_user_attr);
  for (uint16_t idx = 0; idx < NSS_SCHEME_MAX; ++idx)
    {
      if (home_scheme[idx].method == NSS_SCHEME_FREEATTR)
	ldap_user_attr[freeattr_idx++] = home_scheme[idx].attrib;
      if (shell_scheme[idx].method == NSS_SCHEME_FREEATTR)
	ldap_user_attr[freeattr_idx++] = shell_scheme[idx].attrib;
      if (gecos_scheme[idx].method == NSS_SCHEME_FREEATTR)
	ldap_user_attr[freeattr_idx++] = gecos_scheme[idx].attrib;
    }
}

/* ================================================================= */
/* Helper methods.						     */
/* ================================================================= */

inline int
cyg_ldap::map_ldaperr_to_errno (ULONG lerr)
{
  switch (lerr)
    {
    case LDAP_SUCCESS:
      return NO_ERROR;
    case LDAP_NO_RESULTS_RETURNED:
      /* LdapMapErrorToWin32 maps LDAP_NO_RESULTS_RETURNED to ERROR_MORE_DATA,
	 which in turn is mapped to EMSGSIZE by geterrno_from_win_error.  This
	 is SO wrong, especially considering that LDAP_MORE_RESULTS_TO_RETURN
	 is mapped to ERROR_MORE_DATA as well :-P */
      return ENMFILE;
    default:
      break;
    }
  return geterrno_from_win_error (LdapMapErrorToWin32 (lerr));
}

inline int
cyg_ldap::wait (cygthread *thr)
{
  if (!thr)
    return EIO;
  if (cygwait (*thr, cw_infinite, cw_sig | cw_sig_restart) != WAIT_OBJECT_0)
    {
      thr->terminate_thread ();
      return EIO;
    }
  thr->detach ();
  return 0;
}

/* ================================================================= */
/* Helper struct and functions for interruptible LDAP initalization. */
/* ================================================================= */

struct cyg_ldap_init {
  cyg_ldap *that;
  PCWSTR domain;
  bool ssl;
  ULONG ret;
};

ULONG
cyg_ldap::connect_ssl (PCWSTR domain)
{
  ULONG ret;

  if (!(lh = ldap_sslinitW ((PWCHAR) domain, LDAP_SSL_PORT, 1)))
    {
      debug_printf ("ldap_init(%W) error 0x%02x", domain, LdapGetLastError ());
      return LdapGetLastError ();
    }
  if ((ret = ldap_bind_s (lh, NULL, NULL, LDAP_AUTH_NEGOTIATE)) != LDAP_SUCCESS)
    debug_printf ("ldap_bind(%W) 0x%02x", domain, ret);
  else if ((ret = ldap_search_sW (lh, NULL, LDAP_SCOPE_BASE,
				  (PWCHAR) L"(objectclass=*)", rootdse_attr,
				  0, &msg))
      != LDAP_SUCCESS)
    debug_printf ("ldap_search(%W, ROOTDSE) error 0x%02x", domain, ret);
  return ret;
}

ULONG
cyg_ldap::connect_non_ssl (PCWSTR domain)
{
  ULONG ret;

  if (!(lh = ldap_initW ((PWCHAR) domain, LDAP_PORT)))
    {
      debug_printf ("ldap_init(%W) error 0x%02x", domain, LdapGetLastError ());
      return LdapGetLastError ();
    }
  if ((ret = ldap_set_option (lh, LDAP_OPT_SIGN, LDAP_OPT_ON))
      != LDAP_SUCCESS)
    debug_printf ("ldap_set_option(LDAP_OPT_SIGN) error 0x%02x", ret);
  if ((ret = ldap_set_option (lh, LDAP_OPT_ENCRYPT, LDAP_OPT_ON))
      != LDAP_SUCCESS)
    debug_printf ("ldap_set_option(LDAP_OPT_ENCRYPT) error 0x%02x", ret);
  if ((ret = ldap_bind_s (lh, NULL, NULL, LDAP_AUTH_NEGOTIATE)) != LDAP_SUCCESS)
    debug_printf ("ldap_bind(%W) 0x%02x", domain, ret);
  else if ((ret = ldap_search_sW (lh, NULL, LDAP_SCOPE_BASE,
				  (PWCHAR) L"(objectclass=*)", rootdse_attr,
				  0, &msg))
      != LDAP_SUCCESS)
    debug_printf ("ldap_search(%W, ROOTDSE) error 0x%02x", domain, ret);
  return ret;
}

static DWORD WINAPI
ldap_init_thr (LPVOID param)
{
  cyg_ldap_init *cl = (cyg_ldap_init *) param;
  cl->ret = cl->ssl ? cl->that->connect_ssl (cl->domain)
		    : cl->that->connect_non_ssl (cl->domain);
  return 0;
}

inline int
cyg_ldap::connect (PCWSTR domain)
{
  /* FIXME?  connect_ssl can take ages even when failing, so we're trying to
     do everything the non-SSL (but still encrypted) way. */
  cyg_ldap_init cl = { this, domain, false, NO_ERROR };
  cygthread *thr = new cygthread (ldap_init_thr, &cl, "ldap_init");
  return wait (thr) ?: map_ldaperr_to_errno (cl.ret);
}

/* ================================================================= */
/* Helper struct and functions for interruptible LDAP search.        */
/* ================================================================= */

struct cyg_ldap_search {
  cyg_ldap *that;
  PWCHAR base;
  ULONG scope;
  PWCHAR filter;
  PWCHAR *attrs;
  ULONG ret;
};

ULONG
cyg_ldap::search_s (PWCHAR base, ULONG scope, PWCHAR filter, PWCHAR *attrs)
{
  ULONG ret;

  if ((ret = ldap_search_sW (lh, base, scope, filter, attrs, 0, &msg))
      != LDAP_SUCCESS)
    debug_printf ("ldap_search_sW(%W,%W) error 0x%02x", base, filter, ret);
  return ret;
}

static DWORD WINAPI
ldap_search_thr (LPVOID param)
{
  cyg_ldap_search *cl = (cyg_ldap_search *) param;
  cl->ret = cl->that->search_s (cl->base, cl->scope, cl->filter, cl->attrs);
  return 0;
}

inline int
cyg_ldap::search (PWCHAR base, ULONG scope, PWCHAR filter, PWCHAR *attrs)
{
  cyg_ldap_search cl = { this, base, scope, filter, attrs, NO_ERROR };
  cygthread *thr = new cygthread (ldap_search_thr, &cl, "ldap_search");
  return wait (thr) ?: map_ldaperr_to_errno (cl.ret);
}

/* ================================================================= */
/* Helper struct and functions for interruptible LDAP page search.        */
/* ================================================================= */

struct cyg_ldap_next_page {
  cyg_ldap *that;
  ULONG ret;
};

ULONG
cyg_ldap::next_page_s ()
{
  ULONG total;
  ULONG ret;

  do
    {
      ret = ldap_get_next_page_s (lh, srch_id, NULL, CYG_LDAP_ENUM_PAGESIZE,
				  &total, &msg);
    }
  while (ret == LDAP_SUCCESS && ldap_count_entries (lh, msg) == 0);
  if (ret && ret != LDAP_NO_RESULTS_RETURNED)
    debug_printf ("ldap_result() error 0x%02x", ret);
  return ret;
}

static DWORD WINAPI
ldap_next_page_thr (LPVOID param)
{
  cyg_ldap_next_page *cl = (cyg_ldap_next_page *) param;
  cl->ret = cl->that->next_page_s ();
  return 0;
}

inline int
cyg_ldap::next_page ()
{
  cyg_ldap_next_page cl = { this, NO_ERROR };
  cygthread *thr = new cygthread (ldap_next_page_thr, &cl, "ldap_next_page");
  return wait (thr) ?: map_ldaperr_to_errno (cl.ret);
}

/* ================================================================= */
/* Public methods.						     */
/* ================================================================= */

int
cyg_ldap::open (PCWSTR domain)
{
  int ret = NO_ERROR;

  /* Already open? */
  if (lh)
    return NO_ERROR;

  if ((ret = connect (domain)) != NO_ERROR)
    goto err;
  /* Prime `ret' and fetch ROOTDSE search result. */
  ret = EIO;
  if (!(entry = ldap_first_entry (lh, msg)))
    {
      debug_printf ("No ROOTDSE entry for %W", domain);
      goto err;
    }
  if (!(val = ldap_get_valuesW (lh, entry, rootdse_attr[0])))
    {
      debug_printf ("No %W value for %W", rootdse_attr[0], domain);
      goto err;
    }
  if (!(def_context = wcsdup (val[0])))
    {
      debug_printf ("wcsdup(%W, %W) %d", domain, rootdse_attr[0],
					 get_errno ());
      goto err;
    }
  ldap_value_freeW (val);
  if ((val = ldap_get_valuesW (lh, entry, rootdse_attr[1])))
    {
      for (ULONG idx = 0; idx < ldap_count_valuesW (val); ++idx)
	if (!wcscmp (val[idx], LDAP_CAP_ACTIVE_DIRECTORY_OID_W))
	  {
	    isAD = true;
	    break;
	  }
    }
  ldap_value_freeW (val);
  val = NULL;
  ldap_msgfree (msg);
  msg = entry = NULL;
  return NO_ERROR;
err:
  close ();
  return ret;
}

void
cyg_ldap::close ()
{
  if (srch_id != NULL)
    ldap_search_abandon_page (lh, srch_id);
  if (lh)
    ldap_unbind (lh);
  if (msg)
    ldap_msgfree (msg);
  if (val)
    ldap_value_freeW (val);
  if (def_context)
    free (def_context);
  lh = NULL;
  msg = entry = NULL;
  val = NULL;
  def_context = NULL;
  srch_id = NULL;
  last_fetched_sid = NO_SID;
}

PWCHAR
cyg_ldap::get_string_attribute (PCWSTR name)
{
  if (val)
    ldap_value_freeW (val);
  val = ldap_get_valuesW (lh, entry, (PWCHAR) name);
  if (val)
    return val[0];
  return NULL;
}

uint32_t
cyg_ldap::get_num_attribute (PCWSTR name)
{
  PWCHAR ret = get_string_attribute (name);
  if (ret)
    return (uint32_t) wcstoul (ret, NULL, 10);
  return (uint32_t) -1;
}

#define ACCOUNT_FILTER_START	L"(&(|(&(objectCategory=Person)" \
				       "(objectClass=User))" \
				     "(objectClass=Group))" \
				   "(objectSid="

#define ACCOUNT_FILTER_END	L"))"

bool
cyg_ldap::fetch_ad_account (PSID sid, bool group, PCWSTR domain)
{
  WCHAR filter[sizeof (ACCOUNT_FILTER_START) + sizeof (ACCOUNT_FILTER_END)
	       + 3 * SECURITY_MAX_SID_SIZE + 1];
  PWCHAR f, base = NULL;
  LONG len = (LONG) RtlLengthSid (sid);
  PBYTE s = (PBYTE) sid;
  static WCHAR hex_wchars[] = L"0123456789abcdef";
  tmp_pathbuf tp;

  if (last_fetched_sid == sid)
    return true;

  if (open (NULL) != NO_ERROR)
    return false;

  if (msg)
    {
      ldap_msgfree (msg);
      msg = entry = NULL;
    }
  if (val)
    {
      ldap_value_freeW (val);
      val = NULL;
    }
  f = wcpcpy (filter, ACCOUNT_FILTER_START);
  while (len-- > 0)
    {
      *f++ = L'\\';
      *f++ = hex_wchars[*s >> 4];
      *f++ = hex_wchars[*s++ & 0xf];
    }
  wcpcpy (f, ACCOUNT_FILTER_END);
  if (domain)
    {
      /* FIXME:  This is a hack.  The most correct solution is probably to
         open a connection to the DC of the trusted domain.  But this always
	 takes extra time, so we're trying to avoid it.  If this results in
	 problems, we know what to do. */
      base = tp.w_get ();
      PWCHAR b = base;
      for (PCWSTR dotp = domain; dotp && *dotp; domain = dotp)
	{
	  dotp = wcschr (domain, L'.');
	  if (b > base)
	    *b++ = L',';
	  b = wcpcpy (b, L"DC=");
	  b = dotp ? wcpncpy (b, domain, dotp++ - domain) : wcpcpy (b, domain);
	}
      debug_printf ("naming context <%W>", base);
    }
  else
    {
      /* def_context is only valid after open. */
      base = def_context;
    }
  if (!user_attr)
    cygheap->pg.init_ldap_user_attr ();
  attr = group ? group_attr : user_attr;
  if (search (base, LDAP_SCOPE_SUBTREE, filter, attr) != 0)
      return false;
  if (!(entry = ldap_first_entry (lh, msg)))
    {
      debug_printf ("No entry for %W in base %W", filter, base);
      return false;
    }
  last_fetched_sid = sid;
  return true;
}

int
cyg_ldap::enumerate_ad_accounts (PCWSTR domain, bool group)
{
  int ret;
  tmp_pathbuf tp;
  PCWSTR filter;

  close ();
  if ((ret = open (domain)) != NO_ERROR)
    return ret;

  if (!group)
    filter = L"(&(objectCategory=Person)"
		"(objectClass=User)"
		/* 512 == ADS_UF_NORMAL_ACCOUNT
		   Without checking this flag we'd enumerate undesired accounts
		   like, e.g., interdomain trusts. */
	        "(userAccountControl:" LDAP_MATCHING_RULE_BIT_AND ":=512)"
	        "(objectSid=*))";
  else if (!domain)
    /* From the local domain, we fetch well-known groups. */
    filter = L"(&(objectClass=Group)"
		"(objectSid=*))";
  else
    /* From foreign domains, we don't. */
    filter = L"(&(objectClass=Group)"
		/* 1 == BUILTIN_LOCAL_GROUP */
		"(!(groupType:" LDAP_MATCHING_RULE_BIT_AND ":=1))"
		"(objectSid=*))";
  if (!user_attr)
    cygheap->pg.init_ldap_user_attr ();
  attr = group ? group_attr : user_attr;
  srch_id = ldap_search_init_pageW (lh, def_context, LDAP_SCOPE_SUBTREE,
				    (PWCHAR) filter, attr, 0, NULL, NULL,
				    INFINITE, CYG_LDAP_ENUM_PAGESIZE, NULL);
  if (srch_id == NULL)
    {
      debug_printf ("ldap_search_init_pageW(%W,%W) error 0x%02x",
		    def_context, filter, LdapGetLastError ());
      return map_ldaperr_to_errno (LdapGetLastError ());
    }
  return NO_ERROR;
}

int
cyg_ldap::next_account (cygsid &sid)
{
  ULONG ret;
  PLDAP_BERVAL *bval;

  if (entry)
    {
      if ((entry = ldap_next_entry (lh, entry))
	  && (bval = ldap_get_values_lenW (lh, entry, (PWCHAR) L"objectSid")))
	{
	  last_fetched_sid = sid = (PSID) bval[0]->bv_val;
	  ldap_value_free_len (bval);
	  return NO_ERROR;
	}
      ldap_msgfree (msg);
      msg = entry = NULL;
    }
  ret = next_page ();
  if (ret == NO_ERROR)
    {
      if ((entry = ldap_first_entry (lh, msg))
	  && (bval = ldap_get_values_lenW (lh, entry, (PWCHAR) L"objectSid")))
	{
	  last_fetched_sid = sid = (PSID) bval[0]->bv_val;
	  ldap_value_free_len (bval);
	  return NO_ERROR;
	}
      ret = EIO;
    }
  ldap_search_abandon_page (lh, srch_id);
  srch_id = NULL;
  return ret;
}

#define SYSTEM_CONTAINER	L"CN=System,"

#define PSX_OFFSET_FILTER	L"(&(objectClass=trustedDomain)(name=%W))"
#define PSX_OFFSET_FILTER_FLAT	L"(&(objectClass=trustedDomain)(flatName=%W))"

/* Return UINT32_MAX on error to allow differing between not being able
   to fetch a value and a real 0 offset. */
uint32_t
cyg_ldap::fetch_posix_offset_for_domain (PCWSTR domain)
{
  WCHAR base[wcslen (def_context) + sizeof (SYSTEM_CONTAINER) / sizeof (WCHAR)];
  WCHAR filter[sizeof (PSX_OFFSET_FILTER_FLAT) + wcslen (domain) + 1];

  if (msg)
    {
      ldap_msgfree (msg);
      msg = entry = NULL;
    }
  if (val)
    {
      ldap_value_freeW (val);
      val = NULL;
    }
  /* As base, use system container within default naming context to restrict
     the search to this container only. */
  wcpcpy (wcpcpy (base, SYSTEM_CONTAINER), def_context);
  /* If domain name has no dot, it's a Netbios name.  In that case, filter
     by flatName rather than by name. */
  __small_swprintf (filter, wcschr (domain, L'.') ? PSX_OFFSET_FILTER
						  : PSX_OFFSET_FILTER_FLAT,
		    domain);
  if (search (base, LDAP_SCOPE_ONELEVEL, filter, attr = tdom_attr) != 0)
    return UINT32_MAX;
  if (!(entry = ldap_first_entry (lh, msg)))
    {
      debug_printf ("No entry for %W in def_context %W", filter, def_context);
      return UINT32_MAX;
    }
  return get_num_attribute (tdom_attr[0]);
}

#define UXID_FILTER_GRP L"(&(objectClass=Group)" \
			   "(gidNumber=%u))"

#define UXID_FILTER_USR L"(&(objectCategory=Person)" \
			   "(objectClass=User)" \
			   "(uidNumber=%u))"

bool
cyg_ldap::fetch_unix_sid_from_ad (uint32_t id, cygsid &sid, bool group)
{
  WCHAR filter[MAX (sizeof (UXID_FILTER_GRP), sizeof (UXID_FILTER_USR)) + 16];
  PLDAP_BERVAL *bval;

  if (msg)
    {
      ldap_msgfree (msg);
      msg = entry = NULL;
    }
  __small_swprintf (filter, group ? UXID_FILTER_GRP : UXID_FILTER_USR, id);
  if (search (def_context, LDAP_SCOPE_SUBTREE, filter, sid_attr) != 0)
    return false;
  if ((entry = ldap_first_entry (lh, msg))
      && (bval = ldap_get_values_lenW (lh, entry, (PWCHAR) L"objectSid")))
    {
      sid = (PSID) bval[0]->bv_val;
      ldap_value_free_len (bval);
      return true;
    }
  return false;
}

#define PSXID_FILTER_GRP L"(&(objectClass=posixGroup)" \
			    "(gidNumber=%u))"

#define PSXID_FILTER_USR L"(&(objectClass=posixAccount)" \
			    "(uidNumber=%u))"

PWCHAR
cyg_ldap::fetch_unix_name_from_rfc2307 (uint32_t id, bool group)
{
  WCHAR filter[MAX (sizeof (PSXID_FILTER_GRP), sizeof (PSXID_FILTER_USR)) + 16];

  if (msg)
    {
      ldap_msgfree (msg);
      msg = entry = NULL;
    }
  if (val)
    {
      ldap_value_freeW (val);
      val = NULL;
    }
  attr = group ? rfc2307_gid_attr : rfc2307_uid_attr;
  __small_swprintf (filter, group ? PSXID_FILTER_GRP : PSXID_FILTER_USR, id);
  if (search (def_context, LDAP_SCOPE_SUBTREE, filter, attr) != 0)
    return NULL;
  if (!(entry = ldap_first_entry (lh, msg)))
    {
      debug_printf ("No entry for %W in def_context %W", filter, def_context);
      return NULL;
    }
  return get_string_attribute (attr[0]);
}

uid_t
cyg_ldap::remap_uid (uid_t uid)
{
  cygsid user (NO_SID);
  PWCHAR name;
  struct passwd *pw;

  if (isAD)
    {
      if (fetch_unix_sid_from_ad (uid, user, false)
	  && user != NO_SID
	  && (pw = internal_getpwsid (user, this)))
	return pw->pw_uid;
    }
  else if ((name = fetch_unix_name_from_rfc2307 (uid, false)))
    {
      char *mbname = NULL;
      sys_wcstombs_alloc (&mbname, HEAP_NOTHEAP, name);
      if ((pw = internal_getpwnam (mbname)))
	return pw->pw_uid;
    }
  return ILLEGAL_UID;
}

gid_t
cyg_ldap::remap_gid (gid_t gid)
{
  cygsid group (NO_SID);
  PWCHAR name;
  struct group *gr;

  if (isAD)
    {
      if (fetch_unix_sid_from_ad (gid, group, true)
	  && group != NO_SID
	  && (gr = internal_getgrsid (group, this)))
	return gr->gr_gid;
    }
  else if ((name = fetch_unix_name_from_rfc2307 (gid, true)))
    {
      char *mbname = NULL;
      sys_wcstombs_alloc (&mbname, HEAP_NOTHEAP, name);
      if ((gr = internal_getgrnam (mbname)))
	return gr->gr_gid;
    }
  return ILLEGAL_GID;
}
