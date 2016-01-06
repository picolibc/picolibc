/* sec_posixacl.h: Internal definitions for POSIX ACLs.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include <cygwin/acl.h>
#include <sys/acl.h>
#include <acl/libacl.h>

/* Magic marker for acl_t. */
#define ACL_MAGIC		(0xacdccdcadcaccacdULL)

/* Only used internally as a_type for deleted entries. */
#define ACL_DELETED_TAG		(0xffff)

/* Only used internally from acl_to_text/acl_to_any_text. */
#define TEXT_END_SEPARATOR	(0x1000)
#define TEXT_IS_POSIX		(0x2000)

/* Internal ACL representation. */
struct __acl_t
{
  uint64_t magic;	/* Must be ACL_MAGIC.				*/
  uint16_t max_count;	/* Max. number of entries.			*/
  uint16_t count;	/* Number of used entries.			*/
  uint16_t deleted;	/* Number of used but deleted entries.		*/
  uint16_t next;	/* Next entry to be returned by acl_get_entry.	*/
  aclent_t *entry;	/* Pointer to variable array of ACL entries.	*/
};

inline acl_entry_t
__to_entry (acl_t acl, uint16_t idx)
{
  return ((uint64_t) idx << 48) | (uint64_t) acl;
}
#define __to_permset(a,i)	((acl_permset_t)__to_entry((a),(i)))

inline acl_t
__from_entry (acl_entry_t entry_d, uint16_t &idx)
{
  idx = entry_d >> 48;
  acl_t acl = (acl_t) (entry_d & ~((uint64_t) 0xffff << 48));
  if (acl->magic != ACL_MAGIC)
    return NULL;
  if (idx >= acl->count)
    return NULL;
  if (acl->entry[idx].a_type == ACL_DELETED_TAG)
    return NULL;
  return acl;
}
#define __from_permset(p,i)	__from_entry((acl_permset_t)(p),(i))

/* External (but opaque) ACL representation. */
struct __acl_ext_t
{
  uint16_t count;	/* Number of used entries.			*/
  aclent_t entry[0];	/* Variable array of ACL entries.		*/
};

/* Shared functions defined in sec_acl.cc. */
mode_t __aclcalcmask (aclent_t *, int);
int __aclsort (int, aclent_t *);
int __aclcheck (aclent_t *, int, int *, bool);
char *__acltotext (aclent_t *, int, const char *, char, int);
void *__aclfromtext (const char *, int *, bool);
