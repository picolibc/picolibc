/* security.cc: NT security functions

   Copyright 1997, 1998, 1999, 2000, 2001 Cygnus Solutions.

   Originaly written by Gunther Ebert, gunther.ebert@ixos-leipzig.de
   Completely rewritten by Corinna Vinschen <corinna@vinschen.de>

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
#include <wingdi.h>
#include <winuser.h>
#include "cygerrno.h"
#include "perprocess.h"
#include "fhandler.h"
#include "path.h"
#include "dtable.h"
#include "sync.h"
#include "sigproc.h"
#include "pinfo.h"
#include "cygheap.h"
#include "security.h"

extern BOOL allow_ntea;
BOOL allow_ntsec = FALSE;
/* allow_smbntsec is handled exclusively in path.cc (path_conv::check).
   It's defined here because of it's strong relationship to allow_ntsec.
   The default is TRUE to reflect the old behaviour. */
BOOL allow_smbntsec = TRUE;

extern "C"
void
cygwin_set_impersonation_token (const HANDLE hToken)
{
  debug_printf ("set_impersonation_token (%d)", hToken);
  if (cygheap->user.token != hToken)
    {
      if (cygheap->user.token != INVALID_HANDLE_VALUE)
	CloseHandle (cygheap->user.token);
      cygheap->user.token = hToken;
      cygheap->user.impersonated = FALSE;
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
  debug_printf ("pw_gecos = %x (%s)", pw->pw_gecos, pw->pw_gecos);
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
  debug_printf ("LogonUserA (%s, %s, %s, ...)", nt_user, nt_domain, password);
  if (!LogonUserA (nt_user, nt_domain, (char *) password,
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
  if (!sd_size)
    {
      set_errno (EINVAL);
      return -1;
    }

  debug_printf("file = %s", file);

  DWORD len = 0;
  const char *pfile = file;
  char fbuf [PATH_MAX];
  if (current_codepage == oem_cp)
    {
      DWORD fname_len = min (sizeof (fbuf) - 1, strlen (file));
      bzero (fbuf, sizeof (fbuf));
      OemToCharBuff(file, fbuf, fname_len);
      pfile = fbuf;
    }

  if (!GetFileSecurity (pfile,
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
  if (!sd_buf || !sd_size)
    {
      set_errno (EINVAL);
      return -1;
    }

  /* No need to be thread save. */
  static BOOL first_time = TRUE;
  if (first_time)
    {
      set_process_privilege (SE_RESTORE_NAME);
      first_time = FALSE;
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

static int
get_nt_attribute (const char *file, int *attribute,
		  uid_t *uidret, gid_t *gidret)
{
  if (os_being_run != winNT)
    return 0;

  syscall_printf ("file: %s", file);

  /* Yeah, sounds too much, but I've seen SDs of 2100 bytes!*/
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

  if (!GetSecurityDescriptorOwner (psd, &owner_sid, &dummy))
    debug_printf ("GetSecurityDescriptorOwner %E");
  if (!GetSecurityDescriptorGroup (psd, &group_sid, &dummy))
    debug_printf ("GetSecurityDescriptorGroup %E");

  PACL acl;
  BOOL acl_exists;

  if (!GetSecurityDescriptorDacl (psd, &acl_exists, &acl, &dummy))
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

  if (!attribute)
    {
      syscall_printf ("file: %s uid %d, gid %d", file, uid, gid);
      return 0;
    }

  BOOL grp_member = is_grp_member (uid, gid);

  if (!acl_exists || !acl)
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

      cygsid ace_sid ((PSID) &ace->SidStart);
      if (owner_sid && ace_sid == owner_sid)
	{
	  if (ace->Mask & FILE_READ_DATA)
	    *flags |= S_IRUSR;
	  if (ace->Mask & FILE_WRITE_DATA)
	    *flags |= S_IWUSR;
	  if (ace->Mask & FILE_EXECUTE)
	    *flags |= S_IXUSR;
	}
      else if (group_sid && ace_sid == group_sid)
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
      else if (ace_sid == get_world_sid ())
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
	      /* Sticky bit for directories according to linux rules. */
	      if (!(ace->Mask & FILE_DELETE_CHILD)
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
  int res;

  if (use_ntsec && allow_ntsec)
    {
      res = get_nt_attribute (file, attribute, uidret, gidret);
      if (attribute && (*attribute & S_IFLNK) == S_IFLNK)
	*attribute |= S_IRWXU | S_IRWXG | S_IRWXO;
      return res;
    }

  if (uidret)
    *uidret = getuid ();
  if (gidret)
    *gidret = getgid ();

  if (!attribute)
    return 0;

  res = NTReadEA (file, ".UNIXATTR", (char *) attribute, sizeof (*attribute));

  /* symlinks are everything for everyone!*/
  if ((*attribute & S_IFLNK) == S_IFLNK)
    *attribute |= S_IRWXU | S_IRWXG | S_IRWXO;

  if (res <= 0)
    set_errno (ENOSYS);
  return res > 0 ? 0 : -1;
}

BOOL
add_access_allowed_ace (PACL acl, int offset, DWORD attributes,
			PSID sid, size_t &len_add, DWORD inherit)
{
  if (!AddAccessAllowedAce (acl, ACL_REVISION, attributes, sid))
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

BOOL
add_access_denied_ace (PACL acl, int offset, DWORD attributes,
		       PSID sid, size_t &len_add, DWORD inherit)
{
  if (!AddAccessDeniedAce (acl, ACL_REVISION, attributes, sid))
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

  if (!sd_ret || !sd_size_ret)
    {
      set_errno (EINVAL);
      return NULL;
    }

  /* Get SID and name of new owner. */
  char owner[UNLEN + 1];
  cygsid owner_sid;
  struct passwd *pw = getpwuid (uid);
  strcpy (owner, pw ? pw->pw_name : getlogin ());
  if ((!pw || !get_pw_sid (owner_sid, pw))
      && !lookup_name (owner, logsrv, owner_sid))
    return NULL;
  debug_printf ("owner: %s [%d]", owner,
		*GetSidSubAuthority(owner_sid,
		*GetSidSubAuthorityCount(owner_sid) - 1));

  /* Get SID and name of new group. */
  cygsid group_sid (NULL);
  struct group *grp = getgrgid (gid);
  if (grp)
    {
      if ((!grp || !get_gr_sid (group_sid.set (), grp))
	  && !lookup_name (grp->gr_name, logsrv, group_sid))
	return NULL;
    }
  else
    debug_printf ("no group");

  /* Initialize local security descriptor. */
  SECURITY_DESCRIPTOR sd;
  PSECURITY_DESCRIPTOR psd = NULL;
  if (!InitializeSecurityDescriptor (&sd, SECURITY_DESCRIPTOR_REVISION))
    {
      __seterrno ();
      return NULL;
    }

  /*
   * We set the SE_DACL_PROTECTED flag here to prevent the DACL from being
   * modified by inheritable ACEs.
   * This flag as well as the SetSecurityDescriptorControl call are available
   * only since Win2K.
   */
  static int win2KorHigher = -1;
  if (win2KorHigher == -1)
    {
      DWORD version = GetVersion ();
      win2KorHigher = (version & 0x80000000) || (version & 0xff) < 5 ? 0 : 1;
    }
  if (win2KorHigher > 0)
    SetSecurityDescriptorControl (&sd, SE_DACL_PROTECTED, SE_DACL_PROTECTED);

  /* Create owner for local security descriptor. */
  if (!SetSecurityDescriptorOwner(&sd, owner_sid, FALSE))
    {
      __seterrno ();
      return NULL;
    }

  /* Create group for local security descriptor. */
  if (group_sid && !SetSecurityDescriptorGroup(&sd, group_sid, FALSE))
    {
      __seterrno ();
      return NULL;
    }

  /* Initialize local access control list. */
  char acl_buf[3072];
  PACL acl = (PACL) acl_buf;
  if (!InitializeAcl (acl, 3072, ACL_REVISION))
    {
      __seterrno ();
      return NULL;
    }

  /*
   * VTX bit may only be set if executable for `other' is set.
   * For correct handling under WinNT, FILE_DELETE_CHILD has to
   * be (un)set in each ACE.
   */
  if (!(attribute & S_IXOTH))
    attribute &= ~S_ISVTX;
  if (!(attribute & S_IFDIR))
    attribute |= S_ISVTX;

  /* From here fill ACL. */
  size_t acl_len = sizeof (ACL);
  int ace_off = 0;

  /* Construct allow attribute for owner. */
  DWORD owner_allow = (STANDARD_RIGHTS_ALL & ~DELETE)
		      | FILE_WRITE_ATTRIBUTES | FILE_WRITE_EA;
  if (attribute & S_IRUSR)
    owner_allow |= FILE_GENERIC_READ;
  if (attribute & S_IWUSR)
    owner_allow |= FILE_GENERIC_WRITE | DELETE;
  if (attribute & S_IXUSR)
    owner_allow |= FILE_GENERIC_EXECUTE;
  if (!(attribute & S_ISVTX))
    owner_allow |= FILE_DELETE_CHILD;

  /* Construct allow attribute for group. */
  DWORD group_allow = STANDARD_RIGHTS_READ
		      | FILE_READ_ATTRIBUTES | FILE_READ_EA;
  if (attribute & S_IRGRP)
    group_allow |= FILE_GENERIC_READ;
  if (attribute & S_IWGRP)
    group_allow |= STANDARD_RIGHTS_WRITE | FILE_GENERIC_WRITE | DELETE;
  if (attribute & S_IXGRP)
    group_allow |= FILE_GENERIC_EXECUTE;
  if (!(attribute & S_ISVTX))
    group_allow |= FILE_DELETE_CHILD;

  /* Construct allow attribute for everyone. */
  DWORD other_allow = STANDARD_RIGHTS_READ
		      | FILE_READ_ATTRIBUTES | FILE_READ_EA;
  if (attribute & S_IROTH)
    other_allow |= FILE_GENERIC_READ;
  if (attribute & S_IWOTH)
    other_allow |= STANDARD_RIGHTS_WRITE | FILE_GENERIC_WRITE | DELETE;
  if (attribute & S_IXOTH)
    other_allow |= FILE_GENERIC_EXECUTE;
  if (!(attribute & S_ISVTX))
    other_allow |= FILE_DELETE_CHILD;

  /* Construct deny attributes for owner and group. */
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

  /* Construct appropriate inherit attribute. */
  DWORD inherit = (attribute & S_IFDIR) ? INHERIT_ALL : DONT_INHERIT;

  /* Set deny ACE for owner. */
  if (owner_deny
      && !add_access_denied_ace (acl, ace_off++, owner_deny,
				  owner_sid, acl_len, inherit))
      return NULL;
  /* Set allow ACE for owner. */
  if (!add_access_allowed_ace (acl, ace_off++, owner_allow,
				owner_sid, acl_len, inherit))
    return NULL;
  /* Set deny ACE for group. */
  if (group_deny
      && !add_access_denied_ace (acl, ace_off++, group_deny,
				  group_sid, acl_len, inherit))
      return NULL;
  /* Set allow ACE for group. */
  if (!add_access_allowed_ace (acl, ace_off++, group_allow,
				group_sid, acl_len, inherit))
    return NULL;

  /* Set allow ACE for everyone. */
  if (!add_access_allowed_ace (acl, ace_off++, other_allow,
				get_world_sid (), acl_len, inherit))
    return NULL;

  /* Get owner and group from current security descriptor. */
  PSID cur_owner_sid = NULL;
  PSID cur_group_sid = NULL;
  if (!GetSecurityDescriptorOwner (sd_ret, &cur_owner_sid, &dummy))
    debug_printf ("GetSecurityDescriptorOwner %E");
  if (!GetSecurityDescriptorGroup (sd_ret, &cur_group_sid, &dummy))
    debug_printf ("GetSecurityDescriptorGroup %E");

  /* Fill ACL with unrelated ACEs from current security descriptor. */
  PACL oacl;
  BOOL acl_exists;
  ACCESS_ALLOWED_ACE *ace;
  if (GetSecurityDescriptorDacl (sd_ret, &acl_exists, &oacl, &dummy)
      && acl_exists && oacl)
    for (DWORD i = 0; i < oacl->AceCount; ++i)
      if (GetAce (oacl, i, (PVOID *) &ace))
	{
	  cygsid ace_sid ((PSID) &ace->SidStart);
	  /* Check for related ACEs. */
	  if ((cur_owner_sid && ace_sid == cur_owner_sid)
	      || (owner_sid && ace_sid == owner_sid)
	      || (cur_group_sid && ace_sid == cur_group_sid)
	      || (group_sid && ace_sid == group_sid)
	      || (ace_sid == get_world_sid ()))
	    continue;
	  /*
	   * Add unrelated ACCESS_DENIED_ACE to the beginning but
	   * behind the owner_deny, ACCESS_ALLOWED_ACE to the end.
	   */
	  if (!AddAce(acl, ACL_REVISION,
		       ace->Header.AceType == ACCESS_DENIED_ACE_TYPE ?
		       (owner_deny ? 1 : 0) : MAXDWORD,
		       (LPVOID) ace, ace->Header.AceSize))
	    {
	      __seterrno ();
	      return NULL;
	    }
	  acl_len += ace->Header.AceSize;
	}

  /* Set AclSize to computed value. */
  acl->AclSize = acl_len;
  debug_printf ("ACL-Size: %d", acl_len);

  /* Create DACL for local security descriptor. */
  if (!SetSecurityDescriptorDacl (&sd, TRUE, acl, FALSE))
    {
      __seterrno ();
      return NULL;
    }

  /* Make self relative security descriptor. */
  *sd_size_ret = 0;
  MakeSelfRelativeSD (&sd, sd_ret, sd_size_ret);
  if (*sd_size_ret <= 0)
    {
      __seterrno ();
      return NULL;
    }
  if (!MakeSelfRelativeSD (&sd, sd_ret, sd_size_ret))
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
  if (!(psd = alloc_sd (uid, gid, logsrv, attribute, psd, &sd_size)))
    return -1;

  return write_sd (file, psd, sd_size);
}

int
set_file_attribute (int use_ntsec, const char *file,
		    uid_t uid, gid_t gid,
		    int attribute, const char *logsrv)
{
  /* symlinks are anything for everyone!*/
  if ((attribute & S_IFLNK) == S_IFLNK)
    attribute |= S_IRWXU | S_IRWXG | S_IRWXO;

  if (!use_ntsec || !allow_ntsec)
    {
      if (!NTWriteEA (file, ".UNIXATTR",
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
			     attribute, cygheap->user.logsrv ());
}
