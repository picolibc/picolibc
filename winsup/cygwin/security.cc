/* security.cc: NT file access control functions

   Copyright 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007,
   2008, 2009, 2010, 2011, 2012, 2013, 2014 Red Hat, Inc.

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
#include "tls_pbuf.h"
#include <aclapi.h>

#define ALL_SECURITY_INFORMATION (DACL_SECURITY_INFORMATION \
				  | GROUP_SECURITY_INFORMATION \
				  | OWNER_SECURITY_INFORMATION)

static GENERIC_MAPPING NO_COPY_RO file_mapping = { FILE_GENERIC_READ,
						   FILE_GENERIC_WRITE,
						   FILE_GENERIC_EXECUTE,
						   FILE_ALL_ACCESS };

LONG
get_file_sd (HANDLE fh, path_conv &pc, security_descriptor &sd,
	     bool justcreated)
{
  NTSTATUS status;
  OBJECT_ATTRIBUTES attr;
  IO_STATUS_BLOCK io;
  ULONG len = SD_MAXIMUM_SIZE, rlen;

  /* Allocate space for the security descriptor. */
  if (!sd.malloc (len))
    {
      set_errno (ENOMEM);
      return -1;
    }
  /* Try to fetch the security descriptor if the handle is valid. */
  if (fh)
    {
      status = NtQuerySecurityObject (fh, ALL_SECURITY_INFORMATION,
				      sd, len, &rlen);
      if (!NT_SUCCESS (status))
	{
	  debug_printf ("NtQuerySecurityObject (%S), status %y",
			pc.get_nt_native_path (), status);
	  fh = NULL;
	}
    }
  /* If the handle was NULL, or fetching with the original handle didn't work,
     try to reopen the file with READ_CONTROL and fetch the security descriptor
     using that handle. */
  if (!fh)
    {
      status = NtOpenFile (&fh, READ_CONTROL,
			   pc.get_object_attr (attr, sec_none_nih), &io,
			   FILE_SHARE_VALID_FLAGS, FILE_OPEN_FOR_BACKUP_INTENT);
      if (!NT_SUCCESS (status))
	{
	  sd.free ();
	  __seterrno_from_nt_status (status);
	  return -1;
	}
      status = NtQuerySecurityObject (fh, ALL_SECURITY_INFORMATION,
				      sd, len, &rlen);
      NtClose (fh);
      if (!NT_SUCCESS (status))
	{
	  sd.free ();
	  __seterrno_from_nt_status (status);
	  return -1;
	}
    }
  /* Ok, so we have a security descriptor now.  Unfortunately, if you want
     to know if an ACE is inherited from the parent object, you can't just
     call NtQuerySecurityObject once.  The problem is this:

     In the simple case, the SDs control word contains one of the
     SE_DACL_AUTO_INHERITED or SE_DACL_PROTECTED flags, or at least one of
     the ACEs has the INHERITED_ACE flag set.  In all of these cases the
     GetSecurityInfo function calls NtQuerySecurityObject only once, too,
     apparently because it figures that the DACL is self-sufficient, which
     it usually is.  Windows Explorer, for instance, takes great care to
     set these flags in a security descriptor if you change the ACL in the
     GUI property dialog.

     The tricky case is if none of these flags is set in the SD.  That means
     the information whether or not an ACE has been inherited is not available
     in the DACL of the object.  In this case GetSecurityInfo also fetches the
     SD from the parent directory and tests if the object's SD contains
     inherited ACEs from the parent.  The below code is closly emulating the
     behaviour of GetSecurityInfo so we can get rid of this advapi32 dependency.

     However, this functionality is slow, and the extra information is only
     required when the file has been created and the permissions are about
     to be set to POSIX permissions.  Therefore we only use it in case the
     file just got created.

     Note that GetSecurityInfo has a problem on 5.1 and 5.2 kernels.  Sometimes
     it returns ERROR_INVALID_ADDRESS if a former request for the parent
     directories' SD used NtQuerySecurityObject, rather than GetSecurityInfo
     as well.  See http://cygwin.com/ml/cygwin-developers/2011-03/msg00027.html
     for the solution.  This problem does not occur with the below code, so
     the workaround has been removed. */
  if (justcreated)
    {
      SECURITY_DESCRIPTOR_CONTROL ctrl;
      ULONG dummy;
      PACL dacl;
      BOOLEAN exists, def;
      ACCESS_ALLOWED_ACE *ace;
      UNICODE_STRING dirname;
      PSECURITY_DESCRIPTOR psd, nsd;
      tmp_pathbuf tp;

      /* Check SDs control flags.  If SE_DACL_AUTO_INHERITED or
	 SE_DACL_PROTECTED is set we're done. */
      RtlGetControlSecurityDescriptor (sd, &ctrl, &dummy);
      if (ctrl & (SE_DACL_AUTO_INHERITED | SE_DACL_PROTECTED))
	return 0;
      /* Otherwise iterate over the ACEs and see if any one of them has the
	 INHERITED_ACE flag set.  If so, we're done. */
      if (NT_SUCCESS (RtlGetDaclSecurityDescriptor (sd, &exists, &dacl, &def))
	  && exists && dacl)
	for (ULONG idx = 0; idx < dacl->AceCount; ++idx)
	  if (NT_SUCCESS (RtlGetAce (dacl, idx, (PVOID *) &ace))
	      && (ace->Header.AceFlags & INHERITED_ACE))
	    return 0;
      /* Otherwise, open the parent directory with READ_CONTROL... */
      RtlSplitUnicodePath (pc.get_nt_native_path (), &dirname, NULL);
      InitializeObjectAttributes (&attr, &dirname, pc.objcaseinsensitive (),
				  NULL, NULL);
      status = NtOpenFile (&fh, READ_CONTROL, &attr, &io,
			   FILE_SHARE_VALID_FLAGS,
			   FILE_OPEN_FOR_BACKUP_INTENT
			   | FILE_OPEN_REPARSE_POINT);
      if (!NT_SUCCESS (status))
	{
	  debug_printf ("NtOpenFile (%S), status %y", &dirname, status);
	  return 0;
	}
      /* ... fetch the parent's security descriptor ... */
      psd = (PSECURITY_DESCRIPTOR) tp.w_get ();
      status = NtQuerySecurityObject (fh, ALL_SECURITY_INFORMATION,
				      psd, len, &rlen);
      NtClose (fh);
      if (!NT_SUCCESS (status))
	{
	  debug_printf ("NtQuerySecurityObject (%S), status %y",
			&dirname, status);
	  return 0;
	}
      /* ... and create a new security descriptor in which all inherited ACEs
	 are marked with the INHERITED_ACE flag.  For a description of the
	 undocumented RtlConvertToAutoInheritSecurityObject function from
	 ntdll.dll see the MSDN man page for the advapi32 function
	 ConvertToAutoInheritPrivateObjectSecurity.  Fortunately the latter
	 is just a shim. */
      status = RtlConvertToAutoInheritSecurityObject (psd, sd, &nsd, NULL,
						      pc.isdir (),
						      &file_mapping);
      if (!NT_SUCCESS (status))
	{
	  debug_printf ("RtlConvertToAutoInheritSecurityObject (%S), status %y",
			&dirname, status);
	  return 0;
	}
      /* Eventually copy the new security descriptor into sd and delete the
	 original one created by RtlConvertToAutoInheritSecurityObject from
	 the heap. */
      len = RtlLengthSecurityDescriptor (nsd);
      memcpy ((PSECURITY_DESCRIPTOR) sd, nsd, len);
      RtlDeleteSecurityObject (&nsd);
    }
  return 0;
}

LONG
set_file_sd (HANDLE fh, path_conv &pc, security_descriptor &sd, bool is_chown)
{
  NTSTATUS status = STATUS_SUCCESS;
  int retry = 0;
  int res = -1;

  for (; retry < 2; ++retry)
    {
      if (fh)
	{
	  status = NtSetSecurityObject (fh,
					is_chown ? ALL_SECURITY_INFORMATION
						 : DACL_SECURITY_INFORMATION,
					sd);
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
	  status = NtOpenFile (&fh, (is_chown ? WRITE_OWNER  : 0) | WRITE_DAC,
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
      if (!NT_SUCCESS (RtlGetAce (acl, i, (PVOID *) &ace)))
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
  if (owner_sid && group_sid && RtlEqualSid (owner_sid, group_sid)
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
		  uid_t *uidret, gid_t *gidret)
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
  NTSTATUS status;
  BOOLEAN dummy;

  status = RtlGetOwnerSecurityDescriptor (psd, (PSID *) &owner_sid, &dummy);
  if (!NT_SUCCESS (status))
    debug_printf ("RtlGetOwnerSecurityDescriptor: %y", status);
  status = RtlGetGroupSecurityDescriptor (psd, (PSID *) &group_sid, &dummy);
  if (!NT_SUCCESS (status))
    debug_printf ("RtlGetGroupSecurityDescriptor: %y", status);

  uid_t uid;
  gid_t gid;
  bool grp_member = get_sids_info (owner_sid, group_sid, &uid, &gid);
  if (uidret)
    *uidret = uid;
  if (gidret)
    *gidret = gid;

  if (!attribute)
    {
      syscall_printf ("uid %u, gid %u", uid, gid);
      return;
    }

  PACL acl;
  BOOLEAN acl_exists;

  status = RtlGetDaclSecurityDescriptor (psd, &acl_exists, &acl, &dummy);
  if (!NT_SUCCESS (status))
    {
      __seterrno_from_nt_status (status);
      *attribute &= ~(S_IRWXU | S_IRWXG | S_IRWXO);
    }
  else if (!acl_exists || !acl)
    *attribute |= S_IRWXU | S_IRWXG | S_IRWXO;
  else
    get_attribute_from_acl (attribute, acl, owner_sid, group_sid, grp_member);

  syscall_printf ("%sACL %y, uid %u, gid %u",
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
get_reg_attribute (HKEY hkey, mode_t *attribute, uid_t *uidret,
		   gid_t *gidret)
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
		    mode_t *attribute, uid_t *uidret, gid_t *gidret)
{
  if (pc.has_acls ())
    {
      security_descriptor sd;

      if (!get_file_sd (handle, pc, sd, false))
	{
	  get_info_from_sd (sd, attribute, uidret, gidret);
	  return 0;
	}
      /* ENOSYS is returned by get_file_sd if fetching the DACL from a remote
	 share returns STATUS_INVALID_NETWORK_RESPONSE, which in turn is
	 converted to ERROR_BAD_NET_RESP.  This potentially occurs when trying
	 to fetch DACLs from a NT4 machine which is not part of the domain of
	 the requesting machine. */
      else if (get_errno () != ENOSYS)
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
  NTSTATUS status = RtlAddAccessAllowedAceEx (acl, ACL_REVISION, inherit,
					      attributes, sid);
  if (!NT_SUCCESS (status))
    {
      __seterrno_from_nt_status (status);
      return false;
    }
  len_add += sizeof (ACCESS_ALLOWED_ACE) - sizeof (DWORD) + RtlLengthSid (sid);
  return true;
}

bool
add_access_denied_ace (PACL acl, int offset, DWORD attributes,
		       PSID sid, size_t &len_add, DWORD inherit)
{
  NTSTATUS status = RtlAddAccessDeniedAceEx (acl, ACL_REVISION, inherit,
					     attributes, sid);
  if (!NT_SUCCESS (status))
    {
      __seterrno_from_nt_status (status);
      return false;
    }
  len_add += sizeof (ACCESS_DENIED_ACE) - sizeof (DWORD) + RtlLengthSid (sid);
  return true;
}

static PSECURITY_DESCRIPTOR
alloc_sd (path_conv &pc, uid_t uid, gid_t gid, int attribute,
	  security_descriptor &sd_ret)
{
  NTSTATUS status;
  BOOLEAN dummy;
  tmp_pathbuf tp;

  /* NOTE: If the high bit of attribute is set, we have just created
     a file or directory.  See below for an explanation. */

  debug_printf("uid %u, gid %u, attribute %y", uid, gid, attribute);

  /* Get owner and group from current security descriptor. */
  PSID cur_owner_sid = NULL;
  PSID cur_group_sid = NULL;
  status = RtlGetOwnerSecurityDescriptor (sd_ret, &cur_owner_sid, &dummy);
  if (!NT_SUCCESS (status))
    debug_printf ("RtlGetOwnerSecurityDescriptor: %y", status);
  status = RtlGetGroupSecurityDescriptor (sd_ret, &cur_group_sid, &dummy);
  if (!NT_SUCCESS (status))
    debug_printf ("RtlGetGroupSecurityDescriptor: %y", status);

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
  RtlCreateSecurityDescriptor (&sd, SECURITY_DESCRIPTOR_REVISION);

  /* We set the SE_DACL_PROTECTED flag here to prevent the DACL from being
     modified by inheritable ACEs. */
  RtlSetControlSecurityDescriptor (&sd, SE_DACL_PROTECTED, SE_DACL_PROTECTED);

  /* Create owner for local security descriptor. */
  status = RtlSetOwnerSecurityDescriptor (&sd, owner_sid, FALSE);
  if (!NT_SUCCESS (status))
    {
      __seterrno_from_nt_status (status);
      return NULL;
    }

  /* Create group for local security descriptor. */
  status = RtlSetGroupSecurityDescriptor (&sd, group_sid, FALSE);
  if (!NT_SUCCESS (status))
    {
      __seterrno_from_nt_status (status);
      return NULL;
    }

  /* Initialize local access control list. */
  PACL acl = (PACL) tp.w_get ();
  RtlCreateAcl (acl, ACL_MAXIMUM_SIZE, ACL_REVISION);

  /* From here fill ACL. */
  size_t acl_len = sizeof (ACL);
  int ace_off = 0;
  /* Only used for sync objects (for ttys).  The admins group should
     always have the right to manipulate the ACL, so we have to make sure
     that the ACL gives the admins group STANDARD_RIGHTS_ALL access. */
  bool saw_admins = false;

  /* Construct allow attribute for owner.
     Don't set FILE_READ/WRITE_ATTRIBUTES unconditionally on Samba, otherwise
     it enforces read permissions.  Same for other's below. */
  DWORD owner_allow = STANDARD_RIGHTS_ALL
		      | (pc.fs_is_samba ()
			 ? 0 : (FILE_READ_ATTRIBUTES | FILE_WRITE_ATTRIBUTES));
  if (attribute & S_IRUSR)
    owner_allow |= FILE_GENERIC_READ;
  if (attribute & S_IWUSR)
    owner_allow |= FILE_GENERIC_WRITE;
  if (attribute & S_IXUSR)
    owner_allow |= FILE_GENERIC_EXECUTE & ~FILE_READ_ATTRIBUTES;
  if (S_ISDIR (attribute)
      && (attribute & (S_IWUSR | S_IXUSR)) == (S_IWUSR | S_IXUSR))
    owner_allow |= FILE_DELETE_CHILD;
  /* For sync objects note that the owner is admin. */
  if (S_ISCHR (attribute) && owner_sid == well_known_admins_sid)
    saw_admins = true;

  /* Construct allow attribute for group. */
  DWORD group_allow = STANDARD_RIGHTS_READ | SYNCHRONIZE
		      | (pc.fs_is_samba () ? 0 : FILE_READ_ATTRIBUTES);
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
  /* For sync objects, add STANDARD_RIGHTS_ALL for admins group. */
  if (S_ISCHR (attribute) && group_sid == well_known_admins_sid)
    {
      group_allow |= STANDARD_RIGHTS_ALL;
      saw_admins = true;
    }

  /* Construct allow attribute for everyone. */
  DWORD other_allow = STANDARD_RIGHTS_READ | SYNCHRONIZE
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
		  | FILE_READ_ATTRIBUTES | FILE_WRITE_ATTRIBUTES);

  DWORD group_deny = ~group_allow & other_allow;
  group_deny &= ~(STANDARD_RIGHTS_READ | FILE_READ_ATTRIBUTES);

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

  /* For sync objects, if we didn't see the admins group so far, add entry
     with STANDARD_RIGHTS_ALL access. */
  if (S_ISCHR (attribute) && !saw_admins)
    {
      if (!add_access_allowed_ace (acl, ace_off++, STANDARD_RIGHTS_ALL,
				   well_known_admins_sid, acl_len,
				   NO_INHERITANCE))
	return NULL;
      saw_admins = true;
    }

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
  BOOLEAN acl_exists = FALSE;
  ACCESS_ALLOWED_ACE *ace;

  status = RtlGetDaclSecurityDescriptor (sd_ret, &acl_exists, &oacl, &dummy);
  if (NT_SUCCESS (status) && acl_exists && oacl)
    for (DWORD i = 0; i < oacl->AceCount; ++i)
      if (NT_SUCCESS (RtlGetAce (oacl, i, (PVOID *) &ace)))
	{
	  cygpsid ace_sid ((PSID) &ace->SidStart);

	  /* Always skip NULL SID as well as admins SID on virtual device files
	     in /proc/sys. */
	  if (ace_sid == well_known_null_sid
	      || (S_ISCHR (attribute) && ace_sid == well_known_admins_sid))
	    continue;
	  /* Check for ACEs which are always created in the preceding code
	     and check for the default inheritence ACEs which will be created
	     for just created directories.  Skip them for just created
	     directories or if they are not inherited.  If they are inherited,
	     make sure they are *only* inherited, so they don't collide with
	     the permissions set in this function. */
	  if ((ace_sid == cur_owner_sid)
	      || (ace_sid == owner_sid)
	      || (ace_sid == cur_group_sid)
	      || (ace_sid == group_sid)
	      || (ace_sid == well_known_creator_owner_sid)
	      || (ace_sid == well_known_creator_group_sid)
	      || (ace_sid == well_known_world_sid))
	    {
	      if ((S_ISDIR (attribute) && (attribute & S_JUSTCREATED))
		  || (ace->Header.AceFlags
		      & (CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE)) == 0)
		continue;
	      else
		ace->Header.AceFlags |= INHERIT_ONLY_ACE;
	    }
	  if (attribute & S_JUSTCREATED)
	    {
	      /* Since files and dirs are created with a NULL descriptor,
		 inheritence rules kick in.  If no inheritable entries exist
		 in the parent object, Windows will create entries from the
		 user token's default DACL in the file DACL.  These entries
		 are not desired and we drop them silently. */
	      if (!(ace->Header.AceFlags & INHERITED_ACE))
		continue;
	      /* Remove the INHERITED_ACE flag since on POSIX systems
		 inheritance is settled when the file has been created.
		 This also avoids error messages in Windows Explorer when
		 opening a file's security tab.  Explorer complains if
		 inheritable ACEs are preceding non-inheritable ACEs. */
	      ace->Header.AceFlags &= ~INHERITED_ACE;
	    }
	  /*
	   * Add unrelated ACCESS_DENIED_ACE to the beginning but
	   * behind the owner_deny, ACCESS_ALLOWED_ACE to the end.
	   * FIXME: this would break the order of the inherit-only ACEs
	   */
	  status = RtlAddAce (acl, ACL_REVISION,
			      ace->Header.AceType == ACCESS_DENIED_ACE_TYPE
			      ?  (owner_deny ? 1 : 0) : MAXDWORD,
			      (LPVOID) ace, ace->Header.AceSize);
	  if (!NT_SUCCESS (status))
	    {
	      __seterrno_from_nt_status (status);
	      return NULL;
	    }
	  ace_off++;
	  acl_len += ace->Header.AceSize;
	}

  /* Construct appropriate inherit attribute for new directories.  Keep in
     mind that we do this only for the sake of non-Cygwin applications.
     Cygwin applications don't need this. */
  if (S_ISDIR (attribute) && (attribute & S_JUSTCREATED))
    {
      const DWORD inherit = CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE
			    | INHERIT_ONLY_ACE;
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
				   well_known_creator_owner_sid, acl_len,
				   inherit))
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
				   well_known_creator_group_sid, acl_len,
				   inherit))
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
  status = RtlSetDaclSecurityDescriptor (&sd, TRUE, acl, FALSE);
  if (!NT_SUCCESS (status))
    {
      __seterrno_from_nt_status (status);
      return NULL;
    }

  /* Make self relative security descriptor. */
  DWORD sd_size = 0;
  RtlAbsoluteToSelfRelativeSD (&sd, sd_ret, &sd_size);
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
  status = RtlAbsoluteToSelfRelativeSD (&sd, sd_ret, &sd_size);
  if (!NT_SUCCESS (status))
    {
      __seterrno_from_nt_status (status);
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
  RtlCreateSecurityDescriptor ((PSECURITY_DESCRIPTOR) psa->lpSecurityDescriptor,
				SECURITY_DESCRIPTOR_REVISION);
  psa->lpSecurityDescriptor = alloc_sd (pc, geteuid32 (), getegid32 (),
					attribute, sd);
}

int
get_object_sd (HANDLE handle, security_descriptor &sd)
{
  ULONG len = 0;
  NTSTATUS status;

  status = NtQuerySecurityObject (handle, ALL_SECURITY_INFORMATION,
				  sd, len, &len);
  if (status != STATUS_BUFFER_TOO_SMALL)
    {
      __seterrno_from_nt_status (status);
      return -1;
    }
  if (!sd.malloc (len))
    {
      set_errno (ENOMEM);
      return -1;
    }
  status = NtQuerySecurityObject (handle, ALL_SECURITY_INFORMATION,
				  sd, len, &len);
  if (!NT_SUCCESS (status))
    {
      __seterrno_from_nt_status (status);
      return -1;
    }
  return 0;
}

int
get_object_attribute (HANDLE handle, uid_t *uidret, gid_t *gidret,
		      mode_t *attribute)
{
  security_descriptor sd;

  if (get_object_sd (handle, sd))
    return -1;
  get_info_from_sd (sd, attribute, uidret, gidret);
  return 0;
}

int
create_object_sd_from_attribute (HANDLE handle, uid_t uid, gid_t gid,
				 mode_t attribute, security_descriptor &sd)
{
  path_conv pc;
  if ((handle && get_object_sd (handle, sd))
      || !alloc_sd (pc, uid, gid, attribute, sd))
    return -1;
  return 0;
}

int
set_object_sd (HANDLE handle, security_descriptor &sd, bool chown)
{
  NTSTATUS status;
  status = NtSetSecurityObject (handle, chown ? ALL_SECURITY_INFORMATION
					      : DACL_SECURITY_INFORMATION, sd);
  if (!NT_SUCCESS (status))
    {
      __seterrno_from_nt_status (status);
      return -1;
    }
  return 0;
}

int
set_object_attribute (HANDLE handle, uid_t uid, gid_t gid,
		      mode_t attribute)
{
  security_descriptor sd;

  if (create_object_sd_from_attribute (handle, uid, gid, attribute, sd)
      || set_object_sd (handle, sd, uid != ILLEGAL_UID || gid != ILLEGAL_GID))
    return -1;
  return 0;
}

int
set_file_attribute (HANDLE handle, path_conv &pc,
		    uid_t uid, gid_t gid, mode_t attribute)
{
  int ret = -1;

  if (pc.has_acls ())
    {
      security_descriptor sd;

      if (!get_file_sd (handle, pc, sd, (bool)(attribute & S_JUSTCREATED))
	  && alloc_sd (pc, uid, gid, attribute, sd))
	ret = set_file_sd (handle, pc, sd,
			   uid != ILLEGAL_UID || gid != ILLEGAL_GID);
    }
  else
    ret = 0;
  syscall_printf ("%d = set_file_attribute(%S, %d, %d, %y)",
		  ret, pc.get_nt_native_path (), uid, gid, attribute);
  return ret;
}

static int
check_access (security_descriptor &sd, GENERIC_MAPPING &mapping,
	      ACCESS_MASK desired, int flags, bool effective)
{
  int ret = -1;
  NTSTATUS status, allow;
  ACCESS_MASK granted;
  DWORD plen = sizeof (PRIVILEGE_SET) + 3 * sizeof (LUID_AND_ATTRIBUTES);
  PPRIVILEGE_SET pset = (PPRIVILEGE_SET) alloca (plen);
  HANDLE tok = ((effective && cygheap->user.issetuid ())
		? cygheap->user.imp_token ()
		: hProcImpToken);

  if (!tok)
    {
      if (!DuplicateTokenEx (hProcToken, MAXIMUM_ALLOWED, NULL,
			    SecurityImpersonation, TokenImpersonation,
			    &hProcImpToken))
	 {
	    __seterrno ();
	    return ret;
	 }
      tok = hProcImpToken;
    }

  status = NtAccessCheck (sd, tok, desired, &mapping, pset, &plen, &granted,
			  &allow);
  if (!NT_SUCCESS (status))
    __seterrno ();
  else if (!NT_SUCCESS (allow))
    {
      /* CV, 2006-10-16: Now, that's really weird.  Imagine a user who has no
	 standard access to a file, but who has backup and restore privileges
	 and these privileges are enabled in the access token.  One would
	 expect that the AccessCheck function takes this into consideration
	 when returning the access status.  Otherwise, why bother with the
	 pset parameter, right?
	 But not so.  AccessCheck actually returns a status of "false" here,
	 even though opening a file with backup resp. restore intent
	 naturally succeeds for this user.  This definitely spoils the results
	 of access(2) for administrative users or the SYSTEM account.  So, in
	 case the access check fails, another check against the user's
	 backup/restore privileges has to be made.  Sigh. */
      int granted_flags = 0;
      BOOLEAN has_priv;

      if (flags & R_OK)
	{
	  pset->PrivilegeCount = 1;
	  pset->Control = 0;
	  pset->Privilege[0].Luid.HighPart = 0L;
	  pset->Privilege[0].Luid.LowPart = SE_BACKUP_PRIVILEGE;
	  pset->Privilege[0].Attributes = 0;
	  status = NtPrivilegeCheck (tok, pset, &has_priv);
	  if (NT_SUCCESS (status) && has_priv)
	    granted_flags |= R_OK;
	}
      if (flags & W_OK)
	{
	  pset->PrivilegeCount = 1;
	  pset->Control = 0;
	  pset->Privilege[0].Luid.HighPart = 0L;
	  pset->Privilege[0].Luid.LowPart = SE_RESTORE_PRIVILEGE;
	  pset->Privilege[0].Attributes = 0;
	  status = NtPrivilegeCheck (tok, pset, &has_priv);
	  if (NT_SUCCESS (status) && has_priv)
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

/* Samba override.  Check security descriptor for Samba UNIX user and group
   accounts and check if we have an RFC 2307 mapping to a Windows account.
   Create a new security descriptor with all of the UNIX acocunts with
   valid mapping replaced with their WIndows counterpart. */
static void
convert_samba_sd (security_descriptor &sd_ret)
{
  NTSTATUS status;
  BOOLEAN dummy;
  PSID sid;
  cygsid owner;
  cygsid group;
  SECURITY_DESCRIPTOR sd;
  cyg_ldap cldap;
  tmp_pathbuf tp;
  PACL acl, oacl;
  size_t acl_len;
  PACCESS_ALLOWED_ACE ace;

  if (!NT_SUCCESS (RtlGetOwnerSecurityDescriptor (sd_ret, &sid, &dummy)))
    return;
  owner = sid;
  if (!NT_SUCCESS (RtlGetGroupSecurityDescriptor (sd_ret, &sid, &dummy)))
    return;
  group = sid;

  if (sid_id_auth (owner) == 22)
    {
      struct passwd *pwd;
      uid_t uid = owner.get_uid (&cldap);
      if (uid < UNIX_POSIX_OFFSET && (pwd = internal_getpwuid (uid)))
      	owner.getfrompw (pwd);
    }
  if (sid_id_auth (group) == 22)
    {
      struct group *grp;
      gid_t gid = group.get_gid (&cldap);
      if (gid < UNIX_POSIX_OFFSET && (grp = internal_getgrgid (gid)))
      	group.getfromgr (grp);
    }

  if (!NT_SUCCESS (RtlGetDaclSecurityDescriptor (sd_ret, &dummy,
						 &oacl, &dummy)))
    return;
  acl = (PACL) tp.w_get ();
  RtlCreateAcl (acl, ACL_MAXIMUM_SIZE, ACL_REVISION);
  acl_len = sizeof (ACL);

  for (DWORD i = 0; i < oacl->AceCount; ++i)
    if (NT_SUCCESS (RtlGetAce (oacl, i, (PVOID *) &ace)))
      {
	cygsid ace_sid ((PSID) &ace->SidStart);
	if (sid_id_auth (ace_sid) == 22)
	  {
	    if (sid_sub_auth (ace_sid, 0) == 1) /* user */
	      {
		struct passwd *pwd;
		uid_t uid = ace_sid.get_uid (&cldap);
		if (uid < UNIX_POSIX_OFFSET && (pwd = internal_getpwuid (uid)))
		  ace_sid.getfrompw (pwd);
	      }
	    else /* group */
	      {
		struct group *grp;
		gid_t gid = ace_sid.get_gid (&cldap);
		if (gid < UNIX_POSIX_OFFSET && (grp = internal_getgrgid (gid)))
		  ace_sid.getfromgr (grp);
	      }
	    if (!add_access_allowed_ace (acl, i, ace->Mask, ace_sid, acl_len,
					 ace->Header.AceFlags))
	      return;
	  }
      }
  acl->AclSize = acl_len;

  RtlCreateSecurityDescriptor (&sd, SECURITY_DESCRIPTOR_REVISION);
  RtlSetControlSecurityDescriptor (&sd, SE_DACL_PROTECTED, SE_DACL_PROTECTED);
  RtlSetOwnerSecurityDescriptor (&sd, owner, FALSE);
  RtlSetGroupSecurityDescriptor (&sd, group, FALSE);

  status = RtlSetDaclSecurityDescriptor (&sd, TRUE, acl, FALSE);
  if (!NT_SUCCESS (status))
    return;
  DWORD sd_size = 0;
  status = RtlAbsoluteToSelfRelativeSD (&sd, sd_ret, &sd_size);
  if (sd_size > 0 && sd_ret.malloc (sd_size))
    RtlAbsoluteToSelfRelativeSD (&sd, sd_ret, &sd_size);
}

int
check_file_access (path_conv &pc, int flags, bool effective)
{
  security_descriptor sd;
  int ret = -1;
  ACCESS_MASK desired = 0;
  if (flags & R_OK)
    desired |= FILE_READ_DATA;
  if (flags & W_OK)
    desired |= FILE_WRITE_DATA;
  if (flags & X_OK)
    desired |= FILE_EXECUTE;
  if (!get_file_sd (pc.handle (), pc, sd, false))
    {
      /* Tweak Samba security descriptor as necessary. */
      if (pc.fs_is_samba ())
	convert_samba_sd (sd);
      ret = check_access (sd, file_mapping, desired, flags, effective);
    }
  debug_printf ("flags %y, ret %d", flags, ret);
  return ret;
}

int
check_registry_access (HANDLE hdl, int flags, bool effective)
{
  security_descriptor sd;
  int ret = -1;
  static GENERIC_MAPPING NO_COPY_RO reg_mapping = { KEY_READ,
						    KEY_WRITE,
						    KEY_EXECUTE,
						    KEY_ALL_ACCESS };
  ACCESS_MASK desired = 0;
  if (flags & R_OK)
    desired |= KEY_ENUMERATE_SUB_KEYS;
  if (flags & W_OK)
    desired |= KEY_SET_VALUE;
  if (flags & X_OK)
    desired |= KEY_QUERY_VALUE;

  if ((HKEY) hdl == HKEY_PERFORMANCE_DATA)
    /* RegGetKeySecurity() always fails with ERROR_INVALID_HANDLE.  */
    ret = 0;
  else if (!get_reg_sd (hdl, sd))
    ret = check_access (sd, reg_mapping, desired, flags, effective);

  /* As long as we can't write the registry... */
  if (flags & W_OK)
    {
      set_errno (EROFS);
      ret = -1;
    }
  debug_printf ("flags %y, ret %d", flags, ret);
  return ret;
}
