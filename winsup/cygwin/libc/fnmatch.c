/*
 * Copyright (c) 1989, 1993, 1994
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Guido van Rossum.
 *
 * Copyright (c) 2011 The FreeBSD Foundation
 * All rights reserved.
 * Portions of this software were developed by David Chisnall
 * under sponsorship from the FreeBSD Foundation.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
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
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)fnmatch.c	8.2 (Berkeley) 4/16/94";
#endif /* LIBC_SCCS and not lint */
#include "winsup.h"
#include <sys/cdefs.h>
__FBSDID("$FreeBSD: head/lib/libc/gen/fnmatch.c 288309 2015-09-27 12:52:18Z jilles $");

/*
 * Function fnmatch() as specified in POSIX 1003.2-1992, section B.6.
 * Compares a filename or pathname to a pattern.
 */

/*
 * Some notes on multibyte character support:
 * 1. Patterns with illegal byte sequences match nothing.
 * 2. Illegal byte sequences in the "string" argument are handled by treating
 *    them as single-byte characters with a value of the first byte of the
 *    sequence cast to wint_t.
 * 3. Multibyte conversion state objects (mbstate_t) are passed around and
 *    used for most, but not all, conversions. Further work will be required
 *    to support state-dependent encodings.
 */

#include <fnmatch.h>
#include <limits.h>
#include <string.h>
#include <wchar.h>
#include <wctype.h>

#include "collate.h"

#define	EOS	'\0'

#define RANGE_MATCH     1
#define RANGE_NOMATCH   0
#define RANGE_ERROR     (-1)

static int rangematch(const wint_t *, wint_t *, int, wint_t **, mbstate_t *);

int
fnmatch(const char *in_pattern, const char *in_string, int flags)
{
	size_t pclen = strlen (in_pattern);
	size_t sclen = strlen (in_string);
	wint_t *pattern = (wint_t *) alloca ((pclen + 1) * sizeof (wint_t));
	wint_t *string = (wint_t *) alloca ((sclen + 1) * sizeof (wint_t));

	const wint_t *stringstart = string;
	const wint_t *bt_pattern, *bt_string;
	mbstate_t patmbs = { 0 };
	mbstate_t strmbs = { 0 };
	mbstate_t bt_patmbs, bt_strmbs;
	wint_t *newp;
	wint_t *c;
	wint_t *pc, *sc;

	pclen = mbsnrtowci (pattern, &in_pattern, (size_t) -1, pclen, &patmbs);
	if (pclen == (size_t) -1)
		return (FNM_NOMATCH);
	pattern[pclen] = '\0';
	sclen = mbsnrtowci (string, &in_string, (size_t) -1, sclen, &strmbs);
	if (sclen == (size_t) -1)
		return (FNM_NOMATCH);
	string[sclen] = '\0';

	bt_pattern = bt_string = NULL;
	for (;;) {
		pc = pattern++;
		sc = string;
		switch (*pc) {
		case EOS:
			if ((flags & FNM_LEADING_DIR) && *sc == '/')
				return (0);
			if (*sc == EOS)
				return (0);
			goto backtrack;
		case '?':
			if (*sc == EOS)
				return (FNM_NOMATCH);
			if (*sc == '/' && (flags & FNM_PATHNAME))
				goto backtrack;
			if (*sc == '.' && (flags & FNM_PERIOD) &&
			    (string == stringstart ||
			    ((flags & FNM_PATHNAME) && *(string - 1) == '/')))
				goto backtrack;
			++string;
			break;
		case '*':
			c = pattern;
			/* Collapse multiple stars. */
			while (*c == '*')
				*c = *++pattern;

			if (*sc == '.' && (flags & FNM_PERIOD) &&
			    (string == stringstart ||
			    ((flags & FNM_PATHNAME) && *(string - 1) == '/')))
				goto backtrack;

			/* Optimize for pattern with * at end or before /. */
			if (*c == EOS)
				if (flags & FNM_PATHNAME)
					return ((flags & FNM_LEADING_DIR) ||
					    wcichr(string, '/') == NULL ?
					    0 : FNM_NOMATCH);
				else
					return (0);
			else if (*c == '/' && flags & FNM_PATHNAME) {
				if ((string = wcichr(string, '/')) == NULL)
					return (FNM_NOMATCH);
				break;
			}

			/*
			 * First try the shortest match for the '*' that
			 * could work. We can forget any earlier '*' since
			 * there is no way having it match more characters
			 * can help us, given that we are already here.
			 */
			bt_pattern = pattern;
			bt_patmbs = patmbs;
			bt_string = string;
			bt_strmbs = strmbs;
			break;
		case '[':
			if (*sc == EOS)
				return (FNM_NOMATCH);
			if (*sc == '/' && (flags & FNM_PATHNAME))
				goto backtrack;
			if (*sc == '.' && (flags & FNM_PERIOD) &&
			    (string == stringstart ||
			    ((flags & FNM_PATHNAME) && *(string - 1) == '/')))
				goto backtrack;

			int ret = rangematch(pattern, sc, flags, &newp,
					     &patmbs);
			switch (ret) {
			case RANGE_ERROR:
				goto norm;
			case RANGE_NOMATCH:
				goto backtrack;
			default: /* > 0 ... case RANGE_MATCH */
				pattern = newp;
				break;
			}
			string += ret;
			break;
		case '\\':
			if (!(flags & FNM_NOESCAPE)) {
				pc = pattern++;
			}
			fallthrough;
		default:
		norm:
			++string;
			if (*pc == *sc)
				;
			else if ((flags & FNM_CASEFOLD) &&
				 (towlower(*pc) == towlower(*sc)))
				;
			else {
		backtrack:
				/*
				 * If we have a mismatch (other than hitting
				 * the end of the string), go back to the last
				 * '*' seen and have it match one additional
				 * character.
				 */
				if (bt_pattern == NULL)
					return (FNM_NOMATCH);
				sc = (wint_t *) bt_string;
				if (*sc == EOS)
					return (FNM_NOMATCH);
				if (*sc == '/' && flags & FNM_PATHNAME)
					return (FNM_NOMATCH);
				++bt_string;
				pattern = (wint_t *) bt_pattern;
				patmbs = bt_patmbs;
				string = (wint_t *) bt_string;
				strmbs = bt_strmbs;
			}
			break;
		}
	}
	/* NOTREACHED */
}

/* Return value is either '\0', ':', '.', '=', or '[' if no class
   expression found.  cptr_p is set to the next character which needs
   checking. */
static inline wint_t
check_classes_expr(const wint_t **cptr_p, wint_t *classbuf, size_t classbufsize)
{
	const wint_t *ctype = NULL;
	const wint_t *cptr = *cptr_p;

	if (*cptr == '[' &&
	    (cptr[1] == ':' || cptr[1] == '.' || cptr[1] == '=')) {
		ctype = ++cptr;
		while (*++cptr && (*cptr != *ctype || cptr[1] != ']'))
			;
		if (!*cptr)
			return '\0';
		if (classbuf) {
			const wint_t *class_p = ctype + 1;
			size_t clen = cptr - class_p;

			if (clen < classbufsize)
				*wcipncpy (classbuf, class_p, clen) = '\0';
			else
				ctype = NULL;
		}
		cptr += 2; /* Advance cptr to next char after class expr. */
	}
	*cptr_p = cptr;
	return ctype ? *ctype : '[';
}

static int
rangematch(const wint_t *pattern, wint_t *test, int flags, wint_t **newp,
    mbstate_t *patmbs)
{
	int negate, ok;
	wint_t *c, *c2;
	//size_t pclen;
	const wint_t *origpat;
	size_t tlen = next_unicode_char (test);

	/*
	 * A bracket expression starting with an unquoted circumflex
	 * character produces unspecified results (IEEE 1003.2-1992,
	 * 3.13.2).  This implementation treats it like '!', for
	 * consistency with the regular expression syntax.
	 * J.T. Conklin (conklin@ngai.kaleida.com)
	 */
	if ( (negate = (*pattern == '!' || *pattern == '^')) )
		++pattern;

	if (flags & FNM_CASEFOLD) {
		for (int idx = 0; idx < tlen; ++idx)
			test[idx] = towlower(test[idx]);
	}

	/*
	 * A right bracket shall lose its special meaning and represent
	 * itself in a bracket expression if it occurs first in the list.
	 * -- POSIX.2 2.8.3.2
	 */
	ok = 0;
	origpat = pattern;
	for (;;) {
		wint_t wclass[64], wclass2[64];
		char cclass[64];
		wint_t ctype;
		size_t clen = 1, c2len = 1;

		if (*pattern == ']' && pattern > origpat) {
			pattern++;
			break;
		} else if (*pattern == '\0') {
			return (RANGE_ERROR);
		} else if (*pattern == '/' && (flags & FNM_PATHNAME)) {
			return (RANGE_NOMATCH);
		} else if (*pattern == '\\' && !(flags & FNM_NOESCAPE))
			pattern++;
		switch (ctype = check_classes_expr (&pattern, wclass, 64)) {
		case ':':
			/* No worries, char classes are ASCII-only */
			wcitoascii (cclass, wclass);
			if (iswctype (*test, wctype (cclass)))
				ok = 1;
			continue;
		case '=':
			if (wcilen (wclass) == 1 &&
			    is_unicode_equiv (*test, *wclass))
				ok = 1;
			continue;
		case '.':
			if (!is_unicode_coll_elem (wclass))
				return (RANGE_NOMATCH);
			c = wclass;
			clen = wcilen (wclass);
			break;
		default:
			c = (wint_t *) pattern++;
			break;
		}
		if (flags & FNM_CASEFOLD) {
			for (int idx = 0; idx < tlen; ++idx)
				c[idx] = towlower(c[idx]);
		}

		if (*pattern == '-' && *(pattern + 1) != EOS &&
		    *(pattern + 1) != ']') {
			if (*++pattern == '\\' && !(flags & FNM_NOESCAPE))
				if (*pattern != EOS)
					pattern++;
			const wint_t *orig_pattern = pattern;
			switch (ctype = check_classes_expr (&pattern, wclass2,
							    64)) {
			case '.':
				if (!is_unicode_coll_elem (wclass2))
					return (RANGE_NOMATCH);
				c2 = wclass2;
				c2len = wcilen (wclass2);
				break;
			default:
				pattern = orig_pattern;
				c2 = (wint_t *) pattern++;
			}
			if (*c2 == EOS)
				return (RANGE_ERROR);

			if (flags & FNM_CASEFOLD) {
				for (int idx = 0; idx < tlen; ++idx)
					c2[idx] = towlower(c2[idx]);
			}

			if ((!__get_current_collate_locale ()->win_locale[0]) ?
			    c <= test && test <= c2 :
			       __wscollate_range_cmp(c, test, clen, tlen) <= 0
			    && __wscollate_range_cmp(test, c2, tlen, c2len) <= 0
			   )
				ok = 1;
		} else if (clen == tlen && wcincmp (c, test, clen) == 0)
			ok = 1;
	}

	*newp = (wint_t *) pattern;
	return (ok == negate ? RANGE_NOMATCH : tlen);
}
