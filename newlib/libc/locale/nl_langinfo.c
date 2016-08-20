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
 */

#include <sys/cdefs.h>

#include <locale.h>
#include <langinfo.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include "setlocale.h"

#undef offsetoff
#define _O(TYPE, MEMBER)  __builtin_offsetof (TYPE, MEMBER)

#define _NLITEM(cat,memb) { { cat:__get_##cat##_locale }, \
			      _O (struct lc_##cat##_T, memb) }

#ifdef __HAVE_LOCALE_INFO_EXTENDED__
static struct _nl_item_t
{
  union {
    const struct lc_ctype_T *    (*ctype)(struct __locale_t *);
    const struct lc_time_T *     (*time)(struct __locale_t *);
    const struct lc_numeric_T *  (*numeric)(struct __locale_t *);
    const struct lc_monetary_T * (*monetary)(struct __locale_t *);
    const struct lc_messages_T * (*messages)(struct __locale_t *);
    void *			 (*base)(struct __locale_t *);
  };
  _off_t offset;
} nl_ext[] =
{
  /* First element has an nl_item value of _NL_LOCALE_EXTENDED_FIRST_ENTRY */
  _NLITEM (ctype, outdigits[0]),
  _NLITEM (ctype, outdigits[1]),
  _NLITEM (ctype, outdigits[2]),
  _NLITEM (ctype, outdigits[3]),
  _NLITEM (ctype, outdigits[4]),
  _NLITEM (ctype, outdigits[5]),
  _NLITEM (ctype, outdigits[6]),
  _NLITEM (ctype, outdigits[7]),
  _NLITEM (ctype, outdigits[8]),
  _NLITEM (ctype, outdigits[9]),
  _NLITEM (ctype, woutdigits[0]),
  _NLITEM (ctype, woutdigits[1]),
  _NLITEM (ctype, woutdigits[2]),
  _NLITEM (ctype, woutdigits[3]),
  _NLITEM (ctype, woutdigits[4]),
  _NLITEM (ctype, woutdigits[5]),
  _NLITEM (ctype, woutdigits[6]),
  _NLITEM (ctype, woutdigits[7]),
  _NLITEM (ctype, woutdigits[8]),
  _NLITEM (ctype, woutdigits[9]),
  _NLITEM (time, codeset),
  _NLITEM (time, wmon[1]),
  _NLITEM (time, wmon[2]),
  _NLITEM (time, wmon[3]),
  _NLITEM (time, wmon[4]),
  _NLITEM (time, wmon[5]),
  _NLITEM (time, wmon[6]),
  _NLITEM (time, wmon[7]),
  _NLITEM (time, wmon[8]),
  _NLITEM (time, wmon[9]),
  _NLITEM (time, wmon[10]),
  _NLITEM (time, wmon[11]),
  _NLITEM (time, wmon[12]),
  _NLITEM (time, wmonth[1]),
  _NLITEM (time, wmonth[2]),
  _NLITEM (time, wmonth[3]),
  _NLITEM (time, wmonth[4]),
  _NLITEM (time, wmonth[5]),
  _NLITEM (time, wmonth[6]),
  _NLITEM (time, wmonth[7]),
  _NLITEM (time, wmonth[8]),
  _NLITEM (time, wmonth[9]),
  _NLITEM (time, wmonth[10]),
  _NLITEM (time, wmonth[11]),
  _NLITEM (time, wmonth[12]),
  _NLITEM (time, wwday[1]),
  _NLITEM (time, wwday[2]),
  _NLITEM (time, wwday[3]),
  _NLITEM (time, wwday[4]),
  _NLITEM (time, wwday[5]),
  _NLITEM (time, wwday[6]),
  _NLITEM (time, wwday[7]),
  _NLITEM (time, wweekday[1]),
  _NLITEM (time, wweekday[2]),
  _NLITEM (time, wweekday[3]),
  _NLITEM (time, wweekday[4]),
  _NLITEM (time, wweekday[5]),
  _NLITEM (time, wweekday[6]),
  _NLITEM (time, wweekday[7]),
  _NLITEM (time, wX_fmt),
  _NLITEM (time, wx_fmt),
  _NLITEM (time, wc_fmt),
  _NLITEM (time, wam_pm[0]),
  _NLITEM (time, wam_pm[1]),
  _NLITEM (time, wdate_fmt),
  _NLITEM (time, wampm_fmt),
  _NLITEM (time, wera),
  _NLITEM (time, wera_d_fmt),
  _NLITEM (time, wera_d_t_fmt),
  _NLITEM (time, wera_t_fmt),
  _NLITEM (time, walt_digits),
  _NLITEM (numeric, codeset),
  _NLITEM (numeric, grouping),
  _NLITEM (numeric, wdecimal_point),
  _NLITEM (numeric, wthousands_sep),
  _NLITEM (monetary, int_curr_symbol),
  _NLITEM (monetary, currency_symbol),
  _NLITEM (monetary, mon_decimal_point),
  _NLITEM (monetary, mon_thousands_sep),
  _NLITEM (monetary, mon_grouping),
  _NLITEM (monetary, positive_sign),
  _NLITEM (monetary, negative_sign),
  _NLITEM (monetary, int_frac_digits),
  _NLITEM (monetary, frac_digits),
  _NLITEM (monetary, p_cs_precedes),
  _NLITEM (monetary, p_sep_by_space),
  _NLITEM (monetary, n_cs_precedes),
  _NLITEM (monetary, n_sep_by_space),
  _NLITEM (monetary, p_sign_posn),
  _NLITEM (monetary, n_sign_posn),
  _NLITEM (monetary, int_p_cs_precedes),
  _NLITEM (monetary, int_p_sep_by_space),
  _NLITEM (monetary, int_n_cs_precedes),
  _NLITEM (monetary, int_n_sep_by_space),
  _NLITEM (monetary, int_p_sign_posn),
  _NLITEM (monetary, int_n_sign_posn),
  _NLITEM (monetary, codeset),
  _NLITEM (monetary, wint_curr_symbol),
  _NLITEM (monetary, wcurrency_symbol),
  _NLITEM (monetary, wmon_decimal_point),
  _NLITEM (monetary, wmon_thousands_sep),
  _NLITEM (monetary, wpositive_sign),
  _NLITEM (monetary, wnegative_sign),
  _NLITEM (messages, codeset),
  _NLITEM (messages, wyesexpr),
  _NLITEM (messages, wnoexpr),
  _NLITEM (messages, wyesstr),
  _NLITEM (messages, wnostr),
};
#endif /* __HAVE_LOCALE_INFO_EXTENDED__ */

#define _REL(BASE) ((int)item-BASE)

char *nl_langinfo_l (nl_item item, struct __locale_t *locale)
{
   char *ret, *cs;
#ifndef __CYGWIN__
   char *s;
#endif
   static char *csym = NULL;
   char *nptr;

   switch (item) {
#ifdef __HAVE_LOCALE_INFO__
	case _NL_MESSAGES_CODESET:
		ret = (char *) __get_messages_locale (locale)->codeset;
		goto do_codeset;
#ifdef __HAVE_LOCALE_INFO_EXTENDED__
	case _NL_TIME_CODESET:
		ret = (char *) __get_time_locale (locale)->codeset;
		goto do_codeset;
	case _NL_NUMERIC_CODESET:
		ret = (char *) __get_numeric_locale (locale)->codeset;
		goto do_codeset;
	case _NL_MONETARY_CODESET:
		ret = (char *) __get_monetary_locale (locale)->codeset;
		goto do_codeset;
#ifdef __CYGWIN__
	case _NL_COLLATE_CODESET:
		{
		  ret = (char *) __get_collate_locale (locale)->codeset;
		  goto do_codeset;
		}
#endif /* __CYGWIN__ */
#endif /* __HAVE_LOCALE_INFO_EXTENDED__ */
#endif /* __HAVE_LOCALE_INFO__ */
	case CODESET:
#ifdef _MB_CAPABLE
		ret = (char *) __locale_charset (locale);
#endif
do_codeset:
#ifdef __CYGWIN__
		/* Convert charset to Linux compatible codeset string. */
		if (ret[0] == 'A'/*SCII*/)
		  ret = "ANSI_X3.4-1968";
		else if (ret[0] == 'E')
		  {
		    if (strcmp (ret, "EUCJP") == 0)
		      ret = "EUC-JP";
		    else if (strcmp (ret, "EUCKR") == 0)
		      ret = "EUC-KR";
		    else if (strcmp (ret, "EUCCN") == 0)
		      ret = "GB2312";
		  }
		else if (ret[0] == 'C'/*Pxxxx*/)
		  {
		    if (strcmp (ret + 2, "874") == 0)
		      ret = "TIS-620";
		    else if (strcmp (ret + 2, "20866") == 0)
		      ret = "KOI8-R";
		    else if (strcmp (ret + 2, "21866") == 0)
		      ret = "KOI8-U";
		    else if (strcmp (ret + 2, "101") == 0)
		      ret = "GEORGIAN-PS";
		    else if (strcmp (ret + 2, "102") == 0)
		      ret = "PT154";
		  }
		else if (ret[0] == 'S'/*JIS*/)
		  {
		    /* Cygwin uses MSFT's implementation of SJIS, which differs
		       in some codepoints from the real thing, especially
		       0x5c: yen sign instead of backslash,
		       0x7e: overline instead of tilde.
		       We can't use the real SJIS since otherwise Win32
		       pathnames would become invalid.  OTOH, if we return
		       "SJIS" here, then libiconv will do mb<->wc conversion
		       differently to our internal functions.  Therefore we
		       return what we really implement, CP932.  This is handled
		       fine by libiconv. */
		    ret = "CP932";
		  }
#elif !defined (_MB_CAPABLE)
		ret = "US-ASCII";
#endif /* __CYGWIN__ */
		break;
	case D_T_FMT:
		ret = (char *) __get_time_locale (locale)->c_fmt;
		break;
	case D_FMT:
		ret = (char *) __get_time_locale (locale)->x_fmt;
		break;
	case T_FMT:
		ret = (char *) __get_time_locale (locale)->X_fmt;
		break;
	case T_FMT_AMPM:
		ret = (char *) __get_time_locale (locale)->ampm_fmt;
		break;
	case AM_STR:
		ret = (char *) __get_time_locale (locale)->am_pm[0];
		break;
	case PM_STR:
		ret = (char *) __get_time_locale (locale)->am_pm[1];
		break;
	case DAY_1: case DAY_2: case DAY_3:
	case DAY_4: case DAY_5: case DAY_6: case DAY_7:
		ret = (char*) __get_time_locale (locale)->weekday[_REL(DAY_1)];
		break;
	case ABDAY_1: case ABDAY_2: case ABDAY_3:
	case ABDAY_4: case ABDAY_5: case ABDAY_6: case ABDAY_7:
		ret = (char*) __get_time_locale (locale)->wday[_REL(ABDAY_1)];
		break;
	case MON_1: case MON_2: case MON_3: case MON_4:
	case MON_5: case MON_6: case MON_7: case MON_8:
	case MON_9: case MON_10: case MON_11: case MON_12:
		ret = (char*) __get_time_locale (locale)->month[_REL(MON_1)];
		break;
	case ABMON_1: case ABMON_2: case ABMON_3: case ABMON_4:
	case ABMON_5: case ABMON_6: case ABMON_7: case ABMON_8:
	case ABMON_9: case ABMON_10: case ABMON_11: case ABMON_12:
		ret = (char*) __get_time_locale (locale)->mon[_REL(ABMON_1)];
		break;
	case ERA:
		ret = (char*) __get_time_locale (locale)->era;
		break;
	case ERA_D_FMT:
		ret = (char*) __get_time_locale (locale)->era_d_fmt;
		break;
	case ERA_D_T_FMT:
		ret = (char*) __get_time_locale (locale)->era_d_t_fmt;
		break;
	case ERA_T_FMT:
		ret = (char*) __get_time_locale (locale)->era_t_fmt;
		break;
	case ALT_DIGITS:
		ret = (char*) __get_time_locale (locale)->alt_digits;
		break;
	case _DATE_FMT:	/* GNU extension */
		ret = (char*) __get_time_locale (locale)->date_fmt;
		break;
	case RADIXCHAR:
		ret = (char*) __get_numeric_locale (locale)->decimal_point;
		break;
	case THOUSEP:
		ret = (char*) __get_numeric_locale (locale)->thousands_sep;
		break;
	case YESEXPR:
		ret = (char*) __get_messages_locale (locale)->yesexpr;
		break;
	case NOEXPR:
		ret = (char*) __get_messages_locale (locale)->noexpr;
		break;
	/*
	 * All items marked with LEGACY are available, but not recomended
	 * by SUSv2 to be used in portable applications since they're subject
	 * to remove in future specification editions
	 */
	case YESSTR:            /* LEGACY  */
		ret = (char*) __get_messages_locale (locale)->yesstr;
		break;
	case NOSTR:             /* LEGACY  */
		ret = (char*) __get_messages_locale (locale)->nostr;
		break;
	case CRNCYSTR:
		ret = "";
		cs = (char*) __get_monetary_locale (locale)->currency_symbol;
		if (*cs != '\0') {
			char pos = __localeconv_l (locale)->p_cs_precedes;

			if (pos == __localeconv_l (locale)->n_cs_precedes) {
				char psn = '\0';

				if (pos == CHAR_MAX) {
					if (strcmp(cs, __get_monetary_locale (locale)->mon_decimal_point) == 0)
						psn = '.';
				} else
					psn = pos ? '-' : '+';
				if (psn != '\0') {
					int clen = strlen(cs);

                                        nptr = realloc(csym, clen + 2);
                                        if (!nptr && csym)
                                          free (csym);

                                        csym = nptr;

					if (csym != NULL) {
						*csym = psn;
						strcpy(csym + 1, cs);
						ret = csym;
					}
				}
			}
		}
		break;
	case D_MD_ORDER:        /* local extension */
		ret = (char *) __get_time_locale (locale)->md_order;
		break;
#ifdef __HAVE_LOCALE_INFO__
	case _NL_CTYPE_MB_CUR_MAX:
		ret = (char *) __get_ctype_locale (locale)->mb_cur_max;
		break;
#endif
	default:
#ifdef __HAVE_LOCALE_INFO_EXTENDED__
		if (item > _NL_LOCALE_EXTENDED_FIRST_ENTRY
		    && item < _NL_LOCALE_EXTENDED_LAST_ENTRY) {
			int idx = item - _NL_LOCALE_EXTENDED_FIRST_ENTRY - 1;
			return *(char **) ((char *) (*nl_ext[idx].base)(locale)
					   + nl_ext[idx].offset);
		}
#endif
		ret = "";
   }
   return (ret);
}

char *nl_langinfo (nl_item item)
{
  return nl_langinfo_l (item, __get_current_locale ());
}
