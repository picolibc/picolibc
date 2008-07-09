/* mkgroup.c:

   Copyright 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006,
   2007, 2008 Red Hat, Inc.

   This file is part of Cygwin.

   This software is a copyrighted work licensed under the terms of the
   Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
   details. */

#include <ctype.h>
#include <stdlib.h>
#include <wchar.h>
#include <stdio.h>
#include <windows.h>
#include <sys/cygwin.h>
#include <getopt.h>
#include <lmaccess.h>
#include <lmapibuf.h>
#include <wininet.h>
#include <iptypes.h>
#include <ntsecapi.h>
#include <ntdef.h>

#define print_win_error(x) _print_win_error(x, __LINE__)

static const char version[] = "$Revision$";

#define MAX_SID_LEN 40

typedef struct {
  LPWSTR DomainControllerName;
  LPWSTR DomainControllerAddress;
  ULONG  DomainControllerAddressType;
  GUID   DomainGuid;
  LPWSTR DomainName;
  LPWSTR DnsForestName;
  ULONG  Flags;
  LPWSTR DcSiteName;
  LPWSTR ClientSiteName;
} *PDOMAIN_CONTROLLER_INFOW;

SID_IDENTIFIER_AUTHORITY sid_world_auth = {SECURITY_WORLD_SID_AUTHORITY};
SID_IDENTIFIER_AUTHORITY sid_nt_auth = {SECURITY_NT_AUTHORITY};

NET_API_STATUS WINAPI (*dsgetdcname)(LPWSTR,LPWSTR,GUID*,LPWSTR,ULONG,PDOMAIN_CONTROLLER_INFOW*);

#ifndef min
#define min(a,b) (((a)<(b))?(a):(b))
#endif

void
load_netapi ()
{
  HANDLE h = LoadLibrary ("netapi32.dll");
  if (h)
    dsgetdcname = (void *) GetProcAddress (h, "DsGetDcNameW");
}

char *
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

void
_print_win_error(DWORD code, int line)
{
  char buf[4096];

  if (FormatMessage (FORMAT_MESSAGE_FROM_SYSTEM
      | FORMAT_MESSAGE_IGNORE_INSERTS,
      NULL,
      code,
      MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT),
      (LPTSTR) buf, sizeof (buf), NULL))
    fprintf (stderr, "mkgroup (%d): [%lu] %s", line, code, buf);
  else
    fprintf (stderr, "mkgroup (%d): error %lu", line, code);
}

void
enum_local_users (LPWSTR servername, LPWSTR groupname)
{
  LOCALGROUP_MEMBERS_INFO_1 *buf1;
  DWORD entries = 0;
  DWORD total = 0;
  DWORD reshdl = 0;

  if (!NetLocalGroupGetMembers (servername, groupname, 1, (void *) &buf1,
				MAX_PREFERRED_LENGTH,
				&entries, &total, &reshdl))
    {
      unsigned i, first = 1;

      for (i = 0; i < entries; ++i)
	if (buf1[i].lgrmi1_sidusage == SidTypeUser)
	  {
	    if (!first)
	      printf (",");
	    first = 0;
	    printf ("%ls", buf1[i].lgrmi1_name);
	  }
      NetApiBufferFree (buf1);
    }
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

int
enum_local_groups (LPWSTR servername, int print_sids, int print_users,
		   int id_offset, char *disp_groupname)
{
  LOCALGROUP_INFO_0 *buffer;
  DWORD entriesread = 0;
  DWORD totalentries = 0;
  DWORD resume_handle = 0;
  WCHAR uni_name[GNLEN + 1];
  DWORD rc;

  do
    {
      DWORD i;

      if (disp_groupname != NULL)
	{
	  mbstowcs (uni_name, disp_groupname, GNLEN + 1);
	  rc = NetApiBufferAllocate (sizeof (LOCALGROUP_INFO_0),
				     (void *) &buffer);
	  buffer[0].lgrpi0_name = uni_name;
	  entriesread = 1;
	}
      else 
	rc = NetLocalGroupEnum (servername, 0, (void *) &buffer,
				MAX_PREFERRED_LENGTH, &entriesread,
				&totalentries, &resume_handle);
      switch (rc)
	{
	case ERROR_ACCESS_DENIED:
	  print_win_error(rc);
	  exit (1);

	case ERROR_MORE_DATA:
	case ERROR_SUCCESS:
	  break;

	default:
	  print_win_error(rc);
	  exit (1);
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

	  if (!LookupAccountNameW (servername, buffer[i].lgrpi0_name, psid,
				   &sid_length, domain_name, &domname_len,
				   &acc_type))
	    {
	      print_win_error(rc);
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
              if (!LookupAccountNameW (servername, domname,
                                       psid, &sid_length,
                                       domain_name, &domname_len,
                                       &acc_type))
                {
                  print_win_error(rc);
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

	      is_builtin = TRUE;
	      if (servername && builtin_sid_cnt)
		for (b = 0; b < builtin_sid_cnt; b++)
		  if (EqualSid (&builtin_sid_list[b], psid))
		    goto skip_group;
	      if (builtin_sid_cnt < MAX_BUILTIN_SIDS)
		CopySid (sizeof (DBGSID), &builtin_sid_list[builtin_sid_cnt++],
			 psid);
	    }

	  gid = *GetSidSubAuthority (psid, *GetSidSubAuthorityCount(psid) - 1);

	  printf ("%ls:%s:%ld:", buffer[i].lgrpi0_name,
                                print_sids ? put_sid (psid) : "",
                                gid + (is_builtin ? 0 : id_offset));
	  if (print_users)
	    enum_local_users (servername, buffer[i].lgrpi0_name);
	  printf ("\n");
skip_group:
	  ;
	}

      NetApiBufferFree (buffer);

    }
  while (rc == ERROR_MORE_DATA);

  return 0;
}

void
enum_users (LPWSTR servername, LPWSTR groupname)
{
  GROUP_USERS_INFO_0 *buf1;
  DWORD entries = 0;
  DWORD total = 0;
  DWORD reshdl = 0;

  if (!NetGroupGetUsers (servername, groupname, 0, (void *) &buf1,
			 MAX_PREFERRED_LENGTH, &entries, &total, &reshdl))
    {
      unsigned i, first = 1;

      for (i = 0; i < entries; ++i)
	{
	  if (!first)
	    printf (",");
	  first = 0;
	  printf ("%ls", buf1[i].grui0_name);
	}
      NetApiBufferFree (buf1);
    }
}

void
enum_groups (LPWSTR servername, int print_sids, int print_users, int id_offset,
	     char *disp_groupname)
{
  GROUP_INFO_2 *buffer;
  DWORD entriesread = 0;
  DWORD totalentries = 0;
  DWORD resume_handle = 0;
  WCHAR uni_name[GNLEN + 1];
  DWORD rc;

  do
    {
      DWORD i;

      if (disp_groupname != NULL)
	{
	  mbstowcs (uni_name, disp_groupname, GNLEN + 1);
	  rc = NetGroupGetInfo (servername, (LPWSTR) & uni_name, 2,
				(void *) &buffer);
	  entriesread=1;
	}
      else 
	rc = NetGroupEnum (servername, 2, (void *) & buffer,
			   MAX_PREFERRED_LENGTH, &entriesread, &totalentries,
			   &resume_handle);
      switch (rc)
	{
	case ERROR_ACCESS_DENIED:
	  print_win_error(rc);
	  exit (1);

	case ERROR_MORE_DATA:
	case ERROR_SUCCESS:
	  break;

	default:
	  print_win_error(rc);
	  exit (1);
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
          if (print_sids)
            {
              if (!LookupAccountNameW (servername, buffer[i].grpi2_name,
                                       psid, &sid_length,
                                       domain_name, &domname_len,
			               &acc_type))
                {
                  print_win_error(rc);
		  fprintf(stderr, " (%ls)\n", buffer[i].grpi2_name);
                  continue;
                }
              else if (acc_type == SidTypeDomain)
                {
                  WCHAR domname[MAX_DOMAIN_NAME_LEN + GNLEN + 2];

                  wcscpy (domname, domain_name);
                  wcscat (domname, L"\\");
                  wcscat (domname, buffer[i].grpi2_name);
                  sid_length = MAX_SID_LEN;
                  domname_len = MAX_DOMAIN_NAME_LEN + 1;
                  if (!LookupAccountNameW (servername, domname,
					   psid, &sid_length,
					   domain_name, &domname_len,
					   &acc_type))
                    {
                      print_win_error(rc);
		      fprintf(stderr, " (%ls)\n", domname);
                      continue;
                    }
                }
            }
	  printf ("%ls:%s:%u:", buffer[i].grpi2_name,
                               print_sids ? put_sid (psid) : "",
                               gid + id_offset);
	  if (print_users)
	    enum_users (servername, buffer[i].grpi2_name);
	  printf ("\n");
	}

      NetApiBufferFree (buffer);

    }
  while (rc == ERROR_MORE_DATA);
}

void
print_special (int print_sids,
	       PSID_IDENTIFIER_AUTHORITY auth, BYTE cnt,
	       DWORD sub1, DWORD sub2, DWORD sub3, DWORD sub4,
	       DWORD sub5, DWORD sub6, DWORD sub7, DWORD sub8)
{
  char name[UNLEN + 1], dom[MAX_DOMAIN_NAME_LEN + 1];
  DWORD len, len2, rid;
  PSID sid;
  SID_NAME_USE use;

  if (AllocateAndInitializeSid (auth, cnt, sub1, sub2, sub3, sub4,
  				sub5, sub6, sub7, sub8, &sid))
    {
      if (LookupAccountSid (NULL, sid,
			    name, (len = UNLEN + 1, &len),
			    dom, (len2 = MAX_DOMAIN_NAME_LEN + 1, &len),
			    &use))
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
	  printf ("%s:%s:%lu:\n", name,
				 print_sids ? put_sid (sid) : "",
				 rid);
        }
      FreeSid (sid);
    }
}

void
current_group (int print_sids, int print_users, int id_offset)
{
  char name[UNLEN + 1], *envname, *envdomain;
  DWORD len;
  HANDLE ptok;
  int errpos = 0;
  struct {
    PSID psid;
    char buffer[MAX_SID_LEN];
  } tg;


  if ((!GetUserName (name, (len = sizeof (name), &len)) && (errpos = __LINE__))
      || !name[0]
      || !(envname = getenv("USERNAME"))
      || strcasecmp (envname, name)
      || (!GetComputerName (name, (len = sizeof (name), &len))
	  && (errpos = __LINE__))
      || !(envdomain = getenv("USERDOMAIN"))
      || !envdomain[0]
      || !strcasecmp (envdomain, name)
      || (!OpenProcessToken (GetCurrentProcess (), TOKEN_QUERY, &ptok)
	  && (errpos = __LINE__))
      || (!GetTokenInformation (ptok, TokenPrimaryGroup, &tg, sizeof tg, &len)
	  && (errpos = __LINE__))
      || (!CloseHandle (ptok) && (errpos = __LINE__)))
    {
      if (errpos)
	{
	  print_win_error (GetLastError ());
	}
      return;
    }

  int gid = *GetSidSubAuthority (tg.psid, *GetSidSubAuthorityCount(tg.psid) - 1);

  printf ("mkgroup_l_d:%s:%u:", print_sids ? put_sid (tg.psid) : "",
                                gid + id_offset);
  if (print_users)
    printf("%s", envname);
  printf ("\n");
}

int
usage (FILE * stream, int isNT)
{
  fprintf (stream, "Usage: mkgroup [OPTION]... [domain]...\n"
	           "Print /etc/group file to stdout\n\n"
	           "Options:\n");
  if (isNT)
    fprintf (stream, "   -l,--local             print machine local group information\n"
	             "   -c,--current           print current group, if a domain account\n"
		     "   -d,--domain            print domain group information (from current\n"
	             "                          domain if no domains specified)\n"
		     "   -o,--id-offset offset  change the default offset (10000) added to gids\n"
		     "                          in domain accounts.\n"
		     "   -s,--no-sids           don't print SIDs in pwd field\n"
		     "                          (this affects ntsec)\n"
	             "   -u,--users             print user list in gr_mem field\n"
                     "   -g,--group groupname   only return information for the specified group\n");
  fprintf (stream, "   -h,--help              print this message\n"
	           "   -v,--version           print version information and exit\n\n");
  if (isNT)
    fprintf (stream, "One of '-l' or '-d' must be given.\n");

  return 1;
}

struct option longopts[] = {
  {"local", no_argument, NULL, 'l'},
  {"current", no_argument, NULL, 'c'},
  {"domain", no_argument, NULL, 'd'},
  {"id-offset", required_argument, NULL, 'o'},
  {"no-sids", no_argument, NULL, 's'},
  {"users", no_argument, NULL, 'u'},
  {"group", required_argument, NULL, 'g'},
  {"help", no_argument, NULL, 'h'},
  {"version", no_argument, NULL, 'v'},
  {0, no_argument, NULL, 0}
};

char opts[] = "lcdo:sug:hv";

void
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
mkgroup (cygwin) %.*s\n\
group File Generator\n\
Copyright 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008 Red Hat, Inc.\n\
Compiled on %s\n\
", len, v, __DATE__);
}

int
main (int argc, char **argv)
{
  LPWSTR servername;
  DWORD rc = ERROR_SUCCESS;
  WCHAR domain_name[MAX_DOMAIN_NAME_LEN + 1];
  int print_local = 0;
  int print_current = 0;
  int print_domain = 0;
  int print_sids = 1;
  int print_users = 0;
  int domain_specified = 0;
  int id_offset = 10000;
  char *disp_groupname = NULL;
  int isRoot = 0;
  int isNT;
  int i;

  char dom[MAX_DOMAIN_NAME_LEN + 1];
  DWORD len, len2;
  PSID psid = NULL;
  SID_NAME_USE use;

  LSA_OBJECT_ATTRIBUTES oa = { 0, 0, 0, 0, 0, 0 };
  LSA_HANDLE lsa = INVALID_HANDLE_VALUE;
  NTSTATUS ret;
  PPOLICY_PRIMARY_DOMAIN_INFO pdi;

  isNT = (GetVersion () < 0x80000000);

  if (isNT && argc == 1)
    return usage(stderr, isNT);
  else
    {
      while ((i = getopt_long (argc, argv, opts, longopts, NULL)) != EOF)
	switch (i)
	{
	case 'l':
	  print_local = 1;
	  break;
	case 'c':
	  print_current = 1;
	  break;
	case 'd':
	  print_domain = 1;
	  break;
	case 'o':
	  id_offset = strtol (optarg, NULL, 10);
	  break;
	case 's':
	  print_sids = 0;
	  break;
	case 'u':
	  print_users = 1;
	  break;
	case 'g':
	  disp_groupname = optarg;
	  isRoot = !strcmp(disp_groupname, "root");
	  break;
	case 'h':
	  usage (stdout, isNT);
	  return 0;
	case 'v':
	  print_version ();
	  return 0;
	default:
	  fprintf (stderr, "Try '%s --help' for more information.\n", argv[0]);
	  return 1;
	}
    }

  /* This takes Windows 9x/ME into account. */
  if (!isNT)
    {
      printf ("all::%ld:\n", DOMAIN_ALIAS_RID_ADMINS);
      return 0;
    }

  if (!print_local && !print_domain)
    {
      fprintf (stderr, "%s: Specify one of '-l' or '-d'\n", argv[0]);
      return 1;
    }
  if (optind < argc)
    {
      if (!print_domain)
        {
	  fprintf (stderr, "%s: A domain name is only accepted "
		   "when '-d' is given.\n", argv[0]);
	  return 1;
	}
      domain_specified = 1;
    }
  load_netapi ();

  if (print_local)
    {
      char machine[INTERNET_MAX_HOST_NAME_LENGTH + 1];
      char sid[MAX_SID_LEN];

      if (isRoot)
        {
	  /*
	   * Very special feature for the oncoming future:
	   * Create a "root" group account, being actually the local
	   * Administrators group.  Since user name, sid and gid are
	   * fixed, there's no need to call print_special() for this.
	   */
	  printf ("root:S-1-5-32-544:0:\n");
	}

      if (disp_groupname == NULL)
        {
      /*
       * Get 'system' group
       */
      print_special (print_sids, &sid_nt_auth, 1, SECURITY_LOCAL_SYSTEM_RID,
		     0, 0, 0, 0, 0, 0, 0);
      /*
       * Get 'None' group
      */
      len = INTERNET_MAX_HOST_NAME_LENGTH + 1;
      GetComputerName (machine, &len);
      len = MAX_SID_LEN;
      len2 = MAX_DOMAIN_NAME_LEN + 1;
      if (LookupAccountName (NULL, machine, (PSID) sid, &len, dom, &len2, &use))
	psid = (PSID) sid;
      else
        {
	  ret = LsaOpenPolicy (NULL, &oa, POLICY_VIEW_LOCAL_INFORMATION, &lsa);
	  if (ret == STATUS_SUCCESS && lsa != INVALID_HANDLE_VALUE)
	    {
	      ret = LsaQueryInformationPolicy (lsa,
					       PolicyPrimaryDomainInformation,
					       (void *) &pdi);
	      if (ret == STATUS_SUCCESS)
	        {
		  if (pdi->Sid)
		    {
		      CopySid (MAX_SID_LEN, (PSID) sid, pdi->Sid);
		      psid = (PSID) sid;
		    }
		  LsaFreeMemory (pdi);
		}
	      LsaClose (lsa);
	    }
	}
      if (!psid)
        fprintf (stderr,
	        "WARNING: Machine local group 513 couldn't get retrieved.  Try mkgroup -d\n");
      else
	print_special (print_sids, GetSidIdentifierAuthority (psid), 5,
				   *GetSidSubAuthority (psid, 0),
				   *GetSidSubAuthority (psid, 1),
				   *GetSidSubAuthority (psid, 2),
				   *GetSidSubAuthority (psid, 3),
				   513,
				   0,
				   0,
				   0);
	}

      if (!isRoot)
	enum_local_groups (NULL, print_sids, print_users, 0, disp_groupname);
    }
  i = 1;
  if (print_domain) 
    do
      {
	PDOMAIN_CONTROLLER_INFOW pdci = NULL;

	if (dsgetdcname)
	  {
	    if (domain_specified)
	      {
	        mbstowcs (domain_name, argv[optind], strlen (argv[optind]) + 1);
		rc = dsgetdcname (NULL, domain_name, NULL, NULL, 0, &pdci);
	      }
	    else
	      rc = dsgetdcname (NULL, NULL, NULL, NULL, 0, &pdci);
	    if (rc != ERROR_SUCCESS)
	      {
	        print_win_error(rc);
		return 1;
	      }
	    servername = pdci->DomainControllerName;
	  }
	else
	  {
	    rc = NetGetDCName (NULL, NULL, (void *) &servername);
	    if (rc == ERROR_SUCCESS && domain_specified)
	      {
		LPWSTR server = servername;
		mbstowcs (domain_name, argv[optind], strlen (argv[optind]) + 1);
		rc = NetGetDCName (NULL, domain_name, (void *) &servername);
		NetApiBufferFree (server);
	      }
	    if (rc != ERROR_SUCCESS)
	      {
		print_win_error(rc);
		return 1;
	      }
	  }
	enum_groups (servername, print_sids, print_users, id_offset * i,
		     disp_groupname);
	enum_local_groups (servername, print_sids, print_users, id_offset * i++,
			   disp_groupname);
	NetApiBufferFree (pdci ? (PVOID) pdci : (PVOID) servername);
      }
    while (++optind < argc);

  if (print_current && !print_domain)
    current_group (print_sids, print_users, id_offset);

  return 0;
}
