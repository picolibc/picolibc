/* sec_acl.cc: Sun compatible ACL functions.

   Copyright 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010,
   2011, 2012, 2014, 2015 Red Hat, Inc.

   Written by Corinna Vinschen <corinna@vinschen.de>

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <stdlib.h>
#include <sys/acl.h>
#include <ctype.h>
#include "cygerrno.h"
#include "security.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include "ntdll.h"
#include "tls_pbuf.h"

/* How does a correctly constructed new-style Windows ACL claiming to be a
   POSIX ACL look like?

   - Cygwin ACE (special bits, CLASS_OBJ).

   - If a USER entry has more permissions than any group, Everyone, or if it
     has more permissions than allowed by the CLASS_OBJ entry:

     USER      deny ACEs   == POSIX allow
			      & ^(mask | all group allows | Everyone allow)

   - USER_OBJ  allow ACE
   - USER      allow ACEs

     The POSIX permissions returned for a USER entry are the allow bits alone!

   - If a GROUP entry has more permissions than Everyone, or if it has more
     permissions than allowed by the CLASS_OBJ entry:

     GROUP	deny ACEs  == POSIX allow & ^(mask | Everyone allow)

   - GROUP_OBJ	allow ACE
   - GROUP	allow ACEs

     The POSIX permissions returned for a GROUP entry are the allow bits alone!

   - OTHER_OBJ	allow ACE

   Rinse and repeat for default ACEs with INHERIT flags set.

   - Default Cygwin ACE (S_ISGID, CLASS_OBJ). */

						/* POSIX <-> Win32 */

/* Historically, these bits are stored in a NULL SID ACE.  To distinguish
   the new ACL style from the old one, we're now using an invented SID, the
   Cygwin SID (S-1-0-1132029815).  The new ACEs can exist twice in an ACL,
   the "normal one" containg CLASS_OBJ and special bits, and the one with
   INHERIT bit set to pass the DEF_CLASS_OBJ bits and the S_ISGID bit on. */
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

static int
searchace (aclent_t *aclp, int nentries, int type, uid_t id = ILLEGAL_UID)
{
  int i;

  for (i = 0; i < nentries; ++i)
    if ((aclp[i].a_type == type && (id == ILLEGAL_UID || aclp[i].a_id == id))
	|| !aclp[i].a_type)
      return i;
  return -1;
}

/* This function *requires* an acl list sorted with aclsort{32}. */
int
setacl (HANDLE handle, path_conv &pc, int nentries, aclent_t *aclbufp,
	bool &writable)
{
  security_descriptor sd_ret;
  tmp_pathbuf tp;

  if (get_file_sd (handle, pc, sd_ret, false))
    return -1;

  NTSTATUS status;
  PACL acl;
  BOOLEAN acl_exists, dummy;

  /* Get owner SID. */
  PSID owner_sid;
  status = RtlGetOwnerSecurityDescriptor (sd_ret, &owner_sid, &dummy);
  if (!NT_SUCCESS (status))
    {
      __seterrno_from_nt_status (status);
      return -1;
    }
  cygsid owner (owner_sid);

  /* Get group SID. */
  PSID group_sid;
  status = RtlGetGroupSecurityDescriptor (sd_ret, &group_sid, &dummy);
  if (!NT_SUCCESS (status))
    {
      __seterrno_from_nt_status (status);
      return -1;
    }
  cygsid group (group_sid);

  /* Search for NULL ACE and store state of SUID, SGID and VTX bits. */
  DWORD null_mask = 0;
  if (NT_SUCCESS (RtlGetDaclSecurityDescriptor (sd_ret, &acl_exists, &acl,
						&dummy)))
    for (USHORT i = 0; i < acl->AceCount; ++i)
      {
	ACCESS_ALLOWED_ACE *ace;
	if (NT_SUCCESS (RtlGetAce (acl, i, (PVOID *) &ace)))
	  {
	    cygpsid ace_sid ((PSID) &ace->SidStart);
	    if (ace_sid == well_known_null_sid)
	      {
		null_mask = ace->Mask;
		break;
	      }
	  }
      }

  /* Initialize local security descriptor. */
  SECURITY_DESCRIPTOR sd;
  RtlCreateSecurityDescriptor (&sd, SECURITY_DESCRIPTOR_REVISION);

  /* As in alloc_sd, set SE_DACL_PROTECTED to prevent the DACL from being
     modified by inheritable ACEs. */
  RtlSetControlSecurityDescriptor (&sd, SE_DACL_PROTECTED, SE_DACL_PROTECTED);

  status = RtlSetOwnerSecurityDescriptor (&sd, owner, FALSE);
  if (!NT_SUCCESS (status))
    {
      __seterrno_from_nt_status (status);
      return -1;
    }
  status = RtlSetGroupSecurityDescriptor (&sd, group, FALSE);
  if (!NT_SUCCESS (status))
    {
      __seterrno_from_nt_status (status);
      return -1;
    }

  /* Fill access control list. */
  acl = (PACL) tp.w_get ();
  size_t acl_len = sizeof (ACL);

  cygsid sid;
  struct passwd *pw;
  struct group *gr;
  int pos;
  cyg_ldap cldap;

  RtlCreateAcl (acl, ACL_MAXIMUM_SIZE, ACL_REVISION);

  writable = false;

  bool *invalid = (bool *) tp.c_get ();
  memset (invalid, 0, nentries * sizeof *invalid);

  /* Pre-compute owner, group, and other permissions to allow creating
     matching deny ACEs as in alloc_sd. */
  DWORD owner_allow = 0, group_allow = 0, other_allow = 0;
  PDWORD allow;
  for (int i = 0; i < nentries; ++i)
    {
      switch (aclbufp[i].a_type)
	{
	case USER_OBJ:
	  allow = &owner_allow;
	  *allow = STANDARD_RIGHTS_ALL
		   | (pc.fs_is_samba () ? 0 : FILE_WRITE_ATTRIBUTES);
	  break;
	case GROUP_OBJ:
	  allow = &group_allow;
	  break;
	case OTHER_OBJ:
	  allow = &other_allow;
	  break;
	default:
	  continue;
	}
      *allow |= STANDARD_RIGHTS_READ | SYNCHRONIZE
		| (pc.fs_is_samba () ? 0 : FILE_READ_ATTRIBUTES);
      if (aclbufp[i].a_perm & S_IROTH)
	*allow |= FILE_GENERIC_READ;
      if (aclbufp[i].a_perm & S_IWOTH)
	{
	  *allow |= FILE_GENERIC_WRITE;
	  writable = true;
	}
      if (aclbufp[i].a_perm & S_IXOTH)
	*allow |= FILE_GENERIC_EXECUTE & ~FILE_READ_ATTRIBUTES;
      /* Keep S_ISVTX rule in sync with alloc_sd. */
      if (pc.isdir ()
	  && (aclbufp[i].a_perm & (S_IWOTH | S_IXOTH)) == (S_IWOTH | S_IXOTH)
	  && (aclbufp[i].a_type == USER_OBJ
	      || !(null_mask & FILE_READ_DATA)))
	*allow |= FILE_DELETE_CHILD;
      invalid[i] = true;
    }
  bool isownergroup = (owner == group);
  DWORD owner_deny = ~owner_allow & (group_allow | other_allow);
  owner_deny &= ~(STANDARD_RIGHTS_READ
		  | FILE_READ_ATTRIBUTES | FILE_WRITE_ATTRIBUTES);
  DWORD group_deny = ~group_allow & other_allow;
  group_deny &= ~(STANDARD_RIGHTS_READ | FILE_READ_ATTRIBUTES);

  /* Set deny ACE for owner. */
  if (owner_deny
      && !add_access_denied_ace (acl, owner_deny, owner, acl_len,
				 NO_INHERITANCE))
    return -1;
  /* Set deny ACE for group here to respect the canonical order,
     if this does not impact owner */
  if (group_deny && !(group_deny & owner_allow) && !isownergroup
      && !add_access_denied_ace (acl, group_deny, group, acl_len,
				 NO_INHERITANCE))
    return -1;
  /* Set allow ACE for owner. */
  if (!add_access_allowed_ace (acl, owner_allow, owner, acl_len,
			       NO_INHERITANCE))
    return -1;
  /* Set deny ACE for group, if still needed. */
  if (group_deny & owner_allow && !isownergroup
      && !add_access_denied_ace (acl, group_deny, group, acl_len,
				 NO_INHERITANCE))
    return -1;
  /* Set allow ACE for group. */
  if (!isownergroup
      && !add_access_allowed_ace (acl, group_allow, group, acl_len,
				  NO_INHERITANCE))
    return -1;
  /* Set allow ACE for everyone. */
  if (!add_access_allowed_ace (acl, other_allow, well_known_world_sid, acl_len,
			       NO_INHERITANCE))
    return -1;
  /* If a NULL ACE exists, copy it verbatim. */
  if (null_mask)
    if (!add_access_allowed_ace (acl, null_mask, well_known_null_sid, acl_len,
				 NO_INHERITANCE))
      return -1;
  for (int i = 0; i < nentries; ++i)
    {
      DWORD allow;
      /* Skip invalidated entries. */
      if (invalid[i])
	continue;

      allow = STANDARD_RIGHTS_READ
	      | (pc.fs_is_samba () ? 0 : FILE_READ_ATTRIBUTES);
      if (aclbufp[i].a_perm & S_IROTH)
	allow |= FILE_GENERIC_READ;
      if (aclbufp[i].a_perm & S_IWOTH)
	{
	  allow |= FILE_GENERIC_WRITE;
	  writable = true;
	}
      if (aclbufp[i].a_perm & S_IXOTH)
	allow |= FILE_GENERIC_EXECUTE & ~FILE_READ_ATTRIBUTES;
      /* Keep S_ISVTX rule in sync with alloc_sd. */
      if (pc.isdir ()
	  && (aclbufp[i].a_perm & (S_IWOTH | S_IXOTH)) == (S_IWOTH | S_IXOTH)
	  && !(null_mask & FILE_READ_DATA))
	allow |= FILE_DELETE_CHILD;
      /* Set inherit property. */
      DWORD inheritance = (aclbufp[i].a_type & ACL_DEFAULT)
			  ? (SUB_CONTAINERS_AND_OBJECTS_INHERIT | INHERIT_ONLY)
			  : NO_INHERITANCE;
      /*
       * If a specific acl contains a corresponding default entry with
       * identical permissions, only one Windows ACE with proper
       * inheritance bits is created.
       */
      if (!(aclbufp[i].a_type & ACL_DEFAULT)
	  && aclbufp[i].a_type & (USER|GROUP)
	  && (pos = searchace (aclbufp + i + 1, nentries - i - 1,
			       aclbufp[i].a_type | ACL_DEFAULT,
			       (aclbufp[i].a_type & (USER|GROUP))
			       ? aclbufp[i].a_id : ILLEGAL_UID)) >= 0
	  && aclbufp[i].a_perm == aclbufp[i + 1 + pos].a_perm)
	{
	  inheritance = SUB_CONTAINERS_AND_OBJECTS_INHERIT;
	  /* invalidate the corresponding default entry. */
	  invalid[i + 1 + pos] = true;
	}
      switch (aclbufp[i].a_type)
	{
	case DEF_USER_OBJ:
	  allow |= STANDARD_RIGHTS_ALL
		   | (pc.fs_is_samba () ? 0 : FILE_WRITE_ATTRIBUTES);
	  if (!add_access_allowed_ace (acl, allow, well_known_creator_owner_sid,
				       acl_len, inheritance))
	    return -1;
	  break;
	case USER:
	case DEF_USER:
	  if (!(pw = internal_getpwuid (aclbufp[i].a_id, &cldap))
	      || !sid.getfrompw (pw))
	    {
	      set_errno (EINVAL);
	      return -1;
	    }
	  if (!add_access_allowed_ace (acl, allow, sid, acl_len, inheritance))
	    return -1;
	  break;
	case DEF_GROUP_OBJ:
	  if (!add_access_allowed_ace (acl, allow, well_known_creator_group_sid,
				       acl_len, inheritance))
	    return -1;
	  break;
	case GROUP:
	case DEF_GROUP:
	  if (!(gr = internal_getgrgid (aclbufp[i].a_id, &cldap))
	      || !sid.getfromgr (gr))
	    {
	      set_errno (EINVAL);
	      return -1;
	    }
	  if (!add_access_allowed_ace (acl, allow, sid, acl_len, inheritance))
	    return -1;
	  break;
	case DEF_OTHER_OBJ:
	  if (!add_access_allowed_ace (acl, allow, well_known_world_sid,
				       acl_len, inheritance))
	    return -1;
	}
    }
  /* Set AclSize to computed value. */
  acl->AclSize = acl_len;
  debug_printf ("ACL-Size: %u", acl_len);
  /* Create DACL for local security descriptor. */
  status = RtlSetDaclSecurityDescriptor (&sd, TRUE, acl, FALSE);
  if (!NT_SUCCESS (status))
    {
      __seterrno_from_nt_status (status);
      return -1;
    }
  /* Make self relative security descriptor in sd_ret. */
  DWORD sd_size = 0;
  RtlAbsoluteToSelfRelativeSD (&sd, sd_ret, &sd_size);
  if (sd_size <= 0)
    {
      __seterrno ();
      return -1;
    }
  if (!sd_ret.realloc (sd_size))
    {
      set_errno (ENOMEM);
      return -1;
    }
  status = RtlAbsoluteToSelfRelativeSD (&sd, sd_ret, &sd_size);
  if (!NT_SUCCESS (status))
    {
      __seterrno_from_nt_status (status);
      return -1;
    }
  debug_printf ("Created SD-Size: %u", sd_ret.size ());
  return set_file_sd (handle, pc, sd_ret, false);
}

/* Temporary access denied bits */
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
   of entries returned in aclbufp, or -1 in case of error. */
int
get_posix_access (PSECURITY_DESCRIPTOR psd,
		  mode_t *attr_ret, uid_t *uid_ret, gid_t *gid_ret,
		  aclent_t *aclbufp, int nentries)
{
  tmp_pathbuf tp;
  NTSTATUS status;
  BOOLEAN dummy, acl_exists;
  PACL acl;
  PACCESS_ALLOWED_ACE ace;
  cygpsid owner_sid, group_sid;
  cyg_ldap cldap;
  uid_t uid;
  gid_t gid;
  mode_t attr = 0;
  aclent_t *lacl = NULL;
  cygpsid ace_sid;
  int pos, type, id, idx;

  bool new_style = false;
  bool saw_user_obj = false;
  bool saw_group_obj = false;
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
        *uid_ret = ILLEGAL_UID;
      if (gid_ret)
        *gid_ret = ILLEGAL_GID;
      if (aclbufp)
	{
	  aclbufp[0].a_type = USER_OBJ;
	  aclbufp[0].a_id = ILLEGAL_UID;
	  aclbufp[0].a_perm = 0;
	  aclbufp[1].a_type = GROUP_OBJ;
	  aclbufp[1].a_id = ILLEGAL_GID;
	  aclbufp[1].a_perm = 0;
	  aclbufp[2].a_type = OTHER_OBJ;
	  aclbufp[2].a_id = ILLEGAL_GID;
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
    attr |= (*attr_ret & S_IFMT);

  /* Create and initialize local aclent_t array. */
  lacl = (aclent_t *) tp.c_get ();
  memset (lacl, 0, MAX_ACL_ENTRIES * sizeof (aclent_t *));
  lacl[0].a_type = USER_OBJ;
  lacl[0].a_id = uid;
  lacl[1].a_type = GROUP_OBJ;
  lacl[1].a_id = gid;
  lacl[2].a_type = OTHER_OBJ;
  lacl[2].a_id = ILLEGAL_GID;

  /* No ACEs?  Everybody has full access. */
  if (!acl_exists || !acl || acl->AceCount == 0)
    {
      for (pos = 0; pos < MIN_ACL_ENTRIES; ++pos)
	lacl[pos].a_perm = S_IROTH | S_IWOTH | S_IXOTH;
      goto out;
    }

  for (idx = 0; idx < acl->AceCount; ++idx)
    {
      if (!NT_SUCCESS (RtlGetAce (acl, idx, (PVOID *) &ace)))
	continue;

      ace_sid = (PSID) &ace->SidStart;

      if (ace_sid == well_known_null_sid)
	{
	  /* Old-style or non-Cygwin ACL.  Fetch only the special bits. */
	  attr |= CYG_ACE_ISBITS_TO_POSIX (ace->Mask);
	  continue;
	}
      if (ace_sid == well_known_cygwin_sid)
	{
	  /* New-style ACL.  Note the fact that a mask value is present since
	     that changes how getace fetches the information.  That's fine,
	     because the Cygwin SID ACE is supposed to precede all USER, GROUP
	     and GROUP_OBJ entries.  Any ACL not created that way has been
	     rearranged by the Windows functionality to create the brain-dead
	     "canonical" ACL order and is broken anyway. */
	  attr |= CYG_ACE_ISBITS_TO_POSIX (ace->Mask);
	  if (ace->Mask & CYG_ACE_MASK_VALID)
	    {
	      new_style = true;
	      type = (ace->Header.AceFlags & SUB_CONTAINERS_AND_OBJECTS_INHERIT)
		     ? DEF_CLASS_OBJ : CLASS_OBJ;
	      if ((pos = searchace (lacl, MAX_ACL_ENTRIES, type)) >= 0)
		{
		  lacl[pos].a_type = type;
		  lacl[pos].a_id = ILLEGAL_GID;
		  lacl[pos].a_perm = CYG_ACE_MASK_TO_POSIX (ace->Mask);
		}
	      if (type == CLASS_OBJ)	/* Needed for POSIX permissions. */
		{
		  has_class_perm = true;
		  class_perm = lacl[pos].a_perm;
		}
	      else
		{
		  has_def_class_perm = true;
		  def_class_perm = lacl[pos].a_perm;
		}
	    }
	  continue;
	}
      else if (ace_sid == owner_sid)
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
	  id = ILLEGAL_GID;
	}
      else if (ace_sid == well_known_creator_owner_sid)
	{
	  type = DEF_USER_OBJ;
	  types_def |= type;
	  id = ILLEGAL_GID;
	}
      else if (ace_sid == well_known_creator_group_sid)
	{
	  type = DEF_GROUP_OBJ;
	  types_def |= type;
	  id = ILLEGAL_GID;
	  saw_def_group_obj = true;
	}
      else
	{
	  id = ace_sid.get_id (TRUE, &type, &cldap);
	  if (!type)
	    continue;
	}
      if (!(ace->Header.AceFlags & INHERIT_ONLY || type & ACL_DEFAULT))
	{
	  if (type == USER_OBJ)
	    {
	      /* If we get a second entry for the owner, it's an additional
		 USER entry.  This can happen when chown'ing a file. */
	      if (saw_user_obj)
		type = USER;
	      if (ace->Header.AceType == ACCESS_ALLOWED_ACE_TYPE)
		saw_user_obj = true;
	    }
	  else if (type == GROUP_OBJ)
	    {
	      /* Same for the primary group. */
	      if (saw_group_obj)
		type = GROUP;
	      if (ace->Header.AceType == ACCESS_ALLOWED_ACE_TYPE)
		saw_group_obj = true;
	    }
	  if ((pos = searchace (lacl, MAX_ACL_ENTRIES, type, id)) >= 0)
	    {
	      getace (lacl[pos], type, id, ace->Mask, ace->Header.AceType,
		      new_style && type & (USER | GROUP));
	      if (!new_style)
		{
		  /* Fix up CLASS_OBJ value. */
		  if (type & (USER | GROUP))
		    {
		      has_class_perm = true;
		      class_perm |= lacl[pos].a_perm;
		    }
		}
	    }
	}
      if ((ace->Header.AceFlags & SUB_CONTAINERS_AND_OBJECTS_INHERIT))
	{
	  if (type == USER_OBJ)
	    type = USER;
	  else if (type == GROUP_OBJ)
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
		      new_style && type & (USER | GROUP));
	      if (!new_style)
		{
		  /* Fix up DEF_CLASS_OBJ value. */
		  if (type & (USER | GROUP))
		    {
		      has_def_class_perm = true;
		      def_class_perm |= lacl[pos].a_perm;
		    }
		  /* And note the position of the DEF_GROUP_OBJ entry. */
		  else if (type == DEF_GROUP_OBJ)
		    def_pgrp_pos = pos;
		}
	    }
	}
    }
  /* If this is an old-style or non-Cygwin ACL, and secondary user and group
     entries exist in the ACL, fake a matching CLASS_OBJ entry. The CLASS_OBJ
     permissions are the or'ed permissions of the primary group permissions
     and all secondary user and group permissions. */
  if (!new_style && has_class_perm
      && (pos = searchace (lacl, MAX_ACL_ENTRIES, 0)) >= 0)
    {
      lacl[pos].a_type = CLASS_OBJ;
      lacl[pos].a_id = ILLEGAL_GID;
      lacl[pos].a_perm = class_perm | lacl[1].a_perm;
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
	  pos++;
	}
      if (!(types_def & GROUP_OBJ) && pos < MAX_ACL_ENTRIES)
	{
	  lacl[pos].a_type = DEF_GROUP_OBJ;
	  lacl[pos].a_id = gid;
	  lacl[pos].a_perm = lacl[1].a_perm;
	  /* Note the position of the DEF_GROUP_OBJ entry. */
	  def_pgrp_pos = pos;
	  pos++;
	}
      if (!(types_def & OTHER_OBJ) && pos < MAX_ACL_ENTRIES)
	{
	  lacl[pos].a_type = DEF_OTHER_OBJ;
	  lacl[pos].a_id = ILLEGAL_GID;
	  lacl[pos].a_perm = lacl[2].a_perm;
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
      lacl[pos].a_id = ILLEGAL_GID;
      lacl[pos].a_perm = def_class_perm;
      if (def_pgrp_pos >= 0)
	lacl[pos].a_perm |= lacl[def_pgrp_pos].a_perm;
    }

  /* Make sure `pos' contains the number of used entries in lacl. */
  if ((pos = searchace (lacl, MAX_ACL_ENTRIES, 0)) < 0)
    pos = MAX_ACL_ENTRIES;

  /* For old-style or non-Cygwin ACLs, check for merging permissions. */
  if (!new_style)
    for (idx = 0; idx < pos; ++idx)
      {
	/* Current user?  If the user entry has a deny ACE, don't check. */
	if (lacl[idx].a_id == myself->uid
	    && lacl[idx].a_type & (USER_OBJ | USER)
	    && !(lacl[idx].a_type & ACL_DEFAULT)
	    && !(lacl[idx].a_perm & DENY_RWX))
	  {
	    int gpos;
	    gid_t grps[NGROUPS_MAX];
	    cyg_ldap cldap;

	    /* Sum up all permissions of groups the user is member of, plus
	       everyone perms, and merge them to user perms.  */
	    mode_t grp_perm = lacl[2].a_perm & S_IRWXO;
	    int gnum = internal_getgroups (NGROUPS_MAX, grps, &cldap);
	    for (int g = 0; g < gnum && grp_perm != S_IRWXO; ++g)
	      if ((gpos = 1, grps[g] == lacl[gpos].a_id)
		  || (gpos = searchace (lacl, MAX_ACL_ENTRIES, GROUP, grps[g]))
		     >= 0)
		grp_perm |= lacl[gpos].a_perm & S_IRWXO;
	    lacl[idx].a_perm |= grp_perm;
	  }
	/* For all groups, if everyone has more permissions, add everyone
	   perms to group perms.  Skip groups with deny ACE. */
	else if (lacl[idx].a_id & (GROUP_OBJ | GROUP)
		 && !(lacl[idx].a_type & ACL_DEFAULT)
		 && !(lacl[idx].a_perm & DENY_RWX))
	  lacl[idx].a_perm |= lacl[2].a_perm & S_IRWXO;
      }
  /* Construct POSIX permission bits.  Fortunately we know exactly where
     to fetch the affecting bits from, at least as long as the array
     hasn't been sorted. */
  attr |= (lacl[0].a_perm & S_IRWXO) << 6;
  attr |= (has_class_perm ? class_perm : (lacl[1].a_perm & S_IRWXO)) << 3;
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

#ifndef __x86_64__
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

extern "C" int
aclcheck32 (aclent_t *aclbufp, int nentries, int *which)
{
  bool has_user_obj = false;
  bool has_group_obj = false;
  bool has_other_obj = false;
  bool has_class_obj = false;
  bool has_ug_objs __attribute__ ((unused)) = false;
  bool has_def_objs __attribute__ ((unused)) = false;
  bool has_def_user_obj __attribute__ ((unused)) = false;
  bool has_def_group_obj = false;
  bool has_def_other_obj = false;
  bool has_def_class_obj = false;
  bool has_def_ug_objs __attribute__ ((unused)) = false;
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
	return ENTRY_ERROR;
      }
  if (!has_user_obj
      || !has_group_obj
      || !has_other_obj
      || (has_def_objs
	  && (!has_def_user_obj || !has_def_group_obj || !has_def_other_obj))
      || (has_ug_objs && !has_class_obj)
      || (has_def_ug_objs && !has_def_class_obj)
     )
    {
      if (which)
	*which = -1;
      return MISS_ERROR;
    }
  return 0;
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

extern "C" int
aclsort32 (int nentries, int, aclent_t *aclbufp)
{
  if (aclcheck32 (aclbufp, nentries, NULL))
    {
      set_errno (EINVAL);
      return -1;
    }
  if (!aclbufp || nentries < 1)
    {
      set_errno (EINVAL);
      return -1;
    }
  qsort ((void *) aclbufp, nentries, sizeof (aclent_t), acecmp);
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
permtostr (mode_t perm)
{
  static char pbuf[4];

  pbuf[0] = (perm & S_IROTH) ? 'r' : '-';
  pbuf[1] = (perm & S_IWOTH) ? 'w' : '-';
  pbuf[2] = (perm & S_IXOTH) ? 'x' : '-';
  pbuf[3] = '\0';
  return pbuf;
}

extern "C" char *
acltotext32 (aclent_t *aclbufp, int aclcnt)
{
  if (!aclbufp || aclcnt < 1 || aclcnt > MAX_ACL_ENTRIES
      || aclcheck32 (aclbufp, aclcnt, NULL))
    {
      set_errno (EINVAL);
      return NULL;
    }
  char buf[32000];
  buf[0] = '\0';
  bool first = true;

  for (int pos = 0; pos < aclcnt; ++pos)
    {
      if (!first)
	strcat (buf, ",");
      first = false;
      if (aclbufp[pos].a_type & ACL_DEFAULT)
	strcat (buf, "default");
      switch (aclbufp[pos].a_type & ~ACL_DEFAULT)
	{
	case USER_OBJ:
	  __small_sprintf (buf + strlen (buf), "user::%s",
		   permtostr (aclbufp[pos].a_perm));
	  break;
	case USER:
	  __small_sprintf (buf + strlen (buf), "user:%d:%s",
		   aclbufp[pos].a_id, permtostr (aclbufp[pos].a_perm));
	  break;
	case GROUP_OBJ:
	  __small_sprintf (buf + strlen (buf), "group::%s",
		   permtostr (aclbufp[pos].a_perm));
	  break;
	case GROUP:
	  __small_sprintf (buf + strlen (buf), "group:%d:%s",
		   aclbufp[pos].a_id, permtostr (aclbufp[pos].a_perm));
	  break;
	case CLASS_OBJ:
	  __small_sprintf (buf + strlen (buf), "mask::%s",
		   permtostr (aclbufp[pos].a_perm));
	  break;
	case OTHER_OBJ:
	  __small_sprintf (buf + strlen (buf), "other::%s",
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
  return mode;
}

extern "C" aclent_t *
aclfromtext32 (char *acltextp, int *)
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
  strcpy (buf, acltextp);
  char *lasts;
  cyg_ldap cldap;
  for (char *c = strtok_r (buf, ",", &lasts);
       c;
       c = strtok_r (NULL, ",", &lasts))
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
		  struct passwd *pw = internal_getpwnam (c, &cldap);
		  if (!pw)
		    {
		      set_errno (EINVAL);
		      return NULL;
		    }
		  lacl[pos].a_id = pw->pw_uid;
		  c = strchrnul (c, ':');
		}
	      else if (isdigit (*c))
		lacl[pos].a_id = strtol (c, &c, 10);
	      if (*c != ':')
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
		  struct group *gr = internal_getgrnam (c, &cldap);
		  if (!gr)
		    {
		      set_errno (EINVAL);
		      return NULL;
		    }
		  lacl[pos].a_id = gr->gr_gid;
		  c = strchrnul (c, ':');
		}
	      else if (isdigit (*c))
		lacl[pos].a_id = strtol (c, &c, 10);
	      if (*c != ':')
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

#ifdef __x86_64__
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
#else
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
aclfromtext (char *acltextp, int * aclcnt)
{
  return (__aclent16_t *) aclfromtext32 (acltextp, aclcnt);
}
#endif /* !__x86_64__ */
