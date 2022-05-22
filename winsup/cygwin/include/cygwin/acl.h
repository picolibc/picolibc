/* cygwin/acl.h header file for Cygwin.
   Written by C. Vinschen.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _CYGWIN_ACL_H
#ifdef __cplusplus
extern "C" {
#endif
#define _CYGWIN_ACL_H

#include <_ansi.h>

#include <sys/types.h>
#include <sys/stat.h>

/* Values for `cmd' in calls to acl(2) and facl(2) */
#define SETACL          (0x0)
#define GETACL          (0x1)
#define GETACLCNT       (0x2)

/* Windows ACLs have a maximum size of 64K.  Counting the most pessimistic way,
   the maximum number of ACEs is 3276.  Technet claims "approximately 1820",
   which uses the length of normal user and group SIDs for the computation.
   We're now going with 2730, the number of aclent_t entries matching a 32K
   buffer.
   On one hand, there are only a limited number of SIDs shorter than the normal
   user/group SIDs, on the other hand there are no deny aclent_t entries, so we
   should be fine with 32K aclbuf_t buffers provided by the caller. */
#define	MIN_ACL_ENTRIES (3)    /* minimal acl entries from GETACLCNT	    */
#define	MAX_ACL_ENTRIES	(2730) /* max entries of each type		    */

/* Return values of aclcheck(3) in case of error */
#define GRP_ERROR       (0x1)
#define USER_ERROR      (0x2)
#define CLASS_ERROR     (0x3)
#define OTHER_ERROR     (0x4)
#define DUPLICATE_ERROR (0x5)
#define ENTRY_ERROR     (0x6)
#define MISS_ERROR      (0x7) /* which = -1 */
#define MEM_ERROR       (0x8) /* which = -1 */

/* Values for entry type of struct acl */
#define USER_OBJ        (0x0001)                /* owner		    */
#define USER            (0x0002)                /* additional user	    */
#define GROUP_OBJ       (0x0004)                /* owning group		    */
#define GROUP           (0x0008)                /* additional group	    */
#define CLASS_OBJ       (0x0010)                /* mask entry		    */
#define OTHER_OBJ       (0x0020)                /* others		    */
#define ACL_DEFAULT     (0x1000)                /* default flag		    */
#define DEF_USER_OBJ    (ACL_DEFAULT|USER_OBJ)  /* default owner	    */
#define DEF_USER        (ACL_DEFAULT|USER)      /* default additional user  */
#define DEF_GROUP_OBJ   (ACL_DEFAULT|GROUP_OBJ) /* default owning group	    */
#define DEF_GROUP       (ACL_DEFAULT|GROUP)     /* default additional group */
#define DEF_CLASS_OBJ   (ACL_DEFAULT|CLASS_OBJ) /* default mask entry	    */
#define DEF_OTHER_OBJ   (ACL_DEFAULT|OTHER_OBJ) /* default others	    */
/* Values with equivalent meanings */
#define USER_OWNER      USER_OBJ
#define GROUP_OWNER     GROUP_OBJ
#define MASK            CLASS_OBJ
#define OTHER           OTHER_OBJ

typedef struct acl {
    int          a_type;    /* entry type  */
    id_t         a_id;      /* UID | GID   */
    mode_t       a_perm;    /* permissions */
} aclent_t;

extern int	 acl (const char *__path, int __cmd, int __nentries,
		      aclent_t *__aclbufp);
extern int	 facl (int __fd, int __cmd, int __nentries,
		       aclent_t *__aclbufp);
extern int	 aclcheck (aclent_t *__aclbufp, int __nentries, int *__which);
extern int	 aclsort (int __nentries, int __calclass, aclent_t *__aclbufp);
extern int	 acltomode (aclent_t *__aclbufp, int __nentries,
			    mode_t *__modep);
extern int	 aclfrommode (aclent_t *__aclbufp, int __nentries,
			      mode_t *__modep);
extern int	 acltopbits (aclent_t *__aclbufp, int __nentries,
			     mode_t *__pbitsp);
extern int	 aclfrompbits (aclent_t *__aclbufp, int __nentries,
			       mode_t *__pbitsp);
extern char	*acltotext (aclent_t *__aclbufp, int __aclcnt);
extern aclent_t *aclfromtext (char *__acltextp, int *__aclcnt);

#ifdef __cplusplus
}
#endif
#endif /* _CYGWIN_ACL_H */
