/* secacl.cc: Sun compatible ACL functions.

   Copyright 2000, 2001 Cygnus Solutions.

   Written by Corinna Vinschen <corinna@vinschen.de>

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

  /* Get owner SID. */
  PSID owner_sid = NULL;
  if (!GetSecurityDescriptorOwner (psd, &owner_sid, &dummy))
    {
      __seterrno ();
      return -1;
    }
  cygsid owner (owner_sid);

  /* Get group SID. */
  PSID group_sid = NULL;
  if (!GetSecurityDescriptorGroup (psd, &group_sid, &dummy))
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
  if (!SetSecurityDescriptorOwner(&sd, owner, FALSE))
    {
      __seterrno ();
      return -1;
    }
  if (group
      && !SetSecurityDescriptorGroup(&sd, group, FALSE))
    {
      __seterrno ();
      return -1;
    }

  /* Fill access control list. */
  char acl_buf[3072];
  PACL acl = (PACL) acl_buf;
  size_t acl_len = sizeof (ACL);
  int ace_off = 0;

  cygsid sid;
  struct passwd *pw;
  struct group *gr;
  int pos;

  if (!InitializeAcl (acl, 3072, ACL_REVISION))
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
      /* Set inherit property. */
      DWORD inheritance = (aclbufp[i].a_type & ACL_DEFAULT)
			  ? INHERIT_ONLY : DONT_INHERIT;
      /*
       * If a specific acl contains a corresponding default entry with
       * identical permissions, only one Windows ACE with proper
       * inheritance bits is created.
       */
      if (!(aclbufp[i].a_type & ACL_DEFAULT)
	  && (pos = searchace (aclbufp, nentries,
			       aclbufp[i].a_type | ACL_DEFAULT,
			       (aclbufp[i].a_type & (USER|GROUP))
			       ? aclbufp[i].a_id : -1)) >= 0
	  && pos < nentries
	  && aclbufp[i].a_perm == aclbufp[pos].a_perm)
	{
	  inheritance = INHERIT_ALL;
	  /* This eliminates the corresponding default entry. */
	  aclbufp[pos].a_type = 0;
	}
      switch (aclbufp[i].a_type)
	{
	case USER_OBJ:
	case DEF_USER_OBJ:
	  allow |= STANDARD_RIGHTS_ALL & ~DELETE;
	  if (!add_access_allowed_ace (acl, ace_off++, allow,
					owner, acl_len, inheritance))
	    return -1;
	  break;
	case USER:
	case DEF_USER:
	  if (!(pw = getpwuid (aclbufp[i].a_id))
	      || !sid.getfrompw (pw)
	      || !add_access_allowed_ace (acl, ace_off++, allow,
					   sid, acl_len, inheritance))
	    return -1;
	  break;
	case GROUP_OBJ:
	case DEF_GROUP_OBJ:
	  if (!add_access_allowed_ace (acl, ace_off++, allow,
					group, acl_len, inheritance))
	    return -1;
	  break;
	case GROUP:
	case DEF_GROUP:
	  if (!(gr = getgrgid (aclbufp[i].a_id))
	      || !sid.getfromgr (gr)
	      || !add_access_allowed_ace (acl, ace_off++, allow,
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
  /* Make self relative security descriptor in psd. */
  sd_size = 0;
  MakeSelfRelativeSD (&sd, psd, &sd_size);
  if (sd_size <= 0)
    {
      __seterrno ();
      return -1;
    }
  if (!MakeSelfRelativeSD (&sd, psd, &sd_size))
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

  if (!GetSecurityDescriptorOwner (psd, &owner_sid, &dummy))
    {
      debug_printf ("GetSecurityDescriptorOwner %E");
      __seterrno ();
      return -1;
    }
  uid = cygsid (owner_sid).get_uid ();

  if (!GetSecurityDescriptorGroup (psd, &group_sid, &dummy))
    {
      debug_printf ("GetSecurityDescriptorGroup %E");
      __seterrno ();
      return -1;
    }
  gid = cygsid (group_sid).get_gid ();

  aclent_t lacl[MAX_ACL_ENTRIES];
  memset (&lacl, 0, MAX_ACL_ENTRIES * sizeof (aclent_t));
  lacl[0].a_type = USER_OBJ;
  lacl[0].a_id = uid;
  lacl[1].a_type = GROUP_OBJ;
  lacl[1].a_id = gid;
  lacl[2].a_type = OTHER_OBJ;

  PACL acl;
  BOOL acl_exists;

  if (!GetSecurityDescriptorDacl (psd, &acl_exists, &acl, &dummy))
    {
      __seterrno ();
      debug_printf ("GetSecurityDescriptorDacl %E");
      return -1;
    }

  int pos, i;

  if (!acl_exists || !acl)
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

      cygsid ace_sid ((PSID) &ace->SidStart);
      int id;
      int type = 0;

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
	  id = 0;
	}
      else
	{
	  id = ace_sid.get_id (FALSE, &type);
	  if (type != GROUP)
	    {
	      int type2 = 0;
	      int id2 = ace_sid.get_id (TRUE, &type2);
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

  /* Only check existance. */
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
	      /*
	       * Check if user is a NT group:
	       * Take SID from passwd, search SID in group, check is_grp_member.
	       */
	      cygsid owner;
	      cygsid group;
	      struct passwd *pw;
	      struct group *gr = NULL;

	      if ((pw = getpwuid (acls[i].a_id)) != NULL
		  && owner.getfrompw (pw))
		{
		  for (int gidx = 0; (gr = internal_getgrent (gidx)); ++gidx)
		    if (group.getfromgr (gr)
			&& owner == group
			&& is_grp_member (myself->uid, gr->gr_gid))
		      break;
	        }
	      if (!gr)
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
	  else if ((nofollow && !lstat (path, &st))
		   || (!nofollow && !stat (path, &st)))
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
  if (cygheap->fdtab.not_open (fd))
    {
      syscall_printf ("-1 = facl (%d)", fd);
      set_errno (EBADF);
      return -1;
    }
  const char *path = cygheap->fdtab[fd]->get_name ();
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

  if (!aclbufp || nentries < 1 || !modep)
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

  if (!aclbufp || nentries < 1 || !modep)
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

