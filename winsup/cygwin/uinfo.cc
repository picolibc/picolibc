/* uinfo.cc: user info (uid, gid, etc...)

   Copyright 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006,
   2007, 2008, 2009, 2010, 2011, 2012 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <unistd.h>
#include <wininet.h>
#include <stdlib.h>
#include <wchar.h>
#include <lm.h>
#include <iptypes.h>
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
#include "pwdgrp.h"
#include "tls_pbuf.h"
#include "ntdll.h"

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

  cygpsid psid = user.sid ();
  pw = internal_getpwsid (psid);

  if (!pw && !(pw = internal_getpwnam (user.name ()))
      && !(pw = internal_getpwuid (DEFAULT_UID)))
    debug_printf ("user not found in augmented /etc/passwd");
  else
    {
      cygsid gsid;

      myself->uid = pw->pw_uid;
      myself->gid = pw->pw_gid;
      user.set_name (pw->pw_name);
      if (gsid.getfromgr (internal_getgrgid (pw->pw_gid)))
	{
	  if (gsid != user.groups.pgsid)
	    {
	      /* Set primary group to the group in /etc/passwd. */
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
  __try
    {
      strncpy (name, login, len);
    }
  __except (NO_ERROR)
    {
      return EFAULT;
    }
  __endtry
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
  if (get_logon_server (wdomain, wlogsrv, false))
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
      lptr = eptr;
      eptr = strchr (lptr, '\n');
      if (eptr)
	{
	  if (eptr > lptr && eptr[-1] == '\r')
	    eptr[-1] = '\0';
	  else
	    *eptr = '\0';
	  eptr++;
	}
      if (curr_lines >= max_lines)
	{
	  max_lines += 10;
	  *pwdgrp_buf = realloc (*pwdgrp_buf, max_lines * pwdgrp_buf_elem_size);
	}
      if ((this->*parse) ())
	curr_lines++;
    }
  return eptr;
}

void
pwdgrp::load (const wchar_t *rel_path)
{
  static const char failed[] = "failed";
  static const char succeeded[] = "succeeded";
  const char *res = failed;
  HANDLE fh = NULL;

  NTSTATUS status;
  OBJECT_ATTRIBUTES attr;
  IO_STATUS_BLOCK io;
  FILE_STANDARD_INFORMATION fsi;

  if (buf)
    free (buf);
  buf = NULL;
  curr_lines = 0;

  if (!path &&
      !(path = (PWCHAR) malloc ((wcslen (cygheap->installation_root)
				 + wcslen (rel_path) + 1) * sizeof (WCHAR))))
    {
      paranoid_printf ("malloc (%W) failed", rel_path);
      goto out;
    }
  wcpcpy (wcpcpy (path, cygheap->installation_root), rel_path);
  RtlInitUnicodeString (&upath, path);

  InitializeObjectAttributes (&attr, &upath, OBJ_CASE_INSENSITIVE, NULL, NULL);
  etc_ix = etc::init (etc_ix, &attr);

  paranoid_printf ("%S", &upath);

  status = NtOpenFile (&fh, SYNCHRONIZE | FILE_READ_DATA, &attr, &io,
		       FILE_SHARE_VALID_FLAGS,
		       FILE_SYNCHRONOUS_IO_NONALERT
		       | FILE_OPEN_FOR_BACKUP_INTENT);
  if (!NT_SUCCESS (status))
    {
      paranoid_printf ("NtOpenFile(%S) failed, status %y", &upath, status);
      goto out;
    }
  status = NtQueryInformationFile (fh, &io, &fsi, sizeof fsi,
				   FileStandardInformation);
  if (!NT_SUCCESS (status))
    {
      paranoid_printf ("NtQueryInformationFile(%S) failed, status %y",
		       &upath, status);
      goto out;
    }
  /* FIXME: Should we test for HighPart set?  If so, the
     passwd or group file is way beyond what we can handle. */
  /* FIXME 2: It's still ugly that we keep the file in memory.
     Big organizations have naturally large passwd files. */
  buf = (char *) malloc (fsi.EndOfFile.LowPart + 1);
  if (!buf)
    {
      paranoid_printf ("malloc (%u) failed", fsi.EndOfFile.LowPart);
      goto out;
    }
  status = NtReadFile (fh, NULL, NULL, NULL, &io, buf, fsi.EndOfFile.LowPart,
		       NULL, NULL);
  if (!NT_SUCCESS (status))
    {
      paranoid_printf ("NtReadFile(%S) failed, status %y", &upath, status);
      free (buf);
      goto out;
    }
  buf[fsi.EndOfFile.LowPart] = '\0';
  for (char *eptr = buf; (eptr = add_line (eptr)); )
    continue;
  debug_printf ("%W curr_lines %d", rel_path, curr_lines);
  res = succeeded;

out:
  if (fh)
    NtClose (fh);
  debug_printf ("%W load %s", rel_path, res);
  initialized = true;
}
