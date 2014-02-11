/* uinfo.cc: user info (uid, gid, etc...)

   Copyright 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006,
   2007, 2008, 2009, 2010, 2011, 2012, 2014 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <iptypes.h>
#include <lm.h>
#include <ntsecapi.h>
#include <wininet.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <wchar.h>
#include <sys/cygwin.h>
#include "cygerrno.h"
#include "pinfo.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include "shared_info.h"
#include "registry.h"
#include "child_info.h"
#include "environ.h"
#include "tls_pbuf.h"
#include "miscfuncs.h"
#include "ntdll.h"

#include "ldap.h"

/* Initialize the part of cygheap_user that does not depend on files.
   The information is used in shared.cc for the user shared.
   Final initialization occurs in uinfo_init */
void
cygheap_user::init ()
{
  WCHAR user_name[UNLEN + 1];
  DWORD user_name_len = UNLEN + 1;

  /* This code is only run if a Cygwin process gets started by a native
     Win32 process.  We try to get the username from the environment,
     first USERNAME (Win32), then USER (POSIX).  If that fails (which is
     very unlikely), it only has an impact if we don't have an entry in
     /etc/passwd for this user either.  In that case the username sticks
     to "unknown".  Since this is called early in initialization, and
     since we don't want pull in a dependency to any other DLL except
     ntdll and kernel32 at this early stage, don't call GetUserName,
     GetUserNameEx, NetWkstaUserGetInfo, etc. */
  if (GetEnvironmentVariableW (L"USERNAME", user_name, user_name_len)
      || GetEnvironmentVariableW (L"USER", user_name, user_name_len))
    {
      char mb_user_name[user_name_len = sys_wcstombs (NULL, 0, user_name)];
      sys_wcstombs (mb_user_name, user_name_len, user_name);
      set_name (mb_user_name);
    }
  else
    set_name ("unknown");

  NTSTATUS status;
  ULONG size;
  PSECURITY_DESCRIPTOR psd;

  status = NtQueryInformationToken (hProcToken, TokenPrimaryGroup,
				    &groups.pgsid, sizeof (cygsid), &size);
  if (!NT_SUCCESS (status))
    system_printf ("NtQueryInformationToken (TokenPrimaryGroup), %y", status);

  /* Get the SID from current process and store it in effec_cygsid */
  status = NtQueryInformationToken (hProcToken, TokenUser, &effec_cygsid,
				    sizeof (cygsid), &size);
  if (!NT_SUCCESS (status))
    {
      system_printf ("NtQueryInformationToken (TokenUser), %y", status);
      return;
    }

  /* Set token owner to the same value as token user */
  status = NtSetInformationToken (hProcToken, TokenOwner, &effec_cygsid,
				  sizeof (cygsid));
  if (!NT_SUCCESS (status))
    debug_printf ("NtSetInformationToken(TokenOwner), %y", status);

  /* Standard way to build a security descriptor with the usual DACL */
  PSECURITY_ATTRIBUTES sa_buf = (PSECURITY_ATTRIBUTES) alloca (1024);
  psd = (PSECURITY_DESCRIPTOR)
		(sec_user_nih (sa_buf, sid()))->lpSecurityDescriptor;

  BOOLEAN acl_exists, dummy;
  TOKEN_DEFAULT_DACL dacl;

  status = RtlGetDaclSecurityDescriptor (psd, &acl_exists, &dacl.DefaultDacl,
					 &dummy);
  if (NT_SUCCESS (status) && acl_exists && dacl.DefaultDacl)
    {

      /* Set the default DACL and the process DACL */
      status = NtSetInformationToken (hProcToken, TokenDefaultDacl, &dacl,
				      sizeof (dacl));
      if (!NT_SUCCESS (status))
	system_printf ("NtSetInformationToken (TokenDefaultDacl), %y", status);
      if ((status = NtSetSecurityObject (NtCurrentProcess (),
					 DACL_SECURITY_INFORMATION, psd)))
	system_printf ("NtSetSecurityObject, %y", status);
    }
  else
    system_printf("Cannot get dacl, %E");
}

void
internal_getlogin (cygheap_user &user)
{
  struct passwd *pw = NULL;
  struct group *gr, *gr2;

  cygpsid psid = user.sid ();
  pw = internal_getpwsid (psid);

  if (!pw && !(pw = internal_getpwnam (user.name ())))
    debug_printf ("user not found in /etc/passwd");
  else
    {
      cygsid gsid;

      myself->uid = pw->pw_uid;
      myself->gid = pw->pw_gid;
      user.set_name (pw->pw_name);
      if (gsid.getfromgr (gr = internal_getgrgid (pw->pw_gid)))
	{
	  /* We might have a group file with a group entry for the current
	     user's primary group, but the current user has no entry in passwd.
	     If so, pw_gid is taken from windows and might disagree with the
	     gr_gid from the group file.  Overwrite it brutally. */
	  if ((gr2 = internal_getgrsid (gsid)) && gr2 != gr)
	    myself->gid = pw->pw_gid = gr2->gr_gid;
	  /* Set primary group to the group in /etc/passwd. */
	  if (gsid != user.groups.pgsid)
	    {
	      NTSTATUS status = NtSetInformationToken (hProcToken,
						       TokenPrimaryGroup,
						       &gsid, sizeof gsid);
	      if (!NT_SUCCESS (status))
		debug_printf ("NtSetInformationToken (TokenPrimaryGroup), %y",
			      status);
	      else
		user.groups.pgsid = gsid;
	      clear_procimptoken ();
	    }
	}
      else
	debug_printf ("gsid not found in augmented /etc/group");
    }
  cygheap->user.ontherange (CH_HOME, pw);
}

void
uinfo_init ()
{
  if (child_proc_info && !cygheap->user.has_impersonation_tokens ())
    return;

  if (!child_proc_info)
    internal_getlogin (cygheap->user); /* Set the cygheap->user. */
  /* Conditions must match those in spawn to allow starting child
     processes with ruid != euid and rgid != egid. */
  else if (cygheap->user.issetuid ()
	   && cygheap->user.saved_uid == cygheap->user.real_uid
	   && cygheap->user.saved_gid == cygheap->user.real_gid
	   && !cygheap->user.groups.issetgroups ()
	   && !cygheap->user.setuid_to_restricted)
    {
      cygheap->user.reimpersonate ();
      return;
    }
  else
    cygheap->user.close_impersonation_tokens ();

  cygheap->user.saved_uid = cygheap->user.real_uid = myself->uid;
  cygheap->user.saved_gid = cygheap->user.real_gid = myself->gid;
  cygheap->user.external_token = NO_IMPERSONATION;
  cygheap->user.internal_token = NO_IMPERSONATION;
  cygheap->user.curr_primary_token = NO_IMPERSONATION;
  cygheap->user.curr_imp_token = NO_IMPERSONATION;
  cygheap->user.ext_token_is_restricted = false;
  cygheap->user.curr_token_is_restricted = false;
  cygheap->user.setuid_to_restricted = false;
  cygheap->user.set_saved_sid ();	/* Update the original sid */
  cygheap->user.deimpersonate ();
}

extern "C" int
getlogin_r (char *name, size_t namesize)
{
  const char *login = cygheap->user.name ();
  size_t len = strlen (login) + 1;
  if (len > namesize)
    return ERANGE;
  myfault efault;
  if (efault.faulted ())
    return EFAULT;
  strncpy (name, login, len);
  return 0;
}

extern "C" char *
getlogin (void)
{
  static char username[UNLEN];
  int ret = getlogin_r (username, UNLEN);
  if (ret)
    {
      set_errno (ret);
      return NULL;
    }
  return username;
}

extern "C" uid_t
getuid32 (void)
{
  return cygheap->user.real_uid;
}

#ifdef __x86_64__
EXPORT_ALIAS (getuid32, getuid)
#else
extern "C" __uid16_t
getuid (void)
{
  return cygheap->user.real_uid;
}
#endif

extern "C" gid_t
getgid32 (void)
{
  return cygheap->user.real_gid;
}

#ifdef __x86_64__
EXPORT_ALIAS (getgid32, getgid)
#else
extern "C" __gid16_t
getgid (void)
{
  return cygheap->user.real_gid;
}
#endif

extern "C" uid_t
geteuid32 (void)
{
  return myself->uid;
}

#ifdef __x86_64__
EXPORT_ALIAS (geteuid32, geteuid)
#else
extern "C" uid_t
geteuid (void)
{
  return myself->uid;
}
#endif

extern "C" gid_t
getegid32 (void)
{
  return myself->gid;
}

#ifdef __x86_64__
EXPORT_ALIAS (getegid32, getegid)
#else
extern "C" __gid16_t
getegid (void)
{
  return myself->gid;
}
#endif

/* Not quite right - cuserid can change, getlogin can't */
extern "C" char *
cuserid (char *src)
{
  if (!src)
    return getlogin ();

  strcpy (src, getlogin ());
  return src;
}

const char *
cygheap_user::ontherange (homebodies what, struct passwd *pw)
{
  LPUSER_INFO_3 ui = NULL;
  WCHAR wuser[UNLEN + 1];
  NET_API_STATUS ret;
  char homedrive_env_buf[3];
  char *newhomedrive = NULL;
  char *newhomepath = NULL;
  tmp_pathbuf tp;

  debug_printf ("what %d, pw %p", what, pw);
  if (what == CH_HOME)
    {
      char *p;

      if ((p = getenv ("HOME")))
	debug_printf ("HOME is already in the environment %s", p);
      else
	{
	  if (pw && pw->pw_dir && *pw->pw_dir)
	    {
	      debug_printf ("Set HOME (from /etc/passwd) to %s", pw->pw_dir);
	      setenv ("HOME", pw->pw_dir, 1);
	    }
	  else
	    {
	      char home[strlen (name ()) + 8];

	      debug_printf ("Set HOME to default /home/USER");
	      __small_sprintf (home, "/home/%s", name ());
	      setenv ("HOME", home, 1);
	    }
	}
    }

  if (what != CH_HOME && homepath == NULL && newhomepath == NULL)
    {
      char *homepath_env_buf = tp.c_get ();
      if (!pw)
	pw = internal_getpwnam (name ());
      if (pw && pw->pw_dir && *pw->pw_dir)
	cygwin_conv_path (CCP_POSIX_TO_WIN_A, pw->pw_dir, homepath_env_buf,
			  NT_MAX_PATH);
      else
	{
	  homepath_env_buf[0] = homepath_env_buf[1] = '\0';
	  if (logsrv ())
	    {
	      WCHAR wlogsrv[INTERNET_MAX_HOST_NAME_LENGTH + 3];
	      sys_mbstowcs (wlogsrv, sizeof (wlogsrv) / sizeof (*wlogsrv),
			    logsrv ());
	     sys_mbstowcs (wuser, sizeof (wuser) / sizeof (*wuser), winname ());
	      if (!(ret = NetUserGetInfo (wlogsrv, wuser, 3, (LPBYTE *) &ui)))
		{
		  sys_wcstombs (homepath_env_buf, NT_MAX_PATH,
				ui->usri3_home_dir);
		  if (!homepath_env_buf[0])
		    {
		      sys_wcstombs (homepath_env_buf, NT_MAX_PATH,
				    ui->usri3_home_dir_drive);
		      if (homepath_env_buf[0])
			strcat (homepath_env_buf, "\\");
		      else
			cygwin_conv_path (CCP_POSIX_TO_WIN_A | CCP_ABSOLUTE,
					  "/", homepath_env_buf, NT_MAX_PATH);
		    }
		}
	    }
	  if (ui)
	    NetApiBufferFree (ui);
	}

      if (homepath_env_buf[1] != ':')
	{
	  newhomedrive = almost_null;
	  newhomepath = homepath_env_buf;
	}
      else
	{
	  homedrive_env_buf[0] = homepath_env_buf[0];
	  homedrive_env_buf[1] = homepath_env_buf[1];
	  homedrive_env_buf[2] = '\0';
	  newhomedrive = homedrive_env_buf;
	  newhomepath = homepath_env_buf + 2;
	}
    }

  if (newhomedrive && newhomedrive != homedrive)
    cfree_and_set (homedrive, (newhomedrive == almost_null)
			      ? almost_null : cstrdup (newhomedrive));

  if (newhomepath && newhomepath != homepath)
    cfree_and_set (homepath, cstrdup (newhomepath));

  switch (what)
    {
    case CH_HOMEDRIVE:
      return homedrive;
    case CH_HOMEPATH:
      return homepath;
    default:
      return homepath;
    }
}

const char *
cygheap_user::test_uid (char *&what, const char *name, size_t namelen)
{
  if (!what && !issetuid ())
    what = getwinenveq (name, namelen, HEAP_STR);
  return what;
}

const char *
cygheap_user::env_logsrv (const char *name, size_t namelen)
{
  if (test_uid (plogsrv, name, namelen))
    return plogsrv;

  const char *mydomain = domain ();
  const char *myname = winname ();
  if (!mydomain || ascii_strcasematch (myname, "SYSTEM"))
    return almost_null;

  WCHAR wdomain[MAX_DOMAIN_NAME_LEN + 1];
  WCHAR wlogsrv[INTERNET_MAX_HOST_NAME_LENGTH + 3];
  sys_mbstowcs (wdomain, MAX_DOMAIN_NAME_LEN + 1, mydomain);
  cfree_and_set (plogsrv, almost_null);
  if (get_logon_server (wdomain, wlogsrv, DS_IS_FLAT_NAME))
    sys_wcstombs_alloc (&plogsrv, HEAP_STR, wlogsrv);
  return plogsrv;
}

const char *
cygheap_user::env_domain (const char *name, size_t namelen)
{
  if (pwinname && test_uid (pdomain, name, namelen))
    return pdomain;

  DWORD ulen = UNLEN + 1;
  WCHAR username[ulen];
  DWORD dlen = MAX_DOMAIN_NAME_LEN + 1;
  WCHAR userdomain[dlen];
  SID_NAME_USE use;

  cfree_and_set (pwinname, almost_null);
  cfree_and_set (pdomain, almost_null);
  if (!LookupAccountSidW (NULL, sid (), username, &ulen,
			  userdomain, &dlen, &use))
    __seterrno ();
  else
    {
      sys_wcstombs_alloc (&pwinname, HEAP_STR, username);
      sys_wcstombs_alloc (&pdomain, HEAP_STR, userdomain);
    }
  return pdomain;
}

const char *
cygheap_user::env_userprofile (const char *name, size_t namelen)
{
  if (test_uid (puserprof, name, namelen))
    return puserprof;

  /* User hive path is never longer than MAX_PATH. */
  WCHAR userprofile_env_buf[MAX_PATH];
  WCHAR win_id[UNLEN + 1]; /* Large enough for SID */

  cfree_and_set (puserprof, almost_null);
  if (get_registry_hive_path (get_windows_id (win_id), userprofile_env_buf))
    sys_wcstombs_alloc (&puserprof, HEAP_STR, userprofile_env_buf);

  return puserprof;
}

const char *
cygheap_user::env_homepath (const char *name, size_t namelen)
{
  return ontherange (CH_HOMEPATH);
}

const char *
cygheap_user::env_homedrive (const char *name, size_t namelen)
{
  return ontherange (CH_HOMEDRIVE);
}

const char *
cygheap_user::env_name (const char *name, size_t namelen)
{
  if (!test_uid (pwinname, name, namelen))
    domain ();
  return pwinname;
}

const char *
cygheap_user::env_systemroot (const char *name, size_t namelen)
{
  if (!psystemroot)
    {
      int size = GetSystemWindowsDirectoryW (NULL, 0);
      if (size > 0)
	{
	  WCHAR wsystemroot[size];
	  size = GetSystemWindowsDirectoryW (wsystemroot, size);
	  if (size > 0)
	    sys_wcstombs_alloc (&psystemroot, HEAP_STR, wsystemroot);
	}
      if (size <= 0)
	debug_printf ("GetSystemWindowsDirectoryW(), %E");
    }
  return psystemroot;
}

char *
pwdgrp::next_str (char c)
{
  char *res = lptr;
  lptr = strchrnul (lptr, c);
  if (*lptr)
    *lptr++ = '\0';
  return res;
}

bool
pwdgrp::next_num (unsigned long& n)
{
  char *p = next_str (':');
  char *cp;
  n = strtoul (p, &cp, 10);
  return p != cp && !*cp;
}

char *
pwdgrp::add_line (char *eptr)
{
  if (eptr)
    {
      if (curr_lines >= max_lines)
	{
	  max_lines += 10;
	  pwdgrp_buf = crealloc_abort (pwdgrp_buf,
				       max_lines * pwdgrp_buf_elem_size);
	}
      lptr = eptr;
      if ((this->*parse) ())
	curr_lines++;
    }
  return eptr;
}

void
cygheap_pwdgrp::init ()
{
  pwd_cache.file.init_pwd ();
  pwd_cache.win.init_pwd ();
  grp_cache.file.init_grp ();
  grp_cache.win.init_grp ();
  /* Default settings:

     passwd: files db
     group:  files db
     db_prefix: auto
     db_cache: yes
     db_separator: +
  */
  pwd_src = (NSS_FILES | NSS_DB);
  grp_src = (NSS_FILES | NSS_DB);
  prefix = NSS_AUTO;
  separator[0] = L'+';
  caching = true;
}

/* The /etc/nssswitch.conf file is read exactly once by the root process of a
   process tree.  We can't afford methodical changes during the lifetime of a 
   process tree. */
void
cygheap_pwdgrp::nss_init_line (const char *line)
{
  const char *c = line + strspn (line, " \t");
  switch (*c)
    {
    case 'p':
    case 'g':
      {
	int *src = NULL;
	if (!strncmp (c, "passwd:", 7))
	  {
	    src = &pwd_src;
	    c += 7;
	  }
	else if (!strncmp (c, "group:", 6))
	  {
	    src = &grp_src;
	    c += 6;
	  }
	if (src)
	  {
	    *src = 0;
	    while (*c)
	      {
		c += strspn (c, " \t");
		if (!*c || *c == '#')
		  break;
		if (!strncmp (c, "files", 5) && strchr (" \t", c[5]))
		  {
		    *src |= NSS_FILES;
		    c += 5;
		  }
		else if (!strncmp (c, "db", 2) && strchr (" \t", c[2]))
		  {
		    *src |= NSS_DB;
		    c += 2;
		  }
		else
		  {
		    c += strcspn (c, " \t");
		    debug_printf ("Invalid nsswitch.conf content: %s", line);
		  }
	      }
	    if (*src == 0)
	      *src = (NSS_FILES | NSS_DB);
	  }
      }
      break;
    case 'd':
      if (strncmp (c, "db_", 3))
	{
	  debug_printf ("Invalid nsswitch.conf content: %s", line);
	  break;
	}
      c += 3;
      if (!strncmp (c, "prefix:", 7))
	{
	  c += 7;
	  c += strspn (c, " \t");
	  if (!strncmp (c, "auto", 4) && strchr (" \t", c[4]))
	    prefix = NSS_AUTO;
	  else if (!strncmp (c, "primary", 7) && strchr (" \t", c[7]))
	    prefix = NSS_PRIMARY;
	  else if (!strncmp (c, "always", 6) && strchr (" \t", c[6]))
	    prefix = NSS_ALWAYS;
	  else
	    debug_printf ("Invalid nsswitch.conf content: %s", line);
	}
      else if (!strncmp (c, "separator:", 10))
	{
	  c += 10;
	  c += strspn (c, " \t");
	  if ((unsigned char) *c <= 0x7f && strchr (" \t", c[1]))
	    separator[0] = (unsigned char) *c;
	  else
	    debug_printf ("Invalid nsswitch.conf content: %s", line);
	}
      else if (!strncmp (c, "cache:", 6))
	{
	  c += 6;
	  c += strspn (c, " \t");
	  if (!strncmp (c, "yes", 3) && strchr (" \t", c[3]))
	    caching = true;
	  else if (!strncmp (c, "no", 2) && strchr (" \t", c[2]))
	    caching = false;
	  else
	    debug_printf ("Invalid nsswitch.conf content: %s", line);
	}
      break;
    case '\0':
    case '#':
      break;
    default:
      debug_printf ("Invalid nsswitch.conf content: %s", line);
      break;
    }
}

void
cygheap_pwdgrp::_nss_init ()
{
  UNICODE_STRING path;
  OBJECT_ATTRIBUTES attr;
  NT_readline rl;
  tmp_pathbuf tp;
  char *buf = tp.c_get ();

  PCWSTR rel_path = L"\\etc\\nsswitch.conf";
  path.Buffer = (PWCHAR) alloca ((wcslen (cygheap->installation_root)
				  + wcslen (rel_path) + 1) * sizeof (WCHAR));
  wcpcpy (wcpcpy (path.Buffer, cygheap->installation_root), rel_path);
  RtlInitUnicodeString (&path, path.Buffer);
  InitializeObjectAttributes (&attr, &path, OBJ_CASE_INSENSITIVE,
			      NULL, NULL);
  if (rl.init (&attr, buf, NT_MAX_PATH))
    while ((buf = rl.gets ()))
      nss_init_line (buf);
  nss_inited = true;
}

/* Override the ParentIndex value of the PDS_DOMAIN_TRUSTSW entry with the
   PosixOffset. */
#define PosixOffset ParentIndex

bool
cygheap_domain_info::init ()
{
  HANDLE lsa;
  NTSTATUS status;
  ULONG ret;
  /* We *have* to copy the information.  Apart from our wish to have the
     stuff in the cygheap, even when not calling LsaFreeMemory on the result,
     the data will be overwritten later.  From what I gather, the information
     is, in fact, stored on the stack. */
  PPOLICY_DNS_DOMAIN_INFO pdom;
  PPOLICY_ACCOUNT_DOMAIN_INFO adom;
  PDS_DOMAIN_TRUSTSW td;
  ULONG tdom_cnt;

  if (adom_name)
    return true;
  lsa = lsa_open_policy (NULL, POLICY_VIEW_LOCAL_INFORMATION);
  if (!lsa)
    {
      system_printf ("lsa_open_policy(NULL) failed");
      return false;
    }
  /* Fetch primary domain information from local LSA. */
  status = LsaQueryInformationPolicy (lsa, PolicyDnsDomainInformation,
				      (PVOID *) &pdom);
  if (status != STATUS_SUCCESS)
    {
      system_printf ("LsaQueryInformationPolicy(Primary) %y", status);
      return false;
    }
  /* Copy primary domain info to cygheap. */
  pdom_name = cwcsdup (pdom->Name.Buffer);
  pdom_dns_name = pdom->DnsDomainName.Length
		  ? cwcsdup (pdom->DnsDomainName.Buffer) : NULL;
  pdom_sid = pdom->Sid;
  LsaFreeMemory (pdom);
  /* Fetch account domain information from local LSA. */
  status = LsaQueryInformationPolicy (lsa, PolicyAccountDomainInformation,
				      (PVOID *) &adom);
  if (status != STATUS_SUCCESS)
    {
      system_printf ("LsaQueryInformationPolicy(Account) %y", status);
      return false;
    }
  /* Copy account domain info to cygheap.  If we're running on a DC the account
     domain is identical to the primary domain.  This leads to confusion when
     trying to compute the uid/gid values.  Therefore we invalidate the account
     domain name if we're running on a DC. */
  adom_sid = adom->DomainSid;
  adom_name = cwcsdup (pdom_sid == adom_sid ? L"@" : adom->DomainName.Buffer);
  LsaFreeMemory (adom);
  lsa_close_policy (lsa);
  if (cygheap->dom.member_machine ())
    {
      /* For a domain member machine fetch all trusted domain info. */
      lowest_tdo_posix_offset = UNIX_POSIX_OFFSET - 1;
      ret = DsEnumerateDomainTrustsW (NULL, DS_DOMAIN_DIRECT_INBOUND
					    | DS_DOMAIN_DIRECT_OUTBOUND
					    | DS_DOMAIN_IN_FOREST,
				      &td, &tdom_cnt);
      if (ret != ERROR_SUCCESS)
	{
	  SetLastError (ret);
	  debug_printf ("DsEnumerateDomainTrusts: %E");
	  return true;
	}
      if (tdom_cnt == 0)
	{
	  return true;
	}
      /* Copy trusted domain info to cygheap, setting PosixOffset on the fly. */
      tdom = (PDS_DOMAIN_TRUSTSW)
	cmalloc_abort (HEAP_BUF, tdom_cnt * sizeof (DS_DOMAIN_TRUSTSW));
      memcpy (tdom, td, tdom_cnt * sizeof (DS_DOMAIN_TRUSTSW));
      for (ULONG idx = 0; idx < tdom_cnt; ++idx)
	{
	  /* Copy... */
	  tdom[idx].NetbiosDomainName = cwcsdup (td[idx].NetbiosDomainName);
	  tdom[idx].DnsDomainName = cwcsdup (td[idx].DnsDomainName);
	  ULONG len = RtlLengthSid (td[idx].DomainSid);
	  tdom[idx].DomainSid = cmalloc_abort(HEAP_BUF, len);
	  RtlCopySid (len, tdom[idx].DomainSid, td[idx].DomainSid);
	  /* ...and set PosixOffset to 0.  This */
	  tdom[idx].PosixOffset = 0;
	}
      NetApiBufferFree (td);
      tdom_count = tdom_cnt;
    }
  /* If we have NFS installed, we make use of a name mapping server.  This
     can be either Active Directory to map uids/gids directly to Windows SIDs,
     or an AD LDS or other RFC 2307 compatible identity store.  The name of
     the mapping domain can be fetched from the registry key created by the
     NFS client installation and entered by the user via nfsadmin or the
     "Services For NFS" MMC snap-in.

     Reference:
     http://blogs.technet.com/b/filecab/archive/2012/10/09/nfs-identity-mapping-in-windows-server-2012.aspx
     Note that we neither support UNMP nor local passwd/group file mapping,
     nor UUUA.

     This function returns the mapping server from the aforementioned registry
     key, or, if none is configured, NULL, which will be resolved to the
     primary domain of the machine by the ldap_init function.

     The latter is useful to get an RFC 2307 mapping for Samba UNIX accounts,
     even if no NFS name mapping is configured on the machine.  Fortunately,
     the posixAccount and posixGroup schemas are already available in the
     Active Directory default setup since Windows Server 2003 R2. */
  reg_key reg (HKEY_LOCAL_MACHINE, KEY_READ | KEY_WOW64_64KEY,
	       L"SOFTWARE", L"Microsoft", L"ServicesForNFS", NULL);
  if (!reg.error ())
    {
      DWORD rfc2307 = reg.get_dword (L"Rfc2307", 0);
      if (rfc2307)
	{
	  rfc2307_domain_buf = (PWCHAR) ccalloc_abort (HEAP_STR, 257,
						       sizeof (WCHAR));
	  reg.get_string (L"Rfc2307Domain", rfc2307_domain_buf, 257, L"");
	  if (!rfc2307_domain_buf[0])
	    {
	      cfree (rfc2307_domain_buf);
	      rfc2307_domain_buf = NULL;
	    }
	}
    }
  return true;
}

/* Per session, so it changes potentially when switching the user context. */
static cygsid logon_sid ("");

static void
get_logon_sid ()
{
  if (PSID (logon_sid) == NO_SID)
    {
      NTSTATUS status;
      ULONG size;
      tmp_pathbuf tp;
      PTOKEN_GROUPS groups = (PTOKEN_GROUPS) tp.c_get ();

      status = NtQueryInformationToken (hProcToken, TokenGroups, groups,
					NT_MAX_PATH, &size);
      if (!NT_SUCCESS (status))
	debug_printf ("NtQueryInformationToken() %y", status);
      else
	{
	  for (DWORD pg = 0; pg < groups->GroupCount; ++pg)
	    if (groups->Groups[pg].Attributes & SE_GROUP_LOGON_ID)
	      {
		logon_sid = groups->Groups[pg].Sid;
		break;
	      }
	}
    }
}

void *
pwdgrp::add_account_post_fetch (char *line)
{
  if (line)
    { 
      void *ret;
      pglock.init ("pglock")->acquire ();
      add_line (line);
      ret = ((char *) pwdgrp_buf) + (curr_lines - 1) * pwdgrp_buf_elem_size;
      pglock.release ();
      return ret;
    }
  return NULL;
}

void *
pwdgrp::add_account_from_file (cygpsid &sid)
{
  if (!path.MaximumLength)
    return NULL;
  fetch_user_arg_t arg;
  arg.type = SID_arg;
  arg.sid = &sid;
  char *line = fetch_account_from_file (arg);
  return (struct passwd *) add_account_post_fetch (line);
}

void *
pwdgrp::add_account_from_file (const char *name)
{
  if (!path.MaximumLength)
    return NULL;
  fetch_user_arg_t arg;
  arg.type = NAME_arg;
  arg.name = name;
  char *line = fetch_account_from_file (arg);
  return (struct passwd *) add_account_post_fetch (line);
}

void *
pwdgrp::add_account_from_file (uint32_t id)
{
  if (!path.MaximumLength)
    return NULL;
  fetch_user_arg_t arg;
  arg.type = ID_arg;
  arg.id = id;
  char *line = fetch_account_from_file (arg);
  return (struct passwd *) add_account_post_fetch (line);
}

void *
pwdgrp::add_account_from_windows (cygpsid &sid, bool group)
{
  fetch_user_arg_t arg;
  arg.type = SID_arg;
  arg.sid = &sid;
  char *line = fetch_account_from_windows (arg, group);
  if (!line)
    return NULL;
  if (cygheap->pg.nss_db_caching ())
    return add_account_post_fetch (line);
  return (prep_tls_pwbuf ())->add_account_post_fetch (line);
}

void *
pwdgrp::add_account_from_windows (const char *name, bool group)
{
  fetch_user_arg_t arg;
  arg.type = NAME_arg;
  arg.name = name;
  char *line = fetch_account_from_windows (arg, group);
  if (!line)
    return NULL;
  if (cygheap->pg.nss_db_caching ())
    return add_account_post_fetch (line);
  return (prep_tls_pwbuf ())->add_account_post_fetch (line);
}

void *
pwdgrp::add_account_from_windows (uint32_t id, bool group)
{
  fetch_user_arg_t arg;
  arg.type = ID_arg;
  arg.id = id;
  char *line = fetch_account_from_windows (arg, group);
  if (!line)
    return NULL;
  if (cygheap->pg.nss_db_caching ())
    return add_account_post_fetch (line);
  return (prep_tls_pwbuf ())->add_account_post_fetch (line);
}

/* Check if file exists and if it has been written to since last checked.
   If file has been changed, invalidate the current cache.
   
   If the file doesn't exist when this function is called the first time,
   by the first Cygwin process in a process tree, the file will never be
   visited again by any process in this process tree.  This is important,
   because we cannot allow a change of UID/GID values for the lifetime
   of a process tree.
   
   If the file gets deleted or unreadable, the file cache will stay in
   place, but we won't try to read new accounts from the file.
   
   The return code indicates to the calling function if the file exists. */
bool
pwdgrp::check_file (bool group)
{
  FILE_BASIC_INFORMATION fbi;
  NTSTATUS status;

  if (!path.Buffer)
    {
      PCWSTR rel_path = group ? L"\\etc\\group" : L"\\etc\\passwd";
      path.Buffer = (PWCHAR) cmalloc_abort (HEAP_BUF,
					    (wcslen (cygheap->installation_root)
					     + wcslen (rel_path) + 1)
					    * sizeof (WCHAR));
      wcpcpy (wcpcpy (path.Buffer, cygheap->installation_root), rel_path);
      RtlInitUnicodeString (&path, path.Buffer);
      InitializeObjectAttributes (&attr, &path, OBJ_CASE_INSENSITIVE,
				  NULL, NULL);
    }
  else if (path.MaximumLength == 0) /* Indicates that the file doesn't exist. */
    return false;
  status = NtQueryAttributesFile (&attr, &fbi);
  if (!NT_SUCCESS (status))
    {
      if (last_modified.QuadPart)
	last_modified.QuadPart = 0LL;
      else
	path.MaximumLength = 0;
      return false;
    }
  if (fbi.LastWriteTime.QuadPart > last_modified.QuadPart)
    {
      last_modified.QuadPart = fbi.LastWriteTime.QuadPart;
      if (curr_lines > 0)
	{
	  pglock.init ("pglock")->acquire ();
	  int curr = curr_lines;
	  curr_lines = 0;
	  for (int i = 0; i < curr; ++i)
	    cfree (group ? this->group ()[i].g.gr_name
			 : this->passwd ()[i].p.pw_name);
	  pglock.release ();
	}
    }
  return true;
}

char *
pwdgrp::fetch_account_from_line (fetch_user_arg_t &arg, const char *line)
{
  char *p, *e;

  switch (arg.type)
    {
    case SID_arg:
      /* Ignore fields, just scan for SID string. */
      if (!(p = strstr (line, arg.name)) || p[arg.len] != ':')
	return NULL;
      break;
    case NAME_arg:
      /* First field is always name. */
      if (!strncasematch (line, arg.name, arg.len) || line[arg.len] != ':')
	return NULL;
      break;
    case ID_arg:
      /* Skip to third field. */
      if (!(p = strchr (line, ':')) || !(p = strchr (p + 1, ':')))
	return NULL;
      if (strtoul (p + 1, &e, 10) != arg.id || !e || *e != ':')
	return NULL;
      break;
    }
  return cstrdup (line);
}

char *
pwdgrp::fetch_account_from_file (fetch_user_arg_t &arg)
{
  NT_readline rl;
  tmp_pathbuf tp;
  char *buf = tp.c_get ();
  char *str = tp.c_get ();
  char *ret = NULL;

  /* Create search string. */
  switch (arg.type)
    {
    case SID_arg:
      /* Override SID with SID string. */
      arg.sid->string (str);
      arg.name = str;
      /*FALLTHRU*/
    case NAME_arg:
      arg.len = strlen (arg.name);
      break;
    case ID_arg:
      break;
    }
  if (rl.init (&attr, buf, NT_MAX_PATH))
    while ((buf = rl.gets ()))
      if ((ret = fetch_account_from_line (arg, buf)))
	return ret;
  return NULL;
}

char *
pwdgrp::fetch_account_from_windows (fetch_user_arg_t &arg, bool group)
{
  /* Used in LookupAccount calls. */
  WCHAR namebuf[UNLEN + 1], *name = namebuf;
  WCHAR dom[DNLEN + 1] = L"";
  cygsid csid;
  DWORD nlen = UNLEN + 1;
  DWORD dlen = DNLEN + 1;
  DWORD slen = MAX_SID_LEN;
  cygpsid sid = NO_SID;
  SID_NAME_USE acc_type;
  BOOL ret = false;
  /* Cygwin user name style. */
  enum {
    name_only,
    plus_prepended,
    fully_qualified
  } name_style = name_only;
  /* Computed stuff. */
  uid_t uid = ILLEGAL_UID;
  gid_t gid = ILLEGAL_GID;
  bool is_domain_account = true;
  PCWSTR domain = NULL;
  PWCHAR shell = NULL;
  PWCHAR user = NULL;
  PWCHAR home = NULL;
  PWCHAR gecos = NULL;
  PWCHAR p;
  WCHAR sidstr[128];
  /* Temporary stuff. */
  ULONG posix_offset = 0;
  uint32_t id_val;
  cyg_ldap cldap;
  bool ldap_open = false;

  /* Initialize */
  if (!cygheap->dom.init ())
    return NULL;

  switch (arg.type)
    {
    case SID_arg:
      sid = *arg.sid;
      ret = LookupAccountSidW (NULL, sid, name, &nlen, dom, &dlen, &acc_type);
      break;
    case NAME_arg:
      /* Skip leading domain separator.  This denotes an alias or well-known
         group, which will be found first by LookupAccountNameW anyway.
	 Otherwise, if the name has no leading domain name, it's either a
	 standalone machine, or the username must be from the primary domain.
	 In the latter case, prepend the primary domain name so as not to
	 collide with an account from the account domain with the same name. */
      p = name;
      if (*arg.name == cygheap->pg.nss_separator ()[0])
	++arg.name;
      else if (!strchr (arg.name, cygheap->pg.nss_separator ()[0])
	       && cygheap->dom.member_machine ())
	p = wcpcpy (wcpcpy (p, cygheap->dom.primary_flat_name ()),
		    cygheap->pg.nss_separator ());
      /* Now fill up with name to search. */
      sys_mbstowcs (p, UNLEN + 1, arg.name);
      /* Replace domain separator char with backslash and make sure p is NULL
	 or points to the backslash, so... */
      if ((p = wcschr (name, cygheap->pg.nss_separator ()[0])))
      	*p = L'\\';
      sid = csid;
      ret = LookupAccountNameW (NULL, name, sid, &slen, dom, &dlen, &acc_type);
      if (!ret)
	{
	  debug_printf ("LookupAccountNameW (%W), %E", name);
	  return NULL;
	}
      /* ... we can skip the backslash in the rest of this function. */
      if (p)
	name = p + 1;
      break;
    case ID_arg:
      /* Construct SID from ID using the SFU rules, just like the code below
         goes the opposite route. */
#ifndef INTERIX_COMPATIBLE
      /* Except for Builtin and Alias groups in the SECURITY_NT_AUTHORITY.
	 We create uid/gid values compatible with the old values generated
	 by mkpasswd/mkgroup. */
      if (arg.id < 0x200)
	__small_swprintf (sidstr, L"S-1-5-%u", arg.id & 0x1ff);
      else if (arg.id <= 0x7ff)
	__small_swprintf (sidstr, L"S-1-5-32-%u", arg.id & 0x7ff);
      else
#endif
      if (arg.id == 0xffe)
	{
	  /* OtherSession != Logon SID. */
	  get_logon_sid ();
	  /* LookupAccountSidW will fail. */
	  sid = csid = logon_sid;
	  sid_sub_auth_rid (sid) = 0;
	  break;
	}
      else if (arg.id == 0xfff)
	{
	  /* CurrentSession == Logon SID. */
	  get_logon_sid ();
	  /* LookupAccountSidW will fail. */
	  sid = logon_sid;
	  break;
	}
      else if (arg.id < 0x10000)
	{
	  /* Nothing. */
	  debug_printf ("Invalid POSIX id %u", arg.id);
	  return NULL;
	}
      else if (arg.id < 0x20000)
	{
	  /* Well-Known Group */
	  arg.id -= 0x10000;
	  __small_swprintf (sidstr, L"S-1-%u-%u", arg.id >> 8, arg.id & 0xff);
	}
      else if (arg.id >= 0x30000 && arg.id < 0x40000)
	{
	  /* Account domain user or group. */
	  PWCHAR s = cygheap->dom.account_sid ().pstring (sidstr);
	  __small_swprintf (s, L"-%u", arg.id & 0xffff);
	}
      else if (arg.id < 0x60000)
	{
	  /* Builtin Alias */
	  __small_swprintf (sidstr, L"S-1-5-%u-%u",
			    arg.id >> 12, arg.id & 0xffff);
	}
      else if (arg.id < 0x70000)
	{
	  /* Mandatory Label. */
	  __small_swprintf (sidstr, L"S-1-16-%u", arg.id & 0xffff);
	}
      else if (arg.id < 0x80000)
	{
	  /* Identity assertion SIDs. */
	  __small_swprintf (sidstr, L"S-1-18-%u", arg.id & 0xffff);
	}
      else if (arg.id < 0x100000)
	{
	  /* Nothing. */
	  debug_printf ("Invalid POSIX id %u", arg.id);
	  return NULL;
	}
      else if (arg.id == ILLEGAL_UID)
	{
	  /* Just some fake. */
	  sid = csid = "S-1-99-0";
	  break;
	}
      else if (arg.id >= UNIX_POSIX_OFFSET)
	{
	  /* UNIX (unknown NFS or Samba) user account. */
	  __small_swprintf (sidstr, L"S-1-22-%u-%u",
			    group ? 2 : 1,  arg.id & UNIX_POSIX_MASK);
	  /* LookupAccountSidW will fail. */
	  sid = csid = sidstr;
	  break;
	}
      else
	{
	  /* Some trusted domain? */
	  PDS_DOMAIN_TRUSTSW td = NULL;

	  for (ULONG idx = 0; (td = cygheap->dom.trusted_domain (idx)); ++idx)
	    {
	      /* If we don't have the PosixOffset of the domain, fetch it.
		 Skip primary domain. */
	      if (!td->PosixOffset && !(td->Flags & DS_DOMAIN_PRIMARY))
		{
		  if (!ldap_open && !(ldap_open = cldap.open (NULL)))
		    id_val = cygheap->dom.lowest_tdo_posix_offset
			     - 0x01000000;
		  else
		    id_val =
		      cldap.fetch_posix_offset_for_domain (td->DnsDomainName);
		  if (id_val)
		    {
		      td->PosixOffset = id_val;
		      if (id_val < cygheap->dom.lowest_tdo_posix_offset)
			cygheap->dom.lowest_tdo_posix_offset = id_val;
		    }
		}
	      if (td->PosixOffset > posix_offset && td->PosixOffset <= arg.id)
		posix_offset = td->PosixOffset;
	    }
	  if (posix_offset)
	    {
	      cygpsid tsid (td->DomainSid);
	      PWCHAR s = tsid.pstring (sidstr);
	      __small_swprintf (s, L"-%u", arg.id - posix_offset);
	    }
	  else
	    {
	      /* Primary domain */
	      PWCHAR s = cygheap->dom.primary_sid ().pstring (sidstr);
	      __small_swprintf (s, L"-%u", arg.id - 0x100000);
	    }
	  posix_offset = 0;
	}
      sid = csid = sidstr;
      ret = LookupAccountSidW (NULL, sid, name, &nlen, dom, &dlen, &acc_type);
      if (!ret)
	{
	  debug_printf ("LookupAccountSidW (%W), %E", sidstr);
	  return NULL;
	}
      break;
    }
  if (ret)
    {
      /* Builtin account?  SYSTEM, for instance, is returned as SidTypeUser,
	 if a process is running as LocalSystem service. */
      if (acc_type == SidTypeUser && sid_sub_auth_count (sid) <= 3)
	acc_type = SidTypeWellKnownGroup;
      switch (acc_type)
      	{
	case SidTypeUser:
	case SidTypeGroup:
	case SidTypeAlias:
	  /* Predefined alias? */
	  if (acc_type == SidTypeAlias
	      && sid_sub_auth (sid, 0) != SECURITY_NT_NON_UNIQUE)
	    {
#ifdef INTERIX_COMPATIBLE
	      posix_offset = 0x30000;
	      uid = 0x1000 * sid_sub_auth (sid, 0)
		    + (sid_sub_auth_rid (sid) & 0xffff);
#else
	      posix_offset = 0;
#endif
	      name_style = (cygheap->pg.nss_prefix_always ()) ? fully_qualified
							      : plus_prepended;
	      domain = cygheap->dom.account_flat_name ();
	      is_domain_account = false;
	    }
	  /* Account domain account? */
	  else if (!wcscmp (dom, cygheap->dom.account_flat_name ()))
	    {
	      posix_offset = 0x30000;
	      if (cygheap->dom.member_machine ()
		  || !cygheap->pg.nss_prefix_auto ())
		name_style = fully_qualified;
	      domain = cygheap->dom.account_flat_name ();
	      is_domain_account = false;
	    }
	  /* Domain member machine? */
	  else if (cygheap->dom.member_machine ())
	    {
	      /* Primary domain account? */
	      if (!wcscmp (dom, cygheap->dom.primary_flat_name ()))
		{
		  posix_offset = 0x100000;
		  /* In theory domain should have been set to
		     cygheap->dom.primary_dns_name (), but it turns out
		     that not setting the domain here has advantages.
		     We open the ldap connection to NULL (== some domain
		     control of our primary domain) anyway.  So the domain
		     is only used 
		     later on.  So, don't set domain here to non-NULL, unless
		     you're sure you have also changed subsequent assumptions
		     that domain is NULL if it's a primary domain account. */
		  domain = NULL;
		  if (!cygheap->pg.nss_prefix_auto ())
		    name_style = fully_qualified;
		}
	      else
		{
		  /* No, fetch POSIX offset. */
		  PDS_DOMAIN_TRUSTSW td = NULL;

		  name_style = fully_qualified;
		  for (ULONG idx = 0;
		       (td = cygheap->dom.trusted_domain (idx));
		       ++idx)
		    {
		      if (wcscmp (dom, td->NetbiosDomainName))
			continue;
		      domain = td->DnsDomainName;
		      posix_offset = td->PosixOffset;
		      /* If we don't have the PosixOffset of the domain,
			 fetch it. */
		      if (!posix_offset)
			{
			  if (!ldap_open && !(ldap_open = cldap.open (NULL)))
			    {
			      /* We're probably running under a local account,
				 so we're not allowed to fetch any information
				 from AD beyond the most obvious.  Never mind,
				 just fake a reasonable posix offset. */
			      id_val = cygheap->dom.lowest_tdo_posix_offset
				       - 0x01000000;
			    }
			  else
			    id_val =
			      cldap.fetch_posix_offset_for_domain (domain);
			  if (id_val)
			    {
			      td->PosixOffset = posix_offset = id_val;
			      if (id_val < cygheap->dom.lowest_tdo_posix_offset)
				cygheap->dom.lowest_tdo_posix_offset = id_val;
			    }
			}
		      break;
		    }

		  if (!domain)
		    {
		      debug_printf ("Unknown domain %W", dom);
		      return NULL;
		    }
		}
	    }
	  /* If the domain returned by LookupAccountSid is not our machine
	     name, and if our machine is no domain member, we lose.  We have
	     nobody to ask for the POSIX offset. */
	  else
	    {
	      debug_printf ("Unknown domain %W", dom);
	      return NULL;
	    }
	  /* Generate values. */
	  if (uid == ILLEGAL_UID)
	    uid = posix_offset + sid_sub_auth_rid (sid);
	  gid = posix_offset + DOMAIN_GROUP_RID_USERS; /* Default. */

	  if (is_domain_account)
	    {
	      /* Use LDAP to fetch domain account infos. */
	      if (!ldap_open && !cldap.open (NULL))
		break;
	      if (cldap.fetch_ad_account (sid, group))
		{
		  PWCHAR val;
		  if (acc_type == SidTypeUser)
		    {
		      if ((id_val = cldap.get_primary_gid ()) != ILLEGAL_GID)
			gid = posix_offset + id_val;
		      if ((val = cldap.get_user_name ())
			  && wcscmp (name, val))
			user = wcscpy ((PWCHAR) alloca ((wcslen (val) + 1)
				       * sizeof (WCHAR)), val);
		      if ((val = cldap.get_gecos ()))
			gecos = wcscpy ((PWCHAR) alloca ((wcslen (val) + 1)
					* sizeof (WCHAR)), val);
		      if ((val = cldap.get_home ()))
			home = wcscpy ((PWCHAR) alloca ((wcslen (val) + 1)
				       * sizeof (WCHAR)), val);
		      if ((val = cldap.get_shell ()))
			shell = wcscpy ((PWCHAR) alloca ((wcslen (val) + 1)
					* sizeof (WCHAR)), val);
		      /* Check and, if necessary, add unix<->windows
			 id mapping on the fly. */
		      id_val = cldap.get_unix_uid ();
		      if (id_val != ILLEGAL_UID
			  && cygheap->ugid_cache.get_uid (id_val)
			     == ILLEGAL_UID)
			cygheap->ugid_cache.add_uid (id_val, uid);
		    }
		  else /* SidTypeGroup */
		    {
		      if ((val = cldap.get_group_name ())
			  && wcscmp (name, val))
			user = wcscpy ((PWCHAR) alloca ((wcslen (val) + 1)
				       * sizeof (WCHAR)), val);
		      id_val = cldap.get_unix_gid ();
		      if (id_val != ILLEGAL_GID
			  && cygheap->ugid_cache.get_gid (id_val)
			     == ILLEGAL_GID)
			cygheap->ugid_cache.add_gid (id_val, uid);
		    }
		}
	    }
	  /* Otherwise check account domain (local SAM).*/
	  else
	    {
	      NET_API_STATUS nas;
	      PUSER_INFO_4 ui;
	      PLOCALGROUP_INFO_1 gi;
	      PCWSTR comment;
	      PWCHAR pgrp = NULL;
	      PWCHAR uxid = NULL;
	      struct {
		PCWSTR str;
		size_t len;
		PWCHAR *tgt;
		bool group;
	      } search[] = {
		{ L"name=\"", 6, &user, true },
		{ L"unix=\"", 6, &uxid, true },
		{ L"home=\"", 6, &home, false },
		{ L"shell=\"", 7, &shell, false },
		{ L"group=\"", 7, &pgrp, false },
		{ NULL, 0, NULL }
	      };
	      PWCHAR s, e;

	      if (acc_type == SidTypeUser)
		{
		  nas = NetUserGetInfo (NULL, name, 4, (PBYTE *) &ui);
		  if (nas != NERR_Success)
		    {
		      debug_printf ("NetUserGetInfo(%W) %u", name, nas);
		      break;
		    }
		  /* Set comment variable for below attribute loop. */
		  comment = ui->usri4_comment;
		}
	      else /* SidTypeGroup || SidTypeAlias */
		{
		  nas = NetLocalGroupGetInfo (NULL, name, 1, (PBYTE *) &gi);
		  if (nas != NERR_Success)
		    {
		      debug_printf ("NetLocalGroupGetInfo(%W) %u", name, nas);
		      break;
		    }
		  /* Set comment variable for below attribute loop. */
		  comment = gi->lgrpi1_comment;
		}
	      /* Local SAM accounts have only a handful attributes
		 available to home users.  Therefore, fetch additional
		 passwd/group attributes from the "Description" field
		 in XML short style. */
	      if ((s = wcsstr (comment, L"<cygwin "))
		  && (e = wcsstr (s + 8, L"/>")))
		{
		  s += 8;
		  *e = L'\0';
		  while (*s)
		    {
		      bool found = false;

		      while (*s == L' ')
			++s;
		      for (size_t i = 0; search[i].str; ++i)
			if ((acc_type == SidTypeUser || search[i].group)
			    && !wcsncmp (s, search[i].str, search[i].len))
			  {
			    s += search[i].len;
			    if ((e = wcschr (s, L'"'))
				&& (i > 0 || wcsncmp (name, s, e - s)))
			      {
				*search[i].tgt =
				    (PWCHAR) alloca ((e - s + 1)
						     * sizeof (WCHAR));
				*wcpncpy (*search[i].tgt, s, e - s) = L'\0';
				s = e + 1;
				found = true;
			      }
			    else
			      break;
			  }
		      if (!found)
			break;
		    }
		}
	      if (acc_type == SidTypeUser)
		NetApiBufferFree (ui);
	      else
		NetApiBufferFree (gi);
	      if (pgrp)
		{
		  /* For setting the primary group, we have to test 
		     with and without prepended separator. */
		  char gname[2 * UNLEN + 2];
		  struct group *gr;

		  *gname = cygheap->pg.nss_separator ()[0];
		  sys_wcstombs (gname + 1, 2 * UNLEN + 1, pgrp);
		  if ((gr = internal_getgrnam (gname))
		      || (gr = internal_getgrnam (gname + 1)))
		    gid = gr->gr_gid;
		}
	      if (uxid && ((id_val = wcstoul (uxid, &e, 10)), !*e))
		{
		  if (acc_type == SidTypeUser)
		    {
		      if (cygheap->ugid_cache.get_uid (id_val) == ILLEGAL_UID)
			cygheap->ugid_cache.add_uid (id_val, uid);
		    }
		  else if (cygheap->ugid_cache.get_gid (id_val) == ILLEGAL_GID)
		    cygheap->ugid_cache.add_gid (id_val, uid);
		}
	    }
	  break;
	case SidTypeWellKnownGroup:
	  name_style = (cygheap->pg.nss_prefix_always ()) ? fully_qualified
							  : plus_prepended;
#ifdef INTERIX_COMPATIBLE
	  if (sid_id_auth (sid) == 5 /* SECURITY_NT_AUTHORITY */
	      && sid_sub_auth_count (sid) > 1)
	    {
	      uid = 0x1000 * sid_sub_auth (sid, 0)
		    + (sid_sub_auth_rid (sid) & 0xffff);
	      name_style = fully_qualified;
	    }
	  else
	    uid = 0x10000 + 0x100 * sid_id_auth (sid)
		  + (sid_sub_auth_rid (sid) & 0xff);
#else
	  if (sid_id_auth (sid) != 5 /* SECURITY_NT_AUTHORITY */)
	    uid = 0x10000 + 0x100 * sid_id_auth (sid)
		  + (sid_sub_auth_rid (sid) & 0xff);
	  else if (sid_sub_auth (sid, 0) < SECURITY_PACKAGE_BASE_RID)
	    uid = sid_sub_auth_rid (sid) & 0x7ff;
	  else
	    {
	      uid = 0x1000 * sid_sub_auth (sid, 0)
		    + (sid_sub_auth_rid (sid) & 0xffff);
	    }
#endif
	  /* Special case for "Everyone".  We don't want to return Everyone
	     as user or group.  Ever. */
	  if (uid == 0x10100)	/* Computed from S-1-1-0. */
	    return NULL;
	  break;
	case SidTypeLabel:
	  uid = 0x60000 + sid_sub_auth_rid (sid);
	  name_style = (cygheap->pg.nss_prefix_always ()) ? fully_qualified
							  : plus_prepended;
	  break;
	default:
	  return NULL;
	}
    } 
  else if (sid_id_auth (sid) == 5 /* SECURITY_NT_AUTHORITY */
	   && sid_sub_auth (sid, 0) == SECURITY_LOGON_IDS_RID)
    {
      /* Logon ID. Mine or other? */
      get_logon_sid ();
      if (PSID (logon_sid) == NO_SID)
	return NULL;
      if (RtlEqualSid (sid, logon_sid))
	{
	  uid = 0xfff;
	  wcpcpy (name = namebuf, L"CurrentSession");
	}
      else
	{
	  uid = 0xffe;
	  wcpcpy (name = namebuf, L"OtherSession");
	}
    }
  else if (sid_id_auth (sid) == 18)
    {
      /* Authentication assertion SIDs.
      
         Available when using a 2012R2 DC, but not supported by
	 LookupAccountXXX on pre Windows 8/2012 machines */
      uid = 0x11200 + sid_sub_auth_rid (sid);
      wcpcpy (name = namebuf, sid_sub_auth_rid (sid) == 1
	      ? (PWCHAR) L"Authentication authority asserted identity"
	      : (PWCHAR) L"Service asserted identity");
      name_style = plus_prepended;
    }
  else if (sid_id_auth (sid) == 22)
    {
      /* Samba UNIX Users/Groups

         This *might* colide with a posix_offset of some trusted domain.
         It's just very unlikely. */
      uid = MAP_UNIX_TO_CYGWIN_ID (sid_sub_auth_rid (sid));
      /* Unfortunately we have no access to the file server from here,
	 so we can't generate correct user names. */
      p = wcpcpy (dom, L"UNIX_");
      wcpcpy (p, sid_sub_auth (sid, 0) == 1 ? L"User" : L"Group");
      __small_swprintf (name = namebuf, L"%d", uid & UNIX_POSIX_MASK);
      name_style = fully_qualified;
    }
  else
    {
      wcpcpy (dom, L"Unknown");
      wcpcpy (name = namebuf, group ? L"Group" : L"User");
      name_style = fully_qualified;
    }

  tmp_pathbuf tp;
  PWCHAR linebuf = tp.w_get ();
  char *line = NULL;

  WCHAR posix_name[UNLEN + 1 + DNLEN + 1];
  p = posix_name;
  if (gid == ILLEGAL_GID)
    gid = uid;
  if (name_style >= fully_qualified)
    p = wcpcpy (p, user ? group ? L"Posix_Group" : L"Posix_User" : dom);
  if (name_style >= plus_prepended)
    p = wcpcpy (p, cygheap->pg.nss_separator ());
  wcpcpy (p, user ?: name);

  if (group)
    __small_swprintf (linebuf, L"%W:%W:%u:",
		      posix_name, sid.string (sidstr), uid);
  else
    __small_swprintf (linebuf, L"%W:*:%u:%u:%W%WU-%W\\%W,%W:%W%W:%W",
		      posix_name, uid, gid,
		      gecos ?: L"", gecos ? L"," : L"",
		      dom, name,
		      sid.string (sidstr),
		      home ? L"" : L"/home/", home ?: user ?: name,
		      shell ?: L"/bin/sh");
  sys_wcstombs_alloc (&line, HEAP_BUF, linebuf);
  debug_printf ("line: <%s>", line);
  return line;
}
