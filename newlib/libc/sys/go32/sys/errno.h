/*
** Copyright (C) 1991 DJ Delorie
**
** This file is distributed under the terms listed in the document
** "copying.dj".
** A copy of "copying.dj" should accompany this file; if not, a copy
** should be available from where this file was obtained.  This file
** may not be distributed without a verbatim copy of "copying.dj".
**
** This file is distributed WITHOUT ANY WARRANTY; without even the implied
** warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

/* ??? This file has been modified to work with newlib's way of storing
   `errno'.  Arguably there's no need and arguably we shouldn't diverge
   from go32 sources.  If you feel strongly about it, please change it.
   The interface between newlib and system's version of errno is via
   __errno, so there's no problem in storing errno in a different place
   (any changes can be dealt with inside __errno).  */

#ifndef _SYS_ERRNO_H_
#define _SYS_ERRNO_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/reent.h>

#ifndef _REENT_ONLY
#define errno (*__errno())
extern int *__errno _PARAMS ((void));
#endif

#define __errno_r(ptr) ((ptr)->_errno)

#define ENOENT   2              /* No such file or directory    */
#define ENOTDIR  3              /* No path                      */
#define EMFILE   4              /* Too many open files          */
#define EACCES   5              /* Permission denied            */
#define EBADF    6              /* Bad file number              */
#define EARENA   7              /* Arena trashed                */
#define ENOMEM   8              /* Not enough core              */
#define ESEGV    9              /* invalid memory address       */
#define EBADENV 10              /* invalid environment          */
#define ENODEV  15              /* No such device               */
#define ENMFILE 18              /* No more files                */
#define EINVAL  19              /* Invalid argument             */
#define E2BIG   EBADENV         /* Arg list too long            */
#define ENOEXEC 21              /* Exec format error            */
#define EXDEV   17              /* Cross-device link            */
#define EPIPE	32		/* POHC                         */
#define EDOM    33              /* Math argument                */
#define ERANGE  34              /* Result too large             */
#if 0 /* readline.c assumes that if this is defined, so is O_NDELAY.
	 Newlib doesn't use it, so comment it out.  */
#define EWOULDBLOCK 35		/* POHC                         */
#endif
#define EEXIST  36              /* File already exists          */
#define EINTR	100		/* Interrupted system call	*/
#define EIO	101		/* I/O or bounds error		*/
#define ENOSPC	102		/* No space left on drive	*/
#define EAGAIN	103		/* No more processes		*/
#define ECHILD  200		/* child exited (porting only)  */
#define EFAULT	201		/* bad address */
#define ENXIO	ENODEV
#define EPERM	EACCES

/* New values required by newlib and the Cygnus toolchain.  */
#define ENOSYS	230		/* Function not implemented */
#define ESPIPE	231		/* Illegal seek */

#endif
