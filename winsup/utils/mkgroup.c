/* mkgroup.c:

   Copyright 1997, 1998, 1999, 2000, 2001, 2002, 2003 Red Hat, Inc.

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
#include <ntsecapi.h>
#include <ntdef.h>

#define print_win_error(x) _print_win_error(x, __LINE__)

static const char version[] = "$Revision$";

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

NET_API_STATUS WINAPI (*netapibufferallocate)(DWORD,PVOID*);
NET_API_STATUS WINAPI (*netapibufferfree)(PVOID);
NET_API_STATUS WINAPI (*netgroupenum)(LPWSTR,DWORD,PBYTE*,DWORD,PDWORD,PDWORD,PDWORD);
NET_API_STATUS WINAPI (*netgroupgetinfo)(LPWSTR,LPWSTR,DWORD,PBYTE*);
NET_API_STATUS WINAPI (*netlocalgroupenum)(LPWSTR,DWORD,PBYTE*,DWORD,PDWORD,PDWORD,PDWORD);
NET_API_STATUS WINAPI (*netlocalgroupgetmembers)(LPWSTR,LPWSTR,DWORD,PBYTE*,DWORD,PDWORD,PDWORD,PDWORD);
NET_API_STATUS WINAPI (*netgetdcname)(LPWSTR,LPWSTR,PBYTE*);
NET_API_STATUS WINAPI (*netgroupgetusers)(LPWSTR,LPWSTR,DWORD,PBYTE*,DWORD,PDWORD,PDWORD,PDWORD);

NTSTATUS NTAPI (*lsaclose)(LSA_HANDLE);
NTSTATUS NTAPI (*lsaopenpolicy)(PLSA_UNICODE_STRING,PLSA_OBJECT_ATTRIBUTES,ACCESS_MASK,PLSA_HANDLE);
NTSTATUS NTAPI (*lsaqueryinformationpolicy)(LSA_HANDLE,POLICY_INFORMATION_CLASS,PVOID*);
NTSTATUS NTAPI (*lsafreememory)(PVOID);

NET_API_STATUS WINAPI (*dsgetdcname)(LPWSTR,LPWSTR,GUID*,LPWSTR,ULONG,PDOMAIN_CONTROLLER_INFOW*);

#ifndef min
#define min(a,b) (((a)<(b))?(a):(b))
#endif

BOOL
load_netapi ()
{
  HANDLE h = LoadLibrary ("netapi32.dll");

  if (!h)
    return FALSE;

  if (!(netapibufferallocate = (void *) GetProcAddress (h, "NetApiBufferAllocate")))
    return FALSE;
  if (!(netapibufferfree = (void *) GetProcAddress (h, "NetApiBufferFree")))
    return FALSE;
  if (!(netgroupenum = (void *) GetProcAddress (h, "NetGroupEnum")))
    return FALSE;
  if (!(netgroupgetinfo = (void *) GetProcAddress (h, "NetGroupGetInfo")))
    return FALSE;
  if (!(netgroupgetusers = (void *) GetProcAddress (h, "NetGroupGetUsers")))
    return FALSE;
  if (!(netlocalgroupenum = (void *) GetProcAddress (h, "NetLocalGroupEnum")))
    return FALSE;
  if (!(netlocalgroupgetmembers = (void *) GetProcAddress (h, "NetLocalGroupGetMembers")))
    return FALSE;
  if (!(netgetdcname = (void *) GetProcAddress (h, "NetGetDCName")))
    return FALSE;

  dsgetdcname = (void *) GetProcAddress (h, "DsGetDcNameW");

  if (!(h = LoadLibrary ("advapi32.dll")))
    return FALSE;

  if (!(lsaclose = (void *) GetProcAddress (h, "LsaClose")))
    return FALSE;
  if (!(lsaopenpolicy = (void *) GetProcAddress (h, "LsaOpenPolicy")))
    return FALSE;
  if (!(lsaqueryinformationpolicy = (void *) GetProcAddress (h, "LsaQueryInformationPolicy")))
    return FALSE;
  if (!(lsafreememory = (void *) GetProcAddress (h, "LsaFreeMemory")))
    return FALSE;

  return TRUE;
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

void
uni2ansi (LPWSTR wcs, char *mbs, int size)
{
  if (wcs)
    WideCharToMultiByte (CP_ACP, 0, wcs, -1, mbs, size, NULL, NULL);
  else
    *mbs = '\0';
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
enum_local_users (LPWSTR groupname)
{
  LOCALGROUP_MEMBERS_INFO_1 *buf1;
  DWORD entries = 0;
  DWORD total = 0;
  DWORD reshdl = 0;

  if (!netlocalgroupgetmembers (NULL, groupname,
				1, (void *) &buf1,
				MAX_PREFERRED_LENGTH,
				&entries, &total, &reshdl))
    {
      unsigned i, first = 1;

      for (i = 0; i < entries; ++i)
	if (buf1[i].lgrmi1_sidusage == SidTypeUser)
	  {
	    char user[256];

	    if (!first)
	      printf (",");
	    first = 0;
	    uni2ansi (buf1[i].lgrmi1_name, user, sizeof (user));
	    printf ("%s", user);
	  }
      netapibufferfree (buf1);
    }
}

int
enum_local_groups (int print_sids, int print_users, char *disp_groupname)
{
  LOCALGROUP_INFO_0 *buffer;
  DWORD entriesread = 0;
  DWORD totalentries = 0;
  DWORD resume_handle = 0;
  WCHAR uni_name[512];
  DWORD rc;

  do
    {
      DWORD i;

      if (disp_groupname != NULL)
	{
	  MultiByteToWideChar (CP_ACP, 0, disp_groupname, -1, uni_name, 512 );
	  rc = netapibufferallocate(sizeof(LOCALGROUP_INFO_0), (void *) &buffer );
	  buffer[0].lgrpi0_name = (LPWSTR) & uni_name;
	  entriesread=1;
	}
      else 
	rc = netlocalgroupenum (NULL, 0, (void *) &buffer, 1024,
				&entriesread, &totalentries, &resume_handle);
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
	  char localgroup_name[100];
	  char domain_name[100];
	  DWORD domname_len = 100;
	  char psid_buffer[1024];
	  PSID psid = (PSID) psid_buffer;
	  DWORD sid_length = 1024;
	  DWORD gid;
	  SID_NAME_USE acc_type;
	  uni2ansi (buffer[i].lgrpi0_name, localgroup_name, sizeof (localgroup_name));

	  if (!LookupAccountName (NULL, localgroup_name, psid,
				  &sid_length, domain_name, &domname_len,
				  &acc_type))
	    {
	      print_win_error(rc);
	      fprintf(stderr, " (%s)\n", localgroup_name);
	      continue;
	    }
          else if (acc_type == SidTypeDomain)
            {
              char domname[356];

              strcpy (domname, domain_name);
              strcat (domname, "\\");
              strcat (domname, localgroup_name);
              sid_length = 1024;
              domname_len = 100;
              if (!LookupAccountName (NULL, domname,
                                      psid, &sid_length,
                                      domain_name, &domname_len,
                                      &acc_type))
                {
                  print_win_error(rc);
		  fprintf(stderr, " (%s)\n", domname);
                  continue;
                }
            }

	  gid = *GetSidSubAuthority (psid, *GetSidSubAuthorityCount(psid) - 1);

	  printf ("%s:%s:%ld:", localgroup_name,
                                print_sids ? put_sid (psid) : "",
                                gid);
	  if (print_users)
	    enum_local_users (buffer[i].lgrpi0_name);
	  printf ("\n");
	}

      netapibufferfree (buffer);

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

  if (!netgroupgetusers (servername, groupname,
			 0, (void *) &buf1,
			 MAX_PREFERRED_LENGTH,
			 &entries, &total, &reshdl))
    {
      unsigned i, first = 1;

      for (i = 0; i < entries; ++i)
	{
	  char user[256];

	  if (!first)
	    printf (",");
	  first = 0;
	  uni2ansi (buf1[i].grui0_name, user, sizeof (user));
	  printf ("%s", user);
	}
      netapibufferfree (buf1);
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
  WCHAR uni_name[512];
  DWORD rc;
  char ansi_srvname[256];

  if (servername)
    uni2ansi (servername, ansi_srvname, sizeof (ansi_srvname));

  do
    {
      DWORD i;

      if (disp_groupname != NULL)
	{
	  MultiByteToWideChar (CP_ACP, 0, disp_groupname, -1, uni_name, 512 );
	  rc = netgroupgetinfo(servername, (LPWSTR) & uni_name, 2,
			       (void *) &buffer );
	  entriesread=1;
	}
      else 
	rc = netgroupenum (servername, 2, (void *) & buffer, MAX_PREFERRED_LENGTH,
			   &entriesread, &totalentries, &resume_handle);
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
	  char groupname[100];
	  char domain_name[100];
	  DWORD domname_len = 100;
	  char psid_buffer[1024];
	  PSID psid = (PSID) psid_buffer;
	  DWORD sid_length = 1024;
	  SID_NAME_USE acc_type;

	  int gid = buffer[i].grpi2_group_id;
	  uni2ansi (buffer[i].grpi2_name, groupname, sizeof (groupname));
          if (print_sids)
            {
              if (!LookupAccountName (servername ? ansi_srvname : NULL,
                                      groupname,
                                      psid, &sid_length,
                                      domain_name, &domname_len,
			              &acc_type))
                {
                  print_win_error(rc);
		  fprintf(stderr, " (%s)\n", groupname);
                  continue;
                }
              else if (acc_type == SidTypeDomain)
                {
                  char domname[356];

                  strcpy (domname, domain_name);
                  strcat (domname, "\\");
                  strcat (domname, groupname);
                  sid_length = 1024;
                  domname_len = 100;
                  if (!LookupAccountName (servername ? ansi_srvname : NULL,
                                          domname,
                                          psid, &sid_length,
                                          domain_name, &domname_len,
			                  &acc_type))
                    {
                      print_win_error(rc);
		      fprintf(stderr, " (%s)\n", domname);
                      continue;
                    }
                }
            }
	  printf ("%s:%s:%u:", groupname,
                               print_sids ? put_sid (psid) : "",
                               gid + id_offset);
	  if (print_users)
	    enum_users (servername, buffer[i].grpi2_name);
	  printf ("\n");
	}

      netapibufferfree (buffer);

    }
  while (rc == ERROR_MORE_DATA);
}

void
print_special (int print_sids,
	       PSID_IDENTIFIER_AUTHORITY auth, BYTE cnt,
	       DWORD sub1, DWORD sub2, DWORD sub3, DWORD sub4,
	       DWORD sub5, DWORD sub6, DWORD sub7, DWORD sub8)
{
  char name[256], dom[256];
  DWORD len, len2, rid;
  PSID sid;
  SID_NAME_USE use;

  if (AllocateAndInitializeSid (auth, cnt, sub1, sub2, sub3, sub4,
  				sub5, sub6, sub7, sub8, &sid))
    {
      if (LookupAccountSid (NULL, sid,
			    name, (len = 256, &len),
			    dom, (len2 = 256, &len),
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
    int buffer[10];
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
    fprintf (stream, "   -l,--local             print local group information\n"
	             "   -c,--current           print current group, if a domain account\n"
		     "   -d,--domain            print global group information (from current\n"
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
Copyright 1997, 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.\n\
Compiled on %s\n\
", len, v, __DATE__);
}

int
main (int argc, char **argv)
{
  LPWSTR servername;
  DWORD rc = ERROR_SUCCESS;
  WCHAR domain_name[100];
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

  char name[256], dom[256];
  DWORD len, len2;
  char buf[1024];
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
  if (!load_netapi ())
    {
      print_win_error(GetLastError ());
      return 1;
    }

  if (print_local)
    {
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
      len = 256;
      GetComputerName (name, &len);
      len = 1024;
      len2 = 256;
      if (LookupAccountName (NULL, name, (PSID) buf, &len, dom, &len, &use))
	psid = (PSID) buf;
      else
        {
	  ret = lsaopenpolicy (NULL, &oa, POLICY_VIEW_LOCAL_INFORMATION, &lsa);
	  if (ret == STATUS_SUCCESS && lsa != INVALID_HANDLE_VALUE)
	    {
	      ret = lsaqueryinformationpolicy (lsa,
					       PolicyPrimaryDomainInformation,
					       (void *) &pdi);
	      if (ret == STATUS_SUCCESS)
	        {
		  if (pdi->Sid)
		    {
		      CopySid (1024, (PSID) buf, pdi->Sid);
		      psid = (PSID) buf;
		    }
		  lsafreememory (pdi);
		}
	      lsaclose (lsa);
	    }
	}
      if (!psid)
        fprintf (stderr,
	        "WARNING: Group 513 couldn't get retrieved.  Try mkgroup -d\n");
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
	{
      enum_local_groups (print_sids, print_users, disp_groupname);
	}
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
	    rc = netgetdcname (NULL, NULL, (void *) &servername);
	    if (rc == ERROR_SUCCESS && domain_specified)
	      {
		LPWSTR server = servername;
		mbstowcs (domain_name, argv[optind], strlen (argv[optind]) + 1);
		rc = netgetdcname (NULL, domain_name, (void *) &servername);
		netapibufferfree (server);
	      }
	    if (rc != ERROR_SUCCESS)
	      {
		print_win_error(rc);
		return 1;
	      }
	  }
	enum_groups (servername, print_sids, print_users, id_offset * i++,
		     disp_groupname);
	netapibufferfree (pdci ? (PVOID) pdci : (PVOID) servername);
      }
    while (++optind < argc);

  if (print_current && !print_domain)
    current_group (print_sids, print_users, id_offset);

  return 0;
}
