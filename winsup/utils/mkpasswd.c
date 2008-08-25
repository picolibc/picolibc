/* mkpasswd.c:

   Copyright 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2005, 2006,
   2008 Red Hat, Inc.

   This file is part of Cygwin.

   This software is a copyrighted work licensed under the terms of the
   Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
   details. */

#define _WIN32_WINNT 0x0600
#include <ctype.h>
#include <stdlib.h>
#include <wchar.h>
#include <wctype.h>
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <io.h>
#include <sys/fcntl.h>
#include <sys/cygwin.h>
#include <windows.h>
#include <lm.h>
#include <iptypes.h>
#include <wininet.h>
#include <ntsecapi.h>
#include <dsgetdc.h>
#include <ntdef.h>

#define print_win_error(x) _print_win_error(x, __LINE__)

#define MAX_SID_LEN 40

static const char version[] = "$Revision$";

extern char *__progname;

SID_IDENTIFIER_AUTHORITY sid_world_auth = {SECURITY_WORLD_SID_AUTHORITY};
SID_IDENTIFIER_AUTHORITY sid_nt_auth = {SECURITY_NT_AUTHORITY};

NET_API_STATUS WINAPI (*dsgetdcname)(LPWSTR,LPWSTR,GUID*,LPWSTR,ULONG,PDOMAIN_CONTROLLER_INFOW*);

#ifndef min
#define min(a,b) (((a)<(b))?(a):(b))
#endif

typedef struct
{
  char *str;
  DWORD id_offset;
  BOOL domain;
  BOOL with_dom;
} domlist_t;

static void
_print_win_error(DWORD code, int line)
{
  char buf[4096];

  if (FormatMessage (FORMAT_MESSAGE_FROM_SYSTEM
      | FORMAT_MESSAGE_IGNORE_INSERTS,
      NULL,
      code,
      MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT),
      (LPTSTR) buf, sizeof (buf), NULL))
    fprintf (stderr, "mkpasswd (%d): [%lu] %s", line, code, buf);
  else
    fprintf (stderr, "mkpasswd (%d): error %lu", line, code);
}

static void
load_dsgetdcname ()
{
  HANDLE h = LoadLibrary ("netapi32.dll");

  if (h)
    dsgetdcname = (void *) GetProcAddress (h, "DsGetDcNameW");
}

static PWCHAR
get_dcname (char *domain)
{
  static WCHAR server[INTERNET_MAX_HOST_NAME_LENGTH + 1];
  DWORD rc;
  PWCHAR servername;
  WCHAR domain_name[MAX_DOMAIN_NAME_LEN + 1];
  PDOMAIN_CONTROLLER_INFOW pdci = NULL;

  if (dsgetdcname)
    {
      if (domain)
	{
	  mbstowcs (domain_name, domain, strlen (domain) + 1);
	  rc = dsgetdcname (NULL, domain_name, NULL, NULL, 0, &pdci);
	}
      else
	rc = dsgetdcname (NULL, NULL, NULL, NULL, 0, &pdci);
      if (rc != ERROR_SUCCESS)
	{
	  print_win_error(rc);
	  return (PWCHAR) -1;
	}
      wcscpy (server, pdci->DomainControllerName);
      NetApiBufferFree (pdci);
    }
  else
    {
      rc = NetGetDCName (NULL, NULL, (void *) &servername);
      if (rc == ERROR_SUCCESS && domain)
	{
	  LPWSTR server = servername;
	  mbstowcs (domain_name, domain, strlen (domain) + 1);
	  rc = NetGetDCName (server, domain_name, (void *) &servername);
	  NetApiBufferFree (server);
	}
      if (rc != ERROR_SUCCESS)
	{
	  print_win_error(rc);
	  return (PWCHAR) -1;
	}
      wcscpy (server, servername);
      NetApiBufferFree ((PVOID) servername);
    }
  return server;
}

static char *
put_sid (PSID sid)
{
  static char s[512];
  char t[32];
  DWORD i;

  strcpy (s, "S-1-");
  sprintf(t, "%u", GetSidIdentifierAuthority (sid)->Value[5]);
  strcat (s, t);
  for (i = 0; i < *GetSidSubAuthorityCount (sid); ++i)
    {
      sprintf(t, "-%lu", *GetSidSubAuthority (sid, i));
      strcat (s, t);
    }
  return s;
}

static void
psx_dir (char *in, char *out)
{
  if (isalpha (in[0]) && in[1] == ':')
    {
      sprintf (out, "/cygdrive/%c", in[0]);
      in += 2;
      out += strlen (out);
    }

  while (*in)
    {
      if (*in == '\\')
	*out = '/';
      else
	*out = *in;
      in++;
      out++;
    }

  *out = '\0';
}

static void
uni2ansi (LPWSTR wcs, char *mbs, int size)
{
  if (wcs)
    wcstombs (mbs, wcs, size);
  else
    *mbs = '\0';
}

typedef struct {
  PSID psid;
  int buffer[10];
} sidbuf;

static sidbuf curr_user;
static sidbuf curr_pgrp;
static BOOL got_curr_user = FALSE;

static void
fetch_current_user_sid ()
{
  DWORD len;
  HANDLE ptok;

  if (!OpenProcessToken (GetCurrentProcess (), TOKEN_QUERY, &ptok)
      || !GetTokenInformation (ptok, TokenUser, &curr_user, sizeof curr_user,
			       &len)
      || !GetTokenInformation (ptok, TokenPrimaryGroup, &curr_pgrp,
			       sizeof curr_pgrp, &len)
      || !CloseHandle (ptok))
    {
      print_win_error (GetLastError ());
      return;
    }
}

static void
current_user (int print_cygpath, const char *sep, const char *passed_home_path,
	      DWORD id_offset, const char *disp_username)
{
  WCHAR user[UNLEN + 1];
  WCHAR dom[MAX_DOMAIN_NAME_LEN + 1];
  DWORD ulen = UNLEN + 1;
  DWORD dlen = MAX_DOMAIN_NAME_LEN + 1;
  SID_NAME_USE acc_type;
  int uid, gid;
  char homedir_psx[PATH_MAX] = {0}, homedir_w32[MAX_PATH] = {0};

  if (!curr_user.psid || !curr_pgrp.psid
      || !LookupAccountSidW (NULL, curr_user.psid, user, &ulen, dom, &dlen,
			     &acc_type))
    {
      print_win_error (GetLastError ());
      return;
    }

  uid = *GetSidSubAuthority (curr_user.psid,
			     *GetSidSubAuthorityCount(curr_user.psid) - 1);
  gid = *GetSidSubAuthority (curr_pgrp.psid,
			     *GetSidSubAuthorityCount(curr_pgrp.psid) - 1);
  if (passed_home_path[0] == '\0')
    {
      char *envhome = getenv ("HOME");
      char *envhomedrive = getenv ("HOMEDRIVE");
      char *envhomepath = getenv ("HOMEPATH");

      if (envhome && envhome[0])
	{
	  if (print_cygpath)
	    cygwin_conv_path (CCP_WIN_A_TO_POSIX | CCP_ABSOLUTE, envhome,
			      homedir_psx, PATH_MAX);
	  else
	    psx_dir (envhome, homedir_psx);
	}
      else if (envhomepath && envhomepath[0])
	{
	  if (envhomedrive)
	    strlcpy (homedir_w32, envhomedrive, sizeof (homedir_w32));
	  if (envhomepath[0] != '\\')
	    strlcat (homedir_w32, "\\", sizeof (homedir_w32));
	  strlcat (homedir_w32, envhomepath, sizeof (homedir_w32));
	  if (print_cygpath)
	    cygwin_conv_path (CCP_WIN_A_TO_POSIX | CCP_ABSOLUTE, homedir_w32,
			      homedir_psx, PATH_MAX);
	  else
	    psx_dir (homedir_w32, homedir_psx);
	}
      else
	{
	  wcstombs (stpncpy (homedir_psx, "/home/", sizeof (homedir_psx)),
		    user, sizeof (homedir_psx) - 6);
	  homedir_psx[PATH_MAX - 1] = '\0';
	}
    }
  else
    {
      char *p = stpncpy (homedir_psx, passed_home_path, sizeof (homedir_psx));
      wcstombs (p, user, sizeof (homedir_psx) - (p - homedir_psx));
      homedir_psx[PATH_MAX - 1] = '\0';
    }

  printf ("%ls%s%ls:unused:%lu:%lu:U-%ls\\%ls,%s:%s:/bin/bash\n",
	  sep ? dom : L"",
	  sep ?: "",
	  user,
	  id_offset + uid,
	  id_offset + gid,
	  dom,
	  user,
	  put_sid (curr_user.psid),
	  homedir_psx);
}

static void
enum_unix_users (domlist_t *dom_or_machine, const char *sep, DWORD id_offset,
		 char *unix_user_list)
{
  WCHAR machine[INTERNET_MAX_HOST_NAME_LENGTH + 1];
  PWCHAR servername = NULL;
  char *d_or_m = dom_or_machine ? dom_or_machine->str : NULL;
  BOOL with_dom = dom_or_machine ? dom_or_machine->with_dom : FALSE;
  SID_IDENTIFIER_AUTHORITY auth = { { 0, 0, 0, 0, 0, 22 } };
  char *ustr, *user_list;
  WCHAR user[UNLEN + sizeof ("Unix User\\") + 1];
  WCHAR dom[MAX_DOMAIN_NAME_LEN + 1];
  DWORD ulen, dlen, sidlen;
  PSID psid;
  char psid_buffer[MAX_SID_LEN];
  SID_NAME_USE acc_type;

  if (!d_or_m)
    return;

  int ret = mbstowcs (machine, d_or_m, INTERNET_MAX_HOST_NAME_LENGTH + 1);
  if (ret < 1 || ret >= INTERNET_MAX_HOST_NAME_LENGTH + 1)
    {
      fprintf (stderr, "%s: Invalid machine name '%s'.  Skipping...\n",
	       __progname, d_or_m);
      return;
    }
  servername = machine;

  if (!AllocateAndInitializeSid (&auth, 2, 1, 0, 0, 0, 0, 0, 0, 0, &psid))
    return;

  if (!(user_list = strdup (unix_user_list)))
    {
      FreeSid (psid);
      return;
    }

  for (ustr = strtok (user_list, ","); ustr; ustr = strtok (NULL, ","))
    {
      if (!isdigit (ustr[0]) && ustr[0] != '-')
	{
	  PWCHAR p = wcpcpy (user, L"Unix User\\");
	  ret = mbstowcs (p, ustr, UNLEN + 1);
	  if (ret < 1 || ret >= UNLEN + 1)
	    fprintf (stderr, "%s: Invalid user name '%s'.  Skipping...\n",
		     __progname, ustr);
	  else if (LookupAccountNameW (servername, user,
				       psid = (PSID) psid_buffer,
				       (sidlen = MAX_SID_LEN, &sidlen),
				       dom,
				       (dlen = MAX_DOMAIN_NAME_LEN + 1, &dlen),
				       &acc_type))
	    printf ("%s%s%ls:unused:%lu:99999:,%s::\n",
		    with_dom ? "Unix User" : "",
		    with_dom ? sep : "",
		    user + 10,
		    id_offset +
		    *GetSidSubAuthority (psid,
					 *GetSidSubAuthorityCount(psid) - 1),
		    put_sid (psid));
	}
      else
	{
	  DWORD start, stop;
	  char *p = ustr;
	  if (*p == '-')
	    start = 0;
	  else
	    start = strtol (p, &p, 10);
	  if (!*p)
	    stop = start;
	  else if (*p++ != '-' || !isdigit (*p)
		   || (stop = strtol (p, &p, 10)) < start || *p)
	    {
	      fprintf (stderr, "%s: Malformed unix user list entry '%s'.  "
			       "Skipping...\n", __progname, ustr);
	      continue;
	    }
	  for (; start <= stop; ++ start)
	    {
	      *GetSidSubAuthority (psid, *GetSidSubAuthorityCount(psid) - 1)
	      = start;
	      if (LookupAccountSidW (servername, psid,
				     user, (ulen = GNLEN + 1, &ulen),
				     dom,
				     (dlen = MAX_DOMAIN_NAME_LEN + 1, &dlen),
				     &acc_type)
		  && !iswdigit (user[0]))
		printf ("%s%s%ls:unused:%lu:99999:,%s::\n",
			with_dom ? "Unix User" : "",
			with_dom ? sep : "",
			user,
			id_offset + start,
			put_sid (psid));
	    }
	}
    }

  free (user_list);
  FreeSid (psid);
}

static int
enum_users (BOOL domain, domlist_t *dom_or_machine, const char *sep,
	    int print_cygpath, const char *passed_home_path, DWORD id_offset,
	    char *disp_username, int print_current)
{
  WCHAR machine[INTERNET_MAX_HOST_NAME_LENGTH + 1];
  PWCHAR servername = NULL;
  char *d_or_m = dom_or_machine ? dom_or_machine->str : NULL;
  BOOL with_dom = dom_or_machine ? dom_or_machine->with_dom : FALSE;
  USER_INFO_3 *buffer;
  DWORD entriesread = 0;
  DWORD totalentries = 0;
  DWORD resume_handle = 0;
  DWORD rc;
  WCHAR uni_name[UNLEN + 1];
  if (domain)
    {
      servername = get_dcname (d_or_m);
      if (servername == (PWCHAR) -1)
	return 1;
    }
  else if (d_or_m)
    {
      int ret = mbstowcs (machine, d_or_m, INTERNET_MAX_HOST_NAME_LENGTH + 1);
      if (ret < 1 || ret >= INTERNET_MAX_HOST_NAME_LENGTH + 1)
	{
	  fprintf (stderr, "%s: Invalid machine name '%s'.  Skipping...\n",
		   __progname, d_or_m);
	  return 1;
	}
      servername = machine;
    }

  do
    {
      DWORD i;

      if (disp_username != NULL)
	{
	  mbstowcs (uni_name, disp_username, UNLEN + 1);
	  rc = NetUserGetInfo (servername, (LPWSTR) &uni_name, 3,
			       (void *) &buffer);
	  entriesread = 1;
	}
      else
	rc = NetUserEnum (servername, 3, FILTER_NORMAL_ACCOUNT,
			  (void *) &buffer, MAX_PREFERRED_LENGTH,
			  &entriesread, &totalentries, &resume_handle);
      switch (rc)
	{
	case ERROR_ACCESS_DENIED:
	  print_win_error(rc);
	  return 1;

	case ERROR_MORE_DATA:
	case ERROR_SUCCESS:
	  break;

	default:
	  print_win_error(rc);
	  return 1;
	}

      for (i = 0; i < entriesread; i++)
	{
	  char homedir_psx[PATH_MAX];
	  char homedir_w32[MAX_PATH];
	  WCHAR domain_name[MAX_DOMAIN_NAME_LEN + 1];
	  DWORD domname_len = MAX_DOMAIN_NAME_LEN + 1;
	  char psid_buffer[MAX_SID_LEN];
	  PSID psid = (PSID) psid_buffer;
	  DWORD sid_length = MAX_SID_LEN;
	  SID_NAME_USE acc_type;

	  int uid = buffer[i].usri3_user_id;
	  int gid = buffer[i].usri3_primary_group_id;
	  homedir_w32[0] = homedir_psx[0] = '\0';
	  if (passed_home_path[0] == '\0')
	    {
	      uni2ansi (buffer[i].usri3_home_dir, homedir_w32,
			sizeof (homedir_w32));
	      if (homedir_w32[0] != '\0')
		{
		  if (print_cygpath)
		    cygwin_conv_path (CCP_WIN_A_TO_POSIX | CCP_ABSOLUTE,
				      homedir_w32, homedir_psx, PATH_MAX);
		  else
		    psx_dir (homedir_w32, homedir_psx);
		}
	      else
		uni2ansi (buffer[i].usri3_name,
			  stpcpy (homedir_psx, "/home/"), PATH_MAX - 6);
	    }
	  else
	    uni2ansi (buffer[i].usri3_name,
		      stpcpy (homedir_psx, passed_home_path),
		      PATH_MAX - strlen (passed_home_path));

	  if (!LookupAccountNameW (servername, buffer[i].usri3_name,
				   psid, &sid_length, domain_name,
				   &domname_len, &acc_type))
	    {
	      print_win_error(GetLastError ());
	      fprintf(stderr, " (%ls)\n", buffer[i].usri3_name);
	      continue;
	    }
	  else if (acc_type == SidTypeDomain)
	    {
	      WCHAR domname[MAX_DOMAIN_NAME_LEN + UNLEN + 2];

	      wcscpy (domname, domain || !servername
			       ? domain_name : servername);
	      wcscat (domname, L"\\");
	      wcscat (domname, buffer[i].usri3_name);
	      sid_length = MAX_SID_LEN;
	      domname_len = sizeof (domname);
	      if (!LookupAccountNameW (servername, domname, psid,
				       &sid_length, domain_name,
				       &domname_len, &acc_type))
		{
		  print_win_error(GetLastError ());
		  fprintf(stderr, " (%ls)\n", domname);
		  continue;
		}
	    }
	  if (!print_current)
	    /* fall through */;
	  else if (EqualSid (curr_user.psid, psid))
	    got_curr_user = TRUE;

	  printf ("%ls%s%ls:unused:%lu:%lu:%ls%sU-%ls\\%ls,%s:%s:/bin/bash\n",
		  with_dom ? domain_name : L"",
		  with_dom ? sep : "",
		  buffer[i].usri3_name,
		  id_offset + uid,
		  id_offset + gid,
		  buffer[i].usri3_full_name ?: L"",
		  buffer[i].usri3_full_name
		  && buffer[i].usri3_full_name[0] ? "," : "",
		  domain_name,
		  buffer[i].usri3_name,
		  put_sid (psid),
		  homedir_psx);
	}

      NetApiBufferFree (buffer);

    }
  while (rc == ERROR_MORE_DATA);

  return 0;
}

static void
print_special (PSID_IDENTIFIER_AUTHORITY auth, BYTE cnt,
	       DWORD sub1, DWORD sub2, DWORD sub3, DWORD sub4,
	       DWORD sub5, DWORD sub6, DWORD sub7, DWORD sub8)
{
  WCHAR user[UNLEN + 1], dom[MAX_DOMAIN_NAME_LEN + 1];
  DWORD len, len2, rid;
  PSID sid;
  SID_NAME_USE acc_type;

  if (AllocateAndInitializeSid (auth, cnt, sub1, sub2, sub3, sub4,
				sub5, sub6, sub7, sub8, &sid))
    {
      if (LookupAccountSidW (NULL, sid,
			     user, (len = UNLEN + 1, &len),
			     dom, (len2 = MAX_DOMAIN_NAME_LEN + 1, &len),
			     &acc_type))
	{
	  if (sub8)
	    rid = sub8;
	  else if (sub7)
	    rid = sub7;
	  else if (sub6)
	    rid = sub6;
	  else if (sub5)
	    rid = sub5;
	  else if (sub4)
	    rid = sub4;
	  else if (sub3)
	    rid = sub3;
	  else if (sub2)
	    rid = sub2;
	  else
	    rid = sub1;
	  printf ("%ls:*:%lu:%lu:,%s::\n",
		  user, rid, rid == 18 ? 544 : rid, /* SYSTEM hack */
		  put_sid (sid));
	}
      FreeSid (sid);
    }
}

static int
usage (FILE * stream)
{
  fprintf (stream,
"Usage: mkpasswd [OPTIONS]...\n"
"Print /etc/passwd file to stdout\n"
"\n"
"Options:\n"
"   -l,--local [machine[,offset]]\n"
"                           print local user accounts with uid offset offset\n"
"                           (from local machine if no machine specified)\n"
"   -L,--Local [machine[,offset]]\n"
"                           ditto, but generate username with machine prefix\n"
"   -d,--domain [domain[,offset]]\n"
"                           print domain accounts with uid offset offset\n"
"                           (from current domain if no domain specified)\n"
"   -D,--Domain [domain[,offset]]\n"
"                           ditto, but generate username with domain prefix\n"
"   -c,--current            print current user\n"
"   -C,--Current            ditto, but generate username with machine or\n"
"                           domain prefix\n"
"   -S,--separator char     for -L, -D, -C use character char as domain\\user\n"
"                           separator in username instead of the default '\\'\n"
"   -o,--id-offset offset   change the default offset (10000) added to uids\n"
"                           in domain or foreign server accounts.\n"
"   -u,--username username  only return information for the specified user\n"
"                           one of -l, -L, -d, -D must be specified, too\n"
"   -p,--path-to-home path  use specified path instead of user account home dir\n"
"                           or /home prefix\n"
"   -m,--no-mount           don't use mount points for home dir\n"
"   -U,--unix userlist      additionally print UNIX users when using -l or -L\n"
"                           on a UNIX Samba server\n"
"                           userlist is a comma-separated list of usernames\n"
"                           or uid ranges (root,-25,50-100).\n"
"                           (enumerating large ranges can take a long time!)\n"
"   -s,--no-sids            (ignored)\n"
"   -g,--local-groups       (ignored)\n"
"   -h,--help               displays this message\n"
"   -v,--version            version information and exit\n"
"\n"
"Default is to print local accounts on stand-alone machines, domain accounts\n"
"on domain controllers and domain member machines.\n");
  return 1;
}

static struct option longopts[] = {
  {"current", no_argument, NULL, 'c'},
  {"Current", no_argument, NULL, 'C'},
  {"domain", optional_argument, NULL, 'd'},
  {"Domain", optional_argument, NULL, 'D'},
  {"local-groups", no_argument, NULL, 'g'},
  {"help", no_argument, NULL, 'h'},
  {"local", optional_argument, NULL, 'l'},
  {"Local", optional_argument, NULL, 'L'},
  {"no-mount", no_argument, NULL, 'm'},
  {"id-offset", required_argument, NULL, 'o'},
  {"path-to-home", required_argument, NULL, 'p'},
  {"no-sids", no_argument, NULL, 's'},
  {"separator", required_argument, NULL, 'S'},
  {"username", required_argument, NULL, 'u'},
  {"unix", required_argument, NULL, 'U'},
  {"version", no_argument, NULL, 'v'},
  {0, no_argument, NULL, 0}
};

static char opts[] = "cCd::D::ghl::L::mo:sS:p:u:U:v";

static void
print_version ()
{
  const char *v = strchr (version, ':');
  int len;
  if (!v)
    {
      v = "?";
      len = 1;
    }
  else
    {
      v += 2;
      len = strchr (v, ' ') - v;
    }
  printf ("\
mkpasswd (cygwin) %.*s\n\
passwd File Generator\n\
Copyright 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2005, 2006, 2008 Red Hat, Inc.\n\
Compiled on %s\n\
", len, v, __DATE__);
}

static void
enum_std_accounts ()
{
  /* Generate service starter account entries. */
  printf ("SYSTEM:*:18:544:,S-1-5-18::\n");
  printf ("LocalService:*:19:544:U-NT AUTHORITY\\LocalService,S-1-5-19::\n");
  printf ("NetworkService:*:20:544:U-NT AUTHORITY\\NetworkService,S-1-5-20::\n");
  /* Get 'administrators' group (has localized name). */
  print_special (&sid_nt_auth, 2, SECURITY_BUILTIN_DOMAIN_RID,
		 DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0);
}

static PPOLICY_PRIMARY_DOMAIN_INFO p_dom;

static BOOL
fetch_primary_domain ()
{
  NTSTATUS status;
  LSA_OBJECT_ATTRIBUTES oa = { 0, 0, 0, 0, 0, 0 };
  LSA_HANDLE lsa;

  if (!p_dom)
    {
      status = LsaOpenPolicy (NULL, &oa, POLICY_VIEW_LOCAL_INFORMATION, &lsa);
      if (!NT_SUCCESS (status))
	return FALSE;
      status = LsaQueryInformationPolicy (lsa, PolicyPrimaryDomainInformation,
					  (PVOID *) ((void *) &p_dom));
      LsaClose (lsa);
      if (!NT_SUCCESS (status))
	return FALSE;
    }
  return !!p_dom->Sid;
}

int
main (int argc, char **argv)
{
  int print_domlist = 0;
  domlist_t domlist[32];
  char *opt, *p, *ep;
  int print_cygpath = 1;
  int print_current = 0;
  char *print_unix = NULL;
  const char *sep_char = "\\";
  DWORD id_offset = 10000, off;
  int c, i;
  char *disp_username = NULL;
  char passed_home_path[PATH_MAX];
  BOOL in_domain;
  int optional_args = 0;

  passed_home_path[0] = '\0';
  if (!isatty (1))
    setmode (1, O_BINARY);

  load_dsgetdcname ();
  in_domain = fetch_primary_domain ();
  fetch_current_user_sid ();

  if (argc == 1)
    {
      enum_std_accounts ();
      if (in_domain)
	enum_users (TRUE, NULL, sep_char, print_cygpath, passed_home_path,
		    10000, disp_username, 0);
      else
	enum_users (FALSE, NULL, sep_char, print_cygpath, passed_home_path, 0,
		    disp_username, 0);
      return 0;
    }

  unsetenv ("POSIXLY_CORRECT"); /* To get optional arg processing right. */
  while ((c = getopt_long (argc, argv, opts, longopts, NULL)) != EOF)
    switch (c)
      {
      case 'd':
      case 'D':
      case 'l':
      case 'L':
	if (print_domlist >= 32)
	  {
	    fprintf (stderr, "%s: Can not enumerate from more than 32 "
			     "domains and machines.\n", __progname);
	    return 1;
	  }
	domlist[print_domlist].domain = (c == 'd' || c == 'D');
	opt = optarg ?:
	      argv[optind] && argv[optind][0] != '-' ? argv[optind] : NULL;
	if (argv[optind] && opt == argv[optind])
	  ++optional_args;
	for (i = 0; i < print_domlist; ++i)
	  if (domlist[i].domain == domlist[print_domlist].domain
	      && ((!domlist[i].str && !opt)
		  || (domlist[i].str && opt
		      && (off = strlen (domlist[i].str))
		      && !strncmp (domlist[i].str, opt, off)
		      && (!opt[off] || opt[off] == ','))))
	    {
	      fprintf (stderr, "%s: Duplicate %s '%s'.  Skipping...\n",
		       __progname, domlist[i].domain ? "domain" : "machine",
		       domlist[i].str);
	      goto skip;
	    }
	domlist[print_domlist].str = opt;
	domlist[print_domlist].id_offset = ULONG_MAX;
	if (opt && (p = strchr (opt, ',')))
	  {
	    if (p == opt
		|| !isdigit (p[1])
		|| (domlist[print_domlist].id_offset = strtol (p + 1, &ep, 10)
		    , *ep))
	      {
		fprintf (stderr, "%s: Malformed domain,offset string '%s'.  "
			 "Skipping...\n", __progname, opt);
		break;
	      }
	    *p = '\0';
	  }
	domlist[print_domlist++].with_dom = (c == 'D' || c == 'L');
skip:
	break;
      case 'S':
	sep_char = optarg;
	if (strlen (sep_char) > 1)
	  {
	    fprintf (stderr, "%s: Only one character allowed as domain\\user "
			     "separator character.\n", __progname);
	    return 1;
	  }
	if (*sep_char == ':')
	  {
	    fprintf (stderr, "%s: Colon not allowed as domain\\user separator "
			     "character.\n", __progname);
	    return 1;
	  }
	break;
      case 'U':
	print_unix = optarg;
	break;
      case 'c':
	sep_char = NULL;
	/*FALLTHRU*/
      case 'C':
	print_current = 1;
	break;
      case 'o':
	id_offset = strtoul (optarg, &ep, 10);
	if (*ep)
	  {
	    fprintf (stderr, "%s: Malformed offset '%s'.  "
		     "Skipping...\n", __progname, optarg);
	    return 1;
	  }
	break;
      case 'g':
	break;
      case 's':
	break;
      case 'm':
	print_cygpath = 0;
	break;
      case 'p':
	if (optarg[0] != '/')
	{
	  fprintf (stderr, "%s: '%s' is not a fully qualified path.\n",
		   __progname, optarg);
	  return 1;
	}
	strcpy (passed_home_path, optarg);
	if (optarg[strlen (optarg)-1] != '/')
	  strcat (passed_home_path, "/");
	break;
      case 'u':
	disp_username = optarg;
	break;
      case 'h':
	usage (stdout);
	return 0;
      case 'v':
	print_version ();
	return 0;
      default:
	fprintf (stderr, "Try '%s --help' for more information.\n", __progname);
	return 1;
      }

  optind += optional_args;
  if (argv[optind])
    {
      fprintf (stderr,
	       "mkpasswd: non-option command line argument `%s' is not allowed.\n"
	       "Try `mkpasswd --help' for more information.\n", argv[optind]);
      exit (1);
    }

  off = id_offset;
  for (i = 0; i < print_domlist; ++i)
    {
      DWORD my_off = (domlist[i].domain || domlist[i].str)
		     ? domlist[i].id_offset != ULONG_MAX
		       ? domlist[i].id_offset : off : 0;
      if (!domlist[i].domain && domlist[i].str && print_unix)
	enum_unix_users (domlist + i, sep_char, my_off, print_unix);
      if (!my_off && !disp_username)
	enum_std_accounts ();
      enum_users (domlist[i].domain, domlist + i, sep_char, print_cygpath,
		  passed_home_path, my_off, disp_username, print_current);
      if (my_off)
	off += id_offset;
    }

  if (print_current && !got_curr_user)
    current_user (print_cygpath, sep_char, passed_home_path, off,
		  disp_username);

  return 0;
}
