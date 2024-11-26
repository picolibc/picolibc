/*
 *  Written by Joel Sherrill <joel@OARcorp.com>.
 *
 *  COPYRIGHT (c) 1989-2014.
 *
 *  On-Line Applications Research Corporation (OAR).
 *
 *  Permission to use, copy, modify, and distribute this software for any
 *  purpose without fee is hereby granted, provided that this entire notice
 *  is included in all copies of any software which is or includes a copy
 *  or modification of this software.
 *
 *  THIS SOFTWARE IS BEING PROVIDED "AS IS", WITHOUT ANY EXPRESS OR IMPLIED
 *  WARRANTY.  IN PARTICULAR,  THE AUTHOR MAKES NO REPRESENTATION
 *  OR WARRANTY OF ANY KIND CONCERNING THE MERCHANTABILITY OF THIS
 *  SOFTWARE OR ITS FITNESS FOR ANY PARTICULAR PURPOSE.
 *
 *  $Id$
 */

#ifndef _SYS_FEATURES_H
#define _SYS_FEATURES_H

#ifdef __cplusplus
extern "C" {
#endif

#include <picolibc.h>

/* Macro to test version of GCC.  Returns 0 for non-GCC or too old GCC. */
#ifndef __GNUC_PREREQ
# if defined __GNUC__ && defined __GNUC_MINOR__
#  define __GNUC_PREREQ(maj, min) \
	((__GNUC__ << 16) + __GNUC_MINOR__ >= ((maj) << 16) + (min))
# else
#  define __GNUC_PREREQ(maj, min) 0
# endif
#endif /* __GNUC_PREREQ */
/* Version with trailing underscores for BSD compatibility. */
#define	__GNUC_PREREQ__(ma, mi)	__GNUC_PREREQ(ma, mi)

/*
 * Feature test macros control which symbols are exposed by the system
 * headers.  Any of these must be defined before including any headers.
 *
 * __STRICT_ANSI__ (defined by gcc -ansi, -std=c90, -std=c99, or -std=c11)
 *	ISO C
 *
 * _POSIX_SOURCE (deprecated by _POSIX_C_SOURCE=1)
 * _POSIX_C_SOURCE >= 1
 *	POSIX.1-1990
 *
 * _POSIX_C_SOURCE >= 2
 *	POSIX.2-1992
 *
 * _POSIX_C_SOURCE >= 199309L
 *	POSIX.1b-1993 Real-time extensions
 *
 * _POSIX_C_SOURCE >= 199506L
 *	POSIX.1c-1995 Threads extensions
 *
 * _POSIX_C_SOURCE >= 200112L
 *	POSIX.1-2001 and C99
 *
 * _POSIX_C_SOURCE >= 200809L
 *	POSIX.1-2008
 *
 * _POSIX_C_SOURCE >= 202405L
 *	POSIX.1-2024
 *
 * _XOPEN_SOURCE
 *	POSIX.1-1990 and XPG4
 *
 * _XOPEN_SOURCE_EXTENDED
 *	SUSv1 (POSIX.2-1992 plus XPG4v2)
 *
 * _XOPEN_SOURCE >= 500
 *	SUSv2 (POSIX.1c-1995 plus XSI)
 *
 * _XOPEN_SOURCE >= 600
 *	SUSv3 (POSIX.1-2001 plus XSI) and C99
 *
 * _XOPEN_SOURCE >= 700
 *	SUSv4 (POSIX.1-2008 plus XSI)
 *
 * _ISOC99_SOURCE or gcc -std=c99 or g++
 *	ISO C99
 *
 * _ISOC11_SOURCE or gcc -std=c11 or g++ -std=c++11
 *	ISO C11
 *
 * _ISOC2x_SOURCE or gcc -std=c2x or -std=c23 or g++ -std=c++20
 *	ISO C11
 *
 * _ISOC23_SOURCE or gcc -std=c2x or -std=c2x or g++ -std=c++20
 *	ISO C23
 *
 * _ATFILE_SOURCE (implied by _POSIX_C_SOURCE >= 200809L)
 *	"at" functions
 *
 * _LARGEFILE_SOURCE (deprecated by _XOPEN_SOURCE >= 500)
 *	fseeko, ftello
 *
 * _GNU_SOURCE
 *	All of the above plus GNU extensions
 *
 * _BSD_SOURCE (deprecated by _DEFAULT_SOURCE)
 * _SVID_SOURCE (deprecated by _DEFAULT_SOURCE)
 * _DEFAULT_SOURCE (or none of the above)
 *	POSIX-1.2024 with BSD and SVr4 extensions
 *
 * _FORTIFY_SOURCE = 1, 2 or 3
 * 	Object Size Checking function wrappers
 *
 * _ZEPHYR_SOURCE
 *      Zephyr. ISO C + a small selection of other APIs.
 */

#ifdef _GNU_SOURCE
#undef _ATFILE_SOURCE
#define	_ATFILE_SOURCE		1
#undef	_DEFAULT_SOURCE
#define	_DEFAULT_SOURCE		1
#undef _ISOC99_SOURCE
#define	_ISOC99_SOURCE		1
#undef _ISOC11_SOURCE
#define	_ISOC11_SOURCE		1
#undef _ISOC23_SOURCE
#define	_ISOC23_SOURCE		1
#undef _POSIX_SOURCE
#define	_POSIX_SOURCE		1
#undef _POSIX_C_SOURCE
#define	_POSIX_C_SOURCE		200809L
#undef _XOPEN_SOURCE
#define	_XOPEN_SOURCE		700
#undef _XOPEN_SOURCE_EXTENDED
#define	_XOPEN_SOURCE_EXTENDED	1
#undef _LARGEFILE64_SOURCE
#define _LARGEFILE64_SOURCE     1
#endif /* _GNU_SOURCE */

/* When building for Zephyr, set _ZEPHYR_SOURCE unless some other API
 * indicator is set by the application. Don't check __STRICT_ANSI__ as that
 * is set by the compiler for -std=cxx, or _POSIX_C_SOURCE as Zephyr defines
 * that for picolibc currently.
 */

#if defined(__ZEPHYR__) && !defined(_ZEPHYR_SOURCE) &&                  \
    !defined(_GNU_SOURCE)&&                                             \
    !defined(_BSD_SOURCE) &&                                            \
    !defined(_SVID_SOURCE) &&                                           \
    !defined(_DEFAULT_SOURCE)
#define _ZEPHYR_SOURCE      1
#endif

#if defined(_BSD_SOURCE) || defined(_SVID_SOURCE) || \
   (!defined(__STRICT_ANSI__) && !defined(_ANSI_SOURCE) && \
   !defined(_ISOC99_SOURCE) && !defined(_POSIX_SOURCE) && \
   !defined(_POSIX_C_SOURCE) && !defined(_XOPEN_SOURCE) && \
   !defined(_ZEPHYR_SOURCE))
#undef _DEFAULT_SOURCE
#define	_DEFAULT_SOURCE		1
#endif

#if defined(_DEFAULT_SOURCE)
#undef _POSIX_SOURCE
#define	_POSIX_SOURCE		1
#undef _POSIX_C_SOURCE
#define	_POSIX_C_SOURCE		202405L
#endif

#if !defined(_POSIX_SOURCE) && !defined(_POSIX_C_SOURCE) && \
  ((!defined(__STRICT_ANSI__) && !defined(_ANSI_SOURCE)) || \
   (_XOPEN_SOURCE - 0) >= 500)
#define	_POSIX_SOURCE		1
#if !defined(_XOPEN_SOURCE) || (_XOPEN_SOURCE - 0) >= 700
#define	_POSIX_C_SOURCE		200809L
#elif (_XOPEN_SOURCE - 0) >= 600
#define	_POSIX_C_SOURCE		200112L
#elif (_XOPEN_SOURCE - 0) >= 500
#define	_POSIX_C_SOURCE		199506L
#elif (_XOPEN_SOURCE - 0) < 500
#define	_POSIX_C_SOURCE		2
#endif
#endif

#if defined(_POSIX_C_SOURCE) && _POSIX_C_SOURCE >= 200809
#undef _ATFILE_SOURCE
#define	_ATFILE_SOURCE		1
#endif

#ifdef _ZEPHYR_SOURCE
#undef _ISOC99_SOURCE
#define	_ISOC99_SOURCE		1
#undef _ISOC11_SOURCE
#define	_ISOC11_SOURCE		1
#undef _ANSI_SOURCE
#define _ANSI_SOURCE            1
#endif

/*
 * The following private macros are used throughout the headers to control
 * which symbols should be exposed.  They are for internal use only, as
 * indicated by the leading double underscore, and must never be used outside
 * of these headers.
 *
 * __POSIX_VISIBLE
 *	any version of POSIX.1; enabled by default, or with _POSIX_SOURCE,
 *	any value of _POSIX_C_SOURCE, or _XOPEN_SOURCE >= 500.
 *
 * __POSIX_VISIBLE >= 2
 *	POSIX.2-1992; enabled by default, with _POSIX_C_SOURCE >= 2,
 *	or _XOPEN_SOURCE >= 500.
 *
 * __POSIX_VISIBLE >= 199309
 *	POSIX.1b-1993; enabled by default, with _POSIX_C_SOURCE >= 199309L,
 *	or _XOPEN_SOURCE >= 500.
 *
 * __POSIX_VISIBLE >= 199506
 *	POSIX.1c-1995; enabled by default, with _POSIX_C_SOURCE >= 199506L,
 *	or _XOPEN_SOURCE >= 500.
 *
 * __POSIX_VISIBLE >= 200112
 *	POSIX.1-2001; enabled by default, with _POSIX_C_SOURCE >= 200112L,
 *	or _XOPEN_SOURCE >= 600.
 *
 * __POSIX_VISIBLE >= 200809
 *	POSIX.1-2008; enabled by default, with _POSIX_C_SOURCE >= 200809L,
 *	or _XOPEN_SOURCE >= 700.
 *
 * __POSIX_VISIBLE >= 202405
 *	POSIX.1-2024; enabled by default, with _POSIX_C_SOURCE >= 202405L,
 *	or _XOPEN_SOURCE >= 700.
 *
 * __XSI_VISIBLE
 *	XPG4 XSI extensions; enabled with any version of _XOPEN_SOURCE.
 *
 * __XSI_VISIBLE >= 4
 *	SUSv1 XSI extensions; enabled with both _XOPEN_SOURCE and
 *	_XOPEN_SOURCE_EXTENDED together.
 *
 * __XSI_VISIBLE >= 500
 *	SUSv2 XSI extensions; enabled with _XOPEN_SOURCE >= 500.
 *
 * __XSI_VISIBLE >= 600
 *	SUSv3 XSI extensions; enabled with _XOPEN_SOURCE >= 600.
 *
 * __XSI_VISIBLE >= 700
 *	SUSv4 XSI extensions; enabled with _XOPEN_SOURCE >= 700.
 *
 * __ISO_C_VISIBLE >= 1999
 *	ISO C99; enabled with gcc -std=c99 or newer (on by default since GCC 5),
 *	any version of C++, or with _ISOC99_SOURCE, _POSIX_C_SOURCE >= 200112L,
 *	or _XOPEN_SOURCE >= 600.
 *
 * __ISO_C_VISIBLE >= 2011
 *	ISO C11; enabled with gcc -std=c11 or newer (on by default since GCC 5),
 *	g++ -std=c++11 or newer (on by default since GCC 6), or with
 *	_ISOC11_SOURCE.
 *
 * __ISO_C_VISIBLE >= 2023
 *	ISO C23; enabled with gcc -std=c23 or -std=c2x or newer,
 *	g++ -std=c++20 or newer, or with
 *	_ISOC2X_SOURCE or _ISOC23_SOURCE.
 *
 * __ATFILE_VISIBLE
 *	"at" functions; enabled by default, with _ATFILE_SOURCE,
 *	_POSIX_C_SOURCE >= 200809L, or _XOPEN_SOURCE >= 700.
 *
 * __LARGEFILE_VISIBLE
 *	fseeko, ftello; enabled with _LARGEFILE_SOURCE or _XOPEN_SOURCE >= 500.
 *
 * __LARGEFILE64_VISIBLE
 *      additional large file extensions; enabled with _LARGEFILE64_SOURCE.
 *
 * __BSD_VISIBLE
 *	BSD extensions; enabled by default, or with _BSD_SOURCE.
 *
 * __SVID_VISIBLE
 *	SVr4 extensions; enabled by default, or with _SVID_SOURCE.
 *
 * __MISC_VISIBLE
 *	Extensions found in both BSD and SVr4 (shorthand for
 *	(__BSD_VISIBLE || __SVID_VISIBLE)), or newlib-specific
 *	extensions; enabled by default.
 *
 * __GNU_VISIBLE
 *	GNU extensions; enabled with _GNU_SOURCE.
 *
 * __SSP_FORTIFY_LEVEL
 *	Object Size Checking; defined to 0 (off), 1, 2 or 3.
 *
 * __ZEPHYR_VISIBLE
 *      Zephyr extensions; enabled with _ZEPHYR_SOURCE.
 *
 * In all cases above, "enabled by default" means either by defining
 * _DEFAULT_SOURCE, or by not defining any of the public feature test macros.
 */

#ifdef _ATFILE_SOURCE
#define	__ATFILE_VISIBLE	1
#else
#define	__ATFILE_VISIBLE	0
#endif

#ifdef _DEFAULT_SOURCE
#define	__BSD_VISIBLE		1
#else
#define	__BSD_VISIBLE		0
#endif

#ifdef _GNU_SOURCE
#define	__GNU_VISIBLE		1
#else
#define	__GNU_VISIBLE		0
#endif

#ifdef _ZEPHYR_SOURCE
#define __ZEPHYR_VISIBLE        1
#else
#define __ZEPHYR_VISIBLE        0
#endif

#ifdef _ISOC2X_SOURCE
#undef _ISOC2X_SOURCE
#undef _ISOC23_SOURCE
#define _ISOC23_SOURCE      1
#endif

#if defined(_ISOC23_SOURCE) ||                                          \
  (__STDC_VERSION__ - 0) > 201710L || (__cplusplus - 0) >= 202002L
#define __ISO_C_VISIBLE		2023
#elif defined(_ISOC11_SOURCE) || \
  (__STDC_VERSION__ - 0) >= 201112L || (__cplusplus - 0) >= 201103L
#define	__ISO_C_VISIBLE		2011
#elif defined(_ISOC99_SOURCE) || (_POSIX_C_SOURCE - 0) >= 200112L || \
  (__STDC_VERSION__ - 0) >= 199901L || defined(__cplusplus)
#define	__ISO_C_VISIBLE		1999
#else
#define	__ISO_C_VISIBLE		1990
#endif

#if defined(_LARGEFILE_SOURCE) || (_XOPEN_SOURCE - 0) >= 500
#define	__LARGEFILE_VISIBLE	1
#else
#define	__LARGEFILE_VISIBLE	0
#endif

#ifdef _LARGEFILE64_SOURCE
#define __LARGEFILE64_VISIBLE   1
#else
#define __LARGEFILE64_VISIBLE   0
#endif

#ifdef _DEFAULT_SOURCE
#define	__MISC_VISIBLE		1
#else
#define	__MISC_VISIBLE		0
#endif

#if (_POSIX_C_SOURCE - 0) >= 202405L
#define	__POSIX_VISIBLE		202405
#elif (_POSIX_C_SOURCE - 0) >= 200809L
#define	__POSIX_VISIBLE		200809
#elif (_POSIX_C_SOURCE - 0) >= 200112L
#define	__POSIX_VISIBLE		200112
#elif (_POSIX_C_SOURCE - 0) >= 199506L
#define	__POSIX_VISIBLE		199506
#elif (_POSIX_C_SOURCE - 0) >= 199309L
#define	__POSIX_VISIBLE		199309
#elif (_POSIX_C_SOURCE - 0) >= 2 || defined(_XOPEN_SOURCE)
#define	__POSIX_VISIBLE		199209
#elif defined(_POSIX_SOURCE) || defined(_POSIX_C_SOURCE)
#define	__POSIX_VISIBLE		199009
#else
#define	__POSIX_VISIBLE		0
#endif

#ifdef _DEFAULT_SOURCE
#define	__SVID_VISIBLE		1
#else
#define	__SVID_VISIBLE		0
#endif

#if (_XOPEN_SOURCE - 0) >= 700
#define	__XSI_VISIBLE		700
#elif (_XOPEN_SOURCE - 0) >= 600
#define	__XSI_VISIBLE		600
#elif (_XOPEN_SOURCE - 0) >= 500
#define	__XSI_VISIBLE		500
#elif defined(_XOPEN_SOURCE) && defined(_XOPEN_SOURCE_EXTENDED)
#define	__XSI_VISIBLE		4
#elif defined(_XOPEN_SOURCE)
#define	__XSI_VISIBLE		1
#else
#define	__XSI_VISIBLE		0
#endif

#if _FORTIFY_SOURCE > 0 && !defined(__cplusplus) && !defined(__lint__) && \
   (__OPTIMIZE__ > 0 || defined(__clang__)) && __GNUC_PREREQ__(4, 1) && \
   !defined(_LIBC)
#  if _FORTIFY_SOURCE > 2 && defined(__has_builtin)
#    if __has_builtin(__builtin_dynamic_object_size)
#      define __SSP_FORTIFY_LEVEL 3
#    else
#      define __SSP_FORTIFY_LEVEL 2
#    endif
#  elif _FORTIFY_SOURCE > 1
#    define __SSP_FORTIFY_LEVEL 2
#  else
#    define __SSP_FORTIFY_LEVEL 1
#  endif
#else
#  define __SSP_FORTIFY_LEVEL 0
#endif

/* RTEMS adheres to POSIX -- 1003.1b with some features from annexes.  */

#ifdef __rtems__
#define _POSIX_JOB_CONTROL		1
#define _POSIX_SAVED_IDS		1
#define _POSIX_VERSION			199309L
#define _POSIX_ASYNCHRONOUS_IO		1
#define _POSIX_FSYNC			1
#define _POSIX_MAPPED_FILES		1
#define _POSIX_MEMLOCK			1
#define _POSIX_MEMLOCK_RANGE		1
#define _POSIX_MEMORY_PROTECTION	1
#define _POSIX_MESSAGE_PASSING		1
#define _POSIX_MONOTONIC_CLOCK		200112L
#define _POSIX_CLOCK_SELECTION		200112L
#define _POSIX_PRIORITIZED_IO		1
#define _POSIX_PRIORITY_SCHEDULING	1
#define _POSIX_REALTIME_SIGNALS		1
#define _POSIX_SEMAPHORES		1
#define _POSIX_SHARED_MEMORY_OBJECTS	1
#define _POSIX_SYNCHRONIZED_IO		1
#define _POSIX_TIMERS			1
#define _POSIX_BARRIERS                 200112L
#define _POSIX_READER_WRITER_LOCKS      200112L
#define _POSIX_SPIN_LOCKS               200112L


/* In P1003.1b but defined by drafts at least as early as P1003.1c/D10  */
#define _POSIX_THREADS				1
#define _POSIX_THREAD_ATTR_STACKADDR		1
#define _POSIX_THREAD_ATTR_STACKSIZE		1
#define _POSIX_THREAD_PRIORITY_SCHEDULING	1
#define _POSIX_THREAD_PRIO_INHERIT		1
#define _POSIX_THREAD_PRIO_PROTECT		1
#define _POSIX_THREAD_PROCESS_SHARED		1
#define _POSIX_THREAD_SAFE_FUNCTIONS		1

/* P1003.4b/D8 defines the constants below this comment. */
#define _POSIX_SPAWN				1
#define _POSIX_TIMEOUTS				1
#define _POSIX_CPUTIME				1
#define _POSIX_THREAD_CPUTIME			1
#define _POSIX_SPORADIC_SERVER			1
#define _POSIX_THREAD_SPORADIC_SERVER		1
#define _POSIX_DEVICE_CONTROL			1
#define _POSIX_DEVCTL_DIRECTION			1
#define _POSIX_INTERRUPT_CONTROL		1
#define _POSIX_ADVISORY_INFO			1

/* UNIX98 added some new pthread mutex attributes */
#define _UNIX98_THREAD_MUTEX_ATTRIBUTES         1

/* POSIX 1003.26-2003 defined device control method */
#define _POSIX_26_VERSION			200312L

#endif

/* XMK loosely adheres to POSIX -- 1003.1 */
#ifdef __XMK__
#define _POSIX_THREADS				1
#define _POSIX_THREAD_PRIORITY_SCHEDULING	1
#endif


#ifdef __svr4__
# define _POSIX_JOB_CONTROL     1
# define _POSIX_SAVED_IDS       1
# define _POSIX_VERSION 199009L
#endif

#ifdef __cplusplus
}
#endif
#endif /* _SYS_FEATURES_H */
