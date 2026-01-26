/* NOTE:  This file defines both strftime() and wcsftime().  Take care when
 * making changes.  See also wcsftime.c, and note the (small) overlap in the
 * manual description, taking care to edit both as needed.  */
/*
 * strftime.c
 * Original Author:	G. Haley
 * Additions from:	Eric Blake, Corinna Vinschen
 * Changes to allow dual use as wcstime, also:	Craig Howland
 *
 * Places characters into the array pointed to by s as controlled by the string
 * pointed to by format. If the total number of resulting characters including
 * the terminating null character is not more than maxsize, returns the number
 * of characters placed into the array pointed to by s (not including the
 * terminating null character); otherwise zero is returned and the contents of
 * the array indeterminate.
 */

#define _DEFAULT_SOURCE
#include <time.h>
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

/* This test code relies on ANSI C features, in particular on the ability
 * of adjacent strings to be pasted together into one string.  */

/* Test output buffer size (should be larger than all expected results) */
#define OUTSIZE   256

#define YEAR_BASE 1900

#if !defined(MAKE_WCSFTIME)
#define CHAR             char    /* string type basis */
#define CQ(a)            a       /* character constant qualifier */
#define t_strncmp        strncmp /* char equivalent function name */
#define SFLG                     /* %s flag (null for normal char) */
#define TOLOWER(c)       tolower((int)(unsigned char)(c))
#define STRTOUL(c, p, b) strtoul((c), (p), (b))
#define STRCPY(a, b)     strcpy((a), (b))
#define STRCHR(a, b)     strchr((a), (b))
#define STRLEN(a)        strlen(a)
#else
#define strftime         wcsftime   /* Alternate function name */
#define strftime_l       wcsftime_l /* Alternate function name */
#define CHAR             wchar_t    /* string type basis */
#define CQ(a)            L##a       /* character constant qualifier */
#define t_strncmp        wcsncmp    /* wide-char equivalent function name */
#define TOLOWER(c)       towlower((wint_t)(c))
#define STRTOUL(c, p, b) wcstoul((c), (p), (b))
#define STRCPY(a, b)     wcscpy((a), (b))
#define STRCHR(a, b)     wcschr((a), (b))
#define STRLEN(a)        wcslen(a)
#define SFLG             "l" /* %s flag (l for wide char) */
#endif                       /* MAKE_WCSFTIME */

struct test {
    CHAR  *fmt; /* Testing format */
    size_t max; /* Testing maxsize */
    size_t ret; /* Expected return value */
    CHAR  *out; /* Expected output string */
};
struct list {
    const struct tm   *tms; /* Time used for these vectors */
    const struct test *vec; /* Test vectors */
    int                cnt; /* Number of vectors */
};

const char      TZ[] = "TZ=EST5EDT";

/* Define list of test inputs and expected outputs, for the given time zone
 * and time.  */
const struct tm tm0 = {
    /* Tue Dec 30 10:53:47 EST 2008 (time_t=1230648827) */
    .tm_sec = 47,   .tm_min = 53, .tm_hour = 9,   .tm_mday = 30, .tm_mon = 11,
    .tm_year = 108, .tm_wday = 2, .tm_yday = 364, .tm_isdst = 0
};
const struct test Vec0[] = {
/* Testing fields one at a time, expecting to pass, using exact
 * allowed length as what is needed.  */
/* Using tm0 for time: */
#define EXP(s) sizeof(s) / sizeof(CHAR) - 1, s
    { CQ("%a"), 3 + 1, EXP(CQ("Tue")) },
    { CQ("%A"), 7 + 1, EXP(CQ("Tuesday")) },
    { CQ("%b"), 3 + 1, EXP(CQ("Dec")) },
    { CQ("%B"), 8 + 1, EXP(CQ("December")) },
    { CQ("%c"), 24 + 1, EXP(CQ("Tue Dec 30 09:53:47 2008")) },
    { CQ("%C"), 2 + 1, EXP(CQ("20")) },
    { CQ("%d"), 2 + 1, EXP(CQ("30")) },
    { CQ("%D"), 8 + 1, EXP(CQ("12/30/08")) },
    { CQ("%e"), 2 + 1, EXP(CQ("30")) },
    { CQ("%F"), 10 + 1, EXP(CQ("2008-12-30")) },
    { CQ("%g"), 2 + 1, EXP(CQ("09")) },
    { CQ("%G"), 4 + 1, EXP(CQ("2009")) },
    { CQ("%h"), 3 + 1, EXP(CQ("Dec")) },
    { CQ("%H"), 2 + 1, EXP(CQ("09")) },
    { CQ("%I"), 2 + 1, EXP(CQ("09")) },
    { CQ("%j"), 3 + 1, EXP(CQ("365")) },
    { CQ("%k"), 2 + 1, EXP(CQ(" 9")) },
    { CQ("%l"), 2 + 1, EXP(CQ(" 9")) },
    { CQ("%m"), 2 + 1, EXP(CQ("12")) },
    { CQ("%M"), 2 + 1, EXP(CQ("53")) },
    { CQ("%n"), 1 + 1, EXP(CQ("\n")) },
    { CQ("%p"), 2 + 1, EXP(CQ("AM")) },
    //	{ CQ("%q"), 1+1, EXP(CQ("4")) },
    { CQ("%r"), 11 + 1, EXP(CQ("09:53:47 AM")) },
    { CQ("%R"), 5 + 1, EXP(CQ("09:53")) },
    { CQ("%s"), 10 + 1, EXP(CQ("1230648827")) },
    { CQ("%S"), 2 + 1, EXP(CQ("47")) },
    { CQ("%t"), 1 + 1, EXP(CQ("\t")) },
    { CQ("%T"), 8 + 1, EXP(CQ("09:53:47")) },
    { CQ("%u"), 1 + 1, EXP(CQ("2")) },
    { CQ("%U"), 2 + 1, EXP(CQ("52")) },
    { CQ("%V"), 2 + 1, EXP(CQ("01")) },
    //	{ CQ("%v"), 11+1, EXP(CQ("30-Dec-2008")) },
    { CQ("%w"), 1 + 1, EXP(CQ("2")) },
    { CQ("%W"), 2 + 1, EXP(CQ("52")) },
    { CQ("%x"), 8 + 1, EXP(CQ("12/30/08")) },
    { CQ("%X"), 8 + 1, EXP(CQ("09:53:47")) },
    { CQ("%y"), 2 + 1, EXP(CQ("08")) },
    { CQ("%Y"), 4 + 1, EXP(CQ("2008")) },
    //	{ CQ("%z"), 5+1, EXP(CQ("-0500")) },
    //	{ CQ("%Z"), 3+1, EXP(CQ("EST")) },
    { CQ("%%"), 1 + 1, EXP(CQ("%")) },
#undef EXP
};
/* Define list of test inputs and expected outputs, for the given time zone
 * and time.  */
const struct tm tm1 = {
    /* Wed Jul  2 23:01:13 EDT 2008 (time_t=1215054073) */
    .tm_sec = 13,   .tm_min = 1,  .tm_hour = 23,  .tm_mday = 2, .tm_mon = 6,
    .tm_year = 108, .tm_wday = 3, .tm_yday = 183, .tm_isdst = 1
};
const struct test Vec1[] = {
/* Testing fields one at a time, expecting to pass, using exact
 * allowed length as what is needed.  */
/* Using tm1 for time: */
#define EXP(s) sizeof(s) / sizeof(CHAR) - 1, s
    { CQ("%a"), 3 + 1, EXP(CQ("Wed")) },
    { CQ("%A"), 9 + 1, EXP(CQ("Wednesday")) },
    { CQ("%b"), 3 + 1, EXP(CQ("Jul")) },
    { CQ("%B"), 4 + 1, EXP(CQ("July")) },
    { CQ("%c"), 24 + 1, EXP(CQ("Wed Jul  2 23:01:13 2008")) },
    { CQ("%C"), 2 + 1, EXP(CQ("20")) },
    { CQ("%d"), 2 + 1, EXP(CQ("02")) },
    { CQ("%D"), 8 + 1, EXP(CQ("07/02/08")) },
    { CQ("%e"), 2 + 1, EXP(CQ(" 2")) },
    { CQ("%F"), 10 + 1, EXP(CQ("2008-07-02")) },
    { CQ("%g"), 2 + 1, EXP(CQ("08")) },
    { CQ("%G"), 4 + 1, EXP(CQ("2008")) },
    { CQ("%h"), 3 + 1, EXP(CQ("Jul")) },
    { CQ("%H"), 2 + 1, EXP(CQ("23")) },
    { CQ("%I"), 2 + 1, EXP(CQ("11")) },
    { CQ("%j"), 3 + 1, EXP(CQ("184")) },
    { CQ("%k"), 2 + 1, EXP(CQ("23")) },
    { CQ("%l"), 2 + 1, EXP(CQ("11")) },
    { CQ("%m"), 2 + 1, EXP(CQ("07")) },
    { CQ("%M"), 2 + 1, EXP(CQ("01")) },
    { CQ("%n"), 1 + 1, EXP(CQ("\n")) },
    { CQ("%p"), 2 + 1, EXP(CQ("PM")) },
    //	{ CQ("%q"), 1+1, EXP(CQ("3")) },
    { CQ("%r"), 11 + 1, EXP(CQ("11:01:13 PM")) },
    { CQ("%R"), 5 + 1, EXP(CQ("23:01")) },
    { CQ("%s"), 10 + 1, EXP(CQ("1215054073")) },
    { CQ("%S"), 2 + 1, EXP(CQ("13")) },
    { CQ("%t"), 1 + 1, EXP(CQ("\t")) },
    { CQ("%T"), 8 + 1, EXP(CQ("23:01:13")) },
    { CQ("%u"), 1 + 1, EXP(CQ("3")) },
    { CQ("%U"), 2 + 1, EXP(CQ("26")) },
    { CQ("%V"), 2 + 1, EXP(CQ("27")) },
    //	{ CQ("%v"), 11+1, EXP(CQ(" 2-Jul-2008")) },
    { CQ("%w"), 1 + 1, EXP(CQ("3")) },
    { CQ("%W"), 2 + 1, EXP(CQ("26")) },
    { CQ("%x"), 8 + 1, EXP(CQ("07/02/08")) },
    { CQ("%X"), 8 + 1, EXP(CQ("23:01:13")) },
    { CQ("%y"), 2 + 1, EXP(CQ("08")) },
    { CQ("%Y"), 4 + 1, EXP(CQ("2008")) },
    //	{ CQ("%z"), 5+1, EXP(CQ("-0400")) },
    //	{ CQ("%Z"), 3+1, EXP(CQ("EDT")) },
    { CQ("%%"), 1 + 1, EXP(CQ("%")) },
#undef EXP
#define VEC(s) s, sizeof(s) / sizeof(CHAR), sizeof(s) / sizeof(CHAR) - 1, s
#define EXP(s) sizeof(s) / sizeof(CHAR), sizeof(s) / sizeof(CHAR) - 1, s
    { VEC(CQ("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz")) },
    { CQ("0123456789%%%h:`~"), EXP(CQ("0123456789%Jul:`~")) },
    { CQ("%R%h:`~ %x %w"), EXP(CQ("23:01Jul:`~ 07/02/08 3")) },
#undef VEC
#undef EXP
};

#if YEAR_BASE == 1900 /* ( */
/* Checks for very large years.  YEAR_BASE value relied upon so that the
 * answer strings can be predetermined.
 * Years more than 4 digits are not mentioned in the standard for %C, so the
 * test for those cases are based on the design intent (which is to print the
 * whole number, being the century).  */
const struct tm tmyr0 = {
    /* Wed Jul  2 23:01:13 EDT [HUGE#] */
    .tm_sec = 13, .tm_min = 1,    .tm_hour = 23,
    .tm_mday = 2, .tm_mon = 6,    .tm_year = INT_MAX - YEAR_BASE,
    .tm_wday = 3, .tm_yday = 183, .tm_isdst = 1
};
#if INT_MAX == 32767
#define YEAR CQ("32767") /* INT_MAX */
#define CENT CQ("327")
#define Year CQ("67")
#elif INT_MAX == 2147483647
#define YEAR CQ("2147483647")
#define CENT CQ("21474836")
#define Year CQ("47")
#elif INT_MAX == 9223372036854775807
#define YEAR CQ("9223372036854775807")
#define CENT CQ("92233720368547758")
#define Year CQ("07")
#else
#error "Unrecognized INT_MAX value:  enhance me to recognize what you have"
#endif
const struct test Vecyr0[] = {
/* Testing fields one at a time, expecting to pass, using a larger
 * allowed length than what is needed.  */
/* Using tmyr0 for time: */
#define EXP(s) sizeof(s) / sizeof(CHAR) - 1, s
    { CQ("%C"), OUTSIZE, EXP(CENT) },
    { CQ("%c"), OUTSIZE, EXP(CQ("Wed Jul  2 23:01:13 ") YEAR) },
    { CQ("%D"), OUTSIZE, EXP(CQ("07/02/") Year) },
    { CQ("%F"), OUTSIZE, EXP(YEAR CQ("-07-02")) },
    //	{ CQ("%v"), OUTSIZE, EXP(CQ(" 2-Jul-")YEAR) },
    { CQ("%x"), OUTSIZE, EXP(CQ("07/02/") Year) },
    { CQ("%y"), OUTSIZE, EXP(Year) },
    { CQ("%Y"), OUTSIZE, EXP(YEAR) },
#undef EXP
};
#undef YEAR
#undef CENT
#undef Year
/* Checks for very large negative years.  YEAR_BASE value relied upon so that
 * the answer strings can be predetermined.  */
const struct tm tmyr1 = {
    /* Wed Jul  2 23:01:13 EDT [HUGE#] */
    .tm_sec = 13,       .tm_min = 1,  .tm_hour = 23,  .tm_mday = 2, .tm_mon = 6,
    .tm_year = INT_MIN, .tm_wday = 3, .tm_yday = 183, .tm_isdst = 1
};
#if INT_MAX == 32767
#define YEAR CQ("-30868") /* INT_MIN - YEAR_BASE */
#define CENT CQ("-309")
#define Year CQ("32")
#elif INT_MAX == 2147483647
#define YEAR CQ("-2147481748")
#define CENT CQ("-21474818")
#define Year CQ("52")
#elif INT_MAX == 9223372036854775807
#define YEAR CQ("-9223372036854773908")
#define CENT CQ("-92233720368547740")
#define Year CQ("08")
#else
#error "Unrecognized INT_MAX value:  enhance me to recognize what you have"
#endif
const struct test Vecyr1[] = {
/* Testing fields one at a time, expecting to pass, using a larger
 * allowed length than what is needed.  */
/* Using tmyr1 for time: */
#define EXP(s) sizeof(s) / sizeof(CHAR) - 1, s
    { CQ("%C"), OUTSIZE, EXP(CENT) },
    { CQ("%c"), OUTSIZE, EXP(CQ("Wed Jul  2 23:01:13 ") YEAR) },
    { CQ("%D"), OUTSIZE, EXP(CQ("07/02/") Year) },
    { CQ("%F"), OUTSIZE, EXP(YEAR CQ("-07-02")) },
    //	{ CQ("%v"), OUTSIZE, EXP(CQ(" 2-Jul-")YEAR) },
    { CQ("%x"), OUTSIZE, EXP(CQ("07/02/") Year) },
    { CQ("%y"), OUTSIZE, EXP(Year) },
    { CQ("%Y"), OUTSIZE, EXP(YEAR) },
#undef EXP
};
#undef YEAR
#undef CENT
#undef Year
#endif /* YEAR_BASE ) */

/* Checks for years just over zero (also test for s=60).
 * Years less than 4 digits are not mentioned for %Y in the standard, so the
 * test for that case is based on the design intent.  */
const struct tm tmyrzp = {
    /* Wed Jul  2 23:01:60 EDT 0007 */
    .tm_sec = 60, .tm_min = 1,    .tm_hour = 23,
    .tm_mday = 2, .tm_mon = 6,    .tm_year = 7 - YEAR_BASE,
    .tm_wday = 3, .tm_yday = 183, .tm_isdst = 1
};
#define YEAR CQ("7") /* Design intent:  %Y=%C%y */
#define CENT CQ("0")
#define Year CQ("07")
const struct test Vecyrzp[] = {
/* Testing fields one at a time, expecting to pass, using a larger
 * allowed length than what is needed.  */
/* Using tmyrzp for time: */
#define EXP(s) sizeof(s) / sizeof(CHAR) - 1, s
    { CQ("%C"), OUTSIZE, EXP(CENT) },
    { CQ("%c"), OUTSIZE, EXP(CQ("Wed Jul  2 23:01:60 ") YEAR) },
    { CQ("%D"), OUTSIZE, EXP(CQ("07/02/") Year) },
    { CQ("%F"), OUTSIZE, EXP(YEAR CQ("-07-02")) },
    //	{ CQ("%v"), OUTSIZE, EXP(CQ(" 2-Jul-")YEAR) },
    { CQ("%x"), OUTSIZE, EXP(CQ("07/02/") Year) },
    { CQ("%y"), OUTSIZE, EXP(Year) },
    { CQ("%Y"), OUTSIZE, EXP(YEAR) },
#undef EXP
};
#undef YEAR
#undef CENT
#undef Year
/* Checks for years just under zero.
 * Negative years are not handled by the standard, so the vectors here are
 * verifying the chosen implemtation.  */
const struct tm tmyrzn = {
    /* Wed Jul  2 23:01:00 EDT -004 */
    .tm_sec = 00, .tm_min = 1,    .tm_hour = 23,
    .tm_mday = 2, .tm_mon = 6,    .tm_year = -4 - YEAR_BASE,
    .tm_wday = 3, .tm_yday = 183, .tm_isdst = 1
};
#define YEAR CQ("-4")
#define CENT CQ("-1")
#define Year CQ("96")
const struct test Vecyrzn[] = {
/* Testing fields one at a time, expecting to pass, using a larger
 * allowed length than what is needed.  */
/* Using tmyrzn for time: */
#define EXP(s) sizeof(s) / sizeof(CHAR) - 1, s
    { CQ("%C"), OUTSIZE, EXP(CENT) },
    { CQ("%c"), OUTSIZE, EXP(CQ("Wed Jul  2 23:01:00 ") YEAR) },
    { CQ("%D"), OUTSIZE, EXP(CQ("07/02/") Year) },
    { CQ("%F"), OUTSIZE, EXP(YEAR CQ("-07-02")) },
    //	{ CQ("%v"), OUTSIZE, EXP(CQ(" 2-Jul-")YEAR) },
    { CQ("%x"), OUTSIZE, EXP(CQ("07/02/") Year) },
    { CQ("%y"), OUTSIZE, EXP(Year) },
    { CQ("%Y"), OUTSIZE, EXP(YEAR) },
#undef EXP
};
#undef YEAR
#undef CENT
#undef Year

const struct list ListYr[] = {
    { &tmyrzp, Vecyrzp, sizeof(Vecyrzp) / sizeof(Vecyrzp[0]) },
    { &tmyrzn, Vecyrzn, sizeof(Vecyrzn) / sizeof(Vecyrzn[0]) },
#if YEAR_BASE == 1900
    { &tmyr0, Vecyr0, sizeof(Vecyr0) / sizeof(Vecyr0[0]) },
    { &tmyr1, Vecyr1, sizeof(Vecyr1) / sizeof(Vecyr1[0]) },
#endif
};

/* List of tests to be run */
const struct list List[] = {
    { &tm0, Vec0, sizeof(Vec0) / sizeof(Vec0[0]) },
    { &tm1, Vec1, sizeof(Vec1) / sizeof(Vec1[0]) },
};

int
main(void)
{
    int         i, errr = 0, erro = 0, tot = 0;
    const char *cp;
    CHAR        out[OUTSIZE];
    size_t      ret;
    size_t      l;

    /* Set timezone so that %z and %Z tests come out right */
    cp = TZ;
    if ((i = putenv((char *)cp))) {
        printf("putenv(%s) FAILED, ret %d\n", cp, i);
        return (-1);
    }
    if (strcmp(getenv("TZ"), strchr(TZ, '=') + 1)) {
        printf("TZ not set properly in environment\n");
        return (-2);
    }
    tzset();

#if defined(VERBOSE)
    printf("_timezone=%d, _daylight=%d, _tzname[0]=%s, _tzname[1]=%s\n", _timezone, _daylight,
           _tzname[0], _tzname[1]);
    {
        long           offset;
        __tzinfo_type *tz = __gettzinfo();
        /* The sign of this is exactly opposite the envvar TZ.  We
           could directly use the global _timezone for tm_isdst==0,
           but have to use __tzrule for daylight savings.  */
        printf("tz->__tzrule[0].offset=%d, tz->__tzrule[1].offset=%d\n", tz->__tzrule[0].offset,
               tz->__tzrule[1].offset);
    }
#endif

    /* Run all of the exact-length tests as-given--results should match */
    for (l = 0; l < sizeof(List) / sizeof(List[0]); l++) {
        const struct list *test = &List[l];
        for (i = 0; i < test->cnt; i++) {
            tot++; /* Keep track of number of tests */
            memset(out, 0, sizeof(out));
            ret = strftime(out, test->vec[i].max, test->vec[i].fmt, test->tms);
            if (ret != test->vec[i].ret) {
                errr++;
                fprintf(stderr,
                        "ERROR:  return %zd != %zd expected for List[%zd].vec[%d] \"%" SFLG "s\"\n",
                        ret, test->vec[i].ret, l, i, test->vec[i].fmt);
            }
            if (t_strncmp(out, test->vec[i].out, test->vec[i].max - 1)) {
                erro++;
                fprintf(stderr,
                        "ERROR:  \"%" SFLG "s\" != \"%" SFLG
                        "s\" expected for List[%zd].vec[%d] \"%" SFLG "s\"\n",
                        out, test->vec[i].out, l, i, test->vec[i].fmt);
            }
        }
    }

    /* Run all of the exact-length tests with the length made too short--expect to
     * fail.  */
    for (l = 0; l < sizeof(List) / sizeof(List[0]); l++) {
        const struct list *test = &List[l];
        for (i = 0; i < test->cnt; i++) {
            tot++; /* Keep track of number of tests */
            memset(out, 0, sizeof(out));
            ret = strftime(out, test->vec[i].max - 1, test->vec[i].fmt, test->tms);
            if (ret != 0) {
                errr++;
                fprintf(stderr,
                        "ERROR:  return %zd != %d expected for List[%zd].vec[%d] \"%" SFLG "s\"\n",
                        ret, 0, l, i, test->vec[i].fmt);
            }
#if 0
            /* Almost every conversion puts out as many characters as possible, so
             * go ahead and test the output even though have failed.  (The test
             * times chosen happen to not hit any of the cases that fail this, so it
             * works.)  */
            if(t_strncmp(out, test->vec[i].out, test->vec[i].max-1-1))  {
                erro++;
                fprintf(stderr,
                        "ERROR:  \"%"SFLG"s\" != \"%"SFLG"s\" expected for List[%zd].vec[%d] \"%"SFLG"s\"\n",
                        out, test->vec[i].out, l, i, test->vec[i].fmt);
	    }
#endif
        }
    }

    /* Run all of the special year test cases */
    for (l = 0; l < sizeof(ListYr) / sizeof(ListYr[0]); l++) {
        const struct list *test = &ListYr[l];
        for (i = 0; i < test->cnt; i++) {
            tot++; /* Keep track of number of tests */
            memset(out, 0, sizeof(out));
            ret = strftime(out, test->vec[i].max, test->vec[i].fmt, test->tms);
            if (ret != test->vec[i].ret) {
                errr++;
                fprintf(stderr,
                        "ERROR:  return %zd != %zd expected for ListYr[%zd].vec[%d] \"%" SFLG
                        "s\"\n",
                        ret, test->vec[i].ret, l, i, test->vec[i].fmt);
            }
            if (t_strncmp(out, test->vec[i].out, test->vec[i].max - 1)) {
                erro++;
                fprintf(stderr,
                        "ERROR:  \"%" SFLG "s\" != \"%" SFLG
                        "s\" expected for ListYr[%zd].vec[%d] \"%" SFLG "s\"\n",
                        out, test->vec[i].out, l, i, test->vec[i].fmt);
            }
        }
    }

#define STRIZE(f) #f
#define NAME(f)   STRIZE(f)
    printf(NAME(strftime) "() test ");
    if (errr || erro)
        printf("FAILED %d/%d of", errr, erro);
    else
        printf("passed");
    printf(" %d test cases.\n", tot);

    return (errr || erro);
}
