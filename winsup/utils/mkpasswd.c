/* mkpasswd.c:

   Copyright 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2005, 2006, 2008, 2009,
   2010, 2011, 2012, 2013, 2014 Red Hat, Inc.

   This file is part of Cygwin.

   This software is a copyrighted work licensed under the terms of the
   Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
   details. */

#define _WIN32_WINNT 0x0600
#include <errno.h>
#include <ctype.h>
#include <stdlib.h>
#include <wchar.h>
#include <wctype.h>
#include <locale.h>
#include <stdio.h>
#include <unistd.h>
#include <inttypes.h>
#include <getopt.h>
#include <io.h>
#include <pwd.h>
#include <sys/fcntl.h>
#include <sys/cygwin.h>
#include <cygwin/version.h>
#include <windows.h>
#include <lm.h>
#include <iptypes.h>
#include <wininet.h>
#include <ntsecapi.h>
#include <dsgetdc.h>
#include <ntdef.h>

#define print_win_error(x) _print_win_error(x, __LINE__)

#define MAX_SID_LEN 40

SID_IDENTIFIER_AUTHORITY sid_world_auth = {SECURITY_WORLD_SID_AUTHORITY};
SID_IDENTIFIER_AUTHORITY sid_nt_auth = {SECURITY_NT_AUTHORITY};

#ifndef min
#define min(a,b) (((a)<(b))?(a):(b))
#endif

typedef struct
{
  char *str;
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
    fprintf (stderr, "mkpasswd (%d): [%" PRIu32 "] %s",
	     line, (unsigned int) code, buf);
  else
    fprintf (stderr, "mkpasswd (%d): error %" PRIu32,
	     line, (unsigned int) code);
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
      sprintf(t, "-%" PRIu32, (unsigned int) *GetSidSubAuthority (sid, i));
      strcat (s, t);
    }
  return s;
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
enum_unix_users (domlist_t *mach, const char *sep, DWORD id_offset,
		 char *unix_user_list)
{
  WCHAR machine[INTERNET_MAX_HOST_NAME_LENGTH + 1];
  SID_IDENTIFIER_AUTHORITY auth = { { 0, 0, 0, 0, 0, 22 } };
  char *ustr, *user_list;
  WCHAR user[UNLEN + sizeof ("Unix User\\") + 1];
  WCHAR dom[MAX_DOMAIN_NAME_LEN + 1];
  DWORD ulen, dlen, sidlen;
  PSID psid;
  char psid_buffer[MAX_SID_LEN];
  SID_NAME_USE acc_type;

  int ret = mbstowcs (machine, mach->str, INTERNET_MAX_HOST_NAME_LENGTH + 1);
  if (ret < 1 || ret >= INTERNET_MAX_HOST_NAME_LENGTH + 1)
    {
      fprintf (stderr, "%s: Invalid machine name '%s'.  Skipping...\n",
	       program_invocation_short_name, mach->str);
      return;
    }

  if (!AllocateAndInitializeSid (&auth, 2, 1, 0, 0, 0, 0, 0, 0, 0, &psid))
    return;

  if (!(user_list = strdup (unix_user_list)))
    {
      FreeSid (psid);
      return;
    }

  for (ustr = strtok (user_list, ","); ustr; ustr = strtok (NULL, ","))
    {
      if (!isdigit ((unsigned char) ustr[0]) && ustr[0] != '-')
	{
	  PWCHAR p = wcpcpy (user, L"Unix User\\");
	  ret = mbstowcs (p, ustr, UNLEN + 1);
	  if (ret < 1 || ret >= UNLEN + 1)
	    fprintf (stderr, "%s: Invalid user name '%s'.  Skipping...\n",
		     program_invocation_short_name, ustr);
	  else if (LookupAccountNameW (machine, user,
				       psid = (PSID) psid_buffer,
				       (sidlen = MAX_SID_LEN, &sidlen),
				       dom,
				       (dlen = MAX_DOMAIN_NAME_LEN + 1, &dlen),
				       &acc_type))
	    printf ("%s%s%ls:unused:%" PRIu32 ":99999:,%s::\n",
		    mach->with_dom ? "Unix_User" : "",
		    mach->with_dom ? sep : "",
		    user + 10,
		    (unsigned int) (id_offset +
		    *GetSidSubAuthority (psid,
					 *GetSidSubAuthorityCount(psid) - 1)),
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
	  else if (*p++ != '-' || !isdigit ((unsigned char) *p)
		   || (stop = strtol (p, &p, 10)) < start || *p)
	    {
	      fprintf (stderr, "%s: Malformed unix user list entry '%s'.  "
			       "Skipping...\n",
			       program_invocation_short_name, ustr);
	      continue;
	    }
	  for (; start <= stop; ++ start)
	    {
	      *GetSidSubAuthority (psid, *GetSidSubAuthorityCount(psid) - 1)
	      = start;
	      if (LookupAccountSidW (machine, psid,
				     user, (ulen = GNLEN + 1, &ulen),
				     dom,
				     (dlen = MAX_DOMAIN_NAME_LEN + 1, &dlen),
				     &acc_type)
		  && !iswdigit (user[0]))
		printf ("%s%s%ls:unused:%" PRIu32 ":99999:,%s::\n",
			mach->with_dom ? "Unix_User" : "",
			mach->with_dom ? sep : "",
			user,
			(unsigned int) (id_offset + start),
			put_sid (psid));
	    }
	}
    }

  free (user_list);
  FreeSid (psid);
}

static int
enum_users (domlist_t *mach, const char *sep, const char *passed_home_path,
	    DWORD id_offset, char *disp_username, int print_current)
{
  WCHAR machine[INTERNET_MAX_HOST_NAME_LENGTH + 1];
  USER_INFO_3 *buffer;
  DWORD entriesread = 0;
  DWORD totalentries = 0;
  DWORD resume_handle = 0;
  DWORD rc;
  WCHAR uni_name[UNLEN + 1];

  int ret = mbstowcs (machine, mach->str, INTERNET_MAX_HOST_NAME_LENGTH + 1);
  if (ret < 1 || ret >= INTERNET_MAX_HOST_NAME_LENGTH + 1)
    {
      fprintf (stderr, "%s: Invalid machine name '%s'.  Skipping...\n",
	       program_invocation_short_name, mach->str);
      return 1;
    }

  do
    {
      DWORD i;

      if (disp_username != NULL)
	{
	  mbstowcs (uni_name, disp_username, UNLEN + 1);
	  rc = NetUserGetInfo (machine, (LPWSTR) &uni_name, 3,
			       (void *) &buffer);
	  entriesread = 1;
	  /* Avoid annoying error messages just because the user hasn't been
	     found. */
	  if (rc == NERR_UserNotFound)
	    return 0;
	}
      else
	rc = NetUserEnum (machine, 3, FILTER_NORMAL_ACCOUNT,
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
	  WCHAR domain_name[MAX_DOMAIN_NAME_LEN + 1];
	  DWORD domname_len = MAX_DOMAIN_NAME_LEN + 1;
	  char psid_buffer[MAX_SID_LEN];
	  PSID psid = (PSID) psid_buffer;
	  DWORD sid_length = MAX_SID_LEN;
	  SID_NAME_USE acc_type;

	  int uid = buffer[i].usri3_user_id;
	  int gid = buffer[i].usri3_primary_group_id;
	  homedir_psx[0] = '\0';
	  if (passed_home_path[0] == '\0')
	    {
	      if (buffer[i].usri3_home_dir[0] != L'\0')
		cygwin_conv_path (CCP_WIN_W_TO_POSIX | CCP_ABSOLUTE,
				  buffer[i].usri3_home_dir, homedir_psx,
				  PATH_MAX);
	      else
		uni2ansi (buffer[i].usri3_name,
			  stpcpy (homedir_psx, "/home/"), PATH_MAX - 6);
	    }
	  else
	    uni2ansi (buffer[i].usri3_name,
		      stpcpy (homedir_psx, passed_home_path),
		      PATH_MAX - strlen (passed_home_path));

	  if (!LookupAccountNameW (machine, buffer[i].usri3_name,
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

	      wcscpy (domname, machine);
	      wcscat (domname, L"\\");
	      wcscat (domname, buffer[i].usri3_name);
	      sid_length = MAX_SID_LEN;
	      domname_len = sizeof (domname);
	      if (!LookupAccountNameW (machine, domname, psid,
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

	  printf ("%ls%s%ls:unused:%" PRIu32 ":%" PRIu32
		  ":%ls%sU-%ls\\%ls,%s:%s:/bin/bash\n",
		  mach->with_dom ? domain_name : L"",
		  mach->with_dom ? sep : "",
		  buffer[i].usri3_name,
		  (unsigned int) (id_offset + uid),
		  (unsigned int) (id_offset + gid),
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

static int
usage (FILE * stream)
{
  fprintf (stream,
"Usage: %s [OPTIONS]...\n"
"\n"
"Write /etc/passwd-like output to stdout\n"
"\n"
"Options:\n"
"\n"
"   -l,--local [machine]    print local user accounts with uid offset offset\n"
"                           (from local machine if no machine specified)\n"
"   -L,--Local machine      ditto, but generate username with machine prefix\n"
"   -d,--domain [domain]    print domain accounts with uid offset offset\n"
"                           (from current domain if no domain specified)\n"
"   -c,--current            print current user\n"
"   -S,--separator char     for -l use character char as domain\\user\n"
"                           separator in username instead of the default '%s'\n"
"   -o,--id-offset offset   change the default offset (0x10000) added to uids\n"
"                           in domain or foreign server accounts.\n"
"   -u,--username username  only return information for the specified user\n"
"                           one of -l, -d must be specified, too\n"
"   -b,--no-builtin         don't print BUILTIN users\n"
"   -p,--path-to-home path  use specified path instead of user account home dir\n"
"                           or /home prefix\n"
"   -U,--unix userlist      print UNIX users when using -l on a UNIX Samba\n"
"                           server.  userlist is a comma-separated list of\n"
"                           usernames or uid ranges (root,-25,50-100).\n"
"                           (enumerating large ranges can take a long time!)\n"
"   -h,--help               displays this message\n"
"   -V,--version            version information and exit\n"
"\n"
"Default is to print local accounts on stand-alone machines, domain accounts\n"
"on domain controllers and domain member machines.\n\n"
"Don't use this command to generate a local /etc/passwd file, unless you\n"
"really need one.  See the Cygwin User's Guide for more information.\n"
"\n", program_invocation_short_name,
      (const char *) cygwin_internal (CW_GETNSSSEP));
  return 1;
}

static struct option longopts[] = {
  {"no-builtin", no_argument, NULL, 'b'},
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
  {"version", no_argument, NULL, 'V'},
  {0, no_argument, NULL, 0}
};

static char opts[] = "bcCd::D::ghl::L::mo:sS:p:u:U:V";

static void
print_version ()
{
  printf ("mkpasswd (cygwin) %d.%d.%d\n"
	  "Passwd File Generator\n"
	  "Copyright (C) 1997 - %s Red Hat, Inc.\n"
	  "This is free software; see the source for copying conditions.  There is NO\n"
	  "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n",
	  CYGWIN_VERSION_DLL_MAJOR / 1000,
	  CYGWIN_VERSION_DLL_MAJOR % 1000,
	  CYGWIN_VERSION_DLL_MINOR,
	  strrchr (__DATE__, ' ') + 1);
}

int
main (int argc, char **argv)
{
  int print_domlist = 0;
  domlist_t domlist[32];
  char *opt, *p, *ep;
  int print_current = 0;
  int print_builtin = 1;
  char *print_unix = NULL;
  const char *nss_sep = (const char *) cygwin_internal (CW_GETNSSSEP);
  const char *sep_char = nss_sep;
  DWORD id_offset = 0x10000, off;
  int c, i;
  char *disp_username = NULL;
  char passed_home_path[PATH_MAX];
  int optional_args = 0;

  passed_home_path[0] = '\0';
  if (!isatty (1))
    setmode (1, O_BINARY);

  /* Use locale from environment.  If not set or set to "C", use UTF-8. */
  setlocale (LC_CTYPE, "");
  if (!strcmp (setlocale (LC_CTYPE, NULL), "C"))
    setlocale (LC_CTYPE, "en_US.UTF-8");
  fetch_current_user_sid ();

  if (argc == 1)
    {
      int enums = ENUM_PRIMARY | ENUM_LOCAL | ENUM_BUILTIN;
      uintptr_t ticket = cygwin_internal (CW_SETENT, FALSE, enums, NULL);
      if (ticket)
	{
	  struct passwd *pwd;

	  while ((pwd = (struct passwd *) cygwin_internal (CW_GETENT, FALSE,
							   ticket)))
	    printf ("%s:%s:%u:%u:%s:%s:%s\n", pwd->pw_name, pwd->pw_passwd,
		    pwd->pw_uid, pwd->pw_gid, pwd->pw_gecos, pwd->pw_dir,
		    pwd->pw_shell);
	  cygwin_internal (CW_ENDENT, FALSE, ticket);
	}
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
			     "domains and machines.\n",
			     program_invocation_short_name);
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
		       program_invocation_short_name,
		       domlist[i].domain ? "domain" : "machine",
		       domlist[i].str);
	      goto skip;
	    }
	domlist[print_domlist].str = opt;
	if (opt && (p = strchr (opt, ',')))
	  {
	    if (p == opt)
	      {
		fprintf (stderr, "%s: Malformed domain,offset string '%s'.  "
			 "Skipping...\n", program_invocation_short_name, opt);
		break;
	      }
	    *p = '\0';
	  }
	domlist[print_domlist++].with_dom = (c == 'L');
skip:
	break;
      case 'S':
	sep_char = optarg;
	if (strlen (sep_char) > 1)
	  {
	    fprintf (stderr, "%s: Only one ASCII character allowed as "
			     "domain\\user separator character.\n",
			     program_invocation_short_name);
	    return 1;
	  }
	if (*sep_char == ':')
	  {
	    fprintf (stderr, "%s: Colon not allowed as domain\\user separator "
			     "character.\n", program_invocation_short_name);
	    return 1;
	  }
	break;
      case 'U':
	print_unix = optarg;
	break;
      case 'c':
      case 'C':
	print_current = 1;
	break;
      case 'o':
	id_offset = strtoul (optarg, &ep, 10);
	break;
      case 'b':
	print_builtin = 0;
	break;
      case 'p':
	if (optarg[0] != '/')
	{
	  fprintf (stderr, "%s: '%s' is not a fully qualified path.\n",
		   program_invocation_short_name, optarg);
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
      case 'V':
	print_version ();
	return 0;
      case 'g':		/* deprecated */
      case 's':		/* deprecated */
      case 'm':		/* deprecated */
	break;
      default:
	fprintf (stderr, "Try `%s --help' for more information.\n",
		 program_invocation_short_name);
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

  struct passwd *ppwd = NULL;
  const char *ppwd_sid = NULL;
  if (print_current)
    {
      ppwd = (struct passwd *) cygwin_internal (CW_GETPWSID, TRUE,
						curr_user.psid);
      if (ppwd)
	ppwd_sid = strrchr (ppwd->pw_gecos, ',');
    }

  int enums = ENUM_NONE;
  WCHAR tdoms[print_domlist * 258];
  PWCHAR t = tdoms;
  if (!disp_username && print_builtin && print_domlist)
    enums |= ENUM_BUILTIN;
  for (i = 0; i < print_domlist; ++i)
    {
      if (domlist[i].domain)
        {
          if (domlist[i].str)
            {
              enums |= ENUM_TDOMS;
              t += mbstowcs (t, domlist[i].str, 257);
              *t++ = L'\0';
            }
          else
            enums |= ENUM_PRIMARY;
        }
      else if (!domlist[i].str)
        enums |= ENUM_LOCAL;
    }
  if (t > tdoms)
    *t++ = L'\0';
  if (enums)
    {
      uintptr_t ticket = cygwin_internal (CW_SETENT, FALSE, enums,
                                          t > tdoms ? tdoms : NULL);
      if (ticket)
        {
          struct passwd *pwd;

          while ((pwd = (struct passwd *)
                        cygwin_internal (CW_GETENT, FALSE, ticket)))
            {
	      p = NULL;
              if (disp_username
                  && strcasecmp (disp_username, pwd->pw_name) != 0
                  && (!(p = strchr (pwd->pw_name, nss_sep[0]))
                      || strcasecmp (disp_username, p + 1) != 0))
                continue;
	      printf ("%s:%s:%u:%u:%s:%s%s:%s\n", pwd->pw_name, pwd->pw_passwd,
		      pwd->pw_uid, pwd->pw_gid, pwd->pw_gecos,
		      passed_home_path[0] ? passed_home_path : "",
		      passed_home_path[0] ? (p ? p + 1 : pwd->pw_name)
					  : pwd->pw_dir,
		      pwd->pw_shell);
	      const char *pwd_sid = strrchr (pwd->pw_gecos, ',');
              if (ppwd && ppwd_sid && pwd_sid && !strcmp (pwd_sid, ppwd_sid))
                got_curr_user = TRUE;
            }
          cygwin_internal (CW_ENDENT, FALSE, ticket);
        }
    }

  if (print_current && !got_curr_user)
    {
      p = strchr (ppwd->pw_name, nss_sep[0]);
      printf ("%s:%s:%u:%u:%s:%s%s:%s\n", ppwd->pw_name, ppwd->pw_passwd,
	      ppwd->pw_uid, ppwd->pw_gid, ppwd->pw_gecos,
	      passed_home_path[0] ? passed_home_path : "",
	      passed_home_path[0] ? (p ? p + 1 : ppwd->pw_name) : ppwd->pw_dir,
	      ppwd->pw_shell);
    }

  off = 0xfd000000;
  for (i = 0; i < print_domlist; ++i)
    {
      if (domlist[i].domain || !domlist[i].str)
	continue;
      enum_users (domlist + i, sep_char, passed_home_path, off, disp_username,
		  print_current);
      if (!domlist[i].domain && domlist[i].str && print_unix)
	enum_unix_users (domlist + i, sep_char, 0xff000000, print_unix);
      off += id_offset;
    }

  return 0;
}
