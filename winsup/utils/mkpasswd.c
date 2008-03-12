/* mkpasswd.c:

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
#include <io.h>
#include <unistd.h>
#include <sys/cygwin.h>
#include <getopt.h>
#include <lmaccess.h>
#include <lmapibuf.h>
#include <sys/fcntl.h>
#include <lmerr.h>
#include <lmcons.h>

#define print_win_error(x) _print_win_error(x, __LINE__)

static const char version[] = "$Revision$";

SID_IDENTIFIER_AUTHORITY sid_world_auth = {SECURITY_WORLD_SID_AUTHORITY};
SID_IDENTIFIER_AUTHORITY sid_nt_auth = {SECURITY_NT_AUTHORITY};

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

NET_API_STATUS WINAPI (*netapibufferfree)(PVOID);
NET_API_STATUS WINAPI (*netuserenum)(LPWSTR,DWORD,DWORD,PBYTE*,DWORD,PDWORD,PDWORD,PDWORD);
NET_API_STATUS WINAPI (*netlocalgroupenum)(LPWSTR,DWORD,PBYTE*,DWORD,PDWORD,PDWORD,PDWORD);
NET_API_STATUS WINAPI (*netgetdcname)(LPWSTR,LPWSTR,PBYTE*);
NET_API_STATUS WINAPI (*netusergetinfo)(LPWSTR,LPWSTR,DWORD,PBYTE*);
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

  if (!(netapibufferfree = (void *) GetProcAddress (h, "NetApiBufferFree")))
    return FALSE;
  if (!(netuserenum = (void *) GetProcAddress (h, "NetUserEnum")))
    return FALSE;
  if (!(netlocalgroupenum = (void *) GetProcAddress (h, "NetLocalGroupEnum")))
    return FALSE;
  if (!(netgetdcname = (void *) GetProcAddress (h, "NetGetDCName")))
    return FALSE;
  if (!(netusergetinfo = (void *) GetProcAddress (h, "NetUserGetInfo")))
    return FALSE;

  dsgetdcname = (void *) GetProcAddress (h, "DsGetDcNameW");

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
    fprintf (stderr, "mkpasswd (%d): [%lu] %s", line, code, buf);
  else
    fprintf (stderr, "mkpasswd (%d): error %lu", line, code);
}

void
current_user (int print_sids, int print_cygpath,
	      const char * passed_home_path, int id_offset, const char * disp_username)
{
  char name[UNLEN + 1], *envname, *envdomain;
  DWORD len;
  HANDLE ptok;
  int errpos = 0;
  struct {
    PSID psid;
    int buffer[10];
  } tu, tg;


  if ((!GetUserName (name, (len = sizeof (name), &len)) && (errpos = __LINE__))
      || !name[0]
      || !(envname = getenv("USERNAME"))
      || strcasecmp (envname, name)
      || (disp_username && strcasecmp(envname, disp_username))
      || (!GetComputerName (name, (len = sizeof (name), &len))
	  && (errpos = __LINE__))
      || !(envdomain = getenv("USERDOMAIN"))
      || !envdomain[0]
      || !strcasecmp (envdomain, name)
      || (!OpenProcessToken (GetCurrentProcess (), TOKEN_QUERY, &ptok)
	  && (errpos = __LINE__))
      || (!GetTokenInformation (ptok, TokenUser, &tu, sizeof tu, &len)
	  && (errpos = __LINE__))
      || (!GetTokenInformation (ptok, TokenPrimaryGroup, &tg, sizeof tg, &len)
	  && (errpos = __LINE__))
      || (!CloseHandle (ptok) && (errpos = __LINE__)))
    {
      if (errpos)
	_print_win_error (GetLastError (), errpos);
      return;
    }

  int uid = *GetSidSubAuthority (tu.psid, *GetSidSubAuthorityCount(tu.psid) - 1);
  int gid = *GetSidSubAuthority (tg.psid, *GetSidSubAuthorityCount(tg.psid) - 1);
  char homedir_psx[MAX_PATH] = {0}, homedir_w32[MAX_PATH] = {0};

  char *envhomedrive = getenv ("HOMEDRIVE");
  char *envhomepath = getenv ("HOMEPATH");

  if (passed_home_path[0] == '\0')
    {
      if (envhomepath && envhomepath[0])
        {
	  if (envhomedrive)
	    strlcpy (homedir_w32, envhomedrive, sizeof (homedir_w32));
	  if (envhomepath[0] != '\\')
	    strlcat (homedir_w32, "\\", sizeof (homedir_w32));
	  strlcat (homedir_w32, envhomepath, sizeof (homedir_w32));
	  if (print_cygpath)
	    cygwin_conv_path (CCP_WIN_A_TO_POSIX | CCP_ABSOLUTE, homedir_w32,
			      homedir_psx, MAX_PATH);
	  else
	    psx_dir (homedir_w32, homedir_psx);
	}
      else
        {
	  strlcpy (homedir_psx, "/home/", sizeof (homedir_psx));
	  strlcat (homedir_psx, envname, sizeof (homedir_psx));
	}
    }
  else
    {
      strlcpy (homedir_psx, passed_home_path, sizeof (homedir_psx));
      strlcat (homedir_psx, envname, sizeof (homedir_psx));
    }

  printf ("%s:unused_by_nt/2000/xp:%u:%u:%s%s%s%s%s%s%s%s:%s:/bin/bash\n",
	  envname,
	  uid + id_offset,
	  gid + id_offset,
	  envname,
	  print_sids ? "," : "",
	  print_sids ? "U-" : "",
	  print_sids ? envdomain : "",
	  print_sids ? "\\" : "",
	  print_sids ? envname : "",
	  print_sids ? "," : "",
	  print_sids ? put_sid (tu.psid) : "",
	  homedir_psx);
}

int
enum_users (LPWSTR servername, int print_sids, int print_cygpath,
	    const char * passed_home_path, int id_offset, char *disp_username)
{
  USER_INFO_3 *buffer;
  DWORD entriesread = 0;
  DWORD totalentries = 0;
  DWORD resume_handle = 0;
  DWORD rc;
  char ansi_srvname[256];
  WCHAR uni_name[512];

  if (servername)
    uni2ansi (servername, ansi_srvname, sizeof (ansi_srvname));

  do
    {
      DWORD i;

    if (disp_username != NULL)
      {
	MultiByteToWideChar (CP_ACP, 0, disp_username, -1, uni_name, 512 );
	rc = netusergetinfo(servername, (LPWSTR) & uni_name, 3,
			    (void *) &buffer );
	entriesread=1;
      }
    else 
      rc = netuserenum (servername, 3, FILTER_NORMAL_ACCOUNT,
			(void *) &buffer, 1024,
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
	  uni2ansi (buffer[i].usri3_name, username, sizeof (username));
	  uni2ansi (buffer[i].usri3_full_name, fullname, sizeof (fullname));
	  homedir_w32[0] = homedir_psx[0] = '\0';
	  if (passed_home_path[0] == '\0')
	    {
	      uni2ansi (buffer[i].usri3_home_dir, homedir_w32,
			sizeof (homedir_w32));
	      if (homedir_w32[0] != '\0')
		{
		  if (print_cygpath)
		    cygwin_conv_path (CCP_WIN_A_TO_POSIX | CCP_ABSOLUTE,
				      homedir_w32, homedir_psx, MAX_PATH);
		  else
		    psx_dir (homedir_w32, homedir_psx);
		}
	      else
		{
		  strcpy (homedir_psx, "/home/");
		  strcat (homedir_psx, username);
		}
	    }
	  else
	    {
	      strcpy (homedir_psx, passed_home_path);
	      strcat (homedir_psx, username);
	    }

	  if (print_sids)
	    {
	      if (!LookupAccountName (servername ? ansi_srvname : NULL,
				      username,
				      psid, &sid_length,
				      domain_name, &domname_len,
				      &acc_type))
		{
	  	  print_win_error(GetLastError ());
		  fprintf(stderr, " (%s)\n", username);
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
		      print_win_error(GetLastError ());
		      fprintf(stderr, " (%s)\n", domname);
		      continue;
		    }
		}
	    }
	  printf ("%s:unused_by_nt/2000/xp:%u:%u:%s%s%s%s%s%s%s%s:%s:/bin/bash\n",
	  	  username,
		  uid + id_offset,
		  gid + id_offset,
		  fullname,
		  print_sids && fullname[0] ? "," : "",
		  print_sids ? "U-" : "",
		  print_sids ? domain_name : "",
		  print_sids && domain_name[0] ? "\\" : "",
		  print_sids ? username : "",
		  print_sids ? "," : "",
		  print_sids ? put_sid (psid) : "",
		  homedir_psx);
	}

      netapibufferfree (buffer);

    }
  while (rc == ERROR_MORE_DATA);

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
	      print_win_error(GetLastError ());
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
		  print_win_error(GetLastError ());
		  fprintf(stderr, " (%s)\n", domname);
		  continue;
		}
	    }

	  gid = *GetSidSubAuthority (psid, *GetSidSubAuthorityCount(psid) - 1);

	  printf ("%s:*:%ld:%ld:%s%s::\n", localgroup_name, gid, gid,
		  print_sids ? "," : "",
		  print_sids ? put_sid (psid) : "");
	}

      netapibufferfree (buffer);

    }
  while (rc == ERROR_MORE_DATA);

  return 0;
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
	  printf ("%s:*:%lu:%lu:%s%s::\n",
		  name, rid, rid == 18 ? 544 : rid, /* SYSTEM hack */
		  print_sids ? "," : "",
		  print_sids ? put_sid (sid) : "");
        }
      FreeSid (sid);
    }
}

int
usage (FILE * stream, int isNT)
{
  fprintf (stream, "Usage: mkpasswd [OPTION]... [domain]...\n"
	           "Print /etc/passwd file to stdout\n\n"
	           "Options:\n");
  if (isNT)
    fprintf (stream, "   -l,--local              print local user accounts\n"
	             "   -c,--current            print current account, if a domain account\n"
                     "   -d,--domain             print domain accounts (from current domain\n"
                     "                           if no domains specified)\n"
                     "   -o,--id-offset offset   change the default offset (10000) added to uids\n"
                     "                           in domain accounts.\n"
                     "   -g,--local-groups       print local group information too\n"
                     "                           if no domain specified\n"
                     "   -m,--no-mount           don't use mount points for home dir\n"
                     "   -s,--no-sids            don't print SIDs in GCOS field\n"
	             "                           (this affects ntsec)\n");
  fprintf (stream, "   -p,--path-to-home path  use specified path and not user account home dir or /home\n"
                   "   -u,--username username  only return information for the specified user\n"
                   "   -h,--help               displays this message\n"
	           "   -v,--version            version information and exit\n\n");
  if (isNT)
    fprintf (stream, "One of '-l', '-d' or '-g' must be given.\n");
  return 1;
}

struct option longopts[] = {
  {"local", no_argument, NULL, 'l'},
  {"current", no_argument, NULL, 'c'},
  {"domain", no_argument, NULL, 'd'},
  {"id-offset", required_argument, NULL, 'o'},
  {"local-groups", no_argument, NULL, 'g'},
  {"no-mount", no_argument, NULL, 'm'},
  {"no-sids", no_argument, NULL, 's'},
  {"path-to-home", required_argument, NULL, 'p'},
  {"username", required_argument, NULL, 'u'},
  {"help", no_argument, NULL, 'h'},
  {"version", no_argument, NULL, 'v'},
  {0, no_argument, NULL, 0}
};

char opts[] = "lcdo:gsmhp:u:v";

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
Copyright 1997, 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.\n\
Compiled on %s\n\
", len, v, __DATE__);
}

int
main (int argc, char **argv)
{
  LPWSTR servername = NULL;
  DWORD rc = ERROR_SUCCESS;
  WCHAR domain_name[200];
  int print_local = 0;
  int print_current = 0;
  int print_domain = 0;
  int print_local_groups = 0;
  int domain_specified = 0;
  int print_sids = 1;
  int print_cygpath = 1;
  int id_offset = 10000;
  int i;
  int isNT;
  char *disp_username = NULL;
  char name[256], passed_home_path[MAX_PATH];
  DWORD len;

  isNT = (GetVersion () < 0x80000000);
  passed_home_path[0] = '\0';
  if (!isatty (1))
    setmode (1, O_BINARY);

  if (isNT && argc == 1)
    return usage (stderr, isNT);
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
	  case 'g':
	    print_local_groups = 1;
	    break;
	  case 's':
	    print_sids = 0;
	    break;
	  case 'm':
	    print_cygpath = 0;
	    break;
	  case 'p':
	    if (optarg[0] != '/')
	    {
	      fprintf (stderr, "%s: '%s' is not a fully qualified path.\n",
		       argv[0], optarg);
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
  if (!isNT)
    {
      /* This takes Windows 9x/ME into account. */
      if (passed_home_path[0] == '\0')
	strcpy (passed_home_path, "/home/");
      if (!disp_username)
        {
	  printf ("admin:use_crypt:%lu:%lu:Administrator:%sadmin:/bin/bash\n", 
		  DOMAIN_USER_RID_ADMIN,
		  DOMAIN_ALIAS_RID_ADMINS,
		  passed_home_path);
	  if (GetUserName (name, (len = 256, &len)))
	    disp_username = name;
	}
      if (disp_username && disp_username[0])
        {
	  /* Create a pseudo random uid */
	  unsigned long uid = 0, i;
	  for (i = 0; disp_username[i]; i++)
	    uid += toupper (disp_username[i]) << ((6 * i) % 25);
	  uid = (uid % (1000 - DOMAIN_USER_RID_ADMIN - 1)) 
	    + DOMAIN_USER_RID_ADMIN + 1;
    	  
	  printf ("%s:use_crypt:%lu:%lu:%s:%s%s:/bin/bash\n", 
		  disp_username,
		  uid,
		  DOMAIN_ALIAS_RID_ADMINS,
		  disp_username,
		  passed_home_path,
		  disp_username);
	}
      return 0;
    }
  if (!print_local && !print_domain && !print_local_groups)
    {
      fprintf (stderr, "%s: Specify one of '-l', '-d' or '-g'\n", argv[0]);
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

  if (disp_username == NULL)
    {
      if (print_local)
        {
	  /* Generate service starter account entries. */
	  printf ("SYSTEM:*:18:544:,S-1-5-18::\n");
	  printf ("LocalService:*:19:544:U-NT AUTHORITY\\LocalService,S-1-5-19::\n");
	  printf ("NetworkService:*:20:544:U-NT AUTHORITY\\NetworkService,S-1-5-20::\n");
	  /* Get 'administrators' group (has localized name). */
	  if (!print_local_groups)
	    print_special (print_sids, &sid_nt_auth, 2, SECURITY_BUILTIN_DOMAIN_RID,
			   DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0);
	}
      if (print_local_groups)
	enum_local_groups (print_sids);
    }

  if (print_local)
    enum_users (NULL, print_sids, print_cygpath, passed_home_path, 0,
    		disp_username);

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
		rc = netgetdcname (server, domain_name, (void *) &servername);
		netapibufferfree (server);
	      }
	    if (rc != ERROR_SUCCESS)
	      {
		print_win_error(rc);
		return 1;
	      }
          }
	enum_users (servername, print_sids, print_cygpath, passed_home_path,
		    id_offset * i++, disp_username);
	netapibufferfree (pdci ? (PVOID) pdci : (PVOID) servername);
      }
    while (++optind < argc);

  if (print_current && !print_domain)
    current_user(print_sids, print_cygpath, passed_home_path,
		 id_offset, disp_username);

  return 0;
}
