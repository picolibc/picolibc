/* sec_posixacl.cc: POSIX ACL functions based on Solaris ACLs.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <unistd.h>
#include "cygerrno.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include "tls_pbuf.h"
#include "sec_posixacl.h"
#include <acl/libacl.h>

#define _ENTRY_SIZE(_cnt) ((_cnt) * sizeof (aclent_t))
#define _ACL_SIZE(_cnt)	  (sizeof (__acl_ext_t) + _ENTRY_SIZE (_cnt))
#define ACL_SIZE(_acl)    ({ acl_t __acl = _acl; \
			     _ACL_SIZE((__acl)->count - (__acl)->deleted); \
			  })
#define ACL_PERM_MASK		(ACL_READ | ACL_WRITE | ACL_EXECUTE)

extern "C" acl_t
acl_init (int count)
{
  acl_t acl;

  if (count < 0 || count > UINT16_MAX)
    {
      set_errno (EINVAL);
      return NULL;
    }
  acl = (acl_t) calloc (1, sizeof (__acl_t));
  if (!acl)
    return NULL;
  acl->entry = (aclent_t *) calloc (count, sizeof (aclent_t));
  if (!acl->entry)
    {
      free (acl);
      return NULL;
    }
  acl->magic = ACL_MAGIC;
  acl->max_count = count;
  return acl;
}

extern "C" acl_t
acl_dup (acl_t acl)
{
  __try
    {
      acl_t new_acl = acl_init (acl->max_count);
      if (new_acl)
	{
	  uint16_t new_idx = 0;

	  for (uint16_t idx = 0; idx < acl->count; ++idx)
	    if (acl->entry[idx].a_type != ACL_DELETED_TAG)
	      new_acl->entry[new_idx++] = acl->entry[idx];
	  new_acl->magic = ACL_MAGIC;
	  new_acl->count = new_idx;
	  return new_acl;
	}
    }
  __except (EINVAL) {}
  __endtry
  return NULL;
}

extern "C" int
acl_free (void *obj_p)
{
  __try
    {
      acl_t acl;

      if (obj_p)
	{
	  if (malloc_usable_size (obj_p) >= sizeof (__acl_t))
	    {
	      acl = (acl_t) obj_p;
	      if (acl->magic == ACL_MAGIC)
		free (acl->entry);
	    }
	  free (obj_p);
	  return 0;
	}
      set_errno (EINVAL);
    }
  __except (EINVAL) {}
  __endtry
  return -1;
}

extern "C" int
acl_valid (acl_t acl)
{
  __try
    {
      if (!(__aclcheck (acl->entry, acl->count, NULL, true)))
	return 0;
      set_errno (EINVAL);
    }
  __except (EINVAL) {}
  __endtry
  return -1;
}

extern "C" int
acl_copy_entry (acl_entry_t dest_d, acl_entry_t src_d)
{
  __try
    {
      uint16_t d_idx, s_idx;
      acl_t d_acl, s_acl;

      d_acl = __from_entry (dest_d, d_idx);
      s_acl = __from_entry (src_d, s_idx);
      if (d_acl && s_acl)
	{
	  d_acl->entry[d_idx] = s_acl->entry[s_idx];
	  return 0;
	}
      set_errno (EINVAL);
    }
  __except (EINVAL) {}
  __endtry
  return -1;
}

extern "C" int
acl_create_entry (acl_t *acl_p, acl_entry_t *entry_p)
{
  __try
    {
      acl_t acl = *acl_p;
      uint16_t idx;

      if (acl->deleted > 0)
	{
	  for (idx = 0; idx < acl->count; ++idx)
	    if (acl->entry[idx].a_type == ACL_DELETED_TAG)
	      {
		*entry_p = __to_entry (acl, idx);
		--acl->deleted;
		goto fill_entry;
	      }
	}
      if (acl->count >= acl->max_count)
	{
	  aclent_t *new_e;

	  new_e = (aclent_t *) realloc (acl->entry,
					_ENTRY_SIZE (acl->max_count + 1));
	  if (!new_e)
	    __leave;
	  acl->entry = new_e;
	  ++acl->max_count;
	}
      idx = acl->count++;
      *entry_p = __to_entry (acl, idx);
    fill_entry:
      acl->entry[idx].a_type = ACL_UNDEFINED_TAG;
      acl->entry[idx].a_id = ACL_UNDEFINED_ID;
      acl->entry[idx].a_perm = 0;
      return 0;
    }
  __except (EINVAL) {}
  __endtry
  return -1;
}

extern "C" int
acl_delete_entry (acl_t acl, acl_entry_t entry_d)
{
  __try
    {
      acl_t acl_p;
      uint16_t idx;

      acl_p = __from_entry (entry_d, idx);

      if (acl_p == acl)
	{
	  acl_p->entry[idx].a_type = ACL_DELETED_TAG;
	  acl_p->entry[idx].a_id = ACL_UNDEFINED_ID;
	  acl_p->entry[idx].a_perm = 0;
	  return 0;
	}
      set_errno (EINVAL);
    }
  __except (EINVAL) {}
  __endtry
  return -1;
}

extern "C" int
acl_get_entry (acl_t acl, int entry_id, acl_entry_t *entry_p)
{
  __try
    {
      uint16_t idx;

      if (entry_id == ACL_FIRST_ENTRY)
	acl->next = 0;
      else if (entry_id != ACL_NEXT_ENTRY)
	{
	  set_errno (EINVAL);
	  __leave;
	}
      do
	{
	  if (acl->next >= acl->count)
	    return 0;
	  idx = acl->next++;
	}
      while (acl->entry[idx].a_type == ACL_DELETED_TAG);
      *entry_p = __to_entry (acl, idx);
      return 1;
    }
  __except (EINVAL) {}
  __endtry
  return -1;
}

extern "C" int
acl_calc_mask (acl_t *acl_p)
{
  __try
    {
      acl_t acl = *acl_p;
      mode_t mask = 0;

      mask = __aclcalcmask (acl->entry, acl->count);
      /* If __aclcalcmask returns -1 we're done.  Otherwise create a
         mask entry here. */
      if (mask != (acl_perm_t) -1)
	{
	  acl_entry_t entry_d;
	  uint16_t mask_idx;

	  if (acl_create_entry (&acl, &entry_d) < 0)
	    __leave;
	  if (!__from_entry (entry_d, mask_idx))
	    {
	      set_errno (EINVAL);
	      __leave;
	    }
	  acl->entry[mask_idx].a_type = ACL_MASK;
	  acl->entry[mask_idx].a_id = ACL_UNDEFINED_ID;
	  acl->entry[mask_idx].a_perm = mask;
	  *acl_p = acl;
	}
      return 0;
    }
  __except (EINVAL) {}
  __endtry
  return -1;
}

extern "C" int
acl_clear_perms (acl_permset_t permset_d)
{
  __try
    {
      acl_t acl;
      uint16_t idx;

      acl = __from_permset (permset_d, idx);
      if (acl)
	{
	  acl->entry[idx].a_perm = 0;
	  return 0;
	}
      set_errno (EINVAL);
    }
  __except (EINVAL) {}
  __endtry
  return -1;
}

extern "C" int
acl_add_perm (acl_permset_t permset_d, acl_perm_t perm)
{
  __try
    {
      acl_t acl;
      uint16_t idx;

      acl = __from_permset (permset_d, idx);
      if (acl && !(perm & ~ACL_PERM_MASK))
	{
	  acl->entry[idx].a_perm |= perm;
	  return 0;
	}
      set_errno (EINVAL);
    }
  __except (EINVAL) {}
  __endtry
  return -1;
}

extern "C" int
acl_delete_perm (acl_permset_t permset_d, acl_perm_t perm)
{
  __try
    {
      acl_t acl;
      uint16_t idx;

      acl = __from_permset (permset_d, idx);
      if (acl && !(perm & ~ACL_PERM_MASK))
	{
	  acl->entry[idx].a_perm &= ~perm;
	  return 0;
	}
      set_errno (EINVAL);
    }
  __except (EINVAL) {}
  __endtry
      return -1;
}

extern "C" int
acl_get_permset (acl_entry_t entry_d, acl_permset_t *permset_p)
{
  __try
    {
      acl_t acl;
      uint16_t idx;

      acl = __from_entry (entry_d, idx);
      if (acl)
	{
	  *permset_p = (acl_permset_t) entry_d;
	  return 0;
	}
      set_errno (EINVAL);
    }
  __except (EINVAL) {}
  __endtry
  return -1;
}

extern "C" int
acl_set_permset (acl_entry_t entry_d, acl_permset_t permset_d)
{
  __try
    {
      acl_t acl_e, acl_p;
      uint16_t idx_e, idx_p;

      acl_e = __from_entry (entry_d, idx_e);
      acl_p = __from_permset (permset_d, idx_p);
      if (acl_e && acl_p && !(acl_p->entry[idx_p].a_perm & ~ACL_PERM_MASK))
	{
	  acl_e->entry[idx_e].a_perm = acl_p->entry[idx_p].a_perm;
	  return 0;
	}
      set_errno (EINVAL);
    }
  __except (EINVAL) {}
  __endtry
  return -1;
}

extern "C" void *
acl_get_qualifier (acl_entry_t entry_d)
{
  __try
    {
      acl_t acl;
      uint16_t idx;

      acl = __from_entry (entry_d, idx);
      if (acl && (acl->entry[idx].a_type & (ACL_USER | ACL_GROUP)))
	{
	  id_t *id = (id_t *) malloc (sizeof (id_t));
	  if (id)
	    {
	      *id = acl->entry[idx].a_id;
	      return (void *) id;
	    }
	}
      else
	set_errno (EINVAL);
    }
  __except (EINVAL) {}
  __endtry
  return NULL;
}

extern "C" int
acl_set_qualifier (acl_entry_t entry_d, const void *qualifier_p)
{
  __try
    {
      acl_t acl;
      uint16_t idx;

      acl = __from_entry (entry_d, idx);
      if (acl && (acl->entry[idx].a_type & (ACL_USER | ACL_GROUP)))
	{
	  acl->entry[idx].a_id = *(id_t *) qualifier_p;
	  return 0;
	}
      set_errno (EINVAL);
    }
  __except (EINVAL) {}
  __endtry
  return -1;
}

extern "C" int
acl_get_tag_type (acl_entry_t entry_d, acl_tag_t *tag_type_p)
{
  __try
    {
      acl_t acl;
      uint16_t idx;

      acl = __from_entry (entry_d, idx);
      if (acl)
	{
	  *tag_type_p = acl->entry[idx].a_type;
	  return 0;
	}
      set_errno (EINVAL);
    }
  __except (EINVAL) {}
  __endtry
  return -1;
}

extern "C" int
acl_set_tag_type (acl_entry_t entry_d, acl_tag_t tag_type)
{
  __try
    {
      acl_t acl;
      uint16_t idx;

      acl = __from_entry (entry_d, idx);
      if (acl)
	switch (tag_type)
	  {
	  case ACL_USER_OBJ:
	  case ACL_GROUP_OBJ:
	  case ACL_MASK:
	  case ACL_OTHER:
	    acl->entry[idx].a_id = ACL_UNDEFINED_ID;
	    /*FALLTHRU*/
	  case ACL_USER:
	  case ACL_GROUP:
	    acl->entry[idx].a_type = tag_type;
	    return 0;
	  default:
	    break;
	  }
      set_errno (EINVAL);
    }
  __except (EINVAL) {}
  __endtry
  return -1;
}

extern "C" ssize_t
acl_size (acl_t acl)
{
  __try
    {
      return (ssize_t) ACL_SIZE (acl);
    }
  __except (EINVAL) {}
  __endtry
  return -1;
}

extern "C" ssize_t
acl_copy_ext (void *buf_p, acl_t acl, ssize_t size)
{
  __try
    {
      ssize_t ext_size = (ssize_t) ACL_SIZE (acl);

      if (size <= 0)
	set_errno (EINVAL);
      else if (ext_size > size)
	set_errno (ERANGE);
      else
	{
	  uint16_t ext_idx = 0;
	  __acl_ext_t *acl_ext = (__acl_ext_t *) buf_p;

	  acl_ext->count = acl->count - acl->deleted;
	  for (uint16_t idx = 0; idx < acl->count; ++idx)
	    if (acl->entry[idx].a_type != ACL_DELETED_TAG)
	      acl_ext->entry[ext_idx++] = acl->entry[idx];
	  return ext_size;
	}
    }
  __except (EINVAL) {}
  __endtry
  return -1;
}

extern "C" acl_t
acl_copy_int (const void *buf_p)
{
  __try
    {
      acl_t acl;
      __acl_ext_t *acl_ext = (__acl_ext_t *) buf_p;

      acl = acl_init (acl_ext->count);
      if (acl)
	{
	  memcpy (acl->entry, acl_ext->entry, _ENTRY_SIZE (acl_ext->count));
	  acl->count = acl_ext->count;
	  return acl;
	}
    }
  __except (EINVAL) {}
  __endtry
  return NULL;
}

extern "C" acl_t
acl_from_text (const char *buf_p)
{
  __try
    {
      return (acl_t) __aclfromtext (buf_p, NULL, true);
    }
  __except (EINVAL) {}
  __endtry
  return NULL;
}

extern "C" char *
acl_to_text (acl_t acl, ssize_t *len_p)
{
  __try
    {
      char *ret = __acltotext (acl->entry, acl->count, NULL, '\n',
			       TEXT_IS_POSIX
			       | TEXT_SOME_EFFECTIVE
			       | TEXT_END_SEPARATOR);
      if (ret && len_p)
	*len_p = strlen (ret);
      return ret;
    }
  __except (EINVAL) {}
  __endtry
  return NULL;
}

acl_t __reg2
fhandler_base::acl_get (acl_type_t type)
{
  set_errno (ENOTSUP);
  return NULL;
}

acl_t __reg2
fhandler_disk_file::acl_get (acl_type_t type)
{
  acl_t acl = NULL;
  int oret = 0;

  __try
    {
      tmp_pathbuf tp;
      aclent_t *aclbufp;
      uint16_t cnt, access_cnt;

      if (!pc.has_acls ())
	{
	  set_errno (ENOTSUP);
	  __leave;
	}
      if (type == ACL_TYPE_DEFAULT && !pc.isdir ())
	{
	  set_errno (ENOTDIR);
	  __leave;
	}
      aclbufp = (aclent_t *) tp.c_get ();
      if (!get_handle ())
	{
	  query_open (query_read_control);
	  if (!(oret = open (O_BINARY, 0)))
	    __leave;
	}
      cnt = facl (GETACL, MAX_ACL_ENTRIES, aclbufp);
      if (cnt < 0)
	__leave;
      /* Set access_cnt to number of non-default entries from file ACL. */
      if (!pc.isdir ())
	access_cnt = cnt;
      else
	for (access_cnt = 0; access_cnt < cnt; ++access_cnt)
	  if (aclbufp[access_cnt].a_type & ACL_DEFAULT)
	    break;
      if (type == ACL_TYPE_ACCESS)
	{
	  acl = acl_init (access_cnt);
	  if (!acl)
	    __leave;
	  memcpy (acl->entry, aclbufp, _ENTRY_SIZE (access_cnt));
	  acl->count = access_cnt;
	}
      else
	{
	  cnt -= access_cnt;
	  acl = acl_init (cnt);
	  if (acl && cnt)
	    {
	      memcpy (acl->entry, aclbufp + access_cnt, _ENTRY_SIZE (cnt));
	      acl->count = cnt;
	      for (cnt = 0; cnt < acl->count; ++cnt)
		acl->entry[cnt].a_type &= ~ACL_DEFAULT;
	    }
	}
    }
  __except (EINVAL) {}
  __endtry
  if (oret)
    close_fs ();
  return acl;
}

extern "C" acl_t
acl_get_fd (int fd)
{
  cygheap_fdget cfd (fd);
  if (cfd < 0)
    return NULL;
  return cfd->acl_get (ACL_TYPE_ACCESS);
}

extern "C" acl_t
acl_get_file (const char *path_p, acl_type_t type)
{
  if (type != ACL_TYPE_ACCESS && type != ACL_TYPE_DEFAULT)
    {
      set_errno (EINVAL);
      return NULL;
    }
  fhandler_base *fh;
  if (!(fh = build_fh_name (path_p, PC_SYM_FOLLOW, stat_suffixes)))
    return NULL;
  if (fh->error ())
    {
      set_errno (fh->error ());
      return NULL;
    }
  acl_t acl = fh->acl_get (type);
  delete fh;
  return acl;
}

int __reg3
fhandler_base::acl_set (acl_t acl, acl_type_t type)
{
  set_errno (ENOTSUP);
  return -1;
}

int __reg3
fhandler_disk_file::acl_set (acl_t acl, acl_type_t type)
{
  int ret = -1;
  int oret = 0;

  __try
    {
      tmp_pathbuf tp;
      aclent_t *aclbufp, *aclbuf_from_file;
      uint16_t cnt, cnt_from_file, access_cnt;

      if (!pc.has_acls ())
	{
	  set_errno (ENOTSUP);
	  __leave;
	}
      if (type == ACL_TYPE_DEFAULT && !pc.isdir ())
	{
	  set_errno (ENOTDIR);
	  __leave;
	}
      if (acl->count > MAX_ACL_ENTRIES)
	{
	  set_errno (EINVAL);
	  __leave;
	}
      aclbuf_from_file = (aclent_t *) tp.c_get ();
      if (!get_handle ())
	{
	  query_open (query_write_dac);
	  if (!(oret = open (O_BINARY, 0)))
	    __leave;
	}
      cnt_from_file = facl (GETACL, MAX_ACL_ENTRIES, aclbuf_from_file);
      if (cnt_from_file < 0)
	__leave;
      aclbufp = (aclent_t *) tp.c_get ();
      /* Set access_cnt to number of non-default entries from file ACL. */
      if (!pc.isdir ())
	access_cnt = cnt_from_file;
      else
	for (access_cnt = 0; access_cnt < cnt_from_file; ++access_cnt)
	  if (aclbuf_from_file[access_cnt].a_type & ACL_DEFAULT)
	    break;
      if (type == ACL_TYPE_ACCESS)
	{
	  /* Check if the number of ACEs fits into the buffer. */
	  if (acl->count - acl->deleted + cnt_from_file - access_cnt
	      > MAX_ACL_ENTRIES)
	    {
	      set_errno (EINVAL);
	      __leave;
	    }
	  /* Copy the new ACL entries. */
	  cnt = 0;
	  for (uint16_t idx = 0; idx < acl->count; ++idx)
	    if (acl->entry[idx].a_type != ACL_DELETED_TAG)
	      aclbufp[cnt++] = acl->entry[idx];
	  /* Append default ACL from file, if any. */
	  if (access_cnt < cnt_from_file)
	    {
	      memcpy (aclbufp + cnt, aclbuf_from_file + access_cnt,
		      _ENTRY_SIZE (cnt_from_file - access_cnt));
	      cnt += cnt_from_file - access_cnt;
	    }
	}
      else
	{
	  /* Check if the number of ACEs fits into the buffer. */
	  if (acl->count - acl->deleted + access_cnt > MAX_ACL_ENTRIES)
	    {
	      set_errno (EINVAL);
	      __leave;
	    }
	  /* Copy non-default entries from file. */
	  memcpy (aclbufp, aclbuf_from_file, _ENTRY_SIZE (access_cnt));
	  cnt = access_cnt;
	  /* Append new default ACL entries (and add ACL_DEFAULT flag). */
	  for (uint16_t idx = 0; idx < acl->count; ++idx)
	    if (acl->entry[idx].a_type != ACL_DELETED_TAG)
	      {
		aclbufp[cnt] = acl->entry[idx];
		aclbufp[cnt++].a_type |= ACL_DEFAULT;
	      }
	}
      ret = facl (SETACL, cnt, aclbufp);
    }
  __except (EINVAL) {}
  __endtry
  if (oret)
    close_fs ();
  return ret;
}

extern "C" int
acl_set_fd (int fd, acl_t acl)
{
  cygheap_fdget cfd (fd);
  if (cfd < 0)
    return -1;
  return cfd->acl_set (acl, ACL_TYPE_ACCESS);
}

extern "C" int
acl_set_file(const char *path_p, acl_type_t type, acl_t acl)
{
  if (type != ACL_TYPE_ACCESS && type != ACL_TYPE_DEFAULT)
    {
      set_errno (EINVAL);
      return -1;
    }
  fhandler_base *fh;
  if (!(fh = build_fh_name (path_p, PC_SYM_FOLLOW, stat_suffixes)))
    return -1;
  if (fh->error ())
    {
      set_errno (fh->error ());
      return -1;
    }
  int ret = fh->acl_set (acl, type);
  delete fh;
  return ret;
}

extern "C" int
acl_delete_def_file (const char *path_p)
{
  acl_t acl = (acl_t) alloca (sizeof (struct __acl_t));
  acl->count = acl->max_count = acl->next = 0;
  if (!acl)
    return -1;
  return acl_set_file(path_p, ACL_TYPE_DEFAULT, acl);
}

/* libacl extensions */

extern "C" int
acl_check (acl_t acl, int *last)
{

  __try
    {
      int ret = 0;

      if (acl->count != 0)
	{
	  ret = __aclcheck (acl->entry, acl->count, last, true);
	  switch (ret)
	    {
	    case GRP_ERROR:
	    case USER_ERROR:
	    case CLASS_ERROR:
	    case OTHER_ERROR:
	      ret = ACL_MULTI_ERROR;
	      break;
	    default:
	      break;
	    }
	}
      return ret;
    }
  __except (EINVAL) {}
  __endtry
  return -1;
}

extern "C" int
acl_cmp (acl_t acl1, acl_t acl2)
{
  int ret = -1;

  __try
    {
      tmp_pathbuf tp;

      __acl_ext_t *acl1d = (__acl_ext_t *) tp.c_get ();
      __acl_ext_t *acl2d = (__acl_ext_t *) tp.c_get ();
      if (acl_copy_ext (acl1d, acl1, NT_MAX_PATH) < 0)
	__leave;
      if (acl_copy_ext (acl2d, acl2, NT_MAX_PATH) < 0)
	__leave;
      if (acl1d->count != acl2d->count)
	return 1;
      if (__aclsort (acl1d->count, acl1d->entry))
	__leave;
      if (__aclsort (acl2d->count, acl2d->entry))
	__leave;
      for (int idx = 0; idx < acl1d->count; ++idx)
	{
	  if (acl1d->entry[idx].a_type != acl2d->entry[idx].a_type)
	    {
	      ret = 1;
	      __leave;
	    }
	  if ((acl1d->entry[idx].a_perm & ACL_PERM_MASK)
	      != (acl2d->entry[idx].a_perm & ACL_PERM_MASK))
	    {
	      ret = 1;
	      __leave;
	    }
	  if ((acl1d->entry[idx].a_type & (ACL_USER | ACL_GROUP))
	      && acl1d->entry[idx].a_id != acl2d->entry[idx].a_id)
	    {
	      ret = 1;
	      __leave;
	    }
	}
      ret = 0;
    }
  __except (EINVAL) {}
  __endtry
  return ret;
}

extern "C" int
acl_entries (acl_t acl)
{
  __try
    {
      return acl->count - acl->deleted;
    }
  __except (EINVAL) {}
  __endtry
  return -1;
}

extern "C" int
acl_equiv_mode (acl_t acl, mode_t *mode_p)
{
  __try
    {
      if (acl->count != 3)
	{
	  set_errno (EINVAL);
	  __leave;
	}
      int u_idx = -1, g_idx = -1, o_idx = -1;
      for (int idx = 0; idx < 3; ++idx)
	switch (acl->entry[idx].a_type)
	  {
	  case ACL_USER_OBJ:
	    u_idx = idx;
	    break;
	  case ACL_GROUP_OBJ:
	    g_idx = idx;
	    break;
	  case ACL_OTHER:
	    o_idx = idx;
	    break;
	  }
      if (u_idx == -1 || g_idx == -1 || o_idx == -1)
	{
	  set_errno (EINVAL);
	  __leave;
	}
      if (mode_p)
	*mode_p = ((acl->entry[u_idx].a_perm & ACL_PERM_MASK) << 6)
		  | ((acl->entry[g_idx].a_perm & ACL_PERM_MASK) << 3)
		  | (acl->entry[o_idx].a_perm & ACL_PERM_MASK);
      return 0;
    }
  __except (EINVAL) {}
  __endtry
  return -1;
}

static const char *acl_err_txt[] =
{
  "Multiple entries",
  "Duplicate entries",
  "Invalid entry type",
  "Missing or wrong entry"
};

extern "C" const char *
acl_error (int code)
{
  if (code < ACL_MULTI_ERROR || code > ACL_MISS_ERROR)
    return NULL;
  return acl_err_txt[code - ACL_MULTI_ERROR];
}

static int
__acl_extended_fh (fhandler_base *fh)
{
  int ret = -1;

  if (!fh->pc.has_acls ())
    set_errno (ENOTSUP);
  else
    {
      ret = fh->facl (GETACLCNT, 0, NULL);
      if (ret >= 0)
	ret = (ret > MIN_ACL_ENTRIES) ? 1 : 0;
    }
  return ret;
}

extern "C" int
acl_extended_fd (int fd)
{
  __try
    {
      cygheap_fdget cfd (fd);
      if (cfd < 0)
	__leave;
      return __acl_extended_fh (cfd);
    }
  __except (EBADF) {}
  __endtry
  return -1;
}

static int
__acl_extended_file (path_conv &pc)
{
  int ret = -1;

  __try
    {
      if (pc.error)
	set_errno (pc.error);
      else if (!pc.exists ())
	set_errno (ENOENT);
      else
	{
	  fhandler_base *fh;

	  if (!(fh = build_fh_pc (pc)))
	    __leave;
	  ret = __acl_extended_fh (fh);
	  delete fh;
	}
    }
  __except (EFAULT) {}
  __endtry
  return ret;
}

extern "C" int
acl_extended_file (const char *path_p)
{
  path_conv pc (path_p, PC_SYM_FOLLOW | PC_POSIX | PC_KEEP_HANDLE,
		stat_suffixes);
  return __acl_extended_file (pc);
}

extern "C" int
acl_extended_file_nofollow (const char *path_p)
{
  path_conv pc (path_p, PC_SYM_NOFOLLOW | PC_POSIX | PC_KEEP_HANDLE,
		stat_suffixes);
  return __acl_extended_file (pc);
}

extern "C" acl_t
acl_from_mode (mode_t mode)
{
  acl_t acl = acl_init (MIN_ACL_ENTRIES);
  if (!acl)
    return NULL;
  acl->count = 3;
  acl->entry[0].a_type = USER_OBJ;
  acl->entry[0].a_id = ACL_UNDEFINED_ID;
  acl->entry[0].a_perm = (mode >> 6) & ACL_PERM_MASK;
  acl->entry[1].a_type = GROUP_OBJ;
  acl->entry[1].a_id = ACL_UNDEFINED_ID;
  acl->entry[1].a_perm = (mode >> 3) & ACL_PERM_MASK;
  acl->entry[2].a_type = OTHER_OBJ;
  acl->entry[2].a_id = ACL_UNDEFINED_ID;
  acl->entry[2].a_perm = mode & ACL_PERM_MASK;
  return acl;
}

extern "C" int
acl_get_perm (acl_permset_t permset_d, acl_perm_t perm)
{
  __try
    {
      acl_t acl;
      uint16_t idx;

      acl = __from_permset (permset_d, idx);
      if (acl && !(perm & ~ACL_PERM_MASK))
	return (~acl->entry[idx].a_perm & perm) ? 0 : 1;
      set_errno (EINVAL);
    }
  __except (EINVAL) {}
  __endtry
  return -1;
}

extern "C" char *
acl_to_any_text (acl_t acl, const char *prefix, char separator, int options)
{
  __try
    {
      return __acltotext (acl->entry, acl->count, prefix, separator,
			  TEXT_IS_POSIX | options);
    }
  __except (EINVAL) {}
  __endtry
  return NULL;
}
