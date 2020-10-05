/* localtime.cc: Wrapper of NetBSD tzcode support for Cygwin. See README file.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include "perprocess.h"
#include "tz_posixrules.h"
#include <cygwin/version.h>
#include <stdlib.h>
#include <sys/_tz_structs.h>

static NO_COPY SRWLOCK tzset_guard = SRWLOCK_INIT;

// Convert these NetBSD rwlock ops into SRWLocks
#define rwlock_wrlock(X) AcquireSRWLockExclusive(&tzset_guard)
#define rwlock_unlock(X) ReleaseSRWLockExclusive(&tzset_guard)

// Set these NetBSD-related option #defines appropriately for Cygwin
//#define STD_INSPIRED	// early-include private.h below does this
#define lint
#define HAVE_POSIX_DECLS 0
#define USG_COMPAT 1
#define NO_ERROR_IN_DST_GAP
#define state __state

// Turn a specific known kind of const parameter into non-const
#define __UNCONST(X) ((char *) (X))

// Turn off these NetBSD audit-related definitions
#define __aconst
#define _DIAGASSERT(X)

// Supply this Cygwin-specific function in advance of its use in localtime.c
static char *
tzgetwintzi (char *wildabbr, char *outbuf)
{
    TIME_ZONE_INFORMATION tzi;
    char *cp, *dst;
    wchar_t *wsrc;
    div_t d;

    GetTimeZoneInformation (&tzi);
    dst = cp = outbuf;
    for (wsrc = tzi.StandardName; *wsrc; wsrc++)
	if (*wsrc >= L'A' && *wsrc <= L'Z')
	    *dst++ = *wsrc;
    if ((dst - cp) < 3)
      {
	/* In non-english Windows, converted tz.StandardName
	   may not contain a valid standard timezone name. */
	strcpy (cp, wildabbr);
	cp += strlen (wildabbr);
      }
    else
	cp = dst;
    d = div (tzi.Bias + tzi.StandardBias, 60);
    __small_sprintf (cp, "%d", d.quot);
    if (d.rem)
	__small_sprintf (cp = strchr (cp, 0), ":%d", abs (d.rem));
    if (tzi.StandardDate.wMonth)
      {
	cp = strchr (cp, 0);
	dst = cp;
	for (wsrc = tzi.DaylightName; *wsrc; wsrc++)
	    if (*wsrc >= L'A' && *wsrc <= L'Z')
		*dst++ = *wsrc;
	if ((dst - cp) < 3)
	  {
	    /* In non-english Windows, converted tz.DaylightName
	       may not contain a valid daylight timezone name. */
	    strcpy (cp, wildabbr);
	    cp += strlen (wildabbr);
	  }
	else
	    cp = dst;
	d = div (tzi.Bias + tzi.DaylightBias, 60);
	__small_sprintf (cp, "%d", d.quot);
	if (d.rem)
	    __small_sprintf (cp = strchr (cp, 0), ":%d", abs (d.rem));
	cp = strchr (cp, 0);
	__small_sprintf (cp = strchr (cp, 0), ",M%d.%d.%d/%d",
			 tzi.DaylightDate.wMonth,
			 tzi.DaylightDate.wDay,
			 tzi.DaylightDate.wDayOfWeek,
			 tzi.DaylightDate.wHour);
	if (tzi.DaylightDate.wMinute || tzi.DaylightDate.wSecond)
	    __small_sprintf (cp = strchr (cp, 0), ":%d",
			     tzi.DaylightDate.wMinute);
	if (tzi.DaylightDate.wSecond)
	    __small_sprintf (cp = strchr (cp, 0), ":%d",
			     tzi.DaylightDate.wSecond);
	cp = strchr (cp, 0);
	__small_sprintf (cp = strchr (cp, 0), ",M%d.%d.%d/%d",
			 tzi.StandardDate.wMonth,
			 tzi.StandardDate.wDay,
			 tzi.StandardDate.wDayOfWeek,
			 tzi.StandardDate.wHour);
	if (tzi.StandardDate.wMinute || tzi.StandardDate.wSecond)
	    __small_sprintf (cp = strchr (cp, 0), ":%d",
			     tzi.StandardDate.wMinute);
	if (tzi.StandardDate.wSecond)
	    __small_sprintf (cp = strchr (cp, 0), ":%d",
			     tzi.StandardDate.wSecond);
      }
    /* __small_printf ("TZ deduced as `%s'\n", outbuf); */
    return outbuf;
}

// Pull these in early to catch any small issues before the real test
#include "private.h"
#include "tzfile.h"

/* Some NetBSD differences were too difficult to work around..
   so #include a patched copy of localtime.c rather than the NetBSD original.
   Here is a list of the patches...
   (1) fix an erroneous decl of tzdirslash size (flagged by g++)
   (2) add conditional call to Cygwin's tzgetwintzi() from tzsetlcl()
   (3) add Cygwin's historical "posixrules" support to tzloadbody()
*/
#include "localtime.patched.c"

// Don't forget these Cygwin-specific additions from this point to EOF
EXPORT_ALIAS (tzset_unlocked, _tzset_unlocked)

long
__cygwin_gettzoffset (const struct tm *tmp)
{
#ifdef TM_GMTOFF
    if (CYGWIN_VERSION_CHECK_FOR_EXTRA_TM_MEMBERS)
	return tmp->TM_GMTOFF;
#endif /* defined TM_GMTOFF */
    __tzinfo_type *tz = __gettzinfo ();
    /* The sign of this is exactly opposite the envvar TZ.  We
       could directly use the global _timezone for tm_isdst==0,
       but have to use __tzrule for daylight savings.  */
    long offset = -tz->__tzrule[tmp->tm_isdst > 0].offset;
    return offset;
}

const char *
__cygwin_gettzname (const struct tm *tmp)
{
#ifdef TM_ZONE
    if (CYGWIN_VERSION_CHECK_FOR_EXTRA_TM_MEMBERS)
	return tmp->TM_ZONE;
#endif
    return _tzname[tmp->tm_isdst > 0];
}
