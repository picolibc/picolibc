/* mkgroup.c:

   Copyright 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007,
   2008, 2009, 2010, 2011, 2012, 2013, 2014 Red Hat, Inc.

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
#include <grp.h>
#include <sys/fcntl.h>
#include <sys/cygwin.h>
#include <cygwin/version.h>
#include <windows.h>
#include <lm.h>
#include <wininet.h>
#include <iptypes.h>
#include <ntsecapi.h>
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
_print_win_error (DWORD code, int line)
{
  char buf[4096];

  if (FormatMessage (FORMAT_MESSAGE_FROM_SYSTEM
      | FORMAT_MESSAGE_IGNORE_INSERTS,
      NULL,
      code,
      MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT),
      (LPTSTR) buf, sizeof (buf), NULL))
    fprintf (stderr, "mkgroup (%d): [%" PRIu32 "] %s",
	     line, (unsigned int) code, buf);
  else
    fprintf (stderr, "mkgroup (%d): error %" PRIu32 ,
	     line, (unsigned int) code);
}

static char *
put_sid (PSID psid)
{
  static char s[512];
  char t[32];
  DWORD i;

  strcpy (s, "S-1-");
  sprintf(t, "%u", GetSidIdentifierAuthority (psid)->Value[5]);
  strcat (s, t);
  for (i = 0; i < *GetSidSubAuthorityCount (psid); ++i)
    {
      sprintf(t, "-%" PRIu32 , (unsigned int) *GetSidSubAuthority (psid, i));
      strcat (s, t);
    }
  return s;
}

typedef struct {
  BYTE  Revision;
  BYTE  SubAuthorityCount;
  SID_IDENTIFIER_AUTHORITY IdentifierAuthority;
  DWORD SubAuthority[8];
} DBGSID, *PDBGSID;

#define MAX_BUILTIN_SIDS 100	/* Should be enough for the forseable future. */
DBGSID builtin_sid_list[MAX_BUILTIN_SIDS];
DWORD builtin_sid_cnt;

typedef struct {
  PSID psid;
  int buffer[10];
} sidbuf;

static sidbuf curr_pgrp;
static BOOL got_curr_pgrp = FALSE;

static void
fetch_current_pgrp_sid ()
{
  DWORD len;
  HANDLE ptok;

  if (!OpenProcessToken (GetCurrentProcess (), TOKEN_QUERY, &ptok)
      || !GetTokenInformation (ptok, TokenPrimaryGroup, &curr_pgrp,
			       sizeof curr_pgrp, &len)
      || !CloseHandle (ptok))
    {
      print_win_error (GetLastError ());
      return;
    }
}

static void
enum_unix_groups (domlist_t *mach, const char *sep, DWORD id_offset,
		  char *unix_grp_list)
{
  WCHAR machine[INTERNET_MAX_HOST_NAME_LENGTH + 1];
  SID_IDENTIFIER_AUTHORITY auth = { { 0, 0, 0, 0, 0, 22 } };
  char *gstr, *grp_list;
  WCHAR grp[GNLEN + sizeof ("Unix Group\\") + 1];
  WCHAR dom[MAX_DOMAIN_NAME_LEN + 1];
  DWORD glen, dlen, sidlen;
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

  if (!AllocateAndInitializeSid (&auth, 2, 2, 0, 0, 0, 0, 0, 0, 0, &psid))
    return;

  if (!(grp_list = strdup (unix_grp_list)))
    {
      FreeSid (psid);
      return;
    }

  for (gstr = strtok (grp_list, ","); gstr; gstr = strtok (NULL, ","))
    {
      if (!isdigit ((unsigned char) gstr[0]) && gstr[0] != '-')
	{
	  PWCHAR p = wcpcpy (grp, L"Unix Group\\");
	  ret = mbstowcs (p, gstr, GNLEN + 1);
	  if (ret < 1 || ret >= GNLEN + 1)
	    fprintf (stderr, "%s: Invalid group name '%s'.  Skipping...\n",
		     program_invocation_short_name, gstr);
	  else if (LookupAccountNameW (machine, grp,
				       psid = (PSID) psid_buffer,
				       (sidlen = MAX_SID_LEN, &sidlen),
				       dom,
				       (dlen = MAX_DOMAIN_NAME_LEN + 1, &dlen),
				       &acc_type))
	    printf ("%s%s%ls:%s:%" PRIu32 ":\n",
		    mach->with_dom ? "Unix_Group" : "",
		    mach->with_dom ? sep : "",
		    p,
		    put_sid (psid),
		    (unsigned int) (id_offset +
		    *GetSidSubAuthority (psid,
					 *GetSidSubAuthorityCount(psid) - 1)));
	}
      else
	{
	  DWORD start, stop;
	  char *p = gstr;
	  if (*p == '-')
	    start = 0;
	  else
	    start = strtol (p, &p, 10);
	  if (!*p)
	    stop = start;
	  else if (*p++ != '-' || !isdigit ((unsigned char) *p)
		   || (stop = strtol (p, &p, 10)) < start || *p)
	    {
	      fprintf (stderr, "%s: Malformed unix group list entry '%s'.  "
			       "Skipping...\n",
			       program_invocation_short_name, gstr);
	      continue;
	    }
	  for (; start <= stop; ++ start)
	    {
	      *GetSidSubAuthority (psid, *GetSidSubAuthorityCount(psid) - 1)
	      = start;
	      if (LookupAccountSidW (machine, psid,
				     grp, (glen = GNLEN + 1, &glen),
				     dom,
				     (dlen = MAX_DOMAIN_NAME_LEN + 1, &dlen),
				     &acc_type)
		  && !iswdigit (grp[0]))
		printf ("%s%s%ls:%s:%" PRIu32 ":\n",
			mach->with_dom ? "Unix_Group" : "",
			mach->with_dom ? sep : "",
			grp,
			put_sid (psid),
			(unsigned int) (id_offset + start));
	    }
	}
    }

  free (grp_list);
  FreeSid (psid);
}

static int
enum_local_groups (domlist_t *mach, const char *sep,
		   DWORD id_offset, char *disp_groupname, int print_builtin,
		   int print_current)
{
  WCHAR machine[INTERNET_MAX_HOST_NAME_LENGTH + 1];
  LOCALGROUP_INFO_0 *buffer;
  DWORD entriesread = 0;
  DWORD totalentries = 0;
  DWORD_PTR resume_handle = 0;
  WCHAR gname[GNLEN + 1];
  DWORD rc;

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

      if (disp_groupname)
	{
	  mbstowcs (gname, disp_groupname, GNLEN + 1);
	  rc = NetLocalGroupGetInfo (machine, gname, 0, (void *) &buffer);
	  if (rc == ERROR_SUCCESS)
	    entriesread = 1;
	  /* Allow further searching for the group and avoid annoying
	     error messages just because the group is not a local group or
	     the group hasn't been found. */
	  else if (rc == ERROR_NO_SUCH_ALIAS || rc == NERR_GroupNotFound)
	    return 0;
	}
      else
	rc = NetLocalGroupEnum (machine, 0, (void *) &buffer,
				MAX_PREFERRED_LENGTH, &entriesread,
				&totalentries, &resume_handle);
      switch (rc)
	{
	case ERROR_ACCESS_DENIED:
	  print_win_error (rc);
	  return 1;

	case ERROR_MORE_DATA:
	case ERROR_SUCCESS:
	  break;

	default:
	  print_win_error (rc);
	  return 1;
	}

      for (i = 0; i < entriesread; i++)
	{
	  WCHAR domain_name[MAX_DOMAIN_NAME_LEN + 1];
	  DWORD domname_len = MAX_DOMAIN_NAME_LEN + 1;
	  char psid_buffer[MAX_SID_LEN];
	  PSID psid = (PSID) psid_buffer;
	  DWORD sid_length = MAX_SID_LEN;
	  DWORD gid;
	  SID_NAME_USE acc_type;
	  PDBGSID pdsid;
	  BOOL is_builtin = FALSE;

	  if (!LookupAccountNameW (machine, buffer[i].lgrpi0_name, psid,
				   &sid_length, domain_name, &domname_len,
				   &acc_type))
	    {
	      print_win_error (GetLastError ());
	      fprintf (stderr, " (%ls)\n", buffer[i].lgrpi0_name);
	      continue;
	    }
	  else if (acc_type == SidTypeDomain)
	    {
	      WCHAR domname[MAX_DOMAIN_NAME_LEN + GNLEN + 2];

	      wcscpy (domname, domain_name);
	      wcscat (domname, L"\\");
	      wcscat (domname, buffer[i].lgrpi0_name);
	      sid_length = MAX_SID_LEN;
	      domname_len = MAX_DOMAIN_NAME_LEN + 1;
	      if (!LookupAccountNameW (machine, domname,
				       psid, &sid_length,
				       domain_name, &domname_len,
				       &acc_type))
		{
		  print_win_error (GetLastError ());
		  fprintf(stderr, " (%ls)\n", domname);
		  continue;
		}
	    }

	  /* Store all local SIDs with prefix "S-1-5-32-" and check if it
	     has been printed already.  This allows to get all builtin
	     groups exactly once and not once per domain. */
	  pdsid = (PDBGSID) psid;
	  if (pdsid->IdentifierAuthority.Value[5] == sid_nt_auth.Value[5]
	      && pdsid->SubAuthority[0] == SECURITY_BUILTIN_DOMAIN_RID)
	    {
	      int b;

	      if (!print_builtin)
		goto skip_group;
	      is_builtin = TRUE;
	      if (builtin_sid_cnt)
		for (b = 0; b < builtin_sid_cnt; b++)
		  if (EqualSid (&builtin_sid_list[b], psid))
		    goto skip_group;
	      if (builtin_sid_cnt < MAX_BUILTIN_SIDS)
		CopySid (sizeof (DBGSID), &builtin_sid_list[builtin_sid_cnt++],
			 psid);
	    }
	  if (!print_current)
	    /* fall through */;
	  else if (EqualSid (curr_pgrp.psid, psid))
	    got_curr_pgrp = TRUE;

	  gid = *GetSidSubAuthority (psid, *GetSidSubAuthorityCount(psid) - 1);
	  printf ("%ls%s%ls:%s:%" PRIu32 ":\n",
		  mach->with_dom && !is_builtin ? domain_name : L"",
		  mach->with_dom || is_builtin ? sep : "",
		  buffer[i].lgrpi0_name,
		  put_sid (psid),
		  (unsigned int) (gid + (is_builtin ? 0 : id_offset)));
skip_group:
	  ;
	}

      NetApiBufferFree (buffer);

    }
  while (rc == ERROR_MORE_DATA);

  /* Return 1 if the single group we're looking for has been found here to
     avoid calling enum_groups for the same group, thus avoiding a spurious
     error message "group name could not be found" in enum_groups. */
  return disp_groupname && entriesread ? 1 : 0;
}

static void
enum_groups (domlist_t *mach, const char *sep, DWORD id_offset,
	     char *disp_groupname, int print_current)
{
  WCHAR machine[INTERNET_MAX_HOST_NAME_LENGTH + 1];
  GROUP_INFO_2 *buffer;
  DWORD entriesread = 0;
  DWORD totalentries = 0;
  DWORD_PTR resume_handle = 0;
  WCHAR gname[GNLEN + 1];
  DWORD rc;

  int ret = mbstowcs (machine, mach->str, INTERNET_MAX_HOST_NAME_LENGTH + 1);
  if (ret < 1 || ret >= INTERNET_MAX_HOST_NAME_LENGTH + 1)
    {
      fprintf (stderr, "%s: Invalid machine name '%s'.  Skipping...\n",
	       program_invocation_short_name, mach->str);
      return;
    }

  do
    {
      DWORD i;

      if (disp_groupname != NULL)
	{
	  mbstowcs (gname, disp_groupname, GNLEN + 1);
	  rc = NetGroupGetInfo (machine, (LPWSTR) & gname, 2, (void *) &buffer);
	  entriesread=1;
	  /* Avoid annoying error messages just because the group hasn't been
	     found. */
	  if (rc == NERR_GroupNotFound)
	    return;
	}
      else
	rc = NetGroupEnum (machine, 2, (void *) & buffer, MAX_PREFERRED_LENGTH,
			   &entriesread, &totalentries, &resume_handle);
      switch (rc)
	{
	case ERROR_ACCESS_DENIED:
	  print_win_error (rc);
	  return;

	case ERROR_MORE_DATA:
	case ERROR_SUCCESS:
	  break;

	default:
	  print_win_error (rc);
	  return;
	}

      for (i = 0; i < entriesread; i++)
	{
	  WCHAR domain_name[MAX_DOMAIN_NAME_LEN + 1];
	  DWORD domname_len = MAX_DOMAIN_NAME_LEN + 1;
	  char psid_buffer[MAX_SID_LEN];
	  PSID psid = (PSID) psid_buffer;
	  DWORD sid_length = MAX_SID_LEN;
	  SID_NAME_USE acc_type;

	  int gid = buffer[i].grpi2_group_id;
	  if (!LookupAccountNameW (machine, buffer[i].grpi2_name,
				   psid, &sid_length,
				   domain_name, &domname_len,
				   &acc_type))
	    {
	      print_win_error (GetLastError ());
	      fprintf(stderr, " (%ls)\n", buffer[i].grpi2_name);
	      continue;
	    }
	  else if (acc_type == SidTypeDomain)
	    {
	      WCHAR domname[MAX_DOMAIN_NAME_LEN + GNLEN + 2];

	      wcscpy (domname, machine);
	      wcscat (domname, L"\\");
	      wcscat (domname, buffer[i].grpi2_name);
	      sid_length = MAX_SID_LEN;
	      domname_len = MAX_DOMAIN_NAME_LEN + 1;
	      if (!LookupAccountNameW (machine, domname, psid, &sid_length,
				       domain_name, &domname_len, &acc_type))
		{
		  print_win_error (GetLastError ());
		  fprintf(stderr, " (%ls)\n", domname);
		  continue;
		}
	    }
	  if (!print_current)
	    /* fall through */;
	  else if (EqualSid (curr_pgrp.psid, psid))
	    got_curr_pgrp = TRUE;

	  printf ("%ls%s%ls:%s:%" PRIu32 ":\n",
		  mach->with_dom ? domain_name : L"",
		  mach->with_dom ? sep : "",
		  buffer[i].grpi2_name,
		  put_sid (psid),
		  (unsigned int) (id_offset + gid));
	}

      NetApiBufferFree (buffer);

    }
  while (rc == ERROR_MORE_DATA);
}

static int
usage (FILE * stream)
{
  fprintf (stream,
"Usage: %s [OPTION]...\n"
"\n"
"Write /etc/group-like output to stdout\n"
"\n"
"Options:\n"
"\n"
"   -l,--local [machine]    print local groups\n"
"                           (from local machine if no machine specified)\n"
"   -L,--Local machine      ditto, but generate groupname with machine prefix\n"
"   -d,--domain [domain]    print domain groups\n"
"                           (from current domain if no domain specified)\n"
"   -c,--current            print current group\n"
"   -S,--separator char     for -l use character char as domain\\group\n"
"                           separator in groupname instead of default '%s'\n"
"   -o,--id-offset offset   change the default offset (0x10000) added to\n"
"                           gids of foreign machine accounts.\n"
"   -g,--group groupname    only return information for the specified group\n"
"                           one of -l, -d must be specified, too\n"
"   -b,--no-builtin         don't print BUILTIN groups\n"
"   -U,--unix grouplist     print UNIX groups when using -l on a UNIX Samba\n"
"                           server.  grouplist is a comma-separated list of\n"
"                           groupnames or gid ranges (root,-25,50-100).\n"
"                           (enumerating large ranges can take a long time!)\n"
"   -h,--help               print this message\n"
"   -v,--version            print version information and exit\n"
"\n"
"Default is to print local groups on stand-alone machines, plus domain\n"
"groups on domain controllers and domain member machines.\n\n"
"Don't use this command to generate a local /etc/group file, unless you\n"
"really need one.  See the Cygwin User's Guide for more information.\n"
"\n", program_invocation_short_name,
      (const char *) cygwin_internal (CW_GETNSSSEP));
  return 1;
}

struct option longopts[] = {
  {"no-builtin", no_argument, NULL, 'b'},
  {"current", no_argument, NULL, 'c'},
  {"Current", no_argument, NULL, 'C'},
  {"domain", optional_argument, NULL, 'd'},
  {"Domain", optional_argument, NULL, 'D'},
  {"group", required_argument, NULL, 'g'},
  {"help", no_argument, NULL, 'h'},
  {"local", optional_argument, NULL, 'l'},
  {"Local", optional_argument, NULL, 'L'},
  {"id-offset", required_argument, NULL, 'o'},
  {"no-sids", no_argument, NULL, 's'},
  {"separator", required_argument, NULL, 'S'},
  {"users", no_argument, NULL, 'u'},
  {"unix", required_argument, NULL, 'U'},
  {"version", no_argument, NULL, 'V'},
  {0, no_argument, NULL, 0}
};

static char opts[] = "bcCd::D::g:hl::L::o:sS:uU:V";

static void
print_version ()
{
  printf ("mkgroup (cygwin) %d.%d.%d\n"
	  "Group File Generator\n"
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
  char *opt, *p;
  int print_current = 0;
  int print_builtin = 1;
  char *print_unix = NULL;
  const char *sep_char = (const char *) cygwin_internal (CW_GETNSSSEP);
  DWORD id_offset = 0x10000, off;
  int c, i;
  char *disp_groupname = NULL;
  //BOOL in_domain;
  int optional_args = 0;

  if (!isatty (1))
    setmode (1, O_BINARY);

  /* Use locale from environment.  If not set or set to "C", use UTF-8. */
  setlocale (LC_CTYPE, "");
  if (!strcmp (setlocale (LC_CTYPE, NULL), "C"))
    setlocale (LC_CTYPE, "en_US.UTF-8");
  fetch_current_pgrp_sid ();

  if (argc == 1)
    {
      int enums = ENUM_PRIMARY | ENUM_LOCAL | ENUM_BUILTIN;
      uintptr_t ticket = cygwin_internal (CW_SETENT, TRUE, enums, NULL);
      if (ticket)
	{
	  struct group *grp;

	  while ((grp = (struct group *) cygwin_internal (CW_GETENT, TRUE,
							  ticket)))
	    printf ("%s:%s:%u:\n", grp->gr_name, grp->gr_passwd, grp->gr_gid);
	  cygwin_internal (CW_ENDENT, TRUE, ticket);
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
		fprintf (stderr, "%s: Malformed machine,offset string '%s'.  "
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
	id_offset = strtol (optarg, NULL, 10);
	break;
      case 'b':
	print_builtin = 0;
	break;
      case 's':
	break;
      case 'u':
	break;
      case 'g':
	disp_groupname = optarg;
	break;
      case 'h':
	usage (stdout);
	return 0;
      case 'V':
	print_version ();
	return 0;
      default:
	fprintf (stderr, "Try `%s --help' for more information.\n", argv[0]);
	return 1;
      }

  optind += optional_args;
  if (argv[optind])
    {
      fprintf (stderr,
	       "mkgroup: non-option command line argument `%s' is not allowed.\n"
	       "Try `mkgroup --help' for more information.\n", argv[optind]);
      exit (1);
    }

  struct group *pgrp = NULL;
  if (print_current)
    pgrp = (struct group *) cygwin_internal (CW_GETGRSID, TRUE, curr_pgrp.psid);

  int enums = ENUM_NONE;
  WCHAR tdoms[print_domlist * 258];
  PWCHAR t = tdoms;
  if (!disp_groupname && print_builtin && print_domlist)
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
      uintptr_t ticket = cygwin_internal (CW_SETENT, TRUE, enums,
					  t > tdoms ? tdoms : NULL);
      if (ticket)
	{
	  struct group *grp;
	  const char *nss_sep = (const char *) cygwin_internal (CW_GETNSSSEP);

	  while ((grp = (struct group *)
			cygwin_internal (CW_GETENT, TRUE, ticket)))
	    {
	      if (disp_groupname
		  && strcasecmp (disp_groupname, grp->gr_name) != 0
		  && (!(p = strchr (grp->gr_name, nss_sep[0]))
		      || strcasecmp (disp_groupname, p + 1) != 0))
		continue;
	      printf ("%s:%s:%u:\n", grp->gr_name, grp->gr_passwd,
				     grp->gr_gid);
	      if (pgrp && !strcmp (grp->gr_passwd, pgrp->gr_passwd))
		got_curr_pgrp = TRUE;
	    }
	  cygwin_internal (CW_ENDENT, TRUE, ticket);
	}
    }

  if (print_current && !got_curr_pgrp)
    printf ("%s:%s:%u:\n", pgrp->gr_name, pgrp->gr_passwd, pgrp->gr_gid);

  off = 0xfd000000;
  for (i = 0; i < print_domlist; ++i)
    {
      if (domlist[i].domain || !domlist[i].str)
	continue;
      if (!enum_local_groups (domlist + i, sep_char, off, disp_groupname,
			      print_builtin, print_current))
	{
	  enum_groups (domlist + i, sep_char, off, disp_groupname,
		       print_current);
	  if (!domlist[i].domain && domlist[i].str && print_unix)
	    enum_unix_groups (domlist + i, sep_char, 0xff000000, print_unix);
	  off += id_offset;
	}
    }

  return 0;
}
