/* security.cc: NT security functions

   Copyright 1997, 1998, 1999, 2000 Cygnus Solutions.

   Originaly written by Gunther Ebert, gunther.ebert@ixos-leipzig.de
   Extensions by Corinna Vinschen <corinna.vinschen@cityweb.de>

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <grp.h>
#include <pwd.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/acl.h>
#include <ctype.h>

extern BOOL allow_ntea;
BOOL allow_ntsec = FALSE;

SID_IDENTIFIER_AUTHORITY sid_auth[] = {
        {SECURITY_NULL_SID_AUTHORITY},
        {SECURITY_WORLD_SID_AUTHORITY},
        {SECURITY_LOCAL_SID_AUTHORITY},
        {SECURITY_CREATOR_SID_AUTHORITY},
        {SECURITY_NON_UNIQUE_AUTHORITY},
        {SECURITY_NT_AUTHORITY}
};

#define DONT_INHERIT (0)
#define INHERIT_ALL  (CONTAINER_INHERIT_ACE|OBJECT_INHERIT_ACE)
#define INHERIT_ONLY (INHERIT_ONLY_ACE|CONTAINER_INHERIT_ACE|OBJECT_INHERIT_ACE)

char *
convert_sid_to_string_sid (PSID psid, char *sid_str)
{
  char t[32];
  DWORD i;

  if (!psid || !sid_str)
    return NULL;
  strcpy (sid_str, "S-1-");
  sprintf(t, "%u", GetSidIdentifierAuthority (psid)->Value[5]);
  strcat (sid_str, t);
  for (i = 0; i < *GetSidSubAuthorityCount (psid); ++i)
    {
      sprintf(t, "-%lu", *GetSidSubAuthority (psid, i));
      strcat (sid_str, t);
    }
  return sid_str;
}

PSID
get_sid (PSID psid, DWORD s, DWORD cnt, DWORD *r)
{
  DWORD i;

  if (! psid || s > 5 || cnt < 1 || cnt > 8)
    return NULL;

  InitializeSid(psid, &sid_auth[s], cnt);
  for (i = 0; i < cnt; ++i)
    memcpy ((char *) psid + 8 + sizeof (DWORD) * i, &r[i], sizeof (DWORD));
  return psid;
}

PSID
convert_string_sid_to_sid (PSID psid, const char *sid_str)
{
  char sid_buf[256];
  char *t;
  DWORD cnt = 0;
  DWORD s = 0;
  DWORD i, r[8];

  if (! sid_str || strncmp (sid_str, "S-1-", 4))
    return NULL;

  strcpy (sid_buf, sid_str);

  for (t = sid_buf + 4, i = 0; cnt < 8 && (t = strtok (t, "-")); t = NULL, ++i)
    if (i == 0)
      s = strtoul (t, NULL, 10);
    else
      r[cnt++] = strtoul (t, NULL, 10);

  return get_sid (psid, s, cnt, r);
}

BOOL
get_pw_sid (PSID sid, struct passwd *pw)
{
  char *sp = strrchr (pw->pw_gecos, ',');

  if (!sp)
    return FALSE;
  return convert_string_sid_to_sid (sid, ++sp) != NULL;
}

BOOL
get_gr_sid (PSID sid, struct group *gr)
{
  return convert_string_sid_to_sid (sid, gr->gr_passwd) != NULL;
}

PSID
get_admin_sid ()
{
  static NO_COPY char admin_sid_buf[MAX_SID_LEN];
  static NO_COPY PSID admin_sid = NULL;

  if (!admin_sid)
    {
      admin_sid = (PSID) admin_sid_buf;
      convert_string_sid_to_sid (admin_sid, "S-1-5-32-544");
    }
  return admin_sid;
}

PSID
get_system_sid ()
{
  static NO_COPY char system_sid_buf[MAX_SID_LEN];
  static NO_COPY PSID system_sid = NULL;

  if (!system_sid)
    {
      system_sid = (PSID) system_sid_buf;
      convert_string_sid_to_sid (system_sid, "S-1-5-18");
    }
  return system_sid;
}

PSID
get_creator_owner_sid ()
{
  static NO_COPY char owner_sid_buf[MAX_SID_LEN];
  static NO_COPY PSID owner_sid = NULL;

  if (!owner_sid)
    {
      owner_sid = (PSID) owner_sid_buf;
      convert_string_sid_to_sid (owner_sid, "S-1-3-0");
    }
  return owner_sid;
}

PSID
get_world_sid ()
{
  static NO_COPY char world_sid_buf[MAX_SID_LEN];
  static NO_COPY PSID world_sid = NULL;

  if (!world_sid)
    {
      world_sid = (PSID) world_sid_buf;
      convert_string_sid_to_sid (world_sid, "S-1-1-0");
    }
  return world_sid;
}

int passwd_sem = 0;
int group_sem = 0;

static int
get_id_from_sid (PSID psid, BOOL search_grp, int *type)
{
  if (!IsValidSid (psid))
    {
      __seterrno ();
      small_printf ("IsValidSid failed with %E");
      return -1;
    }

  /* First try to get SID from passwd or group entry */
  if (allow_ntsec)
    {
      char sidbuf[MAX_SID_LEN];
      PSID sid = (PSID) sidbuf;
      int id = -1;

      if (! search_grp)
        {
          if (passwd_sem > 0)
            return 0;
          ++passwd_sem;

          struct passwd *pw;
          while ((pw = getpwent ()) != NULL)
            {
              if (get_pw_sid (sid, pw) && EqualSid (psid, sid))
                {
                  id = pw->pw_uid;
                  break;
                }
            }
          endpwent ();
          --passwd_sem;
          if (id >= 0)
            {
              if (type)
                *type = USER;
              return id;
            }
        }
      if (search_grp || type)
        {
          if (group_sem > 0)
            return 0;
          ++group_sem;

          struct group *gr;
          while ((gr = getgrent ()) != NULL)
            {
              if (get_gr_sid (sid, gr) && EqualSid (psid, sid))
                {
                  id = gr->gr_gid;
                  break;
                }
            }
          endgrent ();
          --group_sem;
          if (id >= 0)
            {
              if (type)
                *type = GROUP;
              return id;
            }
        }
    }

  /* We use the RID as default UID/GID */
  int id = *GetSidSubAuthority(psid, *GetSidSubAuthorityCount(psid) - 1);

  /*
   * The RID maybe -1 if accountname == computername.
   * In this case we search for the accountname in the passwd and group files.
   * If type is needed, we search in each case.
   */
  if (id == -1 || type)
    {
      char account[MAX_USER_NAME];
      char domain[MAX_COMPUTERNAME_LENGTH+1];
      DWORD acc_len = MAX_USER_NAME;
      DWORD dom_len = MAX_COMPUTERNAME_LENGTH+1;
      SID_NAME_USE acc_type;

      if (!LookupAccountSid (NULL, psid, account, &acc_len,
                             domain, &dom_len, &acc_type))
	{
	  __seterrno ();
	  return -1;
	}

      switch (acc_type)
	{
	  case SidTypeGroup:
	  case SidTypeAlias:
	  case SidTypeWellKnownGroup:
            if (type)
              *type = GROUP;
            if (id == -1)
              {
                struct group *gr = getgrnam (account);
                if (gr)
                  id = gr->gr_gid;
              }
            break;
	  case SidTypeUser:
            if (type)
              *type = USER;
            if (id == -1)
              {
	        struct passwd *pw = getpwnam (account);
                if (pw)
                  id = pw->pw_uid;
	      }
            break;
	  default:
            break;
	}
    }
  if (id == -1)
    id = getuid ();
  return id;
}

int
get_id_from_sid (PSID psid, BOOL search_grp)
{
  return get_id_from_sid (psid, search_grp, NULL);
}

static BOOL
legal_sid_type (SID_NAME_USE type)
{
  return type == SidTypeUser || type == SidTypeGroup
                 || SidTypeAlias || SidTypeWellKnownGroup;
}

BOOL
is_grp_member (uid_t uid, gid_t gid)
{
  extern int getgroups (int, gid_t *, gid_t, const char *);
  BOOL grp_member = TRUE;

  if (!group_sem && !passwd_sem)
    {
      struct passwd *pw = getpwuid (uid);
      gid_t grps[NGROUPS_MAX];
      int cnt = getgroups (NGROUPS_MAX, grps,
                           pw ? pw->pw_gid : myself->gid,
                           pw ? pw->pw_name : myself->username);
      int i;
      for (i = 0; i < cnt; ++i)
        if (grps[i] == gid)
          break;
      grp_member = (i < cnt);
    }
  return grp_member;
}

BOOL
lookup_name (const char *name, const char *logsrv, PSID ret_sid)
{
  char sidbuf[MAX_SID_LEN];
  PSID sid = (PSID) sidbuf;
  DWORD sidlen;
  char domuser[MAX_COMPUTERNAME_LENGTH+MAX_USER_NAME+1];
  char dom[MAX_COMPUTERNAME_LENGTH+1];
  DWORD domlen;
  SID_NAME_USE acc_type;

  debug_printf ("name  : %s", name ? name : "NULL");

  if (! name)
    return FALSE;

  if (*myself->domain)
    {
      strcat (strcat (strcpy (domuser, myself->domain), "\\"), name);
      if (LookupAccountName (NULL, domuser,
                             sid, (sidlen = MAX_SID_LEN, &sidlen),
                             dom, (domlen = MAX_COMPUTERNAME_LENGTH, &domlen),
                             &acc_type)
          && legal_sid_type (acc_type))
        goto got_it;
      if (logsrv && *logsrv
          && LookupAccountName (logsrv, domuser,
                                sid, (sidlen = MAX_SID_LEN, &sidlen),
                                dom, (domlen = MAX_COMPUTERNAME_LENGTH,&domlen),
                                &acc_type)
          && legal_sid_type (acc_type))
        goto got_it;
    }
  if (logsrv && *logsrv)
    {
      if (LookupAccountName (logsrv, name,
                             sid, (sidlen = MAX_SID_LEN, &sidlen),
                             dom, (domlen = MAX_COMPUTERNAME_LENGTH, &domlen),
                             &acc_type)
          && legal_sid_type (acc_type))
        goto got_it;
      if (acc_type == SidTypeDomain)
        {
          strcat (strcat (strcpy (domuser, dom), "\\"), name);
          if (LookupAccountName (logsrv, domuser,
                                 sid,(sidlen = MAX_SID_LEN, &sidlen),
                                 dom,(domlen = MAX_COMPUTERNAME_LENGTH,&domlen),
                                 &acc_type))
            goto got_it;
        }
    }
  if (LookupAccountName (NULL, name,
                         sid, (sidlen = MAX_SID_LEN, &sidlen),
                         dom, (domlen = 100, &domlen),
                         &acc_type)
      && legal_sid_type (acc_type))
    goto got_it;
  if (acc_type == SidTypeDomain)
    {
      strcat (strcat (strcpy (domuser, dom), "\\"), name);
      if (LookupAccountName (NULL, domuser,
                             sid, (sidlen = MAX_SID_LEN, &sidlen),
                             dom, (domlen = MAX_COMPUTERNAME_LENGTH, &domlen),
                             &acc_type))
        goto got_it;
    }
  debug_printf ("LookupAccountName(%s) %E", name);
  __seterrno ();
  return FALSE;

got_it:
  debug_printf ("sid : [%d]", *GetSidSubAuthority((PSID) sid,
                              *GetSidSubAuthorityCount((PSID) sid) - 1));

  if (ret_sid)
    memcpy (ret_sid, sid, sidlen);

  return TRUE;
}

extern "C"
void
cygwin_set_impersonation_token (const HANDLE hToken)
{
  debug_printf ("set_impersonation_token (%d)", hToken);
  if (myself->token != hToken)
    {
      if (myself->token != INVALID_HANDLE_VALUE)
        CloseHandle (myself->token);
      myself->token = hToken;
      myself->impersonated = FALSE;
    }
}

extern "C"
HANDLE
cygwin_logon_user (const struct passwd *pw, const char *password)
{
  if (os_being_run != winNT)
    {
      set_errno (ENOSYS);
      return INVALID_HANDLE_VALUE;
    }
  if (!pw)
    {
      set_errno (EINVAL);
      return INVALID_HANDLE_VALUE;
    }

  char *c, *nt_user, *nt_domain = NULL;
  char usernamebuf[256];
  HANDLE hToken;

  strcpy (usernamebuf, pw->pw_name);
  if (pw->pw_gecos)
    {
      if ((c = strstr (pw->pw_gecos, "U-")) != NULL &&
          (c == pw->pw_gecos || c[-1] == ','))
        {
          usernamebuf[0] = '\0';
          strncat (usernamebuf, c + 2, 255);
          if ((c = strchr (usernamebuf, ',')) != NULL)
            *c = '\0';
        }
    }
  nt_user = usernamebuf;
  if ((c = strchr (nt_user, '\\')) != NULL)
    {
      nt_domain = nt_user;
      *c = '\0';
      nt_user = c + 1;
    }
  if (! LogonUserA (nt_user, nt_domain, (char *) password,
                    LOGON32_LOGON_INTERACTIVE,
                    LOGON32_PROVIDER_DEFAULT,
                    &hToken)
      || !SetHandleInformation (hToken,
                                HANDLE_FLAG_INHERIT,
                                HANDLE_FLAG_INHERIT))
    {
      __seterrno ();
      return INVALID_HANDLE_VALUE;
    }
  debug_printf ("%d = logon_user(%s,...)", hToken, pw->pw_name);
  return hToken;
}

/* read_sd reads a security descriptor from a file.
   In case of error, -1 is returned and errno is set.
   If sd_buf is too small, 0 is returned and sd_size
   is set to the needed buffer size.
   On success, 1 is returned.

   GetFileSecurity() is used instead of BackupRead()
   to avoid access denied errors if the caller has
   not the permission to open that file for read.

   Originally the function should return the size
   of the SD on success. Unfortunately NT returns
   0 in `len' on success, while W2K returns the
   correct size!
*/

LONG
read_sd(const char *file, PSECURITY_DESCRIPTOR sd_buf, LPDWORD sd_size)
{
  /* Check parameters */
  if (! sd_size)
    {
      set_errno (EINVAL);
      return -1;
    }

  debug_printf("file = %s", file);

  DWORD len = 0;
  if (! GetFileSecurity (file,
                         OWNER_SECURITY_INFORMATION
                         | GROUP_SECURITY_INFORMATION
                         | DACL_SECURITY_INFORMATION,
                         sd_buf, *sd_size, &len))
    {
      __seterrno ();
      return -1;
    }
  debug_printf("file = %s: len=%d", file, len);
  if (len > *sd_size)
    {
      *sd_size = len;
      return 0;
    }
  return 1;
}

LONG
write_sd(const char *file, PSECURITY_DESCRIPTOR sd_buf, DWORD sd_size)
{
  /* Check parameters */
  if (! sd_buf || ! sd_size)
    {
      set_errno (EINVAL);
      return -1;
    }

  HANDLE fh;
  fh = CreateFile (file,
                   WRITE_OWNER | WRITE_DAC,
                   FILE_SHARE_READ | FILE_SHARE_WRITE,
                   &sec_none_nih,
                   OPEN_EXISTING,
                   FILE_ATTRIBUTE_NORMAL | FILE_FLAG_BACKUP_SEMANTICS,
                   NULL);

  if (fh == INVALID_HANDLE_VALUE)
    {
      __seterrno ();
      return -1;
    }

  LPVOID context = NULL;
  DWORD bytes_written = 0;
  WIN32_STREAM_ID header;

  memset (&header, 0, sizeof (header));
  /* write new security info header */
  header.dwStreamId = BACKUP_SECURITY_DATA;
  header.dwStreamAttributes = STREAM_CONTAINS_SECURITY;
  header.Size.HighPart = 0;
  header.Size.LowPart = sd_size;
  header.dwStreamNameSize = 0;
  if (!BackupWrite (fh, (LPBYTE) &header,
		    3 * sizeof (DWORD) + sizeof (LARGE_INTEGER),
		    &bytes_written, FALSE, TRUE, &context))
    {
      __seterrno ();
      CloseHandle (fh);
      return -1;
    }

  /* write new security descriptor */
  if (!BackupWrite (fh, (LPBYTE) sd_buf,
		    header.Size.LowPart + header.dwStreamNameSize,
		    &bytes_written, FALSE, TRUE, &context))
    {
      /* Samba returns ERROR_NOT_SUPPORTED.
         FAT returns ERROR_INVALID_SECURITY_DESCR.
         This shouldn't return as error, but better be ignored. */
      DWORD ret = GetLastError ();
      if (ret != ERROR_NOT_SUPPORTED && ret != ERROR_INVALID_SECURITY_DESCR)
	{
	  __seterrno ();
	  BackupWrite (fh, NULL, 0, &bytes_written, TRUE, TRUE, &context);
	  CloseHandle (fh);
	  return -1;
	}
    }

  /* terminate the restore process */
  BackupWrite (fh, NULL, 0, &bytes_written, TRUE, TRUE, &context);
  CloseHandle (fh);
  return 0;
}

int
set_process_privileges ()
{
  HANDLE hProcess = NULL;
  HANDLE hToken = NULL;
  LUID restore_priv;
  LUID backup_priv;
  char buf[sizeof (TOKEN_PRIVILEGES) + 2 * sizeof (LUID_AND_ATTRIBUTES)];
  TOKEN_PRIVILEGES *new_priv = (TOKEN_PRIVILEGES *) buf;
  int ret = -1;

  hProcess = OpenProcess (PROCESS_ALL_ACCESS, FALSE, GetCurrentProcessId ());
  if (! hProcess)
    {
      __seterrno ();
      goto out;
    }

  if (! OpenProcessToken (hProcess,
			  TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES,
			  &hToken))
    {
      __seterrno ();
      goto out;
    }

  if (! LookupPrivilegeValue (NULL, SE_RESTORE_NAME, &restore_priv))
    {
      __seterrno ();
      goto out;
    }
  if (! LookupPrivilegeValue (NULL, SE_BACKUP_NAME, &backup_priv))
    {
      __seterrno ();
      goto out;
    }

  new_priv->PrivilegeCount = 2;
  new_priv->Privileges[0].Luid = restore_priv;
  new_priv->Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
  new_priv->Privileges[1].Luid = backup_priv;
  new_priv->Privileges[1].Attributes = SE_PRIVILEGE_ENABLED;

  if (! AdjustTokenPrivileges (hToken, FALSE, new_priv, 0, NULL, NULL))
    {
      __seterrno ();
      goto out;
    }

  ret = 0;

  if (ret == -1)
    __seterrno ();

out:
  if (hToken)
    CloseHandle (hToken);
  if (hProcess)
    CloseHandle (hProcess);

  syscall_printf ("%d = set_process_privileges ()", ret);
  return ret;
}

static int
get_nt_attribute (const char *file, int *attribute,
                  uid_t *uidret, gid_t *gidret)
{
  if (os_being_run != winNT)
    return 0;

  syscall_printf ("file: %s", file);

  /* Yeah, sounds too much, but I've seen SDs of 2100 bytes! */
  DWORD sd_size = 4096;
  char sd_buf[4096];
  PSECURITY_DESCRIPTOR psd = (PSECURITY_DESCRIPTOR) sd_buf;

  int ret;
  if ((ret = read_sd (file, psd, &sd_size)) <= 0)
    {
      debug_printf ("read_sd %E");
      return ret;
    }

  PSID owner_sid;
  PSID group_sid;
  BOOL dummy;

  if (! GetSecurityDescriptorOwner (psd, &owner_sid, &dummy))
    debug_printf ("GetSecurityDescriptorOwner %E");
  if (! GetSecurityDescriptorGroup (psd, &group_sid, &dummy))
    debug_printf ("GetSecurityDescriptorGroup %E");

  PACL acl;
  BOOL acl_exists;

  if (! GetSecurityDescriptorDacl (psd, &acl_exists, &acl, &dummy))
    {
      __seterrno ();
      debug_printf ("GetSecurityDescriptorDacl %E");
      return -1;
    }

  uid_t uid = get_uid_from_sid (owner_sid);
  gid_t gid = get_gid_from_sid (group_sid);
  if (uidret)
    *uidret = uid;
  if (gidret)
    *gidret = gid;

  if (! attribute)
    {
      syscall_printf ("file: %s uid %d, gid %d", file, uid, gid);
      return 0;
    }

  BOOL grp_member = is_grp_member (uid, gid);

  if (! acl_exists || ! acl)
    {
      *attribute |= S_IRWXU | S_IRWXG | S_IRWXO;
      syscall_printf ("file: %s No ACL = %x, uid %d, gid %d",
                      file, *attribute, uid, gid);
      return 0;
    }

  ACCESS_ALLOWED_ACE *ace;
  int allow = 0;
  int deny = 0;
  int *flags, *anti;

  for (DWORD i = 0; i < acl->AceCount; ++i)
    {
      if (!GetAce (acl, i, (PVOID *) &ace))
        continue;
      if (ace->Header.AceFlags & INHERIT_ONLY_ACE)
        continue;
      switch (ace->Header.AceType)
        {
        case ACCESS_ALLOWED_ACE_TYPE:
          flags = &allow;
          anti = &deny;
          break;
        case ACCESS_DENIED_ACE_TYPE:
          flags = &deny;
          anti = &allow;
          break;
        default:
          continue;
        }

      PSID ace_sid = (PSID) &ace->SidStart;
      if (owner_sid && EqualSid (ace_sid, owner_sid))
        {
          if (ace->Mask & FILE_READ_DATA)
            *flags |= S_IRUSR;
          if (ace->Mask & FILE_WRITE_DATA)
            *flags |= S_IWUSR;
          if (ace->Mask & FILE_EXECUTE)
            *flags |= S_IXUSR;
        }
      else if (group_sid && EqualSid (ace_sid, group_sid))
        {
          if (ace->Mask & FILE_READ_DATA)
            *flags |= S_IRGRP
                      | ((grp_member && !(*anti & S_IRUSR)) ? S_IRUSR : 0);
          if (ace->Mask & FILE_WRITE_DATA)
            *flags |= S_IWGRP
                      | ((grp_member && !(*anti & S_IWUSR)) ? S_IWUSR : 0);
          if (ace->Mask & FILE_EXECUTE)
            *flags |= S_IXGRP
                      | ((grp_member && !(*anti & S_IXUSR)) ? S_IXUSR : 0);
        }
      else if (EqualSid (ace_sid, get_world_sid ()))
        {
          if (ace->Mask & FILE_READ_DATA)
            *flags |= S_IROTH
                      | ((!(*anti & S_IRGRP)) ? S_IRGRP : 0)
                      | ((!(*anti & S_IRUSR)) ? S_IRUSR : 0);
          if (ace->Mask & FILE_WRITE_DATA)
            *flags |= S_IWOTH
                      | ((!(*anti & S_IWGRP)) ? S_IWGRP : 0)
                      | ((!(*anti & S_IWUSR)) ? S_IWUSR : 0);
          if (ace->Mask & FILE_EXECUTE)
            {
              *flags |= S_IXOTH
                        | ((!(*anti & S_IXGRP)) ? S_IXGRP : 0)
                        | ((!(*anti & S_IXUSR)) ? S_IXUSR : 0);
              // Sticky bit for directories according to linux rules.
              // No sense for files.
              if (! (ace->Mask & FILE_DELETE_CHILD)
                  && S_ISDIR(*attribute)
                  && !(*anti & S_ISVTX))
                *flags |= S_ISVTX;
            }
        }
    }
  *attribute &= ~(S_IRWXU|S_IRWXG|S_IRWXO|S_ISVTX);
  *attribute |= allow;
  *attribute &= ~deny;
  syscall_printf ("file: %s %x, uid %d, gid %d", file, *attribute, uid, gid);
  return 0;
}

int
get_file_attribute (int use_ntsec, const char *file,
                    int *attribute, uid_t *uidret, gid_t *gidret)
{
  if (use_ntsec && allow_ntsec)
    return get_nt_attribute (file, attribute, uidret, gidret);

  if (uidret)
    *uidret = getuid ();
  if (gidret)
    *gidret = getgid ();

  if (! attribute)
    return 0;

  int res = NTReadEA (file, ".UNIXATTR",
                      (char *) attribute, sizeof (*attribute));

  // symlinks are anything for everyone!
  if ((*attribute & S_IFLNK) == S_IFLNK)
    *attribute |= S_IRWXU | S_IRWXG | S_IRWXO;

  if (res <= 0)
    set_errno (ENOSYS);
  return res > 0 ? 0 : -1;
}

BOOL add_access_allowed_ace (PACL acl, int offset, DWORD attributes,
                             PSID sid, size_t &len_add, DWORD inherit)
{
  if (! AddAccessAllowedAce (acl, ACL_REVISION, attributes, sid))
    {
      __seterrno ();
      return FALSE;
    }
  ACCESS_ALLOWED_ACE *ace;
  if (GetAce(acl, offset, (PVOID *) &ace))
    ace->Header.AceFlags |= inherit;
  len_add += sizeof (ACCESS_DENIED_ACE) - sizeof (DWORD)
             + GetLengthSid (sid);
  return TRUE;
}

BOOL add_access_denied_ace (PACL acl, int offset, DWORD attributes,
                            PSID sid, size_t &len_add, DWORD inherit)
{
  if (! AddAccessDeniedAce (acl, ACL_REVISION, attributes, sid))
    {
      __seterrno ();
      return FALSE;
    }
  ACCESS_DENIED_ACE *ace;
  if (GetAce(acl, offset, (PVOID *) &ace))
    ace->Header.AceFlags |= inherit;
  len_add += sizeof (ACCESS_DENIED_ACE) - sizeof (DWORD)
             + GetLengthSid (sid);
  return TRUE;
}

PSECURITY_DESCRIPTOR
alloc_sd (uid_t uid, gid_t gid, const char *logsrv, int attribute,
	  PSECURITY_DESCRIPTOR sd_ret, DWORD *sd_size_ret)
{
  BOOL dummy;

  if (os_being_run != winNT)
    return NULL;

  if (! sd_ret || ! sd_size_ret)
    {
      set_errno (EINVAL);
      return NULL;
    }

  // Get SID and name of new owner
  char owner[MAX_USER_NAME];
  char *owner_sid_buf[MAX_SID_LEN];
  PSID owner_sid = NULL;
  struct passwd *pw = getpwuid (uid);
  strcpy (owner, pw ? pw->pw_name : getlogin ());
  owner_sid = (PSID) owner_sid_buf;
  if ((! pw || ! get_pw_sid (owner_sid, pw))
      && ! lookup_name (owner, logsrv, owner_sid))
    return NULL;
  debug_printf ("owner: %s [%d]", owner,
                *GetSidSubAuthority((PSID) owner_sid,
		*GetSidSubAuthorityCount((PSID) owner_sid) - 1));

  // Get SID and name of new group
  char *group_sid_buf[MAX_SID_LEN];
  PSID group_sid = NULL;
  struct group *grp = getgrgid (gid);
  if (grp)
    {
      group_sid = (PSID) group_sid_buf;
      if ((! grp || ! get_gr_sid (group_sid, grp))
          && ! lookup_name (grp->gr_name, logsrv, group_sid))
        return NULL;
    }
  else
    debug_printf ("no group");

  // Initialize local security descriptor
  SECURITY_DESCRIPTOR sd;
  PSECURITY_DESCRIPTOR psd = NULL;
  if (! InitializeSecurityDescriptor (&sd, SECURITY_DESCRIPTOR_REVISION))
    {
      __seterrno ();
      return NULL;
    }

  if (! SetSecurityDescriptorOwner(&sd, owner_sid, FALSE))
    {
      __seterrno ();
      return NULL;
    }
  if (group_sid
      && ! SetSecurityDescriptorGroup(&sd, group_sid, FALSE))
    {
      __seterrno ();
      return NULL;
    }

  // Initialize local access control list
  char acl_buf[3072];
  PACL acl = (PACL) acl_buf;
  if (! InitializeAcl (acl, 3072, ACL_REVISION))
    {
      __seterrno ();
      return NULL;
    }

  // VTX bit may only be set if executable for `other' is set.
  // For correct handling under WinNT, FILE_DELETE_CHILD has to
  // be (un)set in each ACE.
  if (! (attribute & S_IXOTH))
    attribute &= ~S_ISVTX;
  if (! (attribute & S_IFDIR))
    attribute |= S_ISVTX;

  // From here fill ACL
  size_t acl_len = sizeof (ACL);
  int ace_off = 0;

  // Construct allow attribute for owner
  DWORD owner_allow = (STANDARD_RIGHTS_ALL & ~DELETE)
                      | FILE_WRITE_ATTRIBUTES | FILE_WRITE_EA;
  if (attribute & S_IRUSR)
    owner_allow |= FILE_GENERIC_READ;
  if (attribute & S_IWUSR)
    owner_allow |= FILE_GENERIC_WRITE | DELETE;
  if (attribute & S_IXUSR)
    owner_allow |= FILE_GENERIC_EXECUTE;
  if (! (attribute & S_ISVTX))
    owner_allow |= FILE_DELETE_CHILD;

  // Construct allow attribute for group
  DWORD group_allow = STANDARD_RIGHTS_READ
                      | FILE_READ_ATTRIBUTES | FILE_READ_EA;
  if (attribute & S_IRGRP)
    group_allow |= FILE_GENERIC_READ;
  if (attribute & S_IWGRP)
    group_allow |= STANDARD_RIGHTS_WRITE | FILE_GENERIC_WRITE | DELETE;
  if (attribute & S_IXGRP)
    group_allow |= FILE_GENERIC_EXECUTE;
  if (! (attribute & S_ISVTX))
    group_allow |= FILE_DELETE_CHILD;

  // Construct allow attribute for everyone
  DWORD other_allow = STANDARD_RIGHTS_READ
                      | FILE_READ_ATTRIBUTES | FILE_READ_EA;
  if (attribute & S_IROTH)
    other_allow |= FILE_GENERIC_READ;
  if (attribute & S_IWOTH)
    other_allow |= STANDARD_RIGHTS_WRITE | FILE_GENERIC_WRITE | DELETE;
  if (attribute & S_IXOTH)
    other_allow |= FILE_GENERIC_EXECUTE;
  if (! (attribute & S_ISVTX))
    other_allow |= FILE_DELETE_CHILD;
  
  // Construct deny attributes for owner and group
  DWORD owner_deny = 0;
  if (is_grp_member (uid, gid))
    owner_deny = ~owner_allow & (group_allow | other_allow);
  else
    owner_deny = ~owner_allow & other_allow;
  owner_deny &= ~(STANDARD_RIGHTS_READ
                  | FILE_READ_ATTRIBUTES | FILE_READ_EA
                  | FILE_WRITE_ATTRIBUTES | FILE_WRITE_EA);
  DWORD group_deny = ~group_allow & other_allow;
  group_deny &= ~(STANDARD_RIGHTS_READ | FILE_READ_ATTRIBUTES | FILE_READ_EA);

  // Construct appropriate inherit attribute
  DWORD inherit = (attribute & S_IFDIR) ? INHERIT_ALL : DONT_INHERIT;

  // Set deny ACE for owner
  if (owner_deny
      && ! add_access_denied_ace (acl, ace_off++, owner_deny,
                                  owner_sid, acl_len, inherit))
      return NULL;
  // Set allow ACE for owner
  if (! add_access_allowed_ace (acl, ace_off++, owner_allow,
                                owner_sid, acl_len, inherit))
    return NULL;
  // Set deny ACE for group
  if (group_deny
      && ! add_access_denied_ace (acl, ace_off++, group_deny,
                                  group_sid, acl_len, inherit))
      return NULL;
  // Set allow ACE for group
  if (! add_access_allowed_ace (acl, ace_off++, group_allow,
                                group_sid, acl_len, inherit))
    return NULL;

  // Get owner and group from current security descriptor
  PSID cur_owner_sid = NULL;
  PSID cur_group_sid = NULL;
  if (! GetSecurityDescriptorOwner (sd_ret, &cur_owner_sid, &dummy))
    debug_printf ("GetSecurityDescriptorOwner %E");
  if (! GetSecurityDescriptorGroup (sd_ret, &cur_group_sid, &dummy))
    debug_printf ("GetSecurityDescriptorGroup %E");

  // Fill ACL with unrelated ACEs from current security descriptor
  PACL oacl;
  BOOL acl_exists;
  ACCESS_ALLOWED_ACE *ace;
  if (GetSecurityDescriptorDacl (sd_ret, &acl_exists, &oacl, &dummy)
      && acl_exists && oacl)
    for (DWORD i = 0; i < oacl->AceCount; ++i)
      if (GetAce (oacl, i, (PVOID *) &ace))
        {
          PSID ace_sid = (PSID) &ace->SidStart;
          // Check for related ACEs
          if ((cur_owner_sid && EqualSid (ace_sid, cur_owner_sid))
              || (owner_sid && EqualSid (ace_sid, owner_sid))
              || (cur_group_sid && EqualSid (ace_sid, cur_group_sid))
              || (group_sid && EqualSid (ace_sid, group_sid))
              || (EqualSid (ace_sid, get_world_sid ())))
            continue;
          // Add unrelated ACCESS_DENIED_ACE to the beginning but
          // behind the owner_deny, ACCESS_ALLOWED_ACE to the end
          // but in front of the `everyone' ACE.
          if (! AddAce(acl, ACL_REVISION,
                       ace->Header.AceType == ACCESS_DENIED_ACE_TYPE ?
                       (owner_deny ? 1 : 0) : MAXDWORD,
                       (LPVOID) ace, ace->Header.AceSize))
            {
              __seterrno ();
              return NULL;
            }
          acl_len += ace->Header.AceSize;
          ++ace_off;
        }

  // Set allow ACE for everyone
  if (! add_access_allowed_ace (acl, ace_off++, other_allow,
                                get_world_sid (), acl_len, inherit))
    return NULL;

  // Set AclSize to computed value
  acl->AclSize = acl_len;
  debug_printf ("ACL-Size: %d", acl_len);

  // Create DACL for local security descriptor
  if (! SetSecurityDescriptorDacl (&sd, TRUE, acl, FALSE))
    {
      __seterrno ();
      return NULL;
    }

  // Make self relative security descriptor
  *sd_size_ret = 0;
  MakeSelfRelativeSD (&sd, sd_ret, sd_size_ret);
  if (*sd_size_ret <= 0)
    {
      __seterrno ();
      return NULL;
    }
  if (! MakeSelfRelativeSD (&sd, sd_ret, sd_size_ret))
    {
      __seterrno ();
      return NULL;
    }
  psd = sd_ret;
  debug_printf ("Created SD-Size: %d", *sd_size_ret);

  return psd;
}

static int
set_nt_attribute (const char *file, uid_t uid, gid_t gid,
                  const char *logsrv, int attribute)
{
  if (os_being_run != winNT)
    return 0;

  DWORD sd_size = 4096;
  char sd_buf[4096];
  PSECURITY_DESCRIPTOR psd = (PSECURITY_DESCRIPTOR) sd_buf;

  int ret;
  if ((ret = read_sd (file, psd, &sd_size)) <= 0)
    {
      debug_printf ("read_sd %E");
      return ret;
    }

  sd_size = 4096;
  if (! (psd = alloc_sd (uid, gid, logsrv, attribute, psd, &sd_size)))
    return -1;

  return write_sd (file, psd, sd_size);
}

int
set_file_attribute (int use_ntsec, const char *file,
                    uid_t uid, gid_t gid,
                    int attribute, const char *logsrv)
{
  // symlinks are anything for everyone!
  if ((attribute & S_IFLNK) == S_IFLNK)
    attribute |= S_IRWXU | S_IRWXG | S_IRWXO;

  if (!use_ntsec || !allow_ntsec)
    {
      if (! NTWriteEA (file, ".UNIXATTR",
                       (char *) &attribute, sizeof (attribute)))
	{
	  __seterrno ();
	  return -1;
	}
      return 0;
    }

  int ret = set_nt_attribute (file, uid, gid, logsrv, attribute);
  syscall_printf ("%d = set_file_attribute (%s, %d, %d, %p)",
		  ret, file, uid, gid, attribute);
  return ret;
}

int
set_file_attribute (int use_ntsec, const char *file, int attribute)
{
  return set_file_attribute (use_ntsec, file,
                             myself->uid, myself->gid,
                             attribute, myself->logsrv);
}

static int
searchace (aclent_t *aclp, int nentries, int type, int id = -1)
{
  int i;

  for (i = 0; i < nentries; ++i)
    if ((aclp[i].a_type == type && (id < 0 || aclp[i].a_id == id))
        || !aclp[i].a_type)
      return i;
  return -1;
}

static int
setacl (const char *file, int nentries, aclent_t *aclbufp)
{
  DWORD sd_size = 4096;
  char sd_buf[4096];
  PSECURITY_DESCRIPTOR psd = (PSECURITY_DESCRIPTOR) sd_buf;

  if (read_sd (file, psd, &sd_size) <= 0)
    {
      debug_printf ("read_sd %E");
      return -1;
    }

  BOOL dummy;

  // Get owner SID
  PSID owner_sid = NULL;
  if (! GetSecurityDescriptorOwner (psd, &owner_sid, &dummy))
    {
      __seterrno ();
      return -1;
    }
  char owner_buf[MAX_SID_LEN];
  if (!CopySid (MAX_SID_LEN, (PSID) owner_buf, owner_sid))
    {
      __seterrno ();
      return -1;
    }
  owner_sid = (PSID) owner_buf;

  // Get group SID
  PSID group_sid = NULL;
  if (! GetSecurityDescriptorGroup (psd, &group_sid, &dummy))
    {
      __seterrno ();
      return -1;
    }
  char group_buf[MAX_SID_LEN];
  if (!CopySid (MAX_SID_LEN, (PSID) group_buf, group_sid))
    {
      __seterrno ();
      return -1;
    }
  group_sid = (PSID) group_buf;

  // Initialize local security descriptor
  SECURITY_DESCRIPTOR sd;
  if (! InitializeSecurityDescriptor (&sd, SECURITY_DESCRIPTOR_REVISION))
    {
      __seterrno ();
      return -1;
    }
  if (! SetSecurityDescriptorOwner(&sd, owner_sid, FALSE))
    {
      __seterrno ();
      return -1;
    }
  if (group_sid
      && ! SetSecurityDescriptorGroup(&sd, group_sid, FALSE))
    {
      __seterrno ();
      return -1;
    }

  // Fill access control list
  char acl_buf[3072];
  PACL acl = (PACL) acl_buf;
  size_t acl_len = sizeof (ACL);
  int ace_off = 0;

  char sidbuf[MAX_SID_LEN];
  PSID sid = (PSID) sidbuf;
  struct passwd *pw;
  struct group *gr;
  int pos;

  if (! InitializeAcl (acl, 3072, ACL_REVISION))
    {
      __seterrno ();
      return -1;
    }
  for (int i = 0; i < nentries; ++i)
    {
      DWORD allow = STANDARD_RIGHTS_READ
                    | FILE_READ_ATTRIBUTES | FILE_READ_EA;
      if (aclbufp[i].a_perm & S_IROTH)
        allow |= FILE_GENERIC_READ;
      if (aclbufp[i].a_perm & S_IWOTH)
        allow |= STANDARD_RIGHTS_ALL | FILE_GENERIC_WRITE
                 | DELETE | FILE_DELETE_CHILD;
      if (aclbufp[i].a_perm & S_IXOTH)
        allow |= FILE_GENERIC_EXECUTE;
      // Set inherit property
      DWORD inheritance = (aclbufp[i].a_type & ACL_DEFAULT)
                          ? INHERIT_ONLY : DONT_INHERIT;
      // If a specific acl contains a corresponding default entry with
      // identical permissions, only one Windows ACE with proper
      // inheritance bits is created.
      if (!(aclbufp[i].a_type & ACL_DEFAULT)
          && (pos = searchace (aclbufp, nentries,
                               aclbufp[i].a_type | ACL_DEFAULT,
                               (aclbufp[i].a_type & (USER|GROUP))
                               ? aclbufp[i].a_id : -1)) >= 0
          && pos < nentries
          && aclbufp[i].a_perm == aclbufp[pos].a_perm)
        {
          inheritance = INHERIT_ALL;
          // This eliminates the corresponding default entry.
          aclbufp[pos].a_type = 0;
        }
      switch (aclbufp[i].a_type)
        {
        case USER_OBJ:
        case DEF_USER_OBJ:
          allow |= STANDARD_RIGHTS_ALL & ~DELETE;
          if (! add_access_allowed_ace (acl, ace_off++, allow,
                                        owner_sid, acl_len, inheritance))
            return -1;
          break;
        case USER:
        case DEF_USER:
          if (!(pw = getpwuid (aclbufp[i].a_id))
              || ! get_pw_sid (sid, pw)
              || ! add_access_allowed_ace (acl, ace_off++, allow,
                                           sid, acl_len, inheritance))
            return -1;
          break;
        case GROUP_OBJ:
        case DEF_GROUP_OBJ:
          if (! add_access_allowed_ace (acl, ace_off++, allow,
                                        group_sid, acl_len, inheritance))
            return -1;
          break;
        case GROUP:
        case DEF_GROUP:
          if (!(gr = getgrgid (aclbufp[i].a_id))
              || ! get_gr_sid (sid, gr)
              || ! add_access_allowed_ace (acl, ace_off++, allow,
                                           sid, acl_len, inheritance))
            return -1;
          break;
        case OTHER_OBJ:
        case DEF_OTHER_OBJ:
          if (! add_access_allowed_ace (acl, ace_off++, allow,
                                        get_world_sid(), acl_len, inheritance))
            return -1;
          break;
        }
    }
  // Set AclSize to computed value
  acl->AclSize = acl_len;
  debug_printf ("ACL-Size: %d", acl_len);
  // Create DACL for local security descriptor
  if (! SetSecurityDescriptorDacl (&sd, TRUE, acl, FALSE))
    {
      __seterrno ();
      return -1;
    }
  // Make self relative security descriptor in psd
  sd_size = 0;
  MakeSelfRelativeSD (&sd, psd, &sd_size);
  if (sd_size <= 0)
    {
      __seterrno ();
      return -1;
    }
  if (! MakeSelfRelativeSD (&sd, psd, &sd_size))
    {
      __seterrno ();
      return -1;
    }
  debug_printf ("Created SD-Size: %d", sd_size);
  return write_sd (file, psd, sd_size);
}

static void
getace (aclent_t &acl, int type, int id, DWORD win_ace_mask, DWORD win_ace_type)
{
  acl.a_type = type;
  acl.a_id = id;

  if (win_ace_mask & FILE_READ_DATA)
    if (win_ace_type == ACCESS_ALLOWED_ACE_TYPE)
      acl.a_perm |= (acl.a_perm & S_IRGRP) ? 0 : S_IRUSR;
    else if (win_ace_type == ACCESS_DENIED_ACE_TYPE)
      acl.a_perm &= ~S_IRGRP;

  if (win_ace_mask & FILE_WRITE_DATA)
    if (win_ace_type == ACCESS_ALLOWED_ACE_TYPE)
      acl.a_perm |= (acl.a_perm & S_IWGRP) ? 0 : S_IWUSR;
    else if (win_ace_type == ACCESS_DENIED_ACE_TYPE)
      acl.a_perm &= ~S_IWGRP;

  if (win_ace_mask & FILE_EXECUTE)
    if (win_ace_type == ACCESS_ALLOWED_ACE_TYPE)
      acl.a_perm |= (acl.a_perm & S_IXGRP) ? 0 : S_IXUSR;
    else if (win_ace_type == ACCESS_DENIED_ACE_TYPE)
      acl.a_perm &= ~S_IXGRP;
}

static int
getacl (const char *file, DWORD attr, int nentries, aclent_t *aclbufp)
{
  DWORD sd_size = 4096;
  char sd_buf[4096];
  PSECURITY_DESCRIPTOR psd = (PSECURITY_DESCRIPTOR) sd_buf;

  int ret;
  if ((ret = read_sd (file, psd, &sd_size)) <= 0)
    {
      debug_printf ("read_sd %E");
      return ret;
    }

  PSID owner_sid;
  PSID group_sid;
  BOOL dummy;
  uid_t uid;
  gid_t gid;

  if (! GetSecurityDescriptorOwner (psd, &owner_sid, &dummy))
    {
      debug_printf ("GetSecurityDescriptorOwner %E");
      __seterrno ();
      return -1;
    }
  uid = get_uid_from_sid (owner_sid);

  if (! GetSecurityDescriptorGroup (psd, &group_sid, &dummy))
    {
      debug_printf ("GetSecurityDescriptorGroup %E");
      __seterrno ();
      return -1;
    }
  gid = get_gid_from_sid (group_sid);

  aclent_t lacl[MAX_ACL_ENTRIES];
  memset (&lacl, 0, MAX_ACL_ENTRIES * sizeof (aclent_t));
  lacl[0].a_type = USER_OBJ;
  lacl[0].a_id = uid;
  lacl[1].a_type = GROUP_OBJ;
  lacl[1].a_id = gid;
  lacl[2].a_type = OTHER_OBJ;

  PACL acl;
  BOOL acl_exists;

  if (! GetSecurityDescriptorDacl (psd, &acl_exists, &acl, &dummy))
    {
      __seterrno ();
      debug_printf ("GetSecurityDescriptorDacl %E");
      return -1;
    }

  int pos, i;

  if (! acl_exists || ! acl)
    {
      for (pos = 0; pos < MIN_ACL_ENTRIES; ++pos)
        lacl[pos].a_perm = S_IRWXU | S_IRWXG | S_IRWXO;
      pos = nentries < MIN_ACL_ENTRIES ? nentries : MIN_ACL_ENTRIES;
      memcpy (aclbufp, lacl, pos * sizeof (aclent_t));
      return pos;
    }

  for (i = 0; i < acl->AceCount && (!nentries || i < nentries); ++i)
    {
      ACCESS_ALLOWED_ACE *ace;

      if (!GetAce (acl, i, (PVOID *) &ace))
        continue;

      PSID ace_sid = (PSID) &ace->SidStart;
      int id;
      int type = 0;

      if (EqualSid (ace_sid, owner_sid))
        {
          type = USER_OBJ;
          id = uid;
        }
      else if (EqualSid (ace_sid, group_sid))
        {
          type = GROUP_OBJ;
          id = gid;
        }
      else if (EqualSid (ace_sid, get_world_sid ()))
        {
          type = OTHER_OBJ;
          id = 0;
        }
      else
        {
          id = get_id_from_sid (ace_sid, FALSE, &type);
          if (type != GROUP)
            {
              int type2 = 0;
              int id2 = get_id_from_sid (ace_sid, TRUE, &type2);
              if (type2 == GROUP)
                {
                  id = id2;
                  type = GROUP;
                }
            }
        }
      if (!type)
        continue;
      if (!(ace->Header.AceFlags & INHERIT_ONLY_ACE))
        {
          if ((pos = searchace (lacl, MAX_ACL_ENTRIES, type, id)) >= 0)
            getace (lacl[pos], type, id, ace->Mask, ace->Header.AceType);
        }
      if ((ace->Header.AceFlags & INHERIT_ALL)
          && (attr & FILE_ATTRIBUTE_DIRECTORY))
        {
          type |= ACL_DEFAULT;
          if ((pos = searchace (lacl, MAX_ACL_ENTRIES, type, id)) >= 0)
            getace (lacl[pos], type, id, ace->Mask, ace->Header.AceType);
        }
    }
  if ((pos = searchace (lacl, MAX_ACL_ENTRIES, 0)) < 0)
    pos = MAX_ACL_ENTRIES;
  for (i = 0; i < pos; ++i)
    {
      lacl[i].a_perm = (lacl[i].a_perm & S_IRWXU)
                       & ~((lacl[i].a_perm & S_IRWXG) << 3);
      lacl[i].a_perm |= (lacl[i].a_perm & S_IRWXU) >> 3
                        | (lacl[i].a_perm & S_IRWXU) >> 6;
    }
  if ((searchace (lacl, MAX_ACL_ENTRIES, USER) >= 0
       || searchace (lacl, MAX_ACL_ENTRIES, GROUP) >= 0)
      && (pos = searchace (lacl, MAX_ACL_ENTRIES, CLASS_OBJ)) >= 0)
    {
      lacl[pos].a_type = CLASS_OBJ;
      lacl[pos].a_perm =
          lacl[searchace (lacl, MAX_ACL_ENTRIES, GROUP_OBJ)].a_perm;
    }
  int dgpos;
  if ((searchace (lacl, MAX_ACL_ENTRIES, DEF_USER) >= 0
       || searchace (lacl, MAX_ACL_ENTRIES, DEF_GROUP) >= 0)
      && (dgpos = searchace (lacl, MAX_ACL_ENTRIES, DEF_GROUP_OBJ)) >= 0
      && (pos = searchace (lacl, MAX_ACL_ENTRIES, DEF_CLASS_OBJ)) >= 0
      && (attr & FILE_ATTRIBUTE_DIRECTORY))
    {
      lacl[pos].a_type = DEF_CLASS_OBJ;
      lacl[pos].a_perm = lacl[dgpos].a_perm;
    }
  if ((pos = searchace (lacl, MAX_ACL_ENTRIES, 0)) < 0)
    pos = MAX_ACL_ENTRIES;
  if (pos > nentries)
    pos = nentries;
  if (aclbufp)
    memcpy (aclbufp, lacl, pos * sizeof (aclent_t));
  aclsort (pos, 0, aclbufp);
  syscall_printf ("%d = getacl (%s)", pos, file);
  return pos;
}

int
acl_access (const char *path, int flags)
{
  aclent_t acls[MAX_ACL_ENTRIES];
  int cnt;

  if ((cnt = acl (path, GETACL, MAX_ACL_ENTRIES, acls)) < 1)
    return -1;

  // Only check existance.
  if (!(flags & (R_OK|W_OK|X_OK)))
    return 0;

  for (int i = 0; i < cnt; ++i)
    {
      switch (acls[i].a_type)
        {
        case USER_OBJ:
        case USER:
          if (acls[i].a_id != myself->uid)
            {
              // Check if user is a NT group:
              // Take SID from passwd, search SID in group, check is_grp_member
              char owner_sidbuf[MAX_SID_LEN];
              PSID owner_sid = (PSID) owner_sidbuf;
              char group_sidbuf[MAX_SID_LEN];
              PSID group_sid = (PSID) group_sidbuf;
              struct passwd *pw;
              struct group *gr = NULL;

              if (group_sem > 0)
                continue;
              ++group_sem;
              if ((pw = getpwuid (acls[i].a_id)) != NULL
                  && get_pw_sid (owner_sid, pw))
                {
                  while ((gr = getgrent ()))
                    if (get_gr_sid (group_sid, gr)
                        && EqualSid (owner_sid, group_sid)
                        && is_grp_member (myself->uid, gr->gr_gid))
                      break;
                  endgrent ();
                }
              --group_sem;
              if (! gr)
                continue;
            }
          break;
        case GROUP_OBJ:
        case GROUP:
          if (acls[i].a_id != myself->gid &&
              !is_grp_member (myself->uid, acls[i].a_id))
            continue;
          break;
        case OTHER_OBJ:
          break;
        default:
          continue;
        }
      if ((!(flags & R_OK) || (acls[i].a_perm & S_IREAD))
          && (!(flags & W_OK) || (acls[i].a_perm & S_IWRITE))
          && (!(flags & X_OK) || (acls[i].a_perm & S_IEXEC)))
        return 0;
    }
  set_errno (EACCES);
  return -1;
}

static
int
acl_worker (const char *path, int cmd, int nentries, aclent_t *aclbufp,
            int nofollow)
{
  extern suffix_info stat_suffixes[];
  path_conv real_path (path, (nofollow ? PC_SYM_NOFOLLOW : PC_SYM_FOLLOW) | PC_FULL, stat_suffixes);
  if (real_path.error)
    {
      set_errno (real_path.error);
      syscall_printf ("-1 = acl (%s)", path);
      return -1;
    }
  if (!real_path.has_acls ())
    {
      struct stat st;
      int ret = -1;

      switch (cmd)
        {
        case SETACL:
          set_errno (ENOSYS);
          break;
        case GETACL:
          if (nentries < 1)
            set_errno (EINVAL);
          else if ((nofollow && ! lstat (path, &st))
                   || (!nofollow && ! stat (path, &st)))
            {
              aclent_t lacl[4];
              if (nentries > 0)
                {
                  lacl[0].a_type = USER_OBJ;
                  lacl[0].a_id = st.st_uid;
                  lacl[0].a_perm = (st.st_mode & S_IRWXU)
                                   | (st.st_mode & S_IRWXU) >> 3
                                   | (st.st_mode & S_IRWXU) >> 6;
                }
              if (nentries > 1)
                {
                  lacl[1].a_type = GROUP_OBJ;
                  lacl[1].a_id = st.st_gid;
                  lacl[1].a_perm = (st.st_mode & S_IRWXG)
                                   | (st.st_mode & S_IRWXG) << 3
                                   | (st.st_mode & S_IRWXG) >> 3;
                }
              if (nentries > 2)
                {
                  lacl[2].a_type = OTHER_OBJ;
                  lacl[2].a_id = 0;
                  lacl[2].a_perm = (st.st_mode & S_IRWXO)
                                   | (st.st_mode & S_IRWXO) << 6
                                   | (st.st_mode & S_IRWXO) << 3;
                }
              if (nentries > 3)
                {
                  lacl[3].a_type = CLASS_OBJ;
                  lacl[3].a_id = 0;
                  lacl[3].a_perm = (st.st_mode & S_IRWXG)
                                   | (st.st_mode & S_IRWXG) << 3
                                   | (st.st_mode & S_IRWXG) >> 3;
                }
              if (nentries > 4)
                nentries = 4;
              if (aclbufp)
                memcpy (aclbufp, lacl, nentries * sizeof (aclent_t));
              ret = nentries;
            }
          break;
        case GETACLCNT:
          ret = 4;
          break;
        }
      syscall_printf ("%d = acl (%s)", ret, path);
      return ret;
    }
  switch (cmd)
    {
      case SETACL:
        if (!aclsort(nentries, 0, aclbufp))
          return setacl (real_path.get_win32 (),
                         nentries, aclbufp);
        break;
      case GETACL:
        if (nentries < 1)
          break;
        return getacl (real_path.get_win32 (),
                       real_path.file_attributes (),
                       nentries, aclbufp);
      case GETACLCNT:
        return getacl (real_path.get_win32 (),
                       real_path.file_attributes (),
                       0, NULL);
      default:
        break;
    }
  set_errno (EINVAL);
  syscall_printf ("-1 = acl (%s)", path);
  return -1;
}

extern "C"
int
acl (const char *path, int cmd, int nentries, aclent_t *aclbufp)
{
  return acl_worker (path, cmd, nentries, aclbufp, 0);
}

extern "C"
int
lacl (const char *path, int cmd, int nentries, aclent_t *aclbufp)
{
  return acl_worker (path, cmd, nentries, aclbufp, 1);
}

extern "C"
int
facl (int fd, int cmd, int nentries, aclent_t *aclbufp)
{
  if (dtable.not_open (fd))
    {
      syscall_printf ("-1 = facl (%d)", fd);
      set_errno (EBADF);
      return -1;
    }
  const char *path = dtable[fd]->get_name ();
  if (path == NULL)
    {
      syscall_printf ("-1 = facl (%d) (no name)", fd);
      set_errno (ENOSYS);
      return -1;
    }
  syscall_printf ("facl (%d): calling acl (%s)", fd, path);
  return acl_worker (path, cmd, nentries, aclbufp, 0);
}

extern "C"
int
aclcheck (aclent_t *aclbufp, int nentries, int *which)
{
  BOOL has_user_obj = FALSE;
  BOOL has_group_obj = FALSE;
  BOOL has_other_obj = FALSE;
  BOOL has_class_obj = FALSE;
  BOOL has_ug_objs = FALSE;
  BOOL has_def_user_obj = FALSE;
  BOOL has_def_group_obj = FALSE;
  BOOL has_def_other_obj = FALSE;
  BOOL has_def_class_obj = FALSE;
  BOOL has_def_ug_objs = FALSE;
  int pos2;

  for (int pos = 0; pos < nentries; ++pos)
    switch (aclbufp[pos].a_type)
      {
      case USER_OBJ:
        if (has_user_obj)
          {
            if (which)
              *which = pos;
            return USER_ERROR;
          }
        has_user_obj = TRUE;
        break;
      case GROUP_OBJ:
        if (has_group_obj)
          {
            if (which)
              *which = pos;
            return GRP_ERROR;
          }
        has_group_obj = TRUE;
        break;
      case OTHER_OBJ:
        if (has_other_obj)
          {
            if (which)
              *which = pos;
            return OTHER_ERROR;
          }
        has_other_obj = TRUE;
        break;
      case CLASS_OBJ:
        if (has_class_obj)
          {
            if (which)
              *which = pos;
            return CLASS_ERROR;
          }
        has_class_obj = TRUE;
        break;
      case USER:
      case GROUP:
        if ((pos2 = searchace (aclbufp + pos + 1, nentries - pos - 1,
                               aclbufp[pos].a_type, aclbufp[pos].a_id)) >= 0)
          {
            if (which)
              *which = pos2;
            return DUPLICATE_ERROR;
          }
        has_ug_objs = TRUE;
        break;
      case DEF_USER_OBJ:
        if (has_def_user_obj)
          {
            if (which)
              *which = pos;
            return USER_ERROR;
          }
        has_def_user_obj = TRUE;
        break;
      case DEF_GROUP_OBJ:
        if (has_def_group_obj)
          {
            if (which)
              *which = pos;
            return GRP_ERROR;
          }
        has_def_group_obj = TRUE;
        break;
      case DEF_OTHER_OBJ:
        if (has_def_other_obj)
          {
            if (which)
              *which = pos;
            return OTHER_ERROR;
          }
        has_def_other_obj = TRUE;
        break;
      case DEF_CLASS_OBJ:
        if (has_def_class_obj)
          {
            if (which)
              *which = pos;
            return CLASS_ERROR;
          }
        has_def_class_obj = TRUE;
        break;
      case DEF_USER:
      case DEF_GROUP:
        if ((pos2 = searchace (aclbufp + pos + 1, nentries - pos - 1,
                               aclbufp[pos].a_type, aclbufp[pos].a_id)) >= 0)
          {
            if (which)
              *which = pos2;
            return DUPLICATE_ERROR;
          }
        has_def_ug_objs = TRUE;
        break;
      default:
        return ENTRY_ERROR;
      }
  if (!has_user_obj
      || !has_group_obj
      || !has_other_obj
#if 0
      // These checks are not ok yet since CLASS_OBJ isn't fully implemented.
      || (has_ug_objs && !has_class_obj)
      || (has_def_ug_objs && !has_def_class_obj)
#endif
     )
    {
      if (which)
        *which = -1;
      return MISS_ERROR;
    }
  return 0;
}

extern "C"
int acecmp (const void *a1, const void *a2)
{
#define ace(i) ((const aclent_t *) a##i)
  int ret = ace(1)->a_type - ace(2)->a_type;
  if (!ret)
    ret = ace(1)->a_id - ace(2)->a_id;
  return ret;
#undef ace
}

extern "C"
int
aclsort (int nentries, int, aclent_t *aclbufp)
{
  if (aclcheck (aclbufp, nentries, NULL))
    return -1;
  if (!aclbufp || nentries < 1)
    {
      set_errno (EINVAL);
      return -1;
    }
  qsort((void *) aclbufp, nentries, sizeof (aclent_t), acecmp);
  return 0;
}

extern "C"
int
acltomode (aclent_t *aclbufp, int nentries, mode_t *modep)
{
  int pos;

  if (!aclbufp || nentries < 1 || ! modep)
    {
      set_errno (EINVAL);
      return -1;
    }
  *modep = 0;
  if ((pos = searchace (aclbufp, nentries, USER_OBJ)) < 0)
    {
      set_errno (EINVAL);
      return -1;
    }
  *modep |= aclbufp[pos].a_perm & S_IRWXU;
  if ((pos = searchace (aclbufp, nentries, GROUP_OBJ)) < 0)
    {
      set_errno (EINVAL);
      return -1;
    }
  if (searchace (aclbufp, nentries, CLASS_OBJ) < 0)
    pos = searchace (aclbufp, nentries, CLASS_OBJ);
  *modep |= (aclbufp[pos].a_perm & S_IRWXU) >> 3;
  if ((pos = searchace (aclbufp, nentries, OTHER_OBJ)) < 0)
    {
      set_errno (EINVAL);
      return -1;
    }
  *modep |= (aclbufp[pos].a_perm & S_IRWXU) >> 6;
  return 0;
}

extern "C"
int
aclfrommode(aclent_t *aclbufp, int nentries, mode_t *modep)
{
  int pos;

  if (!aclbufp || nentries < 1 || ! modep)
    {
      set_errno (EINVAL);
      return -1;
    }
  if ((pos = searchace (aclbufp, nentries, USER_OBJ)) < 0)
    {
      set_errno (EINVAL);
      return -1;
    }
  aclbufp[pos].a_perm = (*modep & S_IRWXU)
                        | (*modep & S_IRWXU) >> 3
                        | (*modep & S_IRWXU) >> 6;
  if ((pos = searchace (aclbufp, nentries, GROUP_OBJ)) < 0)
    {
      set_errno (EINVAL);
      return -1;
    }
  if (searchace (aclbufp, nentries, CLASS_OBJ) < 0)
    pos = searchace (aclbufp, nentries, CLASS_OBJ);
  aclbufp[pos].a_perm = (*modep & S_IRWXG)
                        | (*modep & S_IRWXG) << 3
                        | (*modep & S_IRWXG) >> 3;
  if ((pos = searchace (aclbufp, nentries, OTHER_OBJ)) < 0)
    {
      set_errno (EINVAL);
      return -1;
    }
  aclbufp[pos].a_perm = (*modep & S_IRWXO)
                        | (*modep & S_IRWXO) << 6
                        | (*modep & S_IRWXO) << 3;
  return 0;
}

extern "C"
int
acltopbits (aclent_t *aclbufp, int nentries, mode_t *pbitsp)
{
  return acltomode (aclbufp, nentries, pbitsp);
}

extern "C"
int
aclfrompbits (aclent_t *aclbufp, int nentries, mode_t *pbitsp)
{
  return aclfrommode (aclbufp, nentries, pbitsp);
}

static char *
permtostr (mode_t perm)
{
  static char pbuf[4];

  pbuf[0] = (perm & S_IREAD) ? 'r' : '-';
  pbuf[1] = (perm & S_IWRITE) ? 'w' : '-';
  pbuf[2] = (perm & S_IEXEC) ? 'x' : '-';
  pbuf[3] = '\0';
  return pbuf;
}

extern "C"
char *
acltotext (aclent_t *aclbufp, int aclcnt)
{
  if (!aclbufp || aclcnt < 1 || aclcnt > MAX_ACL_ENTRIES
      || aclcheck (aclbufp, aclcnt, NULL))
    {
      set_errno (EINVAL);
      return NULL;
    }
  char buf[32000];
  buf[0] = '\0';
  BOOL first = TRUE;

  for (int pos = 0; pos < aclcnt; ++pos)
    {
      if (!first)
        strcat (buf, ",");
      first = FALSE;
      if (aclbufp[pos].a_type & ACL_DEFAULT)
        strcat (buf, "default");
      switch (aclbufp[pos].a_type)
        {
        case USER_OBJ:
          sprintf (buf + strlen (buf), "user::%s",
                   permtostr (aclbufp[pos].a_perm));
          break;
        case USER:
          sprintf (buf + strlen (buf), "user:%d:%s",
                   aclbufp[pos].a_id, permtostr (aclbufp[pos].a_perm));
          break;
        case GROUP_OBJ:
          sprintf (buf + strlen (buf), "group::%s",
                   permtostr (aclbufp[pos].a_perm));
          break;
        case GROUP:
          sprintf (buf + strlen (buf), "group:%d:%s",
                   aclbufp[pos].a_id, permtostr (aclbufp[pos].a_perm));
          break;
        case CLASS_OBJ:
          sprintf (buf + strlen (buf), "mask::%s",
                   permtostr (aclbufp[pos].a_perm));
          break;
        case OTHER_OBJ:
          sprintf (buf + strlen (buf), "other::%s",
                   permtostr (aclbufp[pos].a_perm));
          break;
        default:
          set_errno (EINVAL);
          return NULL;
        }
    }
  return strdup (buf);
}

static mode_t
permfromstr (char *perm)
{
  mode_t mode = 0;

  if (strlen (perm) != 3)
    return 01000;
  if (perm[0] == 'r')
    mode |= S_IRUSR | S_IRGRP | S_IROTH;
  else if (perm[0] != '-')
    return 01000;
  if (perm[1] == 'w')
    mode |= S_IWUSR | S_IWGRP | S_IWOTH;
  else if (perm[1] != '-')
    return 01000;
  if (perm[2] == 'x')
    mode |= S_IXUSR | S_IXGRP | S_IXOTH;
  else if (perm[2] != '-')
    return 01000;
  return mode;
}

extern "C"
aclent_t *
aclfromtext (char *acltextp, int *)
{
  if (!acltextp)
    {
      set_errno (EINVAL);
      return NULL;
    }
  char buf[strlen (acltextp) + 1];
  aclent_t lacl[MAX_ACL_ENTRIES];
  memset (lacl, 0, sizeof lacl);
  int pos = 0;
  for (char *c = strtok (buf, ","); c; c = strtok (NULL, ","))
    {
      if (!strncmp (c, "default", 7))
        {
          lacl[pos].a_type |= ACL_DEFAULT;
          c += 7;
        }
      if (!strncmp (c, "user:", 5))
        {
          if (c[5] == ':')
            lacl[pos].a_type |= USER_OBJ;
          else
            {
              lacl[pos].a_type |= USER;
              c += 5;
              if (isalpha (*c))
                {
                  struct passwd *pw = getpwnam (c);
                  if (!pw)
                    {
                      set_errno (EINVAL);
                      return NULL;
                    }
                  lacl[pos].a_id = pw->pw_uid;
                  c = strchr (c, ':');
                }
              else if (isdigit (*c))
                lacl[pos].a_id = strtol (c, &c, 10);
              if (!c || *c != ':')
                {
                  set_errno (EINVAL);
                  return NULL;
                }
            }
        }
      else if (!strncmp (c, "group:", 6))
        {
          if (c[5] == ':')
            lacl[pos].a_type |= GROUP_OBJ;
          else
            {
              lacl[pos].a_type |= GROUP;
              c += 5;
              if (isalpha (*c))
                {
                  struct group *gr = getgrnam (c);
                  if (!gr)
                    {
                      set_errno (EINVAL);
                      return NULL;
                    }
                  lacl[pos].a_id = gr->gr_gid;
                  c = strchr (c, ':');
                }
              else if (isdigit (*c))
                lacl[pos].a_id = strtol (c, &c, 10);
              if (!c || *c != ':')
                {
                  set_errno (EINVAL);
                  return NULL;
                }
            }
        }
      else if (!strncmp (c, "mask:", 5))
        {
          if (c[5] == ':')
            lacl[pos].a_type |= CLASS_OBJ;
          else
            {
              set_errno (EINVAL);
              return NULL;
            }
        }
      else if (!strncmp (c, "other:", 6))
        {
          if (c[5] == ':')
            lacl[pos].a_type |= OTHER_OBJ;
          else
            {
              set_errno (EINVAL);
              return NULL;
            }
        }
      if ((lacl[pos].a_perm = permfromstr (c)) == 01000)
        {
          set_errno (EINVAL);
          return NULL;
        }
      ++pos;
    }
  aclent_t *aclp = (aclent_t *) malloc (pos * sizeof (aclent_t));
  if (aclp)
    memcpy (aclp, lacl, pos * sizeof (aclent_t));
  return aclp;
}

