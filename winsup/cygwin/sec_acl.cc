/* sec_acl.cc: Solaris compatible ACL functions.

   Written by Corinna Vinschen <corinna@vinschen.de>

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <stdlib.h>
#include <ctype.h>
#include "cygerrno.h"
#include "security.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include "ntdll.h"
#include "tls_pbuf.h"
#include "sec_posixacl.h"

/* How does a correctly constructed new-style Windows ACL claiming to be a
   POSIX ACL look like?

   - NULL deny ACE (special bits, CLASS_OBJ).

   - USER_OBJ deny.  If the user has less permissions than the sum of CLASS_OBJ
     (or GROUP_OBJ if CLASS_OBJ doesn't exist) and OTHER_OBJ, deny the excess
     permissions so that group and other perms don't spill into the owner perms.

       USER_OBJ deny ACE   == ~USER_OBJ & (CLASS_OBJ | OTHER_OBJ)
     or
       USER_OBJ deny ACE   == ~USER_OBJ & (GROUP_OBJ | OTHER_OBJ)

   - USER deny.  If a user has more permissions than CLASS_OBJ, or if the
     user has less permissions than OTHER_OBJ, deny the excess permissions.

       USER deny ACE       == (USER & ~CLASS_OBJ) | (~USER & OTHER_OBJ)

   - USER_OBJ  allow ACE
   - USER      allow ACEs

   The POSIX permissions returned for a USER entry are the allow bits alone!

   - GROUP{_OBJ} deny.  If a group has more permissions than CLASS_OBJ,
     or less permissions than OTHER_OBJ, deny the excess permissions.

       GROUP{_OBJ} deny ACEs  == (GROUP & ~CLASS_OBJ)

   - GROUP_OBJ	allow ACE
   - GROUP	allow ACEs

   The POSIX permissions returned for a GROUP entry are the allow bits alone!

   - 2. GROUP{_OBJ} deny.  If a group has less permissions than OTHER_OBJ,
     deny the excess permissions.

       2. GROUP{_OBJ} deny ACEs  == (~GROUP & OTHER_OBJ)

   - OTHER_OBJ	allow ACE

   Rinse and repeat for default ACEs with INHERIT flags set.

   - Default NULL deny ACE (S_ISGID, CLASS_OBJ). */

						/* POSIX <-> Win32 */

/* Historically, these bits are stored in a NULL allow SID ACE.  To distinguish
   the new ACL style from the old one, we're using an access denied ACE, plus
   setting an as yet unused bit in the access mask.  The new ACEs can exist
   twice in an ACL, the "normal one" containing CLASS_OBJ and special bits
   and the one with INHERIT bit set to pass the DEF_CLASS_OBJ bits and the
   S_ISGID bit on. */
#define CYG_ACE_ISVTX		0x001		/* 0x200 <-> 0x001 */
#define CYG_ACE_ISGID		0x002		/* 0x400 <-> 0x002 */
#define CYG_ACE_ISUID		0x004		/* 0x800 <-> 0x004 */
#define CYG_ACE_ISBITS_TO_POSIX(val)	\
				(((val) & 0x007) << 9)
#define CYG_ACE_ISBITS_TO_WIN(val) \
				(((val) & (S_ISVTX | S_ISUID | S_ISGID)) >> 9)

#define CYG_ACE_MASK_X		0x008		/* 0x001 <-> 0x008 */
#define CYG_ACE_MASK_W		0x010		/* 0x002 <-> 0x010 */
#define CYG_ACE_MASK_R		0x020		/* 0x004 <-> 0x020 */
#define CYG_ACE_MASK_RWX	0x038
#define CYG_ACE_MASK_VALID	0x040		/* has mask if set */
#define CYG_ACE_MASK_TO_POSIX(val)	\
				(((val) & CYG_ACE_MASK_RWX) >> 3)
#define CYG_ACE_MASK_TO_WIN(val)	\
				((((val) & S_IRWXO) << 3) \
				 | CYG_ACE_MASK_VALID)
#define CYG_ACE_NEW_STYLE	READ_CONTROL	/* New style if set. */

/* Define own bit masks rather than using the GENERIC masks.  The latter
   also contain standard rights, which we don't need here. */
#define FILE_ALLOW_READ		(FILE_READ_DATA | FILE_READ_ATTRIBUTES | \
				 FILE_READ_EA)
#define FILE_DENY_READ		(FILE_READ_DATA | FILE_READ_EA)
#define FILE_ALLOW_WRITE	(FILE_WRITE_DATA | FILE_WRITE_ATTRIBUTES | \
				 FILE_WRITE_EA | FILE_APPEND_DATA)
#define FILE_DENY_WRITE		FILE_ALLOW_WRITE | FILE_DELETE_CHILD
#define FILE_DENY_WRITE_OWNER	(FILE_WRITE_DATA | FILE_WRITE_EA | \
				 FILE_APPEND_DATA | FILE_DELETE_CHILD)
#define FILE_ALLOW_EXEC		(FILE_EXECUTE)
#define FILE_DENY_EXEC		FILE_ALLOW_EXEC

#define STD_RIGHTS_OTHER	(STANDARD_RIGHTS_READ | SYNCHRONIZE)
#define STD_RIGHTS_OWNER	(STANDARD_RIGHTS_ALL | SYNCHRONIZE)

int
searchace (aclent_t *aclp, int nentries, int type, uid_t id)
{
  int i;

  for (i = 0; i < nentries; ++i)
    if ((aclp[i].a_type == type
	 && (id == ACL_UNDEFINED_ID || aclp[i].a_id == id))
	|| !aclp[i].a_type)
      return i;
  return -1;
}

/* From the attributes and the POSIX ACL list, compute a new-style Cygwin
   security descriptor.  The function returns a pointer to the
   SECURITY_DESCRIPTOR in sd_ret, or NULL if the function fails.

   This function *requires* a verified and sorted acl list! */
PSECURITY_DESCRIPTOR
set_posix_access (mode_t attr, uid_t uid, gid_t gid,
		  aclent_t *aclbufp, int nentries,
		  security_descriptor &sd_ret,
		  bool is_samba)
{
  SECURITY_DESCRIPTOR sd;
  cyg_ldap cldap;
  PSID owner = NULL, group = NULL;
  cygsid smb_owner, smb_group;
  NTSTATUS status;
  tmp_pathbuf tp;
  cygpsid *aclsid;
  PACL acl;
  size_t acl_len = sizeof (ACL);
  mode_t user_obj, group_obj, other_obj, deny;
  mode_t class_obj = 0;
  DWORD access;
  int idx, start_idx, tmp_idx;
  bool owner_eq_group = false;
  bool dev_has_admins = false;
  bool has_class_obj;

  /* Initialize local security descriptor. */
  RtlCreateSecurityDescriptor (&sd, SECURITY_DESCRIPTOR_REVISION);

  /* As in alloc_sd, set SE_DACL_PROTECTED to prevent the DACL from being
     modified by inheritable ACEs. */
  RtlSetControlSecurityDescriptor (&sd, SE_DACL_PROTECTED, SE_DACL_PROTECTED);

  /* Fetch owner and group and set in security descriptor.

     For Samba we check if there's an RFC2307 mapping in place, otherwise
     we're trying to create an ACL with the wrong Windows SIDs rather than
     the correct Unix SIDs.  Same happens below for mapping other USER and
     GROUP SIDs. */
  if (is_samba)
    {
      uint32_t smb_uid, smb_gid;

      smb_uid = cygheap->ugid_cache.reverse_get_uid (uid);
      if (smb_uid != ILLEGAL_UID)
	owner = smb_owner.create (22, 2, 1, smb_uid);
      smb_gid = cygheap->ugid_cache.reverse_get_gid (gid);
      if (smb_gid != ILLEGAL_GID)
	group = smb_group.create (22, 2, 2, smb_gid);
    }
  if (!owner)
    owner = sidfromuid (uid, &cldap);
  if (!group)
    group = sidfromgid (gid, &cldap);
  if (!owner || !group)
    {
      set_errno (EINVAL);
      return NULL;
    }
  status = RtlSetOwnerSecurityDescriptor (&sd, owner, FALSE);
  if (!NT_SUCCESS (status))
    {
      __seterrno_from_nt_status (status);
      return NULL;
    }
  status = RtlSetGroupSecurityDescriptor (&sd, group, FALSE);
  if (!NT_SUCCESS (status))
    {
      __seterrno_from_nt_status (status);
      return NULL;
    }
  owner_eq_group = RtlEqualSid (owner, group);
  if (S_ISCHR (attr))
    dev_has_admins = well_known_admins_sid == owner
		     || well_known_admins_sid == group;

  /* No POSIX ACL?  Use attr to generate one from scratch. */
  if (!aclbufp)
    {
      aclbufp = (aclent_t *) tp.c_get ();
      aclbufp[0].a_type = USER_OBJ;
      aclbufp[0].a_id = ACL_UNDEFINED_ID;
      aclbufp[0].a_perm = (attr >> 6) & S_IRWXO;
      aclbufp[1].a_type = GROUP_OBJ;
      aclbufp[1].a_id = ACL_UNDEFINED_ID;
      aclbufp[1].a_perm = (attr >> 3) & S_IRWXO;
      aclbufp[2].a_type = OTHER_OBJ;
      aclbufp[2].a_id = ACL_UNDEFINED_ID;
      aclbufp[2].a_perm = attr & S_IRWXO;
      nentries = MIN_ACL_ENTRIES;
      if (S_ISDIR (attr))
	{
	  aclbufp[3].a_type = DEF_USER_OBJ;
	  aclbufp[3].a_id = ACL_UNDEFINED_ID;
	  aclbufp[3].a_perm = (attr >> 6) & S_IRWXO;
	  aclbufp[4].a_type = GROUP_OBJ;
	  aclbufp[4].a_id = ACL_UNDEFINED_ID;
	  aclbufp[4].a_perm = (attr >> 3) & S_IRWXO;
	  aclbufp[5].a_type = OTHER_OBJ;
	  aclbufp[5].a_id = ACL_UNDEFINED_ID;
	  aclbufp[5].a_perm = attr & S_IRWXO;
	  nentries += MIN_ACL_ENTRIES;
	}
    }

  /* Collect SIDs of all entries in aclbufp. */
  aclsid = (cygpsid *) tp.w_get ();
  for (idx = 0; idx < nentries; ++idx)
    switch (aclbufp[idx].a_type)
      {
      case USER_OBJ:
	aclsid[idx] = owner;
	break;
      case DEF_USER_OBJ:
	aclsid[idx] = well_known_creator_owner_sid;
	break;
      case USER:
      case DEF_USER:
	aclsid[idx] = NO_SID;
	if (is_samba)
	  {
	    uint32_t smb_uid;
	    cygsid *smb_sid;

	    smb_uid = cygheap->ugid_cache.reverse_get_uid (aclbufp[idx].a_id);
	    if (smb_uid != ILLEGAL_UID)
	      {
		smb_sid = (cygsid *) alloca (sizeof (cygsid));
		aclsid[idx] = smb_sid->create (22, 2, 1, smb_uid);
	      }
	  }
	if (!aclsid[idx])
	  aclsid[idx] = sidfromuid (aclbufp[idx].a_id, &cldap);
	break;
      case GROUP_OBJ:
	aclsid[idx] = group;
	break;
      case DEF_GROUP_OBJ:
	aclsid[idx] = !(attr & S_ISGID) ? (PSID) well_known_creator_group_sid
					: group;
	break;
      case GROUP:
      case DEF_GROUP:
	aclsid[idx] = NO_SID;
	if (is_samba)
	  {
	    uint32_t smb_gid;
	    cygsid *smb_sid;

	    smb_gid = cygheap->ugid_cache.reverse_get_gid (aclbufp[idx].a_id);
	    if (smb_gid != ILLEGAL_GID)
	      {
		smb_sid = (cygsid *) alloca (sizeof (cygsid));
		aclsid[idx] = smb_sid->create (22, 2, 2, smb_gid);
	      }
	  }
	if (!aclsid[idx])
	  aclsid[idx] = sidfromgid (aclbufp[idx].a_id, &cldap);
	break;
      case CLASS_OBJ:
      case DEF_CLASS_OBJ:
	aclsid[idx] = well_known_null_sid;
	break;
      case OTHER_OBJ:
      case DEF_OTHER_OBJ:
	aclsid[idx] = well_known_world_sid;
	break;
      }

  /* Initialize ACL. */
  acl = (PACL) tp.w_get ();
  RtlCreateAcl (acl, ACL_MAXIMUM_SIZE, ACL_REVISION);

  /* This loop has two runs, the first handling the actual permission,
     the second handling the default permissions. */
  idx = 0;
  for (int def = 0; def <= ACL_DEFAULT; def += ACL_DEFAULT)
    {
      DWORD inherit = def ? SUB_CONTAINERS_AND_OBJECTS_INHERIT | INHERIT_ONLY
			  : NO_INHERITANCE;

      /* No default ACEs on files. */
      if (def && !S_ISDIR (attr))
	{
	  /* Trying to set default ACEs on a non-directory is an error.
	     The underlying functions on Linux return EACCES. */
	  if (idx < nentries && aclbufp[idx].a_type & ACL_DEFAULT)
	    {
	      set_errno (EACCES);
	      return NULL;
	    }
	  break;
	}

      /* To check if the NULL SID deny ACE is required we need user_obj.  */
      tmp_idx = searchace (aclbufp, nentries, def | USER_OBJ);
      /* No default entries present? */
      if (tmp_idx < 0)
	break;
      user_obj = aclbufp[tmp_idx].a_perm;
      /* To compute deny access masks, we need group_obj, other_obj and... */
      tmp_idx = searchace (aclbufp, nentries, def | GROUP_OBJ);
      group_obj = aclbufp[tmp_idx].a_perm;
      tmp_idx = searchace (aclbufp, nentries, def | OTHER_OBJ);
      other_obj = aclbufp[tmp_idx].a_perm;

      /* ... class_obj.  Create NULL deny ACE.  Only the S_ISGID attribute gets
	 inherited.  For directories check if we are also going to generate
	 default entries.  If not we have a problem.  We can't generate only a
	 single, inheritable NULL SID ACE because that leads to (fixable, TODO)
	 access problems when trying to create the matching child permissions.
	 Therefore we remove the S_ISGID bit on the directory because having it
	 set would be misleading. */
      if (!def && S_ISDIR (attr) && (attr & S_ISGID))
	{
	  /* Check for a required entry per POSIX. */
	  tmp_idx = searchace (aclbufp, nentries, DEF_USER_OBJ);
	  if (tmp_idx < 0)
	    attr &= ~S_ISGID;
	}
      access = CYG_ACE_ISBITS_TO_WIN (def ? attr & S_ISGID : attr)
	       | CYG_ACE_NEW_STYLE;
      tmp_idx = searchace (aclbufp, nentries, def | CLASS_OBJ);
      if (tmp_idx >= 0)
	{
	  has_class_obj = true;
	  class_obj = aclbufp[tmp_idx].a_perm;
	  access |= CYG_ACE_MASK_TO_WIN (class_obj);
	}
      else
	{
	  /* Setting class_obj to group_obj allows to write below code without
	     additional checks for existence of a CLASS_OBJ. */
	  has_class_obj = false;
	  class_obj = group_obj;
	}
      /* Note that Windows filters the ACE Mask value so it only reflects
	 the bit values supported by the object type.  The result is that
	 we can't set a CLASS_OBJ value for ptys.  The get_posix_access
	 function has to workaround that.

	 We also don't write the NULL SID ACE in case we have a simple POSIX
	 permission ACL with the user perms >= group perms >= other perms and
	 no special bits set.  In all other cases we either need the NULL SID
	 ACE or we write it to avoid calls to AuthZ from get_posix_access. */
      if (!S_ISCHR (attr)
	  && (has_class_obj
	      || access != CYG_ACE_NEW_STYLE
	      || ((user_obj | group_obj | other_obj) != user_obj
		  || (group_obj | other_obj) != group_obj))
	  && !add_access_denied_ace (acl, access, well_known_null_sid, acl_len,
				     inherit))
	return NULL;

      /* Do we potentially chmod a file with owner SID == group SID?  If so,
	 make sure the owner perms are always >= group perms. */
      if (!def && owner_eq_group)
	  aclbufp[0].a_perm |= group_obj & class_obj;

      /* This loop has two runs, the first w/ check_types == (USER_OBJ | USER),
	 the second w/ check_types == (GROUP_OBJ | GROUP).  Each run creates
	 first the deny, then the allow ACEs for the current types. */
      for (int check_types = USER_OBJ | USER;
	   check_types < CLASS_OBJ;
	   check_types <<= 2)
	{
	  /* Create deny ACEs for users, then 1st run for groups.  For groups,
	     only take CLASS_OBJ permissions into account.  Class permissions
	     are handled in the 2nd deny loop below. */
	  for (start_idx = idx;
	       idx < nentries && aclbufp[idx].a_type & check_types;
	       ++idx)
	    {
	      /* Avoid creating DENY ACEs for the second occurrence of
		 accounts which show up twice, as USER_OBJ and USER, or
		 GROUP_OBJ and GROUP. */
	      if ((aclbufp[idx].a_type & USER && aclsid[idx] == owner)
		  || (aclbufp[idx].a_type & GROUP && aclsid[idx] == group))
		continue;
	      /* For the rules how to construct the deny access mask, see the
		 comment right at the start of this file. */
	      if (aclbufp[idx].a_type & USER_OBJ)
		deny = ~aclbufp[idx].a_perm & (class_obj | other_obj);
	      else if (aclbufp[idx].a_type & USER)
		deny = (aclbufp[idx].a_perm & ~class_obj)
		       | (~aclbufp[idx].a_perm & other_obj);
	      /* Accommodate Windows: Only generate deny masks for SYSTEM
		 and the Administrators group in terms of the execute bit,
		 if they are not the primary group. */
	      else if (aclbufp[idx].a_type & GROUP
		       && (aclsid[idx] == well_known_system_sid
			   || aclsid[idx] == well_known_admins_sid))
		deny = aclbufp[idx].a_perm & ~(class_obj | S_IROTH | S_IWOTH);
	      else
		deny = (aclbufp[idx].a_perm & ~class_obj);
	      if (!deny)
		continue;
	      access = 0;
	      if (deny & S_IROTH)
		access |= FILE_DENY_READ;
	      if (deny & S_IWOTH)
		access |= (aclbufp[idx].a_type & USER_OBJ)
			  ? FILE_DENY_WRITE_OWNER : FILE_DENY_WRITE;
	      if (deny & S_IXOTH)
		access |= FILE_DENY_EXEC;
	      if (!add_access_denied_ace (acl, access, aclsid[idx], acl_len,
					  inherit))
		return NULL;
	    }
	  /* Create allow ACEs for users, then groups. */
	  for (idx = start_idx;
	       idx < nentries && aclbufp[idx].a_type & check_types;
	       ++idx)
	    {
	      /* Don't set FILE_READ/WRITE_ATTRIBUTES unconditionally on Samba,
		 otherwise it enforces read permissions. */
	      access = STD_RIGHTS_OTHER | (is_samba ? 0 : FILE_READ_ATTRIBUTES);
	      if (aclbufp[idx].a_type & USER_OBJ)
		{
		  access |= STD_RIGHTS_OWNER;
		  if (!is_samba)
		    access |= FILE_WRITE_ATTRIBUTES;
		  /* Set FILE_DELETE_CHILD on files with "rwx" perms for the
		     owner so that the owner gets "full control" (Duh). */
		  if (aclbufp[idx].a_perm == S_IRWXO)
		    access |= FILE_DELETE_CHILD;
		}
	      if (aclbufp[idx].a_perm & S_IROTH)
		access |= FILE_ALLOW_READ;
	      if (aclbufp[idx].a_perm & S_IWOTH)
		access |= FILE_ALLOW_WRITE;
	      if (aclbufp[idx].a_perm & S_IXOTH)
		access |= FILE_ALLOW_EXEC;
	      /* Handle S_ISVTX. */
	      if (S_ISDIR (attr)
		  && (aclbufp[idx].a_perm & (S_IWOTH | S_IXOTH))
		     == (S_IWOTH | S_IXOTH)
		  && (!(attr & S_ISVTX) || aclbufp[idx].a_type & USER_OBJ))
		access |= FILE_DELETE_CHILD;
	      /* For ptys, make sure the Administrators group has WRITE_DAC
		 and WRITE_OWNER perms. */
	      if (dev_has_admins && aclsid[idx] == well_known_admins_sid)
		access |= STD_RIGHTS_OWNER;
	      if (!add_access_allowed_ace (acl, access, aclsid[idx], acl_len,
					   inherit))
		return NULL;
	    }
	  /* 2nd deny loop: Create deny ACEs for groups when they have less
	     permissions than OTHER_OBJ. */
	  if (check_types == (GROUP_OBJ | GROUP))
	    for (idx = start_idx;
		 idx < nentries && aclbufp[idx].a_type & check_types;
		 ++idx)
	      {
		if (aclbufp[idx].a_type & GROUP && aclsid[idx] == group)
		  continue;
		/* Only generate deny masks for SYSTEM and the Administrators
		   group if they are the primary group. */
		if (aclbufp[idx].a_type & GROUP
		    && (aclsid[idx] == well_known_system_sid
			|| aclsid[idx] == well_known_admins_sid))
		  deny = 0;
		else
		  deny = (~aclbufp[idx].a_perm & other_obj);
		if (!deny)
		  continue;
		access = 0;
		if (deny & S_IROTH)
		  access |= FILE_DENY_READ;
		if (deny & S_IWOTH)
		  access |= FILE_DENY_WRITE;
		if (deny & S_IXOTH)
		  access |= FILE_DENY_EXEC;
		if (!add_access_denied_ace (acl, access, aclsid[idx], acl_len,
					    inherit))
		  return NULL;
	      }
	}
      /* For ptys if the admins group isn't in the ACL, add an ACE to make
	 sure the admins group has WRITE_DAC and WRITE_OWNER perms. */
      if (S_ISCHR (attr) && !dev_has_admins
	  && !add_access_allowed_ace (acl,
				      STD_RIGHTS_OWNER | FILE_ALLOW_READ
				      | FILE_ALLOW_WRITE,
				      well_known_admins_sid, acl_len,
				      NO_INHERITANCE))
	return NULL;
      /* Create allow ACE for other.  It's preceeded by class_obj if it exists.
	 If so, skip it. */
      if (aclbufp[idx].a_type & CLASS_OBJ)
	++idx;
      access = STD_RIGHTS_OTHER | (is_samba ? 0 : FILE_READ_ATTRIBUTES);
      if (aclbufp[idx].a_perm & S_IROTH)
	access |= FILE_ALLOW_READ;
      if (aclbufp[idx].a_perm & S_IWOTH)
	access |= FILE_ALLOW_WRITE;
      if (aclbufp[idx].a_perm & S_IXOTH)
	access |= FILE_ALLOW_EXEC;
      /* Handle S_ISVTX. */
      if (S_ISDIR (attr)
	  && (aclbufp[idx].a_perm & (S_IWOTH | S_IXOTH)) == (S_IWOTH | S_IXOTH)
	  && !(attr & S_ISVTX))
	access |= FILE_DELETE_CHILD;
      if (!add_access_allowed_ace (acl, access, aclsid[idx++], acl_len,
				   inherit))
	return NULL;
    }

  /* Set AclSize to computed value. */
  acl->AclSize = acl_len;
  debug_printf ("ACL-Size: %u", acl_len);
  /* Create DACL for local security descriptor. */
  status = RtlSetDaclSecurityDescriptor (&sd, TRUE, acl, FALSE);
  if (!NT_SUCCESS (status))
    {
      __seterrno_from_nt_status (status);
      return NULL;
    }
  /* Make self relative security descriptor in sd_ret. */
  DWORD sd_size = 0;
  RtlAbsoluteToSelfRelativeSD (&sd, sd_ret, &sd_size);
  if (sd_size <= 0)
    {
      __seterrno ();
      return NULL;
    }
  if (!sd_ret.realloc (sd_size))
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

/* This function *requires* a verified and sorted acl list! */
int
setacl (HANDLE handle, path_conv &pc, int nentries, aclent_t *aclbufp,
	bool &writable)
{
  security_descriptor sd, sd_ret;
  mode_t attr = pc.isdir () ? S_IFDIR : 0;
  uid_t uid;
  gid_t gid;

  if (get_file_sd (handle, pc, sd, false))
    return -1;
  if (get_posix_access (sd, &attr, &uid, &gid, NULL, 0) < 0)
    return -1;
  if (!set_posix_access (attr, uid, gid, aclbufp, nentries,
			 sd_ret, pc.fs_is_samba ()))
    return -1;
  /* FIXME?  Caller needs to know if any write perms are set to allow removing
     the DOS R/O bit. */
  writable = true;
  return set_file_sd (handle, pc, sd_ret, false);
}

/* Temporary access denied bits used by getace and get_posix_access during
   Windows ACL processing.  These bits get removed before the created POSIX
   ACL gets published. */
#define DENY_R 040000
#define DENY_W 020000
#define DENY_X 010000
#define DENY_RWX (DENY_R | DENY_W | DENY_X)

/* New style ACL means, just read the bits and store them away.  Don't
   create masked values on your own. */
static void
getace (aclent_t &acl, int type, int id, DWORD win_ace_mask,
	DWORD win_ace_type, bool new_style)
{
  acl.a_type = type;
  acl.a_id = id;

  if ((win_ace_mask & FILE_READ_BITS)
      && (new_style || !(acl.a_perm & (S_IROTH | DENY_R))))
    {
      if (win_ace_type == ACCESS_ALLOWED_ACE_TYPE)
	acl.a_perm |= S_IROTH;
      else if (win_ace_type == ACCESS_DENIED_ACE_TYPE)
	acl.a_perm |= DENY_R;
    }

  if ((win_ace_mask & FILE_WRITE_BITS)
      && (new_style || !(acl.a_perm & (S_IWOTH | DENY_W))))
    {
      if (win_ace_type == ACCESS_ALLOWED_ACE_TYPE)
	acl.a_perm |= S_IWOTH;
      else if (win_ace_type == ACCESS_DENIED_ACE_TYPE)
	acl.a_perm |= DENY_W;
    }

  if ((win_ace_mask & FILE_EXEC_BITS)
      && (new_style || !(acl.a_perm & (S_IXOTH | DENY_X))))
    {
      if (win_ace_type == ACCESS_ALLOWED_ACE_TYPE)
	acl.a_perm |= S_IXOTH;
      else if (win_ace_type == ACCESS_DENIED_ACE_TYPE)
	acl.a_perm |= DENY_X;
    }
}

/* From the SECURITY_DESCRIPTOR given in psd, compute user, owner, posix
   attributes, as well as the POSIX acl.  The function returns the number
   of entries returned in aclbufp, or -1 in case of error.

   When called from chmod, it also returns the fact if the ACL is a "standard"
   ACL.  A "standard" ACL is an ACL which only consists of ACEs for owner,
   group, other, as well as (this is Windows) the Administrators group and
   SYSTEM.  See fhandler_disk_file::fchmod for how this is used to fake
   stock POSIX perms even if Administrators and SYSTEM is in the ACE. */
int
get_posix_access (PSECURITY_DESCRIPTOR psd,
		  mode_t *attr_ret, uid_t *uid_ret, gid_t *gid_ret,
		  aclent_t *aclbufp, int nentries, bool *std_acl)
{
  tmp_pathbuf tp;
  NTSTATUS status;
  BOOLEAN dummy, acl_exists;
  SECURITY_DESCRIPTOR_CONTROL ctrl;
  ULONG rev;
  PACL acl;
  PACCESS_ALLOWED_ACE ace;
  cygpsid owner_sid, group_sid;
  cyg_ldap cldap;
  uid_t uid;
  gid_t gid;
  mode_t attr = 0;
  aclent_t *lacl = NULL;
  cygpsid ace_sid, *aclsid;
  int pos, type, id, idx;

  bool owner_eq_group;
  bool just_created = false;
  bool standard_ACEs_only = true;
  bool new_style = false;
  bool saw_user_obj = false;
  bool saw_group_obj = false;
  bool saw_other_obj = false;
  bool saw_def_user_obj = false;
  bool saw_def_group_obj = false;
  bool has_class_perm = false;
  bool has_def_class_perm = false;

  mode_t class_perm = 0;
  mode_t def_class_perm = 0;
  int types_def = 0;
  int def_pgrp_pos = -1;

  if (aclbufp && nentries < MIN_ACL_ENTRIES)
    {
      set_errno (EINVAL);
      return -1;
    }
  /* If reading the security descriptor failed, treat the object as
     unreadable. */
  if (!psd)
    {
      if (attr_ret)
        *attr_ret &= S_IFMT;
      if (uid_ret)
        *uid_ret = ACL_UNDEFINED_ID;
      if (gid_ret)
        *gid_ret = ACL_UNDEFINED_ID;
      if (aclbufp)
	{
	  aclbufp[0].a_type = USER_OBJ;
	  aclbufp[0].a_id = ACL_UNDEFINED_ID;
	  aclbufp[0].a_perm = 0;
	  aclbufp[1].a_type = GROUP_OBJ;
	  aclbufp[1].a_id = ACL_UNDEFINED_ID;
	  aclbufp[1].a_perm = 0;
	  aclbufp[2].a_type = OTHER_OBJ;
	  aclbufp[2].a_id = ACL_UNDEFINED_ID;
	  aclbufp[2].a_perm = 0;
	  return MIN_ACL_ENTRIES;
	}
      return 0;
    }
  /* Fetch owner, group, and ACL from security descriptor. */
  status = RtlGetOwnerSecurityDescriptor (psd, (PSID *) &owner_sid, &dummy);
  if (!NT_SUCCESS (status))
    {
      __seterrno_from_nt_status (status);
      return -1;
    }
  status = RtlGetGroupSecurityDescriptor (psd, (PSID *) &group_sid, &dummy);
  if (!NT_SUCCESS (status))
    {
      __seterrno_from_nt_status (status);
      return -1;
    }
  status = RtlGetDaclSecurityDescriptor (psd, &acl_exists, &acl, &dummy);
  if (!NT_SUCCESS (status))
    {
      __seterrno_from_nt_status (status);
      return -1;
    }
  /* Set uidret, gidret, and initalize attributes. */
  uid = owner_sid.get_uid (&cldap);
  gid = group_sid.get_gid (&cldap);
  if (attr_ret)
    {
      attr = *attr_ret & S_IFMT;
      just_created = *attr_ret & S_JUSTCREATED;
    }
  /* Remember the fact that owner and group are the same account. */
  owner_eq_group = owner_sid == group_sid;

  /* Create and initialize local aclent_t array. */
  lacl = (aclent_t *) tp.c_get ();
  memset (lacl, 0, MAX_ACL_ENTRIES * sizeof (aclent_t *));
  lacl[0].a_type = USER_OBJ;
  lacl[0].a_id = uid;
  lacl[1].a_type = GROUP_OBJ;
  lacl[1].a_id = gid;
  lacl[2].a_type = OTHER_OBJ;
  lacl[2].a_id = ACL_UNDEFINED_ID;
  /* Create array to collect SIDs of all entries in lacl. */
  aclsid = (cygpsid *) tp.w_get ();
  aclsid[0] = owner_sid;
  aclsid[1] = group_sid;
  aclsid[2] = well_known_world_sid;

  /* No ACEs?  Everybody has full access. */
  if (!acl_exists || !acl || acl->AceCount == 0)
    {
      for (pos = 0; pos < MIN_ACL_ENTRIES; ++pos)
	lacl[pos].a_perm = S_IROTH | S_IWOTH | S_IXOTH;
      goto out;
    }

  /* Files and dirs are created with a NULL descriptor, so inheritence
     rules kick in.  If no inheritable entries exist in the parent object,
     Windows will create entries according to the user token's default DACL.
     These entries are not desired and we ignore them at creation time.
     We're just checking the SE_DACL_AUTO_INHERITED flag here, since that's
     what we set in get_file_sd.  Read the longish comment there before
     changing this test! */
  if (just_created
      && NT_SUCCESS (RtlGetControlSecurityDescriptor (psd, &ctrl, &rev))
      && !(ctrl & SE_DACL_AUTO_INHERITED))
    ;
  else for (idx = 0; idx < acl->AceCount; ++idx)
    {
      if (!NT_SUCCESS (RtlGetAce (acl, idx, (PVOID *) &ace)))
	continue;

      ace_sid = (PSID) &ace->SidStart;

      if (ace_sid == well_known_null_sid)
	{
	  /* Fetch special bits. */
	  attr |= CYG_ACE_ISBITS_TO_POSIX (ace->Mask);
	  if (ace->Header.AceType == ACCESS_DENIED_ACE_TYPE
	      && ace->Mask & CYG_ACE_NEW_STYLE)
	    {
	      /* New-style ACL.  Note the fact that a mask value is present
		 since that changes how getace fetches the information.  That's
		 fine, because the NULL deny ACE is supposed to precede all
		 USER, GROUP and GROUP_OBJ entries.  Any ACL not created that
		 way has been rearranged by the Windows functionality to create
		 the brain-dead "canonical" ACL order and is broken anyway. */
	      new_style = true;
	      attr |= CYG_ACE_ISBITS_TO_POSIX (ace->Mask);
	      if (ace->Mask & CYG_ACE_MASK_VALID)
		{
		  if (!(ace->Header.AceFlags & INHERIT_ONLY))
		    {
		      if ((pos = searchace (lacl, MAX_ACL_ENTRIES, CLASS_OBJ))
			  >= 0)
			{
			  lacl[pos].a_type = CLASS_OBJ;
			  lacl[pos].a_id = ACL_UNDEFINED_ID;
			  lacl[pos].a_perm = CYG_ACE_MASK_TO_POSIX (ace->Mask);
			  aclsid[pos] = well_known_null_sid;
			  has_class_perm = true;
			  class_perm = lacl[pos].a_perm;
			}
		    }
		  if (ace->Header.AceFlags & SUB_CONTAINERS_AND_OBJECTS_INHERIT)
		    {
		      if ((pos = searchace (lacl, MAX_ACL_ENTRIES,
					    DEF_CLASS_OBJ)) >= 0)
			{
			  lacl[pos].a_type = DEF_CLASS_OBJ;
			  lacl[pos].a_id = ACL_UNDEFINED_ID;
			  lacl[pos].a_perm = CYG_ACE_MASK_TO_POSIX (ace->Mask);
			  aclsid[pos] = well_known_null_sid;
			  has_def_class_perm = true;
			  def_class_perm = lacl[pos].a_perm;
			}
		    }
		}
	    }
	  continue;
	}
      if (ace_sid == owner_sid)
	{
	  type = USER_OBJ;
	  id = uid;
	}
      else if (ace_sid == group_sid)
	{
	  type = GROUP_OBJ;
	  id = gid;
	}
      else if (ace_sid == well_known_world_sid)
	{
	  type = OTHER_OBJ;
	  id = ACL_UNDEFINED_ID;
	  if (ace->Header.AceType == ACCESS_ALLOWED_ACE_TYPE
	      && !(ace->Header.AceFlags & INHERIT_ONLY))
	    saw_other_obj = true;
	}
      else if (ace_sid == well_known_creator_owner_sid)
	{
	  type = DEF_USER_OBJ;
	  types_def |= type;
	  id = ACL_UNDEFINED_ID;
	  saw_def_user_obj = true;
	}
      else if (ace_sid == well_known_creator_group_sid)
	{
	  type = DEF_GROUP_OBJ;
	  types_def |= type;
	  id = ACL_UNDEFINED_ID;
	  saw_def_group_obj = true;
	}
      else
	{
	  id = ace_sid.get_id (TRUE, &type, &cldap);
	  if (!type)
	    continue;
	}
      /* If the SGID attribute is set on a just created file or dir, the
         first group in the ACL is the desired primary group of the new
	 object.  Alternatively, the first repetition of the owner SID is
	 the desired primary group, and we mark the object as owner_eq_group
	 object. */
      if (just_created && attr & S_ISGID && !saw_group_obj
	  && (type == GROUP || (type == USER_OBJ && saw_user_obj)))
	{
	  type = GROUP_OBJ;
	  lacl[1].a_id = gid = id;
	  if (type == USER_OBJ)
	    owner_eq_group = true;
	}
      if (!(ace->Header.AceFlags & INHERIT_ONLY || type & ACL_DEFAULT))
	{
	  if (type == USER_OBJ)
	    {
	      /* If we get a second entry for the owner SID, it's either a
		 GROUP_OBJ entry for the same SID, if owner SID == group SID,
		 or it's an additional USER entry.  The latter can happen
		 when chown'ing a file. */
	      if (saw_user_obj)
		{
		  if (owner_eq_group && !saw_group_obj)
		    {
		      type = GROUP_OBJ;
		      /* Gid and uid are not necessarily the same even if the
			 SID is the same: /etc/group is in use and the user got
			 added to /etc/group using another gid than the uid.
			 This is a border case but it happened and resetting id
			 to gid is not much of a burden. */
		      id = gid;
		      if (ace->Header.AceType == ACCESS_ALLOWED_ACE_TYPE)
			saw_group_obj = true;
		    }
		  else
		    type = USER;
		}
	      else if (ace->Header.AceType == ACCESS_ALLOWED_ACE_TYPE)
		saw_user_obj = true;
	    }
	  else if (type == GROUP_OBJ)
	    {
	      /* Same for the primary group. */
	      if (ace->Header.AceType == ACCESS_ALLOWED_ACE_TYPE)
		{
		  if (saw_group_obj)
		    type = GROUP;
		  saw_group_obj = true;
		}
	    }
	  if ((pos = searchace (lacl, MAX_ACL_ENTRIES, type, id)) >= 0)
	    {
	      getace (lacl[pos], type, id, ace->Mask, ace->Header.AceType,
		      new_style && type & (USER | GROUP_OBJ | GROUP));
	      aclsid[pos] = ace_sid;
	      if (!new_style)
		{
		  /* Fix up CLASS_OBJ value. */
		  if (type & (USER | GROUP))
		    {
		      has_class_perm = true;
		      /* Accommodate Windows: Never add SYSTEM and Admins to
			 CLASS_OBJ.  Unless (implicitly) if they are the
			 GROUP_OBJ entry. */
		      if (ace_sid != well_known_system_sid
			  && ace_sid != well_known_admins_sid)
			{
			  class_perm |= lacl[pos].a_perm;
			  standard_ACEs_only = false;
			}
		    }
		}
	      /* For a newly created file, we'd like to know if we're running
		 with a standard ACL, one only consisting of POSIX perms, plus
		 SYSTEM and Admins as maximum non-POSIX perms entries.  If it's
		 a standard ACL, we apply umask.  That's not entirely correct,
		 but it's probably the best we can do.  Chmod also wants to
		 know this.  See there for the details. */
	      else if (type & (USER | GROUP)
		       && standard_ACEs_only
		       && ace_sid != well_known_system_sid
		       && ace_sid != well_known_admins_sid)
		standard_ACEs_only = false;
	    }
	}
      if ((ace->Header.AceFlags & SUB_CONTAINERS_AND_OBJECTS_INHERIT))
	{
	  if (type == USER_OBJ)
	    {
	      /* As above: If we get a second entry for the owner SID, it's
		 a GROUP_OBJ entry for the same SID if owner SID == group SID,
		 but this time only if the S_ISGID bit is set. Otherwise it's
		 an additional USER entry. */
	      if (saw_def_user_obj)
		{
		  if (owner_eq_group && !saw_def_group_obj && attr & S_ISGID)
		    {
		      /* Needs post-processing in the following GROUP_OBJ block.
		         Set id to ACL_UNDEFINED_ID to play it safe. */
		      type = GROUP_OBJ;
		      id = ACL_UNDEFINED_ID;
		    }
		  else
		    type = USER;
		}
	      else if (ace->Header.AceType == ACCESS_ALLOWED_ACE_TYPE)
		saw_def_user_obj = true;
	    }
	  if (type == GROUP_OBJ)
	    {
	      /* If the SGID bit is set, the inheritable entry for the
		 primary group is, in fact, the DEF_GROUP_OBJ entry,
		 so don't change the type to GROUP in this case. */
	      if (!new_style || saw_def_group_obj || !(attr & S_ISGID))
		type = GROUP;
	      else if (ace->Header.AceType == ACCESS_ALLOWED_ACE_TYPE)
		saw_def_group_obj = true;
	    }
	  type |= ACL_DEFAULT;
	  types_def |= type;
	  if ((pos = searchace (lacl, MAX_ACL_ENTRIES, type, id)) >= 0)
	    {
	      getace (lacl[pos], type, id, ace->Mask, ace->Header.AceType,
		      new_style && type & (USER | GROUP_OBJ | GROUP));
	      aclsid[pos] = ace_sid;
	      if (!new_style)
		{
		  /* Fix up DEF_CLASS_OBJ value. */
		  if (type & (USER | GROUP))
		    {
		      has_def_class_perm = true;
		      /* Accommodate Windows: Never add SYSTEM and Admins to
			 CLASS_OBJ.  Unless (implicitly) if they are the
			 GROUP_OBJ entry. */
		      if (ace_sid != well_known_system_sid
			  && ace_sid != well_known_admins_sid)
		      def_class_perm |= lacl[pos].a_perm;
		    }
		  /* And note the position of the DEF_GROUP_OBJ entry. */
		  if (type == DEF_GROUP_OBJ)
		    def_pgrp_pos = pos;
		}
	    }
	}
    }
  /* If this is a just created file, and this is an ACL with only standard
     entries, or if standard POSIX permissions are missing (probably no
     inherited ACEs so created from a default DACL), assign the permissions
     specified by the file creation mask.  The values get masked by the
     actually requested permissions by the caller per POSIX 1003.1e draft 17. */
  if (just_created)
    {
      mode_t perms = (S_IRWXU | S_IRWXG | S_IRWXO) & ~cygheap->umask;
      if (standard_ACEs_only || !saw_user_obj)
	lacl[0].a_perm = (perms >> 6) & S_IRWXO;
      if (standard_ACEs_only || !saw_group_obj)
	lacl[1].a_perm = (perms >> 3) & S_IRWXO;
      if (standard_ACEs_only || !saw_other_obj)
	lacl[2].a_perm = perms & S_IRWXO;
    }
  /* If this is an old-style or non-Cygwin ACL, and secondary user and group
     entries exist in the ACL, fake a matching CLASS_OBJ entry. The CLASS_OBJ
     permissions are the or'ed permissions of the primary group permissions
     and all secondary user and group permissions. */
  if (!new_style && has_class_perm
      && (pos = searchace (lacl, MAX_ACL_ENTRIES, 0)) >= 0)
    {
      lacl[pos].a_type = CLASS_OBJ;
      lacl[pos].a_id = ACL_UNDEFINED_ID;
      class_perm |= lacl[1].a_perm;
      lacl[pos].a_perm = class_perm;
      aclsid[pos] = well_known_null_sid;
    }
  /* For ptys, fake a mask if the admins group is neither owner nor group.
     In that case we have an extra ACE for the admins group, and we need a
     CLASS_OBJ to get a valid POSIX ACL.  However, Windows filters the ACE
     Mask value so it only reflects the bit values supported by the object
     type.  The result is that we can't set an explicit CLASS_OBJ value for
     ptys in the NULL SID ACE. */
  else if (S_ISCHR (attr) && owner_sid != well_known_admins_sid
	   && group_sid != well_known_admins_sid
	   && (pos = searchace (lacl, MAX_ACL_ENTRIES, CLASS_OBJ)) >= 0)
    {
      lacl[pos].a_type = CLASS_OBJ;
      lacl[pos].a_id = ACL_UNDEFINED_ID;
      lacl[pos].a_perm = lacl[1].a_perm; /* == group perms */
      aclsid[pos] = well_known_null_sid;
    }
  /* Ensure that the default acl contains at least
     DEF_(USER|GROUP|OTHER)_OBJ entries.  */
  if (types_def && (pos = searchace (lacl, MAX_ACL_ENTRIES, 0)) >= 0)
    {
      if (!(types_def & USER_OBJ))
	{
	  lacl[pos].a_type = DEF_USER_OBJ;
	  lacl[pos].a_id = uid;
	  lacl[pos].a_perm = lacl[0].a_perm;
	  aclsid[pos] = well_known_creator_owner_sid;
	  pos++;
	}
      if (!(types_def & GROUP_OBJ) && pos < MAX_ACL_ENTRIES)
	{
	  lacl[pos].a_type = DEF_GROUP_OBJ;
	  lacl[pos].a_id = gid;
	  lacl[pos].a_perm = lacl[1].a_perm;
	  /* Note the position of the DEF_GROUP_OBJ entry. */
	  def_pgrp_pos = pos;
	  aclsid[pos] = well_known_creator_group_sid;
	  pos++;
	}
      if (!(types_def & OTHER_OBJ) && pos < MAX_ACL_ENTRIES)
	{
	  lacl[pos].a_type = DEF_OTHER_OBJ;
	  lacl[pos].a_id = ACL_UNDEFINED_ID;
	  lacl[pos].a_perm = lacl[2].a_perm;
	  aclsid[pos] = well_known_world_sid;
	  pos++;
	}
    }
  /* If this is an old-style or non-Cygwin ACL, and secondary user default
     and group default entries exist in the ACL, fake a matching DEF_CLASS_OBJ
     entry. The DEF_CLASS_OBJ permissions are the or'ed permissions of the
     primary group default permissions and all secondary user and group def.
     permissions. */
  if (!new_style && has_def_class_perm
      && (pos = searchace (lacl, MAX_ACL_ENTRIES, 0)) >= 0)
    {
      lacl[pos].a_type = DEF_CLASS_OBJ;
      lacl[pos].a_id = ACL_UNDEFINED_ID;
      lacl[pos].a_perm = def_class_perm;
      if (def_pgrp_pos >= 0)
	lacl[pos].a_perm |= lacl[def_pgrp_pos].a_perm;
      aclsid[pos] = well_known_null_sid;
    }

  /* Make sure `pos' contains the number of used entries in lacl. */
  if ((pos = searchace (lacl, MAX_ACL_ENTRIES, 0)) < 0)
    pos = MAX_ACL_ENTRIES;

  /* For old-style or non-Cygwin ACLs, check for merging permissions. */
  if (!new_style)
    for (idx = 0; idx < pos; ++idx)
      {
	if (lacl[idx].a_type & (USER_OBJ | USER)
	    && !(lacl[idx].a_type & ACL_DEFAULT))
	  {
	    mode_t perm;

	    /* Don't merge if the user already has all permissions, or... */
	    if (lacl[idx].a_perm == S_IRWXO)
	      continue;
	    /* ...if the sum of perms is less than or equal the user's perms. */
	    perm = lacl[idx].a_perm
		   | (has_class_perm ? class_perm : lacl[1].a_perm)
		   | lacl[2].a_perm;
	    if (perm == lacl[idx].a_perm)
	      continue;
	    /* Otherwise, if we use the Windows user DB, utilize Authz to make
	       sure all user permissions are correctly reflecting the Windows
	       permissions. */
	    if (cygheap->pg.nss_pwd_db ()
		&& authz_get_user_attribute (&perm, psd, aclsid[idx]))
	      lacl[idx].a_perm = perm;
	    /* Otherwise we only check the current user.  If the user entry
	       has a deny ACE, don't check. */
	    else if (lacl[idx].a_id == myself->uid
		     && !(lacl[idx].a_perm & DENY_RWX))
	      {
		/* Sum up all permissions of groups the user is member of, plus
		   everyone perms, and merge them to user perms.  */
		BOOL ret;

		perm = lacl[2].a_perm & S_IRWXO;
		for (int gidx = 1; gidx < pos; ++gidx)
		  if (lacl[gidx].a_type & (GROUP_OBJ | GROUP)
		      && CheckTokenMembership (cygheap->user.issetuid ()
					       ? cygheap->user.imp_token () : NULL,
					       aclsid[gidx], &ret)
		      && ret)
		    perm |= lacl[gidx].a_perm & S_IRWXO;
		lacl[idx].a_perm |= perm;
	      }
	  }
	/* For all groups, if everyone has more permissions, add everyone
	   perms to group perms.  Skip groups with deny ACE. */
	else if (lacl[idx].a_type & (GROUP_OBJ | GROUP)
		 && !(lacl[idx].a_type & ACL_DEFAULT)
		 && !(lacl[idx].a_perm & DENY_RWX))
	  lacl[idx].a_perm |= lacl[2].a_perm & S_IRWXO;
      }
  /* If owner SID == group SID (Microsoft Accounts) merge group perms into
     user perms but leave group perms intact.  That's a fake, but it allows
     to keep track of the POSIX group perms without much effort. */
  if (owner_eq_group)
    lacl[0].a_perm |= lacl[1].a_perm;
  /* Construct POSIX permission bits.  Fortunately we know exactly where
     to fetch the affecting bits from, at least as long as the array
     hasn't been sorted. */
  attr |= (lacl[0].a_perm & S_IRWXO) << 6;
  attr |= ((has_class_perm ? class_perm : lacl[1].a_perm) & S_IRWXO) << 3;
  attr |= (lacl[2].a_perm & S_IRWXO);

out:
  if (uid_ret)
    *uid_ret = uid;
  if (gid_ret)
    *gid_ret = gid;
  if (attr_ret)
    *attr_ret = attr;
  if (aclbufp)
    {
      if (pos > nentries)
	{
	  set_errno (ENOSPC);
	  return -1;
	}
      memcpy (aclbufp, lacl, pos * sizeof (aclent_t));
      for (idx = 0; idx < pos; ++idx)
	aclbufp[idx].a_perm &= S_IRWXO;
      aclsort32 (pos, 0, aclbufp);
    }
  if (std_acl)
    *std_acl = standard_ACEs_only;
  return pos;
}

int
getacl (HANDLE handle, path_conv &pc, int nentries, aclent_t *aclbufp)
{
  security_descriptor sd;

  if (get_file_sd (handle, pc, sd, false))
    return -1;
  int pos = get_posix_access (sd, NULL, NULL, NULL, aclbufp, nentries);
  syscall_printf ("%R = getacl(%S)", pos, pc.get_nt_native_path ());
  return pos;
}

extern "C" int
acl32 (const char *path, int cmd, int nentries, aclent_t *aclbufp)
{
  int res = -1;

  fhandler_base *fh = build_fh_name (path, PC_SYM_FOLLOW | PC_KEEP_HANDLE,
				     stat_suffixes);
  if (!fh || !fh->exists ())
    set_errno (ENOENT);
  else if (fh->error ())
    {
      debug_printf ("got %d error from build_fh_name", fh->error ());
      set_errno (fh->error ());
    }
  else
    res = fh->facl (cmd, nentries, aclbufp);

  delete fh;
  syscall_printf ("%R = acl(%s)", res, path);
  return res;
}

#ifdef __i386__
extern "C" int
lacl32 (const char *path, int cmd, int nentries, aclent_t *aclbufp)
{
  /* This call was an accident.  Make it absolutely clear. */
  set_errno (ENOSYS);
  return -1;
}
#endif

extern "C" int
facl32 (int fd, int cmd, int nentries, aclent_t *aclbufp)
{
  cygheap_fdget cfd (fd);
  if (cfd < 0)
    {
      syscall_printf ("-1 = facl (%d)", fd);
      return -1;
    }
  int res = cfd->facl (cmd, nentries, aclbufp);
  syscall_printf ("%R = facl(%s) )", res, cfd->get_name ());
  return res;
}

int
__aclcheck (aclent_t *aclbufp, int nentries, int *which, bool posix)
{
  bool has_user_obj = false;
  bool has_group_obj = false;
  bool has_other_obj = false;
  bool has_class_obj = false;
  bool has_ug_objs = false;
  bool has_def_objs = false;
  bool has_def_user_obj = false;
  bool has_def_group_obj = false;
  bool has_def_other_obj = false;
  bool has_def_class_obj = false;
  bool has_def_ug_objs = false;
  int pos2;

  for (int pos = 0; pos < nentries; ++pos)
    {
      /* POSIX ACLs may contain deleted entries.  Just ignore them. */
      if (posix && aclbufp[pos].a_type == ACL_DELETED_TAG)
	continue;
      /* POSIX defines two sorts of ACLs, access and default, none of which
	 is supposed to have the ACL_DEFAULT flag set. */
      if (posix && (aclbufp[pos].a_type & ACL_DEFAULT))
	{
	  if (which)
	    *which = pos;
	  return ENTRY_ERROR;
	}
      switch (aclbufp[pos].a_type)
	{
	case USER_OBJ:
	  if (has_user_obj)
	    {
	      if (which)
		*which = pos;
	      return USER_ERROR;
	    }
	  has_user_obj = true;
	  break;
	case GROUP_OBJ:
	  if (has_group_obj)
	    {
	      if (which)
		*which = pos;
	      return GRP_ERROR;
	    }
	  has_group_obj = true;
	  break;
	case OTHER_OBJ:
	  if (has_other_obj)
	    {
	      if (which)
		*which = pos;
	      return OTHER_ERROR;
	    }
	  has_other_obj = true;
	  break;
	case CLASS_OBJ:
	  if (has_class_obj)
	    {
	      if (which)
		*which = pos;
	      return CLASS_ERROR;
	    }
	  has_class_obj = true;
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
	  has_ug_objs = true;
	  break;
	case DEF_USER_OBJ:
	  if (has_def_user_obj)
	    {
	      if (which)
		*which = pos;
	      return USER_ERROR;
	    }
	  has_def_objs = has_def_user_obj = true;
	  break;
	case DEF_GROUP_OBJ:
	  if (has_def_group_obj)
	    {
	      if (which)
		*which = pos;
	      return GRP_ERROR;
	    }
	  has_def_objs = has_def_group_obj = true;
	  break;
	case DEF_OTHER_OBJ:
	  if (has_def_other_obj)
	    {
	      if (which)
		*which = pos;
	      return OTHER_ERROR;
	    }
	  has_def_objs = has_def_other_obj = true;
	  break;
	case DEF_CLASS_OBJ:
	  if (has_def_class_obj)
	    {
	      if (which)
		*which = pos;
	      return CLASS_ERROR;
	    }
	  has_def_objs = has_def_class_obj = true;
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
	  has_def_objs = has_def_ug_objs = true;
	  break;
	default:
	  if (which)
	    *which = pos;
	  return ENTRY_ERROR;
	}
    }
  if (!has_user_obj
      || !has_group_obj
      || !has_other_obj
      || (has_ug_objs && !has_class_obj))
    {
      if (which)
	*which = -1;
      return MISS_ERROR;
    }
  /* Check for missing default entries only on Solaris ACLs. */
  if (!posix &&
      ((has_def_objs
	&& !(has_def_user_obj && has_def_group_obj && has_def_other_obj))
       || (has_def_ug_objs && !has_def_class_obj)))
    {
      if (which)
	*which = -1;
      return MISS_ERROR;
    }
  return 0;
}

extern "C" int
aclcheck32 (aclent_t *aclbufp, int nentries, int *which)
{
  return __aclcheck (aclbufp, nentries, which, false);
}

/* For the sake of acl_calc_mask, return -1 if the ACL doesn't need a mask
   or if a mask entry already exists (__aclcalcmask sets the mask by itself).
   Otherwise return the mask value so acl_calc_mask can create a mask entry.
   This doesn't matter when called from aclsort. */
mode_t
__aclcalcmask (aclent_t *aclbufp, int nentries)
{
  mode_t mask = 0;
  bool need_mask = false;
  int mask_idx = -1;

  for (int idx = 0; idx < nentries; ++idx)
    switch (aclbufp[idx].a_type)
      {
      case USER:
      case GROUP:
	need_mask = true;
	/*FALLTHRU*/
      case GROUP_OBJ:
	mask |= aclbufp[idx].a_perm;
	break;
      case CLASS_OBJ:
	mask_idx = idx;
	break;
      default:
	break;
      }
  if (mask_idx != -1)
    aclbufp[mask_idx].a_perm = mask;
  if (need_mask && mask_idx == -1)
    return mask;
  return (acl_perm_t) -1;
}

static int
acecmp (const void *a1, const void *a2)
{
#define ace(i) ((const aclent_t *) a##i)
  int ret = ace (1)->a_type - ace (2)->a_type;
  if (!ret)
    ret = ace (1)->a_id - ace (2)->a_id;
  return ret;
#undef ace
}

/* Sorts any acl.  Called from sec_posixacl.cc. */
int
__aclsort (int nentries, aclent_t *aclbufp)
{
  if (!aclbufp || nentries < 0)
    {
      set_errno (EINVAL);
      return -1;
    }
  if (nentries > 0)
    qsort ((void *) aclbufp, nentries, sizeof (aclent_t), acecmp);
  return 0;
}

extern "C" int
aclsort32 (int nentries, int calclass, aclent_t *aclbufp)
{
  if (!aclbufp || nentries < MIN_ACL_ENTRIES
      || aclcheck32 (aclbufp, nentries, NULL))
    {
      set_errno (EINVAL);
      return -1;
    }
  qsort ((void *) aclbufp, nentries, sizeof (aclent_t), acecmp);
  if (calclass)
    __aclcalcmask (aclbufp, nentries);
  return 0;
}

extern "C" int
acltomode32 (aclent_t *aclbufp, int nentries, mode_t *modep)
{
  int pos;

  if (!aclbufp || nentries < 1 || !modep)
    {
      set_errno (EINVAL);
      return -1;
    }
  *modep = 0;
  if ((pos = searchace (aclbufp, nentries, USER_OBJ)) < 0
      || !aclbufp[pos].a_type)
    {
      set_errno (EINVAL);
      return -1;
    }
  *modep |= (aclbufp[pos].a_perm & S_IRWXO) << 6;
  if ((pos = searchace (aclbufp, nentries, GROUP_OBJ)) < 0
      || !aclbufp[pos].a_type)
    {
      set_errno (EINVAL);
      return -1;
    }
  *modep |= (aclbufp[pos].a_perm & S_IRWXO) << 3;
  int cpos;
  if ((cpos = searchace (aclbufp, nentries, CLASS_OBJ)) >= 0
      && aclbufp[cpos].a_type == CLASS_OBJ)
    *modep |= ((aclbufp[pos].a_perm & S_IRWXO) & aclbufp[cpos].a_perm) << 3;
  if ((pos = searchace (aclbufp, nentries, OTHER_OBJ)) < 0
      || !aclbufp[pos].a_type)
    {
      set_errno (EINVAL);
      return -1;
    }
  *modep |= aclbufp[pos].a_perm & S_IRWXO;
  return 0;
}

extern "C" int
aclfrommode32 (aclent_t *aclbufp, int nentries, mode_t *modep)
{
  int pos;

  if (!aclbufp || nentries < 1 || !modep)
    {
      set_errno (EINVAL);
      return -1;
    }
  if ((pos = searchace (aclbufp, nentries, USER_OBJ)) < 0
      || !aclbufp[pos].a_type)
    {
      set_errno (EINVAL);
      return -1;
    }
  aclbufp[pos].a_perm = (*modep & S_IRWXU) >> 6;
  if ((pos = searchace (aclbufp, nentries, GROUP_OBJ)) < 0
      || !aclbufp[pos].a_type)
    {
      set_errno (EINVAL);
      return -1;
    }
  aclbufp[pos].a_perm = (*modep & S_IRWXG) >> 3;
  if ((pos = searchace (aclbufp, nentries, CLASS_OBJ)) >= 0
      && aclbufp[pos].a_type == CLASS_OBJ)
    aclbufp[pos].a_perm = (*modep & S_IRWXG) >> 3;
  if ((pos = searchace (aclbufp, nentries, OTHER_OBJ)) < 0
      || !aclbufp[pos].a_type)
    {
      set_errno (EINVAL);
      return -1;
    }
  aclbufp[pos].a_perm = (*modep & S_IRWXO);
  return 0;
}

extern "C" int
acltopbits32 (aclent_t *aclbufp, int nentries, mode_t *pbitsp)
{
  return acltomode32 (aclbufp, nentries, pbitsp);
}

extern "C" int
aclfrompbits32 (aclent_t *aclbufp, int nentries, mode_t *pbitsp)
{
  return aclfrommode32 (aclbufp, nentries, pbitsp);
}

static char *
permtostr (char *bufp, mode_t perm)
{
  *bufp++ = (perm & S_IROTH) ? 'r' : '-';
  *bufp++ = (perm & S_IWOTH) ? 'w' : '-';
  *bufp++ = (perm & S_IXOTH) ? 'x' : '-';
  return bufp;
}

#define _OPT(o) (options & (o))

#define _CHK(l) \
		  if (bufp + (l) >= buf + 2 * NT_MAX_PATH - 1) \
		    { \
		      set_errno (ENOMEM); \
		      return NULL; \
		    }
#define _CPY(s)	({ \
		  const char *_s = (s); \
		  _CHK (strlen (_s)); \
		  bufp = stpcpy (bufp, _s); \
		})
#define _PTS(p) { \
		  _CHK (3); \
		  bufp = permtostr (bufp, p); \
		}

#define _CMP(s)		(!strncmp (bufp, acl_part[s].str, acl_part[s].len))

struct _acl_part
{
  const char *str;
  size_t len;
};

static _acl_part acl_part_l[] =
{
  { "default:",	8 },
  { "user:",	5 },
  { "group:",	6 },
  { "mask:",	5 },
  { "other:",	6 }
};

static _acl_part acl_part_s[] =
{
  { "d:",	2 },
  { "u:",	2 },
  { "g:",	2 },
  { "m:",	2 },
  { "o:",	2 }
};

enum _acl_type {
  default_s,
  user_s,
  group_s,
  mask_s,
  other_s,
  none_s
};

char *
__acltotext (aclent_t *aclbufp, int aclcnt, const char *prefix, char separator,
	     int options)
{
  if (!aclbufp || aclcnt < 0 || aclcnt > MAX_ACL_ENTRIES
      || (aclcnt > 0 && aclsort32 (aclcnt, 0, aclbufp)))
    {
      set_errno (EINVAL);
      return NULL;
    }
  cyg_ldap cldap;
  tmp_pathbuf tp;
  char *buf = tp.t_get ();
  char *bufp = buf;
  char *entry_start;
  bool first = true;
  struct passwd *pw;
  struct group *gr;
  mode_t mask = S_IRWXO;
  mode_t def_mask = S_IRWXO;
  mode_t effective;
  int pos;
  _acl_part *acl_part = _OPT (TEXT_ABBREVIATE) ? acl_part_s : acl_part_l;

  *bufp = '\0';
  /* If effective rights are requested, fetch mask values. */
  if (_OPT (TEXT_SOME_EFFECTIVE | TEXT_ALL_EFFECTIVE))
    {
      if ((pos = searchace (aclbufp, aclcnt, CLASS_OBJ)) >= 0)
	mask = aclbufp[pos].a_perm;
      if ((pos = searchace (aclbufp, aclcnt, DEF_CLASS_OBJ)) >= 0)
	def_mask = aclbufp[pos].a_perm;
    }
  for (pos = 0; pos < aclcnt; ++pos)
    {
      if (!first)
	{
	  _CHK (1);
	  *bufp++ = separator;
	}
      first = false;
      /* Rememeber start position of entry to compute TEXT_SMART_INDENT tabs. */
      entry_start = bufp;
      /* prefix */
      if (prefix)
	_CPY (prefix);
      /* Solaris default acl? */
      if (!_OPT (TEXT_IS_POSIX) && aclbufp[pos].a_type & ACL_DEFAULT)
	_CPY (acl_part[default_s].str);
      /* acl type */
      switch (aclbufp[pos].a_type & ~ACL_DEFAULT)
	{
	case USER_OBJ:
	case USER:
	  _CPY (acl_part[user_s].str);
	  break;
	case GROUP_OBJ:
	case GROUP:
	  _CPY (acl_part[group_s].str);
	  break;
	case CLASS_OBJ:
	  _CPY (acl_part[mask_s].str);
	  break;
	case OTHER_OBJ:
	  _CPY (acl_part[other_s].str);
	  break;
	}
      /* id, if any  */
      switch (aclbufp[pos].a_type & ~ACL_DEFAULT)
	{
	case USER:
	  if (_OPT (TEXT_NUMERIC_IDS)
	      || !(pw = internal_getpwuid (aclbufp[pos].a_id, &cldap)))
	    {
	      _CHK (11);
	      bufp += __small_sprintf (bufp, "%u:", aclbufp[pos].a_id);
	    }
	  else
	    {
	      _CHK (strlen (pw->pw_name + 1));
	      bufp += __small_sprintf (bufp, "%s:", pw->pw_name);
	    }
	  break;
	case GROUP:
	  if (_OPT (TEXT_NUMERIC_IDS)
	      || !(gr = internal_getgrgid (aclbufp[pos].a_id, &cldap)))
	    {
	      _CHK (11);
	      bufp += __small_sprintf (bufp, "%u:", aclbufp[pos].a_id);
	    }
	  else
	    {
	      _CHK (strlen (gr->gr_name));
	      bufp += __small_sprintf (bufp, "%s:", gr->gr_name);
	    }
	  break;
	default:
	  _CPY (":");
	  break;
	}
      /* real permissions */
      _PTS (aclbufp[pos].a_perm);
      if (!_OPT (TEXT_SOME_EFFECTIVE | TEXT_ALL_EFFECTIVE))
	continue;
      /* effective permissions */
      switch (aclbufp[pos].a_type)
	{
	case USER:
	case GROUP_OBJ:
	case GROUP:
	  effective = aclbufp[pos].a_perm & mask;
	  break;
	case DEF_USER:
	case DEF_GROUP_OBJ:
	case DEF_GROUP:
	  effective = aclbufp[pos].a_perm & def_mask;
	  break;
	default:
	  continue;
	}
      if (_OPT (TEXT_ALL_EFFECTIVE) || effective != aclbufp[pos].a_perm)
	{
	  if (_OPT (TEXT_SMART_INDENT))
	    {
	      int tabs = 3 - (bufp - entry_start) / 8;
	      if (tabs-- > 0)
		{
		  _CHK (tabs);
		  while (tabs-- > 0)
		    *bufp++ = '\t';
		}
	    }
	  _CPY ("\t#effective:");
	  _PTS (effective);
	}
    }
  if (_OPT (TEXT_END_SEPARATOR))
    {
      _CHK (1);
      *bufp++ = separator;
    }
  *bufp = '\0';
  return strdup (buf);
}

extern "C" char *
acltotext32 (aclent_t *aclbufp, int aclcnt)
{
  return __acltotext (aclbufp, aclcnt, NULL, ',', 0);
}

static mode_t
permfromstr (char *perm, bool posix_long)
{
  mode_t mode = 0;
  int idx;

  if (perm[0] == 'r')
    mode |= S_IROTH;
  else if (perm[0] != '-')
    return 01000;
  if (perm[1] == 'w')
    mode |= S_IWOTH;
  else if (perm[1] != '-')
    return 01000;
  if (perm[2] == 'x')
    mode |= S_IXOTH;
  else if (perm[2] != '-')
    return 01000;
  idx = 3;
  /* In posix long mode, only tabs up to a hash sign allowed. */
  if (posix_long)
    while (perm[idx] == '\t')
      ++idx;
  if (perm[idx] == '\0' || (posix_long && perm[idx] == '#'))
    return mode;
  return 01000;
}

void *
__aclfromtext (const char *acltextp, int *aclcnt, bool posix)
{
  if (!acltextp || strlen (acltextp) >= 2 * NT_MAX_PATH)
    {
      set_errno (EINVAL);
      return NULL;
    }
  cyg_ldap cldap;
  tmp_pathbuf tp;
  const char *delim;
  _acl_part *acl_part;
  char *bufp, *lasts, *qualifier;
  int pos = 0;
  int acl_type;

  aclent_t *lacl = (aclent_t *) tp.c_get ();
  memset (lacl, 0, MAX_ACL_ENTRIES * sizeof (aclent_t *));
  char *buf = tp.t_get ();
  stpcpy (buf, acltextp);

  if (posix)
    {
      /* Posix long or short form.  Any \n in the string means long form. */
      if (strchr (buf, '\n'))
	{
	  delim = "\n";
	  acl_part = acl_part_l;
	}
      else
	{
	  delim = ",";
	  acl_part = acl_part_s;
	}
    }
  else
    {
      /* Solaris aclfromtext format. */
      delim = ",";
      acl_part = acl_part_l;
    }

  for (bufp = strtok_r (buf, delim, &lasts);
       bufp;
       bufp = strtok_r (NULL, delim, &lasts))
    {
      /* Handle default acl entries only for Solaris ACLs. */
      if (!posix && _CMP (default_s))
	{
	  lacl[pos].a_type |= ACL_DEFAULT;
	  bufp += acl_part[default_s].len;
	}
      lacl[pos].a_id = ACL_UNDEFINED_ID;
      for (acl_type = user_s; acl_type < none_s; ++acl_type)
	if (_CMP (acl_type))
	  break;
      if (acl_type == none_s)
	{
	  set_errno (EINVAL);
	  return NULL;
	}
      bufp += acl_part[acl_type].len;
      switch (acl_type)
	{
	case user_s:
	case group_s:
	  qualifier = bufp;
	  bufp = strchrnul (bufp, ':');
	  *bufp++ = '\0';
	  /* No qualifier?  USER_OBJ or GROUP_OBJ */
	  if (!*qualifier)
	    {
	      lacl[pos].a_type |= (acl_type == user_s) ? USER_OBJ : GROUP_OBJ;
	      break;
	    }
	  /* Some qualifier, USER or GROUP */
	  lacl[pos].a_type |= (acl_type == user_s) ? USER : GROUP;
	  if (isdigit (*qualifier))
	    {
	      char *ep;

	      id_t id = strtol (qualifier, &ep, 10);
	      if (*ep == '\0')
		{
		  lacl[pos].a_id = id;
		  break;
		}
	    }
	  if (acl_type == user_s)
	    {
	      struct passwd *pw = internal_getpwnam (qualifier, &cldap);
	      if (pw)
		lacl[pos].a_id = pw->pw_uid;
	    }
	  else
	    {
	      struct group *gr = internal_getgrnam (qualifier, &cldap);
	      if (gr)
		lacl[pos].a_id = gr->gr_gid;
	    }
	  if (lacl[pos].a_id == ACL_UNDEFINED_ID)
	    {
	      set_errno (EINVAL);
	      return NULL;
	    }
	  break;
	case mask_s:
	case other_s:
	  if (*bufp++ != ':')
	    {
	      set_errno (EINVAL);
	      return NULL;
	    }
	  lacl[pos].a_type |= (acl_type == mask_s) ? CLASS_OBJ : OTHER_OBJ;
	  break;
	}
      /* In posix long mode, the next char after the permissions may be a tab
	 followed by effective permissions we can ignore here. */
      if ((lacl[pos].a_perm = permfromstr (bufp, *delim == '\n')) == 01000)
	{
	  set_errno (EINVAL);
	  return NULL;
	}
      ++pos;
    }
  if (posix)
    {
      acl_t acl = (acl_t) acl_init (pos);
      if (acl)
	{
	  memcpy (acl->entry, lacl, pos * sizeof (aclent_t));
	  acl->count = pos;
	  if (aclcnt)
	    *aclcnt = pos;
	}
      return (void *) acl;
    }
  else
    {
      aclent_t *aclp = (aclent_t *) malloc (pos * sizeof (aclent_t));
      if (aclp)
	{
	  memcpy (aclp, lacl, pos * sizeof (aclent_t));
	  if (aclcnt)
	    *aclcnt = pos;
	}
      return (void *) aclp;
    }
}

extern "C" aclent_t *
aclfromtext32 (char *acltextp, int *aclcnt)
{
  return (aclent_t *) __aclfromtext (acltextp, aclcnt, false);
}

#ifdef __i386__
typedef struct __acl16 {
    int          a_type;
    __uid16_t    a_id;
    mode_t       a_perm;
} __aclent16_t;

/* __aclent16_t and aclent_t have same size and same member offsets */
static aclent_t *
acl16to32 (__aclent16_t *aclbufp, int nentries)
{
  aclent_t *aclbufp32 = (aclent_t *) aclbufp;
  if (aclbufp32)
    for (int i = 0; i < nentries; i++)
      aclbufp32[i].a_id &= USHRT_MAX;
  return aclbufp32;
}

extern "C" int
acl (const char *path, int cmd, int nentries, __aclent16_t *aclbufp)
{
  return acl32 (path, cmd, nentries, acl16to32 (aclbufp, nentries));
}

extern "C" int
facl (int fd, int cmd, int nentries, __aclent16_t *aclbufp)
{
  return facl32 (fd, cmd, nentries, acl16to32 (aclbufp, nentries));
}

extern "C" int
lacl (const char *path, int cmd, int nentries, __aclent16_t *aclbufp)
{
  /* This call was an accident.  Make it absolutely clear. */
  set_errno (ENOSYS);
  return -1;
}

extern "C" int
aclcheck (__aclent16_t *aclbufp, int nentries, int *which)
{
  return aclcheck32 (acl16to32 (aclbufp, nentries), nentries, which);
}

extern "C" int
aclsort (int nentries, int i, __aclent16_t *aclbufp)
{
  return aclsort32 (nentries, i, acl16to32 (aclbufp, nentries));
}


extern "C" int
acltomode (__aclent16_t *aclbufp, int nentries, mode_t *modep)
{
  return acltomode32 (acl16to32 (aclbufp, nentries), nentries, modep);
}

extern "C" int
aclfrommode (__aclent16_t *aclbufp, int nentries, mode_t *modep)
{
  return aclfrommode32 ((aclent_t *)aclbufp, nentries, modep);
}

extern "C" int
acltopbits (__aclent16_t *aclbufp, int nentries, mode_t *pbitsp)
{
  return acltopbits32 (acl16to32 (aclbufp, nentries), nentries, pbitsp);
}

extern "C" int
aclfrompbits (__aclent16_t *aclbufp, int nentries, mode_t *pbitsp)
{
  return aclfrompbits32 ((aclent_t *)aclbufp, nentries, pbitsp);
}

extern "C" char *
acltotext (__aclent16_t *aclbufp, int aclcnt)
{
  return acltotext32 (acl16to32 (aclbufp, aclcnt), aclcnt);
}

extern "C" __aclent16_t *
aclfromtext (char *acltextp, int *aclcnt)
{
  return (__aclent16_t *) aclfromtext32 (acltextp, aclcnt);
}
#else
EXPORT_ALIAS (acl32, acl)
EXPORT_ALIAS (facl32, facl)
EXPORT_ALIAS (aclcheck32, aclcheck)
EXPORT_ALIAS (aclsort32, aclsort)
EXPORT_ALIAS (acltomode32, acltomode)
EXPORT_ALIAS (aclfrommode32, aclfrommode)
EXPORT_ALIAS (acltopbits32, acltopbits)
EXPORT_ALIAS (aclfrompbits32, aclfrompbits)
EXPORT_ALIAS (acltotext32, acltotext)
EXPORT_ALIAS (aclfromtext32, aclfromtext)
#endif
