/* security.cc: NT file access control functions

   Copyright 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005,
   2006, 2007 Red Hat, Inc.

   Originaly written by Gunther Ebert, gunther.ebert@ixos-leipzig.de
   Completely rewritten by Corinna Vinschen <corinna@vinschen.de>

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <unistd.h>
#include <stdlib.h>
#include "cygerrno.h"
#include "security.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "pinfo.h"
#include "cygheap.h"
#include "ntdll.h"
#include "pwdgrp.h"

#define ALL_SECURITY_INFORMATION (DACL_SECURITY_INFORMATION \
				  | GROUP_SECURITY_INFORMATION \
				  | OWNER_SECURITY_INFORMATION)

LONG
get_file_sd (HANDLE fh, path_conv &pc, security_descriptor &sd)
{
  NTSTATUS status = STATUS_SUCCESS;
  ULONG len = 0;
  int retry = 0;
  int res = -1;

  for (; retry < 2; ++retry)
    {
      if (fh)
	{
	  status = NtQuerySecurityObject (fh, ALL_SECURITY_INFORMATION,
					  sd, len, &len);
	  if (status == STATUS_BUFFER_TOO_SMALL)
	    {
	      if (!sd.malloc (len))
		{
		  set_errno (ENOMEM);
		  break;
		}
	      status = NtQuerySecurityObject (fh, ALL_SECURITY_INFORMATION,
					      sd, len, &len);
	    }
	  if (NT_SUCCESS (status))
	    {
	      res = 0;
	      break;
	    }
	}
      if (!retry)
	{
	  OBJECT_ATTRIBUTES attr;
	  IO_STATUS_BLOCK io;

	  status = NtOpenFile (&fh, READ_CONTROL,
			       pc.get_object_attr (attr, sec_none_nih),
			       &io, FILE_SHARE_VALID_FLAGS,
			       FILE_OPEN_FOR_BACKUP_INTENT);
	  if (!NT_SUCCESS (status))
	    {
	      fh = NULL;
	      break;
	    }
	}
    }
  if (retry && fh)
    NtClose (fh);
  if (!NT_SUCCESS (status))
    __seterrno_from_nt_status (status);
  return res;
}

LONG
set_file_sd (HANDLE fh, path_conv &pc, security_descriptor &sd)
{
  NTSTATUS status = STATUS_SUCCESS;
  int retry = 0;
  int res = -1;

  for (; retry < 2; ++retry)
    {
      if (fh)
	{
	  status = NtSetSecurityObject (fh, ALL_SECURITY_INFORMATION, sd);
	  if (NT_SUCCESS (status))
	    {
	      res = 0;
	      break;
	    }
	}
      if (!retry)
	{
	  OBJECT_ATTRIBUTES attr;
	  IO_STATUS_BLOCK io;

	  status = NtOpenFile (&fh, WRITE_OWNER | WRITE_DAC,
			       pc.get_object_attr (attr, sec_none_nih),
			       &io, FILE_SHARE_VALID_FLAGS,
			       FILE_OPEN_FOR_BACKUP_INTENT
			       | FILE_OPEN_FOR_RECOVERY);
	  if (!NT_SUCCESS (status))
	    {
	      fh = NULL;
	      break;
	    }
	}
    }
  if (retry && fh)
    NtClose (fh);
  if (!NT_SUCCESS (status))
    __seterrno_from_nt_status (status);
  return res;
}

static void
get_attribute_from_acl (mode_t *attribute, PACL acl, PSID owner_sid,
			PSID group_sid, bool grp_member)
{
  ACCESS_ALLOWED_ACE *ace;
  int allow = 0;
  int deny = 0;
  int *flags, *anti;

  for (DWORD i = 0; i < acl->AceCount; ++i)
    {
      if (!GetAce (acl, i, (PVOID *) &ace))
	continue;
      if (ace->Header.AceFlags & INHERIT_ONLY)
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

      cygpsid ace_sid ((PSID) &ace->SidStart);
      if (ace_sid == well_known_world_sid)
	{
	  if (ace->Mask & FILE_READ_BITS)
	    *flags |= ((!(*anti & S_IROTH)) ? S_IROTH : 0)
		      | ((!(*anti & S_IRGRP)) ? S_IRGRP : 0)
		      | ((!(*anti & S_IRUSR)) ? S_IRUSR : 0);
	  if (ace->Mask & FILE_WRITE_BITS)
	    *flags |= ((!(*anti & S_IWOTH)) ? S_IWOTH : 0)
		      | ((!(*anti & S_IWGRP)) ? S_IWGRP : 0)
		      | ((!(*anti & S_IWUSR)) ? S_IWUSR : 0);
	  if (ace->Mask & FILE_EXEC_BITS)
	    *flags |= ((!(*anti & S_IXOTH)) ? S_IXOTH : 0)
		      | ((!(*anti & S_IXGRP)) ? S_IXGRP : 0)
		      | ((!(*anti & S_IXUSR)) ? S_IXUSR : 0);
	  if ((S_ISDIR (*attribute)) &&
	      (ace->Mask & (FILE_WRITE_DATA | FILE_EXECUTE | FILE_DELETE_CHILD))
	      == (FILE_WRITE_DATA | FILE_EXECUTE))
	    *flags |= S_ISVTX;
	}
      else if (ace_sid == well_known_null_sid)
	{
	  /* Read SUID, SGID and VTX bits from NULL ACE. */
	  if (ace->Mask & FILE_READ_DATA)
	    *flags |= S_ISVTX;
	  if (ace->Mask & FILE_WRITE_DATA)
	    *flags |= S_ISGID;
	  if (ace->Mask & FILE_APPEND_DATA)
	    *flags |= S_ISUID;
	}
      else if (ace_sid == owner_sid)
	{
	  if (ace->Mask & FILE_READ_BITS)
	    *flags |= ((!(*anti & S_IRUSR)) ? S_IRUSR : 0);
	  if (ace->Mask & FILE_WRITE_BITS)
	    *flags |= ((!(*anti & S_IWUSR)) ? S_IWUSR : 0);
	  if (ace->Mask & FILE_EXEC_BITS)
	    *flags |= ((!(*anti & S_IXUSR)) ? S_IXUSR : 0);
	}
      else if (ace_sid == group_sid)
	{
	  if (ace->Mask & FILE_READ_BITS)
	    *flags |= ((!(*anti & S_IRGRP)) ? S_IRGRP : 0)
		      | ((grp_member && !(*anti & S_IRUSR)) ? S_IRUSR : 0);
	  if (ace->Mask & FILE_WRITE_BITS)
	    *flags |= ((!(*anti & S_IWGRP)) ? S_IWGRP : 0)
		      | ((grp_member && !(*anti & S_IWUSR)) ? S_IWUSR : 0);
	  if (ace->Mask & FILE_EXEC_BITS)
	    *flags |= ((!(*anti & S_IXGRP)) ? S_IXGRP : 0)
		      | ((grp_member && !(*anti & S_IXUSR)) ? S_IXUSR : 0);
	}
    }
  *attribute &= ~(S_IRWXU | S_IRWXG | S_IRWXO | S_ISVTX | S_ISGID | S_ISUID);
  if (owner_sid && group_sid && EqualSid (owner_sid, group_sid)
      /* FIXME: temporary exception for /var/empty */
      && well_known_system_sid != group_sid)
    {
      allow &= ~(S_IRGRP | S_IWGRP | S_IXGRP);
      allow |= (((allow & S_IRUSR) ? S_IRGRP : 0)
		| ((allow & S_IWUSR) ? S_IWGRP : 0)
		| ((allow & S_IXUSR) ? S_IXGRP : 0));
    }
  *attribute |= allow;
}

static void
get_info_from_sd (PSECURITY_DESCRIPTOR psd, mode_t *attribute,
		  __uid32_t *uidret, __gid32_t *gidret)
{
  if (!psd)
    {
      /* If reading the security descriptor failed, treat the object
	 as unreadable. */
      if (attribute)
	*attribute &= ~(S_IRWXU | S_IRWXG | S_IRWXO);
      if (uidret)
	*uidret = ILLEGAL_UID;
      if (gidret)
	*gidret = ILLEGAL_GID;
      return;
    }

  cygpsid owner_sid;
  cygpsid group_sid;
  BOOL dummy;

  if (!GetSecurityDescriptorOwner (psd, (PSID *) &owner_sid, &dummy))
    debug_printf ("GetSecurityDescriptorOwner %E");
  if (!GetSecurityDescriptorGroup (psd, (PSID *) &group_sid, &dummy))
    debug_printf ("GetSecurityDescriptorGroup %E");

  __uid32_t uid;
  __gid32_t gid;
  bool grp_member = get_sids_info (owner_sid, group_sid, &uid, &gid);
  if (uidret)
    *uidret = uid;
  if (gidret)
    *gidret = gid;

  if (!attribute)
    {
      syscall_printf ("uid %d, gid %d", uid, gid);
      return;
    }

  PACL acl;
  BOOL acl_exists;

  if (!GetSecurityDescriptorDacl (psd, &acl_exists, &acl, &dummy))
    {
      __seterrno ();
      debug_printf ("GetSecurityDescriptorDacl %E");
      *attribute &= ~(S_IRWXU | S_IRWXG | S_IRWXO);
    }
  else if (!acl_exists || !acl)
    *attribute |= S_IRWXU | S_IRWXG | S_IRWXO;
  else
    get_attribute_from_acl (attribute, acl, owner_sid, group_sid, grp_member);

  syscall_printf ("%sACL %x, uid %d, gid %d",
		  (!acl_exists || !acl)?"NO ":"", *attribute, uid, gid);
}

static int
get_reg_sd (HANDLE handle, security_descriptor &sd_ret)
{
  LONG ret;
  DWORD len = 0;

  ret = RegGetKeySecurity ((HKEY) handle, ALL_SECURITY_INFORMATION,
			   sd_ret, &len);
  if (ret == ERROR_INSUFFICIENT_BUFFER)
    {
      if (!sd_ret.malloc (len))
	set_errno (ENOMEM);
      else
	ret = RegGetKeySecurity ((HKEY) handle, ALL_SECURITY_INFORMATION,
				 sd_ret, &len);
    }
  if (ret != ERROR_SUCCESS)
    {
      __seterrno ();
      return -1;
    }
  return 0;
}

int
get_reg_attribute (HKEY hkey, mode_t *attribute, __uid32_t *uidret,
		   __gid32_t *gidret)
{
  security_descriptor sd;

  if (!get_reg_sd (hkey, sd))
    {
      get_info_from_sd (sd, attribute, uidret, gidret);
      return 0;
    }
  /* The entries are already set to default values */
  return -1;
}

int
get_file_attribute (HANDLE handle, path_conv &pc,
		    mode_t *attribute, __uid32_t *uidret, __gid32_t *gidret)
{
  if (pc.has_acls ())
    {
      security_descriptor sd;

      if (!get_file_sd (handle, pc, sd))
	{
	  get_info_from_sd (sd, attribute, uidret, gidret);
	  return 0;
	}
      else
	{
	  if (uidret)
	    *uidret = ILLEGAL_UID;
	  if (gidret)
	    *gidret = ILLEGAL_GID;

	  return -1;
	}
    }

  if (uidret)
    *uidret = myself->uid;
  if (gidret)
    *gidret = myself->gid;

  return -1;
}

bool
add_access_allowed_ace (PACL acl, int offset, DWORD attributes,
			PSID sid, size_t &len_add, DWORD inherit)
{
  if (!AddAccessAllowedAce (acl, ACL_REVISION, attributes, sid))
    {
      __seterrno ();
      return false;
    }
  ACCESS_ALLOWED_ACE *ace;
  if (inherit && GetAce (acl, offset, (PVOID *) &ace))
    ace->Header.AceFlags |= inherit;
  len_add += sizeof (ACCESS_ALLOWED_ACE) - sizeof (DWORD) + GetLengthSid (sid);
  return true;
}

bool
add_access_denied_ace (PACL acl, int offset, DWORD attributes,
		       PSID sid, size_t &len_add, DWORD inherit)
{
  if (!AddAccessDeniedAce (acl, ACL_REVISION, attributes, sid))
    {
      __seterrno ();
      return false;
    }
  ACCESS_DENIED_ACE *ace;
  if (inherit && GetAce (acl, offset, (PVOID *) &ace))
    ace->Header.AceFlags |= inherit;
  len_add += sizeof (ACCESS_DENIED_ACE) - sizeof (DWORD) + GetLengthSid (sid);
  return true;
}

static PSECURITY_DESCRIPTOR
alloc_sd (path_conv &pc, __uid32_t uid, __gid32_t gid, int attribute,
	  security_descriptor &sd_ret)
{
  BOOL dummy;

  debug_printf("uid %d, gid %d, attribute %x", uid, gid, attribute);

  /* Get owner and group from current security descriptor. */
  PSID cur_owner_sid = NULL;
  PSID cur_group_sid = NULL;
  if (!GetSecurityDescriptorOwner (sd_ret, &cur_owner_sid, &dummy))
    debug_printf ("GetSecurityDescriptorOwner %E");
  if (!GetSecurityDescriptorGroup (sd_ret, &cur_group_sid, &dummy))
    debug_printf ("GetSecurityDescriptorGroup %E");

  /* Get SID of owner. */
  cygsid owner_sid;
  /* Check for current user first */
  if (uid == myself->uid)
    owner_sid = cygheap->user.sid ();
  else if (uid == ILLEGAL_UID)
    owner_sid = cur_owner_sid;
  else if (!owner_sid.getfrompw (internal_getpwuid (uid)))
    {
      set_errno (EINVAL);
      return NULL;
    }
  owner_sid.debug_print ("alloc_sd: owner SID =");

  /* Get SID of new group. */
  cygsid group_sid;
  /* Check for current user first */
  if (gid == myself->gid)
    group_sid = cygheap->user.groups.pgsid;
  else if (gid == ILLEGAL_GID)
    group_sid = cur_group_sid;
  else if (!group_sid.getfromgr (internal_getgrgid (gid)))
    {
      set_errno (EINVAL);
      return NULL;
    }
  group_sid.debug_print ("alloc_sd: group SID =");

  /* Initialize local security descriptor. */
  SECURITY_DESCRIPTOR sd;
  if (!InitializeSecurityDescriptor (&sd, SECURITY_DESCRIPTOR_REVISION))
    {
      __seterrno ();
      return NULL;
    }

  /* We set the SE_DACL_PROTECTED flag here to prevent the DACL from being
   * modified by inheritable ACEs.  This flag is available since Win2K.  */
  if (wincap.has_dacl_protect ())
    sd.Control |= SE_DACL_PROTECTED;

  /* Create owner for local security descriptor. */
  if (!SetSecurityDescriptorOwner (&sd, owner_sid, FALSE))
    {
      __seterrno ();
      return NULL;
    }

  /* Create group for local security descriptor. */
  if (!SetSecurityDescriptorGroup (&sd, group_sid, FALSE))
    {
      __seterrno ();
      return NULL;
    }

  /* Initialize local access control list. */
  PACL acl = (PACL) alloca (3072);
  if (!InitializeAcl (acl, 3072, ACL_REVISION))
    {
      __seterrno ();
      return NULL;
    }

  /* From here fill ACL. */
  size_t acl_len = sizeof (ACL);
  int ace_off = 0;

  /* Construct allow attribute for owner.
     Don't set FILE_READ/WRITE_ATTRIBUTES unconditionally on Samba, otherwise
     it enforces read permissions.  Same for other's below. */
  DWORD owner_allow = STANDARD_RIGHTS_ALL
		      | pc.fs_is_samba ()
			? 0 : (FILE_READ_ATTRIBUTES | FILE_WRITE_ATTRIBUTES);
  if (attribute & S_IRUSR)
    owner_allow |= FILE_GENERIC_READ;
  if (attribute & S_IWUSR)
    owner_allow |= FILE_GENERIC_WRITE;
  if (attribute & S_IXUSR)
    owner_allow |= FILE_GENERIC_EXECUTE & ~FILE_READ_ATTRIBUTES;
  if (S_ISDIR (attribute)
      && (attribute & (S_IWUSR | S_IXUSR)) == (S_IWUSR | S_IXUSR))
    owner_allow |= FILE_DELETE_CHILD;

  /* Construct allow attribute for group. */
  DWORD group_allow = STANDARD_RIGHTS_READ |
		      (pc.fs_is_samba () ? 0 : FILE_READ_ATTRIBUTES);
  if (attribute & S_IRGRP)
    group_allow |= FILE_GENERIC_READ;
  if (attribute & S_IWGRP)
    group_allow |= FILE_GENERIC_WRITE;
  if (attribute & S_IXGRP)
    group_allow |= FILE_GENERIC_EXECUTE & ~FILE_READ_ATTRIBUTES;
  if (S_ISDIR (attribute)
      && (attribute & (S_IWGRP | S_IXGRP)) == (S_IWGRP | S_IXGRP)
      && !(attribute & S_ISVTX))
    group_allow |= FILE_DELETE_CHILD;

  /* Construct allow attribute for everyone. */
  DWORD other_allow = STANDARD_RIGHTS_READ
		      | (pc.fs_is_samba () ? 0 : FILE_READ_ATTRIBUTES);
  if (attribute & S_IROTH)
    other_allow |= FILE_GENERIC_READ;
  if (attribute & S_IWOTH)
    other_allow |= FILE_GENERIC_WRITE;
  if (attribute & S_IXOTH)
    other_allow |= FILE_GENERIC_EXECUTE & ~FILE_READ_ATTRIBUTES;
  if (S_ISDIR (attribute)
      && (attribute & (S_IWOTH | S_IXOTH)) == (S_IWOTH | S_IXOTH)
      && !(attribute & S_ISVTX))
    other_allow |= FILE_DELETE_CHILD;

  /* Construct SUID, SGID and VTX bits in NULL ACE. */
  DWORD null_allow = 0L;
  if (attribute & (S_ISUID | S_ISGID | S_ISVTX))
    {
      if (attribute & S_ISUID)
	null_allow |= FILE_APPEND_DATA;
      if (attribute & S_ISGID)
	null_allow |= FILE_WRITE_DATA;
      if (attribute & S_ISVTX)
	null_allow |= FILE_READ_DATA;
    }

  /* Add owner and group permissions if SIDs are equal
     and construct deny attributes for group and owner. */
  bool isownergroup;
  if ((isownergroup = (owner_sid == group_sid)))
    owner_allow |= group_allow;

  DWORD owner_deny = ~owner_allow & (group_allow | other_allow);
  owner_deny &= ~(STANDARD_RIGHTS_READ
		  | FILE_READ_ATTRIBUTES | FILE_READ_EA
		  | FILE_WRITE_ATTRIBUTES | FILE_WRITE_EA);

  DWORD group_deny = ~group_allow & other_allow;
  group_deny &= ~(STANDARD_RIGHTS_READ
		  | FILE_READ_ATTRIBUTES | FILE_READ_EA);

  /* Set deny ACE for owner. */
  if (owner_deny
      && !add_access_denied_ace (acl, ace_off++, owner_deny,
				 owner_sid, acl_len, NO_INHERITANCE))
    return NULL;
  /* Set deny ACE for group here to respect the canonical order,
     if this does not impact owner */
  if (group_deny && !(group_deny & owner_allow) && !isownergroup
      && !add_access_denied_ace (acl, ace_off++, group_deny,
				 group_sid, acl_len, NO_INHERITANCE))
    return NULL;
  /* Set allow ACE for owner. */
  if (!add_access_allowed_ace (acl, ace_off++, owner_allow,
			       owner_sid, acl_len, NO_INHERITANCE))
    return NULL;
  /* Set deny ACE for group, if still needed. */
  if (group_deny & owner_allow && !isownergroup
      && !add_access_denied_ace (acl, ace_off++, group_deny,
				 group_sid, acl_len, NO_INHERITANCE))
    return NULL;
  /* Set allow ACE for group. */
  if (!isownergroup
      && !add_access_allowed_ace (acl, ace_off++, group_allow,
				  group_sid, acl_len, NO_INHERITANCE))
    return NULL;

  /* Set allow ACE for everyone. */
  if (!add_access_allowed_ace (acl, ace_off++, other_allow,
			       well_known_world_sid, acl_len, NO_INHERITANCE))
    return NULL;
  /* Set null ACE for special bits. */
  if (null_allow
      && !add_access_allowed_ace (acl, ace_off++, null_allow,
				  well_known_null_sid, acl_len, NO_INHERITANCE))
    return NULL;

  /* Fill ACL with unrelated ACEs from current security descriptor. */
  PACL oacl;
  BOOL acl_exists = FALSE;
  ACCESS_ALLOWED_ACE *ace;
  if (GetSecurityDescriptorDacl (sd_ret, &acl_exists, &oacl, &dummy)
      && acl_exists && oacl)
    for (DWORD i = 0; i < oacl->AceCount; ++i)
      if (GetAce (oacl, i, (PVOID *) &ace))
	{
	  cygpsid ace_sid ((PSID) &ace->SidStart);

	  /* Check for related ACEs. */
	  if (ace_sid == well_known_null_sid)
	    continue;
	  if ((ace_sid == cur_owner_sid)
	      || (ace_sid == owner_sid)
	      || (ace_sid == cur_group_sid)
	      || (ace_sid == group_sid)
	      || (ace_sid == well_known_world_sid))
	    {
	      if (ace->Header.AceFlags & SUB_CONTAINERS_AND_OBJECTS_INHERIT)
		ace->Header.AceFlags |= INHERIT_ONLY;
	      else
		continue;
	    }
	  /*
	   * Add unrelated ACCESS_DENIED_ACE to the beginning but
	   * behind the owner_deny, ACCESS_ALLOWED_ACE to the end.
	   * FIXME: this would break the order of the inherit_only ACEs
	   */
	  if (!AddAce (acl, ACL_REVISION,
		       ace->Header.AceType == ACCESS_DENIED_ACE_TYPE?
		       (owner_deny ? 1 : 0) : MAXDWORD,
		       (LPVOID) ace, ace->Header.AceSize))
	    {
	      __seterrno ();
	      return NULL;
	    }
	  acl_len += ace->Header.AceSize;
	}

  /* Construct appropriate inherit attribute for new directories */
  if (S_ISDIR (attribute) && !acl_exists)
    {
      const DWORD inherit = SUB_CONTAINERS_AND_OBJECTS_INHERIT | INHERIT_ONLY;

#if 0 /* FIXME: Not done currently as this breaks the canonical order */
      /* Set deny ACE for owner. */
      if (owner_deny
	  && !add_access_denied_ace (acl, ace_off++, owner_deny,
				     well_known_creator_owner_sid, acl_len, inherit))
	return NULL;
      /* Set deny ACE for group here to respect the canonical order,
	 if this does not impact owner */
      if (group_deny && !(group_deny & owner_allow)
	  && !add_access_denied_ace (acl, ace_off++, group_deny,
				     well_known_creator_group_sid, acl_len, inherit))
	return NULL;
#endif
      /* Set allow ACE for owner. */
      if (!add_access_allowed_ace (acl, ace_off++, owner_allow,
				   well_known_creator_owner_sid, acl_len, inherit))
	return NULL;
#if 0 /* FIXME: Not done currently as this breaks the canonical order and
	 won't be preserved on chown and chmod */
      /* Set deny ACE for group, conflicting with owner_allow. */
      if (group_deny & owner_allow
	  && !add_access_denied_ace (acl, ace_off++, group_deny,
				     well_known_creator_group_sid, acl_len, inherit))
	return NULL;
#endif
      /* Set allow ACE for group. */
      if (!add_access_allowed_ace (acl, ace_off++, group_allow,
				   well_known_creator_group_sid, acl_len, inherit))
	return NULL;
      /* Set allow ACE for everyone. */
      if (!add_access_allowed_ace (acl, ace_off++, other_allow,
				   well_known_world_sid, acl_len, inherit))
	return NULL;
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
  DWORD sd_size = 0;
  MakeSelfRelativeSD (&sd, sd_ret, &sd_size);
  if (sd_size <= 0)
    {
      __seterrno ();
      return NULL;
    }
  if (!sd_ret.malloc (sd_size))
    {
      set_errno (ENOMEM);
      return NULL;
    }
  if (!MakeSelfRelativeSD (&sd, sd_ret, &sd_size))
    {
      __seterrno ();
      return NULL;
    }
  debug_printf ("Created SD-Size: %u", sd_ret.size ());

  return sd_ret;
}

void
set_security_attribute (path_conv &pc, int attribute, PSECURITY_ATTRIBUTES psa,
			security_descriptor &sd)
{
  psa->lpSecurityDescriptor = sd.malloc (SECURITY_DESCRIPTOR_MIN_LENGTH);
  InitializeSecurityDescriptor ((PSECURITY_DESCRIPTOR)psa->lpSecurityDescriptor,
				SECURITY_DESCRIPTOR_REVISION);
  psa->lpSecurityDescriptor = alloc_sd (pc, geteuid32 (), getegid32 (),
					attribute, sd);
}

int
set_file_attribute (HANDLE handle, path_conv &pc,
		    __uid32_t uid, __gid32_t gid, int attribute)
{
  int ret = -1;

  if (pc.has_acls ())
    {
      security_descriptor sd;

      if (!get_file_sd (handle, pc, sd)
	  && alloc_sd (pc, uid, gid, attribute, sd))
	ret = set_file_sd (handle, pc, sd);
    }
  else
    ret = 0;
  syscall_printf ("%d = set_file_attribute (%S, %d, %d, %p)",
		  ret, pc.get_nt_native_path (), uid, gid, attribute);
  return ret;
}

static int
check_access (security_descriptor &sd, GENERIC_MAPPING &mapping,
	      DWORD desired, int flags)
{
  int ret = -1;
  BOOL status;
  DWORD granted;
  DWORD plen = sizeof (PRIVILEGE_SET) + 3 * sizeof (LUID_AND_ATTRIBUTES);
  PPRIVILEGE_SET pset = (PPRIVILEGE_SET) alloca (plen);
  HANDLE tok = cygheap->user.issetuid () ? cygheap->user.imp_token ()
					 : hProcImpToken;

  if (!tok && !DuplicateTokenEx (hProcToken, MAXIMUM_ALLOWED, NULL,
				 SecurityImpersonation, TokenImpersonation,
				 &hProcImpToken))
#ifdef DEBUGGING
	system_printf ("DuplicateTokenEx failed, %E");
#else
	syscall_printf ("DuplicateTokenEx failed, %E");
#endif
  else
    tok = hProcImpToken;

  if (!AccessCheck (sd, tok, desired, &mapping, pset, &plen, &granted, &status))
    __seterrno ();
  else if (!status)
    {
      /* CV, 2006-10-16: Now, that's really weird.  Imagine a user who has no
	 standard access to a file, but who has backup and restore privileges
	 and these privileges are enabled in the access token.  One would
	 expect that the AccessCheck function takes this into consideration
	 when returning the access status.  Otherwise, why bother with the
	 pset parameter, right?
	 But not so.  AccessCheck actually returns a status of "false" here,
	 even though opening a file with backup resp.  restore intent
	 naturally succeeds for this user.  This definitely spoils the results
	 of access(2) for administrative users or the SYSTEM account.  So, in
	 case the access check fails, another check against the user's
	 backup/restore privileges has to be made.  Sigh. */
      int granted_flags = 0;
      if (flags & R_OK)
	{
	  pset->PrivilegeCount = 1;
	  pset->Control = 0;
	  pset->Privilege[0].Luid.HighPart = 0L;
	  pset->Privilege[0].Luid.LowPart = SE_BACKUP_PRIVILEGE;
	  pset->Privilege[0].Attributes = 0;
	  if (PrivilegeCheck (tok, pset, &status) && status)
	    granted_flags |= R_OK;
	}
      if (flags & W_OK)
	{
	  pset->PrivilegeCount = 1;
	  pset->Control = 0;
	  pset->Privilege[0].Luid.HighPart = 0L;
	  pset->Privilege[0].Luid.LowPart = SE_RESTORE_PRIVILEGE;
	  pset->Privilege[0].Attributes = 0;
	  if (PrivilegeCheck (tok, pset, &status) && status)
	    granted_flags |= W_OK;
	}
      if (granted_flags == flags)
	ret = 0;
      else
	set_errno (EACCES);
    }
  else
    ret = 0;
  return ret;
}

int
check_file_access (path_conv &pc, int flags)
{
  security_descriptor sd;
  int ret = -1;
  static GENERIC_MAPPING NO_COPY mapping = { FILE_GENERIC_READ,
					     FILE_GENERIC_WRITE,
					     FILE_GENERIC_EXECUTE,
					     FILE_ALL_ACCESS };
  DWORD desired = 0;
  if (flags & R_OK)
    desired |= FILE_READ_DATA;
  if (flags & W_OK)
    desired |= FILE_WRITE_DATA;
  if (flags & X_OK)
    desired |= FILE_EXECUTE;
  if (!get_file_sd (NULL, pc, sd))
    ret = check_access (sd, mapping, desired, flags);
  debug_printf ("flags %x, ret %d", flags, ret);
  return ret;
}

int
check_registry_access (HANDLE hdl, int flags)
{
  security_descriptor sd;
  int ret = -1;
  static GENERIC_MAPPING NO_COPY mapping = { KEY_READ,
					     KEY_WRITE,
					     KEY_EXECUTE,
					     KEY_ALL_ACCESS };
  DWORD desired = 0;
  if (flags & R_OK)
    desired |= KEY_ENUMERATE_SUB_KEYS;
  if (flags & W_OK)
    desired |= KEY_SET_VALUE;
  if (flags & X_OK)
    desired |= KEY_QUERY_VALUE;
  if (!get_reg_sd (hdl, sd))
    ret = check_access (sd, mapping, desired, flags);
  /* As long as we can't write the registry... */
  if (flags & W_OK)
    {
      set_errno (EROFS);
      ret = -1;
    }
  debug_printf ("flags %x, ret %d", flags, ret);
  return ret;
}
