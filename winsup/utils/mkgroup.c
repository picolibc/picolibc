/* mkgroup.c:

   Copyright 1997, 1998 Cygnus Solutions.

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

SID_IDENTIFIER_AUTHORITY sid_world_auth = {SECURITY_WORLD_SID_AUTHORITY};
SID_IDENTIFIER_AUTHORITY sid_nt_auth = {SECURITY_NT_AUTHORITY};

NET_API_STATUS WINAPI (*netapibufferfree)(PVOID);
NET_API_STATUS WINAPI (*netgroupenum)(LPWSTR,DWORD,PBYTE*,DWORD,PDWORD,PDWORD,PDWORD);
NET_API_STATUS WINAPI (*netlocalgroupenum)(LPWSTR,DWORD,PBYTE*,DWORD,PDWORD,PDWORD,PDWORD);
NET_API_STATUS WINAPI (*netgetdcname)(LPWSTR,LPWSTR,PBYTE*);

#ifndef min
#define min(a,b) (((a)<(b))?(a):(b))
#endif

BOOL
load_netapi ()
{
  HANDLE h = LoadLibrary ("netapi32.dll");

  if (!h)
    return FALSE;

  if (!(netapibufferfree = GetProcAddress (h, "NetApiBufferFree")))
    return FALSE;
  if (!(netgroupenum = GetProcAddress (h, "NetGroupEnum")))
    return FALSE;
  if (!(netlocalgroupenum = GetProcAddress (h, "NetLocalGroupEnum")))
    return FALSE;
  if (!(netgetdcname = GetProcAddress (h, "NetGetDCName")))
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
uni2ansi (LPWSTR wcs, char *mbs)
{
  if (wcs)
    wcstombs (mbs, wcs, (wcslen (wcs) + 1) * sizeof (WCHAR));

  else
    *mbs = '\0';
}

int
enum_local_groups (int print_sids)
{
  LOCALGROUP_INFO_0 *buffer;
  DWORD entriesread = 0;
  DWORD totalentries = 0;
  DWORD resume_handle = 0;
  DWORD rc;

  do
    {
      DWORD i;

      rc = netlocalgroupenum (NULL, 0, (LPBYTE *) & buffer, 1024,
			      &entriesread, &totalentries, &resume_handle);
      switch (rc)
	{
	case ERROR_ACCESS_DENIED:
	  fprintf (stderr, "Access denied\n");
	  exit (1);

	case ERROR_MORE_DATA:
	case ERROR_SUCCESS:
	  break;

	default:
	  fprintf (stderr, "NetLocalGroupEnum() failed with %ld\n", rc);
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
	  uni2ansi (buffer[i].lgrpi0_name, localgroup_name);

	  if (!LookupAccountName (NULL, localgroup_name, psid,
				  &sid_length, domain_name, &domname_len,
				  &acc_type))
	    {
	      fprintf (stderr, "LookupAccountName(%s) failed with %ld\n",
		       localgroup_name, GetLastError ());
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
                  fprintf (stderr,
                           "LookupAccountName(%s) failed with error %ld\n",
                           localgroup_name, GetLastError ());
                  continue;
                }
            }

	  gid = *GetSidSubAuthority (psid, *GetSidSubAuthorityCount(psid) - 1);

	  printf ("%s:%s:%ld:\n", localgroup_name,
                                  print_sids ? put_sid (psid) : "",
                                  gid);
	}

      netapibufferfree (buffer);

    }
  while (rc == ERROR_MORE_DATA);

  return 0;
}

void
enum_groups (LPWSTR servername, int print_sids)
{
  GROUP_INFO_2 *buffer;
  DWORD entriesread = 0;
  DWORD totalentries = 0;
  DWORD resume_handle = 0;
  DWORD rc;
  char ansi_srvname[256];

  if (servername)
    uni2ansi (servername, ansi_srvname);

  do
    {
      DWORD i;

      rc = netgroupenum (servername, 2, (LPBYTE *) & buffer, 1024,
		         &entriesread, &totalentries, &resume_handle);
      switch (rc)
	{
	case ERROR_ACCESS_DENIED:
	  fprintf (stderr, "Access denied\n");
	  exit (1);

	case ERROR_MORE_DATA:
	case ERROR_SUCCESS:
	  break;

	default:
	  fprintf (stderr, "NetGroupEnum() failed with %ld\n", rc);
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
	  uni2ansi (buffer[i].grpi2_name, groupname);
          if (print_sids)
            {
              if (!LookupAccountName (servername ? ansi_srvname : NULL,
                                      groupname,
                                      psid, &sid_length,
                                      domain_name, &domname_len,
			              &acc_type))
                {
                  fprintf (stderr,
                           "LookupAccountName (%s, %s) failed with error %ld\n",
                           servername ? ansi_srvname : "NULL",
                           groupname,
                           GetLastError ());
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
                      fprintf (stderr,
                               "LookupAccountName(%s,%s) failed with error %ld\n",
                               servername ? ansi_srvname : "NULL",
                               domname,
                               GetLastError ());
                      continue;
                    }
                }
            }
	  printf ("%s:%s:%d:\n", groupname,
                                 print_sids ? put_sid (psid) : "",
                                 gid);
	}

      netapibufferfree (buffer);

    }
  while (rc == ERROR_MORE_DATA);

  if (servername)
    netapibufferfree (servername);
}

int
usage ()
{
  fprintf (stderr, "Usage: mkgroup [OPTION]... [domain]\n\n");
  fprintf (stderr, "This program prints a /etc/group file to stdout\n\n");
  fprintf (stderr, "Options:\n");
  fprintf (stderr, "   -l,--local           print local group information\n");
  fprintf (stderr, "   -d,--domain          print global group information from the domain\n");
  fprintf (stderr, "                        specified (or from the current domain if there is\n");
  fprintf (stderr, "                        no domain specified)\n");
  fprintf (stderr, "   -s,--no-sids         don't print SIDs in pwd field\n");
  fprintf (stderr, "                         (this affects ntsec)\n");
  fprintf (stderr, "   -?,--help            print this message\n\n");
  fprintf (stderr, "One of `-l' or `-d' must be given on NT/W2K.\n");
  return 1;
}

struct option longopts[] = {
  {"local", no_argument, NULL, 'l'},
  {"domain", no_argument, NULL, 'd'},
  {"no-sids", no_argument, NULL, 's'},
  {"help", no_argument, NULL, 'h'},
  {0, no_argument, NULL, 0}
};

char opts[] = "ldsh";

int
main (int argc, char **argv)
{
  LPWSTR servername;
  DWORD rc = ERROR_SUCCESS;
  WCHAR domain_name[100];
  int print_local = 0;
  int print_domain = 0;
  int print_sids = 1;
  int domain_specified = 0;
  int i;

  char name[256], dom[256];
  DWORD len, len2;
  PSID sid, csid;
  SID_NAME_USE use;

  if (GetVersion () < 0x80000000)
    if (argc == 1)
      return usage ();
    else
      {
        while ((i = getopt_long (argc, argv, opts, longopts, NULL)) != EOF)
          switch (i)
	    {
	    case 'l':
	      print_local = 1;
	      break;
	    case 'd':
	      print_domain = 1;
	      break;
	    case 's':
	      print_sids = 0;
	      break;
	    case 'h':
	      return usage ();
	    default:
	      fprintf (stderr, "Try `%s --help' for more information.\n", argv[0]);
	      return 1;
	    }
        if (!print_local && !print_domain)
          {
	    fprintf (stderr, "%s: Specify one of `-l' or `-d'\n", argv[0]);
	    return 1;
	  }
        if (optind < argc)
          {
	    if (!print_domain)
	      {
	        fprintf (stderr, "%s: A domain name is only accepted "
	      		         "when `-d' is given.\n", argv[0]);
	        return 1;
	      }
	    mbstowcs (domain_name, argv[optind], (strlen (argv[optind]) + 1));
	    domain_specified = 1;
	  }
      }

  /* This takes Windows 9x/ME into account. */
  if (GetVersion () >= 0x80000000)
    {
      printf ("unknown::%ld:\n", DOMAIN_ALIAS_RID_ADMINS);
      return 0;
    }
  
  if (!load_netapi ())
    {
      fprintf (stderr, "Failed loading symbols from netapi32.dll "
      		       "with error %lu\n", GetLastError ());
      return 1;
    }

  /*
   * Get `Everyone' group
  */
  if (AllocateAndInitializeSid (&sid_world_auth, 1, SECURITY_WORLD_RID,
				0, 0, 0, 0, 0, 0, 0, &sid))
    {
      if (LookupAccountSid (NULL, sid,
			    name, (len = 256, &len),
			    dom, (len2 = 256, &len),
			    &use))
	printf ("%s:%s:%d:\n", name,
                               print_sids ? put_sid (sid) : "",
                               SECURITY_WORLD_RID);
      FreeSid (sid);
    }

  /*
   * Get `system' group
  */
  if (AllocateAndInitializeSid (&sid_nt_auth, 1, SECURITY_LOCAL_SYSTEM_RID,
				0, 0, 0, 0, 0, 0, 0, &sid))
    {
      if (LookupAccountSid (NULL, sid,
			    name, (len = 256, &len),
			    dom, (len2 = 256, &len),
			    &use))
	printf ("%s:%s:%d:\n", name,
                               print_sids ? put_sid (sid) : "",
                               SECURITY_LOCAL_SYSTEM_RID);
      FreeSid (sid);
    }

  if (print_local)
    {
      /*
       * Get `None' group
      */
      GetComputerName (name, (len = 256, &len));
      csid = (PSID) malloc (1024);
      LookupAccountName (NULL, name,
			 csid, (len = 1024, &len),
			 dom, (len2 = 256, &len),
			 &use);
      if (AllocateAndInitializeSid (GetSidIdentifierAuthority (csid),
				    5,
				    *GetSidSubAuthority (csid, 0),
				    *GetSidSubAuthority (csid, 1),
				    *GetSidSubAuthority (csid, 2),
				    *GetSidSubAuthority (csid, 3),
				    513,
				    0,
				    0,
				    0,
				    &sid))
	{
	  if (LookupAccountSid (NULL, sid,
				name, (len = 256, &len),
				dom, (len2 = 256, &len),
				&use))
            printf ("%s:%s:513:\n", name,
                                   print_sids ? put_sid (sid) : "");
	  FreeSid (sid);
	}
      free (csid);
    }

  if (print_domain)
    {
      if (domain_specified)
	rc = netgetdcname (NULL, domain_name, (LPBYTE *) & servername);

      else
	rc = netgetdcname (NULL, NULL, (LPBYTE *) & servername);

      if (rc != ERROR_SUCCESS)
	{
	  fprintf (stderr, "Cannot get PDC, code = %ld\n", rc);
	  exit (1);
	}

      enum_groups (servername, print_sids);
    }

  if (print_local)
    enum_local_groups (print_sids);

  return 0;
}
