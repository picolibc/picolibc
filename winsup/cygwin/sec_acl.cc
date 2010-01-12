/* sec_acl.cc: Sun compatible ACL functions.

   Copyright 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008,
   2009, 2010 Red Hat, Inc.

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
#include "pwdgrp.h"

static int
searchace (__aclent32_t *aclp, int nentries, int type, __uid32_t id = ILLEGAL_UID)
{
  int i;

  for (i = 0; i < nentries; ++i)
    if ((aclp[i].a_type == type && (id == ILLEGAL_UID || aclp[i].a_id == id))
	|| !aclp[i].a_type)
      return i;
  return -1;
}

int
setacl (HANDLE handle, path_conv &pc, int nentries, __aclent32_t *aclbufp,
	bool &writable)
{
  security_descriptor sd_ret;

  if (get_file_sd (handle, pc, sd_ret))
    return -1;

  BOOL dummy;

  /* Get owner SID. */
  PSID owner_sid;
  if (!GetSecurityDescriptorOwner (sd_ret, &owner_sid, &dummy))
    {
      __seterrno ();
      return -1;
    }
  cygsid owner (owner_sid);

  /* Get group SID. */
  PSID group_sid;
  if (!GetSecurityDescriptorGroup (sd_ret, &group_sid, &dummy))
    {
      __seterrno ();
      return -1;
    }
  cygsid group (group_sid);

  /* Initialize local security descriptor. */
  SECURITY_DESCRIPTOR sd;
  if (!InitializeSecurityDescriptor (&sd, SECURITY_DESCRIPTOR_REVISION))
    {
      __seterrno ();
      return -1;
    }
  if (!SetSecurityDescriptorOwner (&sd, owner, FALSE))
    {
      __seterrno ();
      return -1;
    }
  if (!SetSecurityDescriptorGroup (&sd, group, FALSE))
    {
      __seterrno ();
      return -1;
    }

  /* Fill access control list. */
  PACL acl = (PACL) alloca (3072);
  size_t acl_len = sizeof (ACL);
  int ace_off = 0;

  cygsid sid;
  struct passwd *pw;
  struct __group32 *gr;
  int pos;

  if (!InitializeAcl (acl, 3072, ACL_REVISION))
    {
      __seterrno ();
      return -1;
    }

  writable = false;

  for (int i = 0; i < nentries; ++i)
    {
      DWORD allow;
      /* Owner has more standard rights set. */
      if ((aclbufp[i].a_type & ~ACL_DEFAULT) == USER_OBJ)
	allow = STANDARD_RIGHTS_ALL
		| (pc.fs_is_samba ()
		   ? 0 : (FILE_READ_ATTRIBUTES | FILE_WRITE_ATTRIBUTES));
      else
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
      if ((aclbufp[i].a_perm & (S_IWOTH | S_IXOTH)) == (S_IWOTH | S_IXOTH))
	allow |= FILE_DELETE_CHILD;
      /* Set inherit property. */
      DWORD inheritance = (aclbufp[i].a_type & ACL_DEFAULT)
			  ? (CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE
			     | INHERIT_ONLY_ACE)
			  : NO_INHERITANCE;
      /*
       * If a specific acl contains a corresponding default entry with
       * identical permissions, only one Windows ACE with proper
       * inheritance bits is created.
       */
      if (!(aclbufp[i].a_type & ACL_DEFAULT)
	  && aclbufp[i].a_type & (USER|GROUP|OTHER_OBJ)
	  && (pos = searchace (aclbufp + i + 1, nentries - i - 1,
			       aclbufp[i].a_type | ACL_DEFAULT,
			       (aclbufp[i].a_type & (USER|GROUP))
			       ? aclbufp[i].a_id : ILLEGAL_UID)) >= 0
	  && aclbufp[i].a_perm == aclbufp[i + 1 + pos].a_perm)
	{
	  inheritance = CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE;
	  /* This invalidates the corresponding default entry. */
	  aclbufp[i + 1 + pos].a_type = USER|GROUP|ACL_DEFAULT;
	}
      switch (aclbufp[i].a_type)
	{
	case USER_OBJ:
	  if (!add_access_allowed_ace (acl, ace_off++, allow,
					owner, acl_len, inheritance))
	    return -1;
	  break;
	case DEF_USER_OBJ:
	  if (!add_access_allowed_ace (acl, ace_off++, allow,
				       well_known_creator_owner_sid, acl_len, inheritance))
	    return -1;
	  break;
	case USER:
	case DEF_USER:
	  if (!(pw = internal_getpwuid (aclbufp[i].a_id))
	      || !sid.getfrompw (pw))
	    {
	      set_errno (EINVAL);
	      return -1;
	    }
	  if (!add_access_allowed_ace (acl, ace_off++, allow,
				       sid, acl_len, inheritance))
	    return -1;
	  break;
	case GROUP_OBJ:
	  if (!add_access_allowed_ace (acl, ace_off++, allow,
				       group, acl_len, inheritance))
	    return -1;
	  break;
	case DEF_GROUP_OBJ:
	  if (!add_access_allowed_ace (acl, ace_off++, allow,
				       well_known_creator_group_sid, acl_len, inheritance))
	    return -1;
	  break;
	case GROUP:
	case DEF_GROUP:
	  if (!(gr = internal_getgrgid (aclbufp[i].a_id))
	      || !sid.getfromgr (gr))
	    {
	      set_errno (EINVAL);
	      return -1;
	    }
	  if (!add_access_allowed_ace (acl, ace_off++, allow,
				       sid, acl_len, inheritance))
	    return -1;
	  break;
	case OTHER_OBJ:
	case DEF_OTHER_OBJ:
	  if (!add_access_allowed_ace (acl, ace_off++, allow,
				       well_known_world_sid,
				       acl_len, inheritance))
	    return -1;
	  break;
	}
    }
  /* Set AclSize to computed value. */
  acl->AclSize = acl_len;
  debug_printf ("ACL-Size: %d", acl_len);
  /* Create DACL for local security descriptor. */
  if (!SetSecurityDescriptorDacl (&sd, TRUE, acl, FALSE))
    {
      __seterrno ();
      return -1;
    }
  /* Make self relative security descriptor in sd_ret. */
  DWORD sd_size = 0;
  MakeSelfRelativeSD (&sd, sd_ret, &sd_size);
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
  if (!MakeSelfRelativeSD (&sd, sd_ret, &sd_size))
    {
      __seterrno ();
      return -1;
    }
  debug_printf ("Created SD-Size: %d", sd_ret.size ());
  return set_file_sd (handle, pc, sd_ret, false);
}

/* Temporary access denied bits */
#define DENY_R 040000
#define DENY_W 020000
#define DENY_X 010000

static void
getace (__aclent32_t &acl, int type, int id, DWORD win_ace_mask,
	DWORD win_ace_type)
{
  acl.a_type = type;
  acl.a_id = id;

  if ((win_ace_mask & FILE_READ_BITS) && !(acl.a_perm & (S_IROTH | DENY_R)))
    {
      if (win_ace_type == ACCESS_ALLOWED_ACE_TYPE)
	acl.a_perm |= S_IROTH;
      else if (win_ace_type == ACCESS_DENIED_ACE_TYPE)
	acl.a_perm |= DENY_R;
    }

  if ((win_ace_mask & FILE_WRITE_BITS) && !(acl.a_perm & (S_IWOTH | DENY_W)))
    {
      if (win_ace_type == ACCESS_ALLOWED_ACE_TYPE)
	acl.a_perm |= S_IWOTH;
      else if (win_ace_type == ACCESS_DENIED_ACE_TYPE)
	acl.a_perm |= DENY_W;
    }

  if ((win_ace_mask & FILE_EXEC_BITS) && !(acl.a_perm & (S_IXOTH | DENY_X)))
    {
      if (win_ace_type == ACCESS_ALLOWED_ACE_TYPE)
	acl.a_perm |= S_IXOTH;
      else if (win_ace_type == ACCESS_DENIED_ACE_TYPE)
	acl.a_perm |= DENY_X;
    }
}

int
getacl (HANDLE handle, path_conv &pc, int nentries, __aclent32_t *aclbufp)
{
  security_descriptor sd;

  if (get_file_sd (handle, pc, sd))
    return -1;

  cygpsid owner_sid;
  cygpsid group_sid;
  BOOL dummy;
  __uid32_t uid;
  __gid32_t gid;

  if (!GetSecurityDescriptorOwner (sd, (PSID *) &owner_sid, &dummy))
    {
      debug_printf ("GetSecurityDescriptorOwner %E");
      __seterrno ();
      return -1;
    }
  uid = owner_sid.get_uid ();

  if (!GetSecurityDescriptorGroup (sd, (PSID *) &group_sid, &dummy))
    {
      debug_printf ("GetSecurityDescriptorGroup %E");
      __seterrno ();
      return -1;
    }
  gid = group_sid.get_gid ();

  __aclent32_t lacl[MAX_ACL_ENTRIES];
  memset (&lacl, 0, MAX_ACL_ENTRIES * sizeof (__aclent32_t));
  lacl[0].a_type = USER_OBJ;
  lacl[0].a_id = uid;
  lacl[1].a_type = GROUP_OBJ;
  lacl[1].a_id = gid;
  lacl[2].a_type = OTHER_OBJ;
  lacl[2].a_id = ILLEGAL_GID;
  lacl[3].a_type = CLASS_OBJ;
  lacl[3].a_id = ILLEGAL_GID;
  lacl[3].a_perm = S_IROTH | S_IWOTH | S_IXOTH;

  PACL acl;
  BOOL acl_exists;

  if (!GetSecurityDescriptorDacl (sd, &acl_exists, &acl, &dummy))
    {
      __seterrno ();
      debug_printf ("GetSecurityDescriptorDacl %E");
      return -1;
    }

  int pos, i, types_def = 0;

  if (!acl_exists || !acl)
    for (pos = 0; pos < 3; ++pos) /* Don't change CLASS_OBJ entry */
      lacl[pos].a_perm = S_IROTH | S_IWOTH | S_IXOTH;
  else
    {
      for (i = 0; i < acl->AceCount; ++i)
	{
	  ACCESS_ALLOWED_ACE *ace;

	  if (!GetAce (acl, i, (PVOID *) &ace))
	    continue;

	  cygpsid ace_sid ((PSID) &ace->SidStart);
	  int id;
	  int type = 0;

	  if (ace_sid == well_known_world_sid)
	    {
	      type = OTHER_OBJ;
	      id = ILLEGAL_GID;
	    }
	  else if (ace_sid == group_sid)
	    {
	      type = GROUP_OBJ;
	      id = gid;
	    }
	  else if (ace_sid == owner_sid)
	    {
	      type = USER_OBJ;
	      id = uid;
	    }
	  else if (ace_sid == well_known_creator_group_sid)
	    {
	      type = GROUP_OBJ | ACL_DEFAULT;
	      id = ILLEGAL_GID;
	    }
	  else if (ace_sid == well_known_creator_owner_sid)
	    {
	      type = USER_OBJ | ACL_DEFAULT;
	      id = ILLEGAL_GID;
	    }
	  else
	    id = ace_sid.get_id (true, &type);

	  if (!type)
	    continue;
	  if (!(ace->Header.AceFlags & INHERIT_ONLY_ACE || type & ACL_DEFAULT))
	    {
	      if ((pos = searchace (lacl, MAX_ACL_ENTRIES, type, id)) >= 0)
		getace (lacl[pos], type, id, ace->Mask, ace->Header.AceType);
	    }
	  if ((ace->Header.AceFlags
	      & (CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE))
	      && pc.isdir ())
	    {
	      if (type == USER_OBJ)
		type = USER;
	      else if (type == GROUP_OBJ)
		type = GROUP;
	      type |= ACL_DEFAULT;
	      types_def |= type;
	      if ((pos = searchace (lacl, MAX_ACL_ENTRIES, type, id)) >= 0)
		getace (lacl[pos], type, id, ace->Mask, ace->Header.AceType);
	    }
	}
      /* Include DEF_CLASS_OBJ if any default ace exists */
      if ((types_def & (USER|GROUP))
	  && ((pos = searchace (lacl, MAX_ACL_ENTRIES, DEF_CLASS_OBJ)) >= 0))
	{
	  lacl[pos].a_type = DEF_CLASS_OBJ;
	  lacl[pos].a_id = ILLEGAL_GID;
	  lacl[pos].a_perm = S_IRWXU | S_IRWXG | S_IRWXO;
	}
    }
  if ((pos = searchace (lacl, MAX_ACL_ENTRIES, 0)) < 0)
    pos = MAX_ACL_ENTRIES;
  if (aclbufp) {
    if (owner_sid == group_sid)
      lacl[0].a_perm = lacl[1].a_perm;
    if (pos > nentries)
      {
	set_errno (ENOSPC);
	return -1;
      }
    memcpy (aclbufp, lacl, pos * sizeof (__aclent32_t));
    for (i = 0; i < pos; ++i)
      aclbufp[i].a_perm &= ~(DENY_R | DENY_W | DENY_X);
    aclsort32 (pos, 0, aclbufp);
  }
  syscall_printf ("%d = getacl (%S)", pos, pc.get_nt_native_path ());
  return pos;
}

static int
acl_worker (const char *path, int cmd, int nentries, __aclent32_t *aclbufp,
	    unsigned fmode)
{
  int res = -1;
  fhandler_base *fh = build_fh_name (path, fmode, stat_suffixes);
  if (fh->error ())
    {
      debug_printf ("got %d error from build_fh_name", fh->error ());
      set_errno (fh->error ());
    }
  else if (!fh->exists ())
    set_errno (ENOENT);
  else
    res = fh->facl (cmd, nentries, aclbufp);

  delete fh;
  syscall_printf ("%d = acl (%s)", res, path);
  return res;
}

extern "C" int
acl32 (const char *path, int cmd, int nentries, __aclent32_t *aclbufp)
{
  return acl_worker (path, cmd, nentries, aclbufp, PC_SYM_FOLLOW);
}

extern "C" int
lacl32 (const char *path, int cmd, int nentries, __aclent32_t *aclbufp)
{
  return acl_worker (path, cmd, nentries, aclbufp, PC_SYM_NOFOLLOW);
}

extern "C" int
facl32 (int fd, int cmd, int nentries, __aclent32_t *aclbufp)
{
  cygheap_fdget cfd (fd);
  if (cfd < 0)
    {
      syscall_printf ("-1 = facl (%d)", fd);
      return -1;
    }
  int res = cfd->facl (cmd, nentries, aclbufp);
  syscall_printf ("%d = facl (%s) )", res, cfd->get_name ());
  return res;
}

extern "C" int
aclcheck32 (__aclent32_t *aclbufp, int nentries, int *which)
{
  bool has_user_obj = false;
  bool has_group_obj = false;
  bool has_other_obj = false;
  bool has_class_obj = false;
  bool has_ug_objs = false;
  bool has_def_user_obj = false;
  bool has_def_group_obj = false;
  bool has_def_other_obj = false;
  bool has_def_class_obj = false;
  bool has_def_ug_objs = false;
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
	has_def_user_obj = true;
	break;
      case DEF_GROUP_OBJ:
	if (has_def_group_obj)
	  {
	    if (which)
	      *which = pos;
	    return GRP_ERROR;
	  }
	has_def_group_obj = true;
	break;
      case DEF_OTHER_OBJ:
	if (has_def_other_obj)
	  {
	    if (which)
	      *which = pos;
	    return OTHER_ERROR;
	  }
	has_def_other_obj = true;
	break;
      case DEF_CLASS_OBJ:
	if (has_def_class_obj)
	  {
	    if (which)
	      *which = pos;
	    return CLASS_ERROR;
	  }
	has_def_class_obj = true;
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
	has_def_ug_objs = true;
	break;
      default:
	return ENTRY_ERROR;
      }
  if (!has_user_obj
      || !has_group_obj
      || !has_other_obj
#if 0
      /* These checks are not ok yet since CLASS_OBJ isn't fully implemented. */
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

static int
acecmp (const void *a1, const void *a2)
{
#define ace(i) ((const __aclent32_t *) a##i)
  int ret = ace (1)->a_type - ace (2)->a_type;
  if (!ret)
    ret = ace (1)->a_id - ace (2)->a_id;
  return ret;
#undef ace
}

extern "C" int
aclsort32 (int nentries, int, __aclent32_t *aclbufp)
{
  if (aclcheck32 (aclbufp, nentries, NULL))
    return -1;
  if (!aclbufp || nentries < 1)
    {
      set_errno (EINVAL);
      return -1;
    }
  qsort ((void *) aclbufp, nentries, sizeof (__aclent32_t), acecmp);
  return 0;
}

extern "C" int
acltomode32 (__aclent32_t *aclbufp, int nentries, mode_t *modep)
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
aclfrommode32 (__aclent32_t *aclbufp, int nentries, mode_t *modep)
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
acltopbits32 (__aclent32_t *aclbufp, int nentries, mode_t *pbitsp)
{
  return acltomode32 (aclbufp, nentries, pbitsp);
}

extern "C" int
aclfrompbits32 (__aclent32_t *aclbufp, int nentries, mode_t *pbitsp)
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
acltotext32 (__aclent32_t *aclbufp, int aclcnt)
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

extern "C" __aclent32_t *
aclfromtext32 (char *acltextp, int *)
{
  if (!acltextp)
    {
      set_errno (EINVAL);
      return NULL;
    }
  char buf[strlen (acltextp) + 1];
  __aclent32_t lacl[MAX_ACL_ENTRIES];
  memset (lacl, 0, sizeof lacl);
  int pos = 0;
  strcpy (buf, acltextp);
  char *lasts;
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
		  struct passwd *pw = internal_getpwnam (c);
		  if (!pw)
		    {
		      set_errno (EINVAL);
		      return NULL;
		    }
		  lacl[pos].a_id = pw->pw_uid;
		  c = strechr (c, ':');
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
		  struct __group32 *gr = internal_getgrnam (c);
		  if (!gr)
		    {
		      set_errno (EINVAL);
		      return NULL;
		    }
		  lacl[pos].a_id = gr->gr_gid;
		  c = strechr (c, ':');
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
  __aclent32_t *aclp = (__aclent32_t *) malloc (pos * sizeof (__aclent32_t));
  if (aclp)
    memcpy (aclp, lacl, pos * sizeof (__aclent32_t));
  return aclp;
}

/* __aclent16_t and __aclent32_t have same size and same member offsets */
static __aclent32_t *
acl16to32 (__aclent16_t *aclbufp, int nentries)
{
  __aclent32_t *aclbufp32 = (__aclent32_t *) aclbufp;
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
  return lacl32 (path, cmd, nentries, acl16to32 (aclbufp, nentries));
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
  return aclfrommode32 ((__aclent32_t *)aclbufp, nentries, modep);
}

extern "C" int
acltopbits (__aclent16_t *aclbufp, int nentries, mode_t *pbitsp)
{
  return acltopbits32 (acl16to32 (aclbufp, nentries), nentries, pbitsp);
}

extern "C" int
aclfrompbits (__aclent16_t *aclbufp, int nentries, mode_t *pbitsp)
{
  return aclfrompbits32 ((__aclent32_t *)aclbufp, nentries, pbitsp);
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
