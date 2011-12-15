//####BSDCOPYRIGHTBEGIN####
//
// -------------------------------------------
//
// Portions of this software may have been derived from OpenBSD, 
// FreeBSD or other sources, and are covered by the appropriate
// copyright disclaimers included herein.
//
// Portions created by Red Hat are
// Copyright (C) 2002 Red Hat, Inc. All Rights Reserved.
//
// -------------------------------------------
//
//####BSDCOPYRIGHTEND####
//==========================================================================

/*
 * Copyright (c) 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)queue.h	8.5 (Berkeley) 8/20/94
 * $FreeBSD: src/sys/sys/queue.h,v 1.32.2.4 2001/03/31 03:33:39 hsu Exp $
 */

#ifndef _SYS_LEONBARE_QUEUE_H_
#define	_SYS_LEONBARE_QUEUE_H_


/*
 * Tail queue definitions.
 */
#define LBTAILQ_HEAD(name, type)						\
struct name {								\
	struct type *tqh_first;	/* first element */			\
	struct type **tqh_last;	/* addr of last next element */		\
        char *tqh_name;                                                 \
}

#define LBTAILQ_HEAD_INITIALIZER(head)					\
	{ NULL, &(head).tqh_first, 0 }

#define LBTAILQ_ENTRY(type)						\
struct {								\
	struct type *tqe_next;	/* next element */			\
	struct type **tqe_prev;	/* address of previous next element */	\
}

/*
 * Tail queue functions.
 */
#define	LBTAILQ_EMPTY(head) (LEONBARE_KERNEL_GETU32P((head)->tqh_first) == NULL)

#define	LBTAILQ_HASTWO(head, field) ((!LBTAILQ_EMPTY(head)) && LBTAILQ_NEXT(LBTAILQ_FIRST(head),field))

#define LBTAILQ_FOREACH(var, head, field)					\
	for (var = LBTAILQ_FIRST(head); var; var = LBTAILQ_NEXT(var, field))

#define	LBTAILQ_FIRST(head) LEONBARE_KERNEL_GETU32P_CAST((head)->tqh_first,__typeof((head)->tqh_first))

#define	LBTAILQ_LAST(head, headname) \
        LEONBARE_KERNEL_GETU32P_BARE(LEONBARE_KERNEL_GETU32P(((struct headname *)(LEONBARE_KERNEL_GETU32P((head)->tqh_last)))->tqh_last))

#define	LBTAILQ_NEXT(elm, field) LEONBARE_KERNEL_GETU32P_CAST((elm)->field.tqe_next,__typeof((elm)->field.tqe_next))

#define LBTAILQ_PREV(elm, headname, field) \
        LEONBARE_KERNEL_GETU32P_BARE(LEONBARE_KERNEL_GETU32P(((struct headname *)(LEONBARE_KERNEL_GETU32P((elm)->field.tqe_prev)))->tqh_last))

/* #define	LBTAILQ_INIT(head) do {						\ */
/* 	(head)->tqh_first = NULL;					\ */
/* 	(head)->tqh_last = &(head)->tqh_first;				\ */
/*         (head)->tqh_name = 0;                                           \ */
/* } while (0) */

#define	LBTAILQ_INIT(head) do {						\
	LEONBARE_KERNEL_SETU32P((head)->tqh_first,NULL);		\
	LEONBARE_KERNEL_SETU32P((head)->tqh_last,&(head)->tqh_first);	\
        LEONBARE_KERNEL_SETU32P((head)->tqh_name,0);			\
} while (0)

/* #define LBTAILQ_INSERT_HEAD(head, elm, field) do {			\ */
/* 	if (((elm)->field.tqe_next = (head)->tqh_first) != NULL)	\ */
/* 		(head)->tqh_first->field.tqe_prev =			\ */
/* 		    &(elm)->field.tqe_next;				\ */
/* 	else								\ */
/* 		(head)->tqh_last = &(elm)->field.tqe_next;		\ */
/* 	(head)->tqh_first = (elm);					\ */
/* 	(elm)->field.tqe_prev = &(head)->tqh_first;			\ */
/* } while (0) */

#define LBTAILQ_INSERT_HEAD(head, elm, field) do {			\
	if ((LEONBARE_KERNEL_SETU32P((elm)->field.tqe_next,LEONBARE_KERNEL_GETU32P((head)->tqh_first))) != NULL) \
	    LEONBARE_KERNEL_SETU32P(LEONBARE_KERNEL_GETU32P_CAST((head)->tqh_first,__typeof ((head)->tqh_first))->field.tqe_prev,&(elm)->field.tqe_next); \
	else								\
	    LEONBARE_KERNEL_SETU32P((head)->tqh_last,&(elm)->field.tqe_next); \
	LEONBARE_KERNEL_SETU32P((head)->tqh_first,(elm));		\
	LEONBARE_KERNEL_SETU32P((elm)->field.tqe_prev,&(head)->tqh_first); \
} while (0)

#define LBTAILQ_INSERT_TAIL(head, elm, field) do {			\
	LEONBARE_KERNEL_SETU32P((elm)->field.tqe_next,NULL);		\
	LEONBARE_KERNEL_SETU32P((elm)->field.tqe_prev,LEONBARE_KERNEL_GETU32P((head)->tqh_last)); \
	LEONBARE_KERNEL_SETU32P_BARE(LEONBARE_KERNEL_GETU32P((head)->tqh_last),(elm)); \
	LEONBARE_KERNEL_SETU32P((head)->tqh_last,&(elm)->field.tqe_next); \
} while (0)

#define LBTAILQ_REMOVE(head, elm, field) do {				\
	if (LEONBARE_KERNEL_GETU32P((elm)->field.tqe_next) != NULL)	\
	    LEONBARE_KERNEL_SETU32P(LEONBARE_KERNEL_GETU32P_CAST((elm)->field.tqe_next, __typeof((elm)->field.tqe_next))->field.tqe_prev, LEONBARE_KERNEL_GETU32P((elm)->field.tqe_prev)); \
	else								\
	    LEONBARE_KERNEL_SETU32P((head)->tqh_last, LEONBARE_KERNEL_GETU32P((elm)->field.tqe_prev)); \
	LEONBARE_KERNEL_SETU32P_BARE(LEONBARE_KERNEL_GETU32P((elm)->field.tqe_prev),LEONBARE_KERNEL_GETU32P((elm)->field.tqe_next)); \
        LEONBARE_KERNEL_SETU32P((elm)->field.tqe_next, 0);		\
        LEONBARE_KERNEL_SETU32P((elm)->field.tqe_prev, 0);     /* mark removed */ \
} while (0)

#define	LBTAILQ_REMOVED(elm, field) (LEONBARE_KERNEL_GETU32P((elm)->field.tqe_next) == NULL && LEONBARE_KERNEL_GETU32P((elm)->field.tqe_prev) == NULL)



#endif /* !_SYS_QUEUE_H_ */
