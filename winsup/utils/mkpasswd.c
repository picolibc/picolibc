/* mkpasswd.c:

   Copyright 1997, 1998, 1999, 2000 Cygnus Solutions.

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
#include <lmaccess.h>
#include <lmapibuf.h>

SID_IDENTIFIER_AUTHORITY sid_world_auth = {SECURITY_WORLD_SID_AUTHORITY};
SID_IDENTIFIER_AUTHORITY sid_nt_auth = {SECURITY_NT_AUTHORITY};

#ifndef min
#define min(a,b) (((a)<(b))?(a):(b))
#endif

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
enum_users (LPWSTR servername, int print_sids, int print_cygpath)
{
  USER_INFO_3 *buffer;
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

      rc = NetUserEnum (servername, 3, FILTER_NORMAL_ACCOUNT,
			(LPBYTE *) & buffer, 1024,
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
	  fprintf (stderr, "NetUserEnum() failed with %ld\n", rc);
	  exit (1);
	}

      for (i = 0; i < entriesread; i++)
	{
	  char username[100];
	  char fullname[100];
	  char homedir_psx[MAX_PATH];
	  char homedir_w32[MAX_PATH];
	  char domain_name[100];
	  DWORD domname_len = 100;
	  char psid_buffer[1024];
	  PSID psid = (PSID) psid_buffer;
	  DWORD sid_length = 1024;
	  SID_NAME_USE acc_type;

	  int uid = buffer[i].usri3_user_id;
	  int gid = buffer[i].usri3_primary_group_id;
	  uni2ansi (buffer[i].usri3_name, username);
	  uni2ansi (buffer[i].usri3_full_name, fullname);
	  homedir_w32[0] = homedir_psx[0] = '\0';
	  uni2ansi (buffer[i].usri3_home_dir, homedir_w32);
          if (print_cygpath)
            cygwin_conv_to_posix_path (homedir_w32, homedir_psx);
          else
	    psx_dir (homedir_w32, homedir_psx);

          if (print_sids)
            {
              if (!LookupAccountName (servername ? ansi_srvname : NULL,
                                      username,
                                      psid, &sid_length,
                                      domain_name, &domname_len,
			              &acc_type))
                {
                  fprintf (stderr,
                           "LookupAccountName(%s,%s) failed with error %ld\n",
                           servername ? ansi_srvname : "NULL",
                           username,
                           GetLastError ());
                  continue;
                }
              else if (acc_type == SidTypeDomain)
                {
                  char domname[356];

                  strcpy (domname, domain_name);
                  strcat (domname, "\\");
                  strcat (domname, username);
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
	  printf ("%s::%d:%d:%s%s%s:%s:/bin/sh\n", username, uid, gid,
		  fullname,
                  print_sids ? "," : "",
                  print_sids ? put_sid (psid) : "",
                  homedir_psx);
	}

      NetApiBufferFree (buffer);

    }
  while (rc == ERROR_MORE_DATA);

  if (servername)
    NetApiBufferFree (servername);

  return 0;
}

int 
enum_local_groups (int print_sids)
{
  LOCALGROUP_INFO_0 *buffer;
  DWORD entriesread = 0;
  DWORD totalentries = 0;
  DWORD resume_handle = 0;
  DWORD rc ;

  do
    {
      DWORD i;

      rc = NetLocalGroupEnum (NULL, 0, (LPBYTE *) & buffer, 1024,
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
	  fprintf (stderr, "NetUserEnum() failed with %ld\n", rc);
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

	  printf ("%s:*:%ld:%ld:%s%s::\n", localgroup_name, gid, gid,
                  print_sids ? "," : "",
                  print_sids ? put_sid (psid) : "");
	}

      NetApiBufferFree (buffer);

    }
  while (rc == ERROR_MORE_DATA);

  return 0;
}

void 
usage ()
{
  fprintf (stderr, "\n");
  fprintf (stderr, "usage: mkpasswd [options] [domain]\n\n");
  fprintf (stderr, "This program prints a /etc/passwd file to stdout\n\n");
  fprintf (stderr, "Options are\n");
  fprintf (stderr, "   -l,--local              print local accounts\n");
  fprintf (stderr, "   -d,--domain             print domain accounts (from current domain\n");
  fprintf (stderr, "                           if no domain specified\n");
  fprintf (stderr, "   -g,--local-groups       print local group information too\n");
  fprintf (stderr, "   -m,--no-mount           don't use mount points for home dir\n");
  fprintf (stderr, "   -s,--no-sids            don't print SIDs in GCOS field\n");
  fprintf (stderr, "                           (this affects ntsec)\n");
  fprintf (stderr, "   -?,--help               displays this message\n\n");
  fprintf (stderr, "This program does only work on Windows NT\n\n");
  exit (1);
}

int 
main (int argc, char **argv)
{
  LPWSTR servername = NULL;
  DWORD rc = ERROR_SUCCESS;
  WCHAR domain_name[200];
  int print_local = 0;
  int print_domain = 0;
  int print_local_groups = 0;
  int domain_name_specified = 0;
  int print_sids = 1;
  int print_cygpath = 1;
  int i;

  char name[256], dom[256];
  DWORD len, len2;
  PSID sid;
  SID_NAME_USE use;

  if (argc == 1)
    usage ();

  else
    {
      for (i = 1; i < argc; i++)
	{
	  if (!strcmp (argv[i], "-l") || !strcmp (argv[i], "--local"))
	    print_local = 1;

	  else if (!strcmp (argv[i], "-d") || !strcmp (argv[i], "--domain"))
	    print_domain = 1;

	  else if (!strcmp (argv[i], "-g") || !strcmp (argv[i], "--local-groups"))
	    print_local_groups = 1;

	  else if (!strcmp (argv[i], "-s") || !strcmp (argv[i], "--no-sids"))
	    print_sids = 0;

	  else if (!strcmp (argv[i], "-m") || !strcmp (argv[i], "--no-mount"))
	    print_cygpath = 0;

	  else if (!strcmp (argv[i], "-?") || !strcmp (argv[i], "--help"))
	    usage ();

	  else
	    {
	      mbstowcs (domain_name, argv[i], (strlen (argv[i]) + 1));
	      domain_name_specified = 1;
	    }
	}
    }

  /* FIXME: this needs to take Windows 98 into account. */
  if (GetVersion () >= 0x80000000)
    {
      fprintf (stderr, "The required functionality is not supported by Windows 95. Sorry.\n");
      exit (1);
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
        printf ("%s:*:%ld:%ld:%s%s::\n", name,
                                         SECURITY_WORLD_RID,
                                         SECURITY_WORLD_RID,
                                         print_sids ? "," : "",
                                         print_sids ? put_sid (sid) : "");
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
        printf ("%s:*:%ld:%ld:%s%s::\n", name,
                                         SECURITY_LOCAL_SYSTEM_RID,
                                         SECURITY_LOCAL_SYSTEM_RID,
                                         print_sids ? "," : "",
                                         print_sids ? put_sid (sid) : "");
      FreeSid (sid);
    }

  /*
   * Get `administrators' group
  */
  if (!print_local
      && AllocateAndInitializeSid (&sid_nt_auth, 2,
                                   SECURITY_BUILTIN_DOMAIN_RID,
                                   DOMAIN_ALIAS_RID_ADMINS,
                                   0, 0, 0, 0, 0, 0, &sid))
    {
      if (LookupAccountSid (NULL, sid,
                            name, (len = 256, &len),
                            dom, (len2 = 256, &len),
                            &use))
        printf ("%s:*:%ld:%ld:%s%s::\n", name,
                                         DOMAIN_ALIAS_RID_ADMINS,
                                         DOMAIN_ALIAS_RID_ADMINS,
                                         print_sids ? "," : "",
                                         print_sids ? put_sid (sid) : "");
      FreeSid (sid);
    }

  if (print_local_groups)
    enum_local_groups (print_sids);

  if (print_domain)
    {
      if (domain_name_specified)
	rc = NetGetDCName (NULL, domain_name, (LPBYTE *) & servername);

      else
	rc = NetGetDCName (NULL, NULL, (LPBYTE *) & servername);

      if (rc != ERROR_SUCCESS)
	{
	  fprintf (stderr, "Cannot get DC, code = %ld\n", rc);
	  exit (1);
	}

      enum_users (servername, print_sids, print_cygpath);
    }

  if (print_local)
    enum_users (NULL, print_sids, print_cygpath);

  if (servername)
    NetApiBufferFree (servername);

  return 0;
}
