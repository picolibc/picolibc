/* acl/libacl.h: Non-POSIX extensions of libacl

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _ACL_LIBACL_H
#define _ACL_LIBACL_H

#include <sys/acl.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Sync'd with cygwin/acl.h values. */
#define ACL_MULTI_ERROR     (0x4)
#define ACL_DUPLICATE_ERROR (0x5)
#define ACL_ENTRY_ERROR     (0x6)
#define ACL_MISS_ERROR      (0x7)

/* acl_to_any_text options. */
#define TEXT_ABBREVIATE		(0x01)
#define TEXT_NUMERIC_IDS	(0x02)
#define TEXT_SOME_EFFECTIVE	(0x04)
#define TEXT_ALL_EFFECTIVE	(0x08)
#define TEXT_SMART_INDENT	(0x10)

extern int acl_check (acl_t __acl, int *__last);
extern int acl_cmp (acl_t __acl1, acl_t __acl2);
extern int acl_entries (acl_t __acl);
extern int acl_equiv_mode (acl_t __acl, mode_t *__mode_p);
extern const char *acl_error (int __code);
extern int acl_extended_fd (int __fd);
extern int acl_extended_file (const char *__path_p);
extern int acl_extended_file_nofollow (const char *__path_p);
extern acl_t acl_from_mode (mode_t __mode);
extern int acl_get_perm (acl_permset_t __permset_d, acl_perm_t __perm);
extern char *acl_to_any_text (acl_t __acl, const char *__prefix,
			      char __separator, int __options);

#if 0
/* TODO */
struct error_context;
extern int perm_copy_file (const char *, const char *, struct error_context *);
extern int perm_copy_fd (const char *, int, const char *, int,
			 struct error_context *);
#endif

#ifdef __cplusplus
}
#endif
#endif /* _ACL_LIBACL_H */
