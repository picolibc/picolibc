/* sys/acl.h header file for Cygwin.
   Written by C. Vinschen.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _SYS_ACL_H
#define _SYS_ACL_H

#include <_ansi.h>

#include <sys/types.h>
#include <sys/stat.h>

#ifdef __cplusplus
extern "C" {
#endif

/* POSIX ACL types and functions.  The implementation is based on the
   internal original Solaris implementation as defined in cygwin/acl.h.
   However, we don't include cygwin/acl.h from here to avoid poisoning
   the namespace. */

/* acl_perm_t constants */
#define ACL_READ		(0x4)
#define ACL_WRITE		(0x2)
#define ACL_EXECUTE		(0x1)

/* acl_tag_t constants, in sync with values from cygwin/acl.h */
#define ACL_UNDEFINED_TAG	(0x0000)
#define ACL_USER_OBJ		(0x0001)
#define ACL_USER		(0x0002)
#define ACL_GROUP_OBJ		(0x0004)
#define ACL_GROUP		(0x0008)
#define ACL_MASK		(0x0010)
#define ACL_OTHER		(0x0020)

/* acl_type_t constants */
#define ACL_TYPE_ACCESS         (0x0)
#define ACL_TYPE_DEFAULT        (0x1)

/* qualifier constant */
#define ACL_UNDEFINED_ID        ((id_t) -1)

/* entry_id constants */
#define ACL_FIRST_ENTRY         (0x0)
#define ACL_NEXT_ENTRY          (0x1)

/* types */
typedef uint32_t acl_perm_t, acl_type_t, acl_tag_t;
typedef uint64_t acl_permset_t;
typedef uint64_t acl_entry_t;

struct __acl_t;
typedef struct __acl_t *acl_t;

extern int	acl_add_perm (acl_permset_t __permset_d, acl_perm_t __perm);
extern int	acl_calc_mask (acl_t *__acl_p);
extern int	acl_clear_perms (acl_permset_t __permset_d);
extern int	acl_copy_entry (acl_entry_t __dest_d, acl_entry_t __src_d);
extern ssize_t	acl_copy_ext (void *__buf_p, acl_t __acl, ssize_t __size);
extern acl_t	acl_copy_int (const void *__buf_p);
extern int	acl_create_entry (acl_t *__acl_p, acl_entry_t *__entry_p);
extern int	acl_delete_def_file (const char *__path_p);
extern int	acl_delete_entry (acl_t __acl, acl_entry_t __entry_d);
extern int	acl_delete_perm (acl_permset_t __permset_d, acl_perm_t __perm);
extern acl_t	acl_dup (acl_t __acl);
extern int	acl_free (void *__obj_p);
extern acl_t	acl_from_text (const char *__buf_p);
extern int	acl_get_entry (acl_t __acl, int __entry_id,
			       acl_entry_t *__entry_p);
extern acl_t	acl_get_fd (int __fd);
extern acl_t	acl_get_file (const char *__path_p, acl_type_t __type);
extern int	acl_get_permset (acl_entry_t __entry_d,
				 acl_permset_t *__permset_p);
extern void    *acl_get_qualifier (acl_entry_t __entry_d);
extern int	acl_get_tag_type (acl_entry_t __entry_d,
				  acl_tag_t *__tag_type_p);
extern acl_t	acl_init (int __count);
extern int	acl_set_fd (int __fd, acl_t __acl);
extern int	acl_set_file (const char *__path_p, acl_type_t __type,
			      acl_t __acl);
extern int	acl_set_permset (acl_entry_t __entry_d,
				 acl_permset_t __permset_d);
extern int	acl_set_qualifier (acl_entry_t __entry_d,
				   const void *__tag_qualifier_p);
extern int	acl_set_tag_type (acl_entry_t __entry_d, acl_tag_t __tag_type);
extern ssize_t	acl_size (acl_t __acl);
extern char    *acl_to_text (acl_t __acl, ssize_t *__len_p);
extern int	acl_valid (acl_t __acl);

#ifdef __cplusplus
}
#endif
#endif /* _SYS_ACL_H */
