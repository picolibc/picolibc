/*-
 * Written by J.T. Conklin <jtc@netbsd.org>
 * Public domain.
 *
 *	$NetBSD: search.h,v 1.12 1999/02/22 10:34:28 christos Exp $
 * $FreeBSD: src/include/search.h,v 1.10 2002/10/16 14:29:23 robert Exp $
 */

#ifndef _SEARCH_H_
#define _SEARCH_H_

#include <sys/cdefs.h>
#include <sys/types.h>

typedef	struct entry
{
  char *key;
  void *data;
} ENTRY;

typedef	enum
{
  FIND, ENTER
} ACTION;

typedef	enum
{
  preorder,
  postorder,
  endorder,
  leaf
} VISIT;

#ifdef _SEARCH_PRIVATE
typedef	struct node
{
  char *key;
  struct node *llink, *rlink;
} node_t;
#endif

struct hsearch_data
{
  struct internal_head *htable;
  size_t htablesize;
};

struct qelem
{
  struct qelem *q_forw;
  struct qelem *q_back;
};

__BEGIN_DECLS
int  hcreate (size_t);
void  hdestroy (void);
ENTRY *hsearch (ENTRY, ACTION);
int hcreate_r (size_t, struct hsearch_data *);
void hdestroy_r (struct hsearch_data *);
int hsearch_r (ENTRY, ACTION, ENTRY **, struct hsearch_data *);
void *tdelete (const void * __restrict, void ** __restrict,
	       int (*) (const void *, const void *));
void tdestroy (void *, void (*)(void *));
void *tfind (const void *, void **,
	     int (*) (const void *, const void *));
void *tsearch (const void *, void **, int (*) (const void *, const void *));
void  twalk (const void *, void (*) (const void *, VISIT, int));
void *lfind (const void *, const void *, size_t *, size_t,
	     int (*) (const void *, const void *));
void *lsearch (const void *, void *, size_t *, size_t,
	       int (*) (const void *, const void *));
void insque (void *, void *);
void remque (void *);
__END_DECLS

#endif /* !_SEARCH_H_ */
