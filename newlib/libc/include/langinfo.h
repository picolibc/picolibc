/*-
 * Copyright (c) 2001 Alexey Zelkin <phantom@FreeBSD.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $FreeBSD: src/include/langinfo.h,v 1.5 2002/03/23 17:24:53 imp Exp $
 */

#ifndef _LANGINFO_H_
#define	_LANGINFO_H_

#include <sys/cdefs.h>
#include <sys/_types.h>
#if __POSIX_VISIBLE >= 200809
#include <sys/_locale.h>
#endif

#ifndef _NL_ITEM_DECLARED
typedef __nl_item nl_item;
#define _NL_ITEM_DECLARED
#endif

enum
{
  /* POSIX and BSD defined items have to stick to the original values
     to maintain backward compatibility. */
  _NL_CTYPE_CODESET_NAME = 0,	/* codeset name */
#define CODESET _NL_CTYPE_CODESET_NAME
  D_T_FMT,			/* string for formatting date and time */
#define D_T_FMT D_T_FMT
  D_FMT,			/* date format string */
#define D_FMT D_FMT
  T_FMT,			/* time format string */
#define T_FMT T_FMT
  T_FMT_AMPM,			/* a.m. or p.m. time formatting string */
#define T_FMT_AMPM T_FMT_AMPM
  AM_STR,			/* Ante Meridian affix */
#define AM_STR AM_STR
  PM_STR,			/* Post Meridian affix */
#define PM_STR PM_STR

/* week day names */
  DAY_1,
#define DAY_1 DAY_1
  DAY_2,
#define DAY_2 DAY_2
  DAY_3,
#define DAY_3 DAY_3
  DAY_4,
#define DAY_4 DAY_4
  DAY_5,
#define DAY_5 DAY_5
  DAY_6,
#define DAY_6 DAY_6
  DAY_7,
#define DAY_7 DAY_7

/* abbreviated week day names */
  ABDAY_1,
#define ABDAY_1 ABDAY_1
  ABDAY_2,
#define ABDAY_2 ABDAY_2
  ABDAY_3,
#define ABDAY_3 ABDAY_3
  ABDAY_4,
#define ABDAY_4 ABDAY_4
  ABDAY_5,
#define ABDAY_5 ABDAY_5
  ABDAY_6,
#define ABDAY_6 ABDAY_6
  ABDAY_7,
#define ABDAY_7 ABDAY_7

/* month names */
  MON_1,
#define MON_1 MON_1
  MON_2,
#define MON_2 MON_2
  MON_3,
#define MON_3 MON_3
  MON_4,
#define MON_4 MON_4
  MON_5,
#define MON_5 MON_5
  MON_6,
#define MON_6 MON_6
  MON_7,
#define MON_7 MON_7
  MON_8,
#define MON_8 MON_8
  MON_9,
#define MON_9 MON_9
  MON_10,
#define MON_10 MON_10
  MON_11,
#define MON_11 MON_11
  MON_12,
#define MON_12 MON_12

/* abbreviated month names */
  ABMON_1,
#define ABMON_1 ABMON_1
  ABMON_2,
#define ABMON_2 ABMON_2
  ABMON_3,
#define ABMON_3 ABMON_3
  ABMON_4,
#define ABMON_4 ABMON_4
  ABMON_5,
#define ABMON_5 ABMON_5
  ABMON_6,
#define ABMON_6 ABMON_6
  ABMON_7,
#define ABMON_7 ABMON_7
  ABMON_8,
#define ABMON_8 ABMON_8
  ABMON_9,
#define ABMON_9 ABMON_9
  ABMON_10,
#define ABMON_10 ABMON_10
  ABMON_11,
#define ABMON_11 ABMON_11
  ABMON_12,
#define ABMON_12 ABMON_12

  ERA,				/* era description segments */
#define ERA ERA
  ERA_D_FMT,			/* era date format string */
#define ERA_D_FMT ERA_D_FMT
  ERA_D_T_FMT,			/* era date and time format string */
#define ERA_D_T_FMT ERA_D_T_FMT
  ERA_T_FMT,			/* era time format string */
#define ERA_T_FMT ERA_T_FMT
  ALT_DIGITS,			/* alternative symbols for digits */
#define ALT_DIGITS ALT_DIGITS

  RADIXCHAR,			/* radix char */
#define RADIXCHAR RADIXCHAR
  THOUSEP,			/* separator for thousands */
#define THOUSEP THOUSEP

  YESEXPR,			/* affirmative response expression */
#define YESEXPR YESEXPR
  NOEXPR,			/* negative response expression */
#define NOEXPR NOEXPR
  YESSTR,			/* affirmative response for yes/no queries */
#define YESSTR YESSTR
  NOSTR,			/* negative response for yes/no queries */
#define NOSTR NOSTR

  CRNCYSTR,			/* currency symbol */
#define CRNCYSTR CRNCYSTR

};

_BEGIN_STD_C
char	*nl_langinfo (nl_item);
#if __POSIX_VISIBLE >= 200809
char	*nl_langinfo_l (nl_item, locale_t);
#endif
_END_STD_C

#endif /* !_LANGINFO_H_ */
