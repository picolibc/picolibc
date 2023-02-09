/* sec/base.cc: NT file access control functions

   Originaly written by Gunther Ebert, gunther.ebert@ixos-leipzig.de
   Completely rewritten by Corinna Vinschen <corinna@vinschen.de>

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <unistd.h>
#include <stdlib.h>
#include <cygwin/acl.h>
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
  NTSTATUS status = STATUS_SUCCESS;
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
	debug_printf ("NtQuerySecurityObject (%S), status %y",
		      pc.get_nt_native_path (), status);
    }
  /* If the handle was NULL, or fetching with the original handle didn't work,
     try to reopen the file with READ_CONTROL and fetch the security descriptor
     using that handle. */
  if (!fh || !NT_SUCCESS (status))
    {
      status = NtOpenFile (&fh, READ_CONTROL,
			   fh ? pc.init_reopen_attr (attr, fh)
			      : pc.get_object_attr (attr, sec_none_nih),
			   &io, FILE_SHARE_VALID_FLAGS,
			   FILE_OPEN_FOR_BACKUP_INTENT
			   | pc.is_known_reparse_point ()
			   ? FILE_OPEN_REPARSE_POINT : 0);
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
  /* We have a security descriptor now.  Unfortunately, if you want to know
     if an ACE is inherited from the parent object, this isn't sufficient.

     In the simple case, the SDs control word contains one of the
     SE_DACL_AUTO_INHERITED or SE_DACL_PROTECTED flags, or at least one of
     the ACEs has the INHERITED_ACE flag set.  In all of these cases we
     know the DACL has been inherited.

     If none of these flags is set in the SD, the information whether
     or not an ACE has been inherited is not available in the DACL of the
     object.  In this case GetSecurityInfo fetches the SD from the parent
     directory and tests if the object's SD contains inherited ACEs from the
     parent.

     Note that we're not testing the SE_DACL_AUTO_INHERITED and
     SE_DACL_PROTECTED flags here because we know the state the file's SD
     is in.  Since we're creating all files with a NULL descriptor, the DACL
     is either inherited from the parent, or it's the default DACL.  In
     neither case, one of these flags is set.

     For speed, we're not calling RtlConvertToAutoInheritSecurityObject
     anymore (but keep the code here for reference).  Rather we just test
     if one of the parent's ACEs is inheritable.  If so, we know we inherited
     it and set the SE_DACL_AUTO_INHERITED flag.  If not, we may assume our
     object's DACL is the default DACL.

     This functionality is slow and the extra information is only required
     when the file has been created and the permissions are about to be set
     to POSIX permissions.  Therefore we only use it in case the file just
     got created. */
  if (justcreated)
    {
      PACL dacl;
      BOOLEAN exists, def;
      ACCESS_ALLOWED_ACE *ace;
      UNICODE_STRING dirname;
      PSECURITY_DESCRIPTOR psd;
      tmp_pathbuf tp;

      /* Open the parent directory with READ_CONTROL... */
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
#if 0
      /* ... and create a new security descriptor in which all inherited ACEs
	 are marked with the INHERITED_ACE flag.  For a description of the
	 undocumented RtlConvertToAutoInheritSecurityObject function from
	 ntdll.dll see the MSDN man page for the advapi32 function
	 ConvertToAutoInheritPrivateObjectSecurity.  Fortunately the latter
	 is just a shim. */
      PSECURITY_DESCRIPTOR nsd;
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
#else
      /* ... and check the parent descriptor for inheritable ACEs matching
	 our current object type (file/dir).  The simple truth in our case
	 is, either the parent dir had inheritable ACEs and all our ACEs are
	 inherited, or the parent dir didn't have inheritable ACEs and all
	 our ACEs are taken from the default DACL. */
      bool inherited = false;
      BYTE search_flags = pc.isdir () ? SUB_CONTAINERS_AND_OBJECTS_INHERIT
				      : SUB_OBJECTS_ONLY_INHERIT;
      if (NT_SUCCESS (RtlGetDaclSecurityDescriptor (psd, &exists, &dacl, &def))
	  && exists && dacl)
	for (ULONG idx = 0; idx < dacl->AceCount; ++idx)
	  if (NT_SUCCESS (RtlGetAce (dacl, idx, (PVOID *) &ace))
	      && (ace->Header.AceFlags & search_flags))
	    {
	      inherited = true;
	      break;
	    }
      /* Then, if the parent descriptor contained inheritable ACEs, we mark
	 the SD as SE_DACL_AUTO_INHERITED.  Note that this requires the
	 matching check in get_posix_access.  If we ever revert to
	 RtlConvertToAutoInheritSecurityObject, the check in get_posix_access
	 has to test every single ACE for the INHERITED_ACE flag again. */
      if (inherited
	  && NT_SUCCESS (RtlGetDaclSecurityDescriptor (sd, &exists, &dacl,
						       &def))
	  && exists && dacl)
	RtlSetControlSecurityDescriptor (sd, SE_DACL_AUTO_INHERITED,
					     SE_DACL_AUTO_INHERITED);
#endif
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
			       fh ? pc.init_reopen_attr (attr, fh)
				  : pc.get_object_attr (attr, sec_none_nih),
			       &io,
			       FILE_SHARE_VALID_FLAGS,
			       FILE_OPEN_FOR_BACKUP_INTENT
			       | pc.is_known_reparse_point ()
			       ? FILE_OPEN_REPARSE_POINT : 0);
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
get_reg_attribute (HKEY hkey, mode_t &attribute, uid_t *uidret,
		   gid_t *gidret)
{
  security_descriptor sd;

  if (!get_reg_sd (hkey, sd))
    {
      get_posix_access (sd, attribute, uidret, gidret, NULL, 0);
      return 0;
    }
  /* The entries are already set to default values */
  return -1;
}

int
get_file_attribute (HANDLE handle, path_conv &pc,
		    mode_t &attribute, uid_t *uidret, gid_t *gidret)
{
  if (pc.has_acls ())
    {
      security_descriptor sd;

      if (!get_file_sd (handle, pc, sd, false))
	{
	  get_posix_access (sd, attribute, uidret, gidret, NULL, 0);
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
add_access_allowed_ace (PACL acl, DWORD attributes, PSID sid, size_t &len_add,
			DWORD inherit)
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
add_access_denied_ace (PACL acl, DWORD attributes, PSID sid, size_t &len_add,
		       DWORD inherit)
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

void
set_security_attribute (path_conv &pc, int attribute, PSECURITY_ATTRIBUTES psa,
			security_descriptor &sd)
{
  psa->lpSecurityDescriptor = sd.malloc (SECURITY_DESCRIPTOR_MIN_LENGTH);
  RtlCreateSecurityDescriptor ((PSECURITY_DESCRIPTOR) psa->lpSecurityDescriptor,
				SECURITY_DESCRIPTOR_REVISION);
  psa->lpSecurityDescriptor = set_posix_access (attribute, geteuid (),
						getegid (), NULL, 0,
						sd, false);
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
		      mode_t &attribute)
{
  security_descriptor sd;

  if (get_object_sd (handle, sd))
    return -1;
  return get_posix_access (sd, attribute, uidret, gidret, NULL, 0)
	 >= 0 ? 0 : -1;
}

int
create_object_sd_from_attribute (uid_t uid, gid_t gid, mode_t attribute,
				 security_descriptor &sd)
{
  return set_posix_access (attribute, uid, gid, NULL, 0, sd, false)
	 ? 0 : -1;
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
set_object_attribute (HANDLE handle, uid_t uid, gid_t gid, mode_t attribute)
{
  security_descriptor sd;

  if (create_object_sd_from_attribute (uid, gid, attribute, sd)
      || set_object_sd (handle, sd, uid != ILLEGAL_UID || gid != ILLEGAL_GID))
    return -1;
  return 0;
}

int
set_created_file_access (HANDLE handle, path_conv &pc, mode_t attr)
{
  int ret = -1;
  security_descriptor sd, sd_ret;
  mode_t attr_rd;
  uid_t uid;
  gid_t gid;
  tmp_pathbuf tp;
  aclent_t *aclp;
  int nentries, idx;
  bool std_acl;

  if (!get_file_sd (handle, pc, sd, true))
    {
      attr |= S_JUSTCREATED;
      if (pc.isdir ())
	attr |= S_IFDIR;
      attr_rd = attr;
      aclp = (aclent_t *) tp.c_get ();
      if ((nentries = get_posix_access (sd, attr_rd, &uid, &gid, aclp,
					MAX_ACL_ENTRIES, &std_acl)) >= 0)
	{
	  if (S_ISLNK (attr))
	    {
	      /* Symlinks always get the request POSIX perms. */
	      aclp[0].a_perm = (attr >> 6) & S_IRWXO;
	      if ((idx = searchace (aclp, nentries, GROUP_OBJ)) >= 0)
		aclp[idx].a_perm = (attr >> 3) & S_IRWXO;
	      if ((idx = searchace (aclp, nentries, CLASS_OBJ)) >= 0)
		aclp[idx].a_perm = (attr >> 3) & S_IRWXO;
	      if ((idx = searchace (aclp, nentries, OTHER_OBJ)) >= 0)
		aclp[idx].a_perm = attr & S_IRWXO;
	    }
	  else
	    {
	      /* Overwrite ACL permissions as required by POSIX 1003.1e
		 draft 17. */
	      aclp[0].a_perm &= (attr >> 6) & S_IRWXO;
	      if ((idx = searchace (aclp, nentries, CLASS_OBJ)) >= 0)
		aclp[idx].a_perm &= (attr >> 3) & S_IRWXO;
	      if (std_acl
		  && (idx = searchace (aclp, nentries, GROUP_OBJ)) >= 0)
		aclp[idx].a_perm &= (attr >> 3) & S_IRWXO;
	      if ((idx = searchace (aclp, nentries, OTHER_OBJ)) >= 0)
		aclp[idx].a_perm &= attr & S_IRWXO;
	    }
	  /* Construct appropriate inherit attribute for new directories.
	     Basically we do this only for the sake of non-Cygwin applications.
	     Cygwin applications don't need these.  Additionally, if the
	     S_ISGID bit is set, propagate it. */
	  if (S_ISDIR (attr))
	    {
	      mode_t def_attr = attr & ~cygheap->umask;

	      if (searchace (aclp, nentries, DEF_USER_OBJ) < 0)
		{
		  aclp[nentries].a_type = DEF_USER_OBJ;
		  aclp[nentries].a_id = ILLEGAL_UID;
		  aclp[nentries++].a_perm = (def_attr >> 6) & S_IRWXO;
		}
	      if (searchace (aclp, nentries, DEF_GROUP_OBJ) < 0)
		{
		  aclp[nentries].a_type = DEF_GROUP_OBJ;
		  aclp[nentries].a_id = ILLEGAL_GID;
		  aclp[nentries++].a_perm = (def_attr >> 3) & S_IRWXO;
		}
	      if (searchace (aclp, nentries, DEF_OTHER_OBJ) < 0)
		{
		  aclp[nentries].a_type = DEF_OTHER_OBJ;
		  aclp[nentries].a_id = ILLEGAL_UID;
		  aclp[nentries++].a_perm = def_attr & S_IRWXO;
		}
	      if (attr_rd & S_ISGID)
		attr |= S_ISGID;
	    }
	  if (set_posix_access (attr, uid, gid, aclp, nentries, sd_ret,
				pc.fs_is_samba ()))
	    ret = set_file_sd (handle, pc, sd_ret, attr_rd & S_ISGID);
	}
    }
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
   Create a new security descriptor with all of the UNIX accounts with
   valid mapping replaced with their Windows counterpart. */
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
	    else if (sid_sub_auth (ace_sid, 0) == 2) /* group */
	      {
		struct group *grp;
		gid_t gid = ace_sid.get_gid (&cldap);
		if (gid < UNIX_POSIX_OFFSET && (grp = internal_getgrgid (gid)))
		  ace_sid.getfromgr (grp);
	      }
	  }
	if (!add_access_allowed_ace (acl, ace->Mask, ace_sid, acl_len,
				     ace->Header.AceFlags))
	  return;
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
