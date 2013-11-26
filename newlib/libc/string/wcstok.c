/*
FUNCTION
	<<wcstok>>---get next token from a string

INDEX
	wcstok


ANSI_SYNOPSIS
	#include <wchar.h>
      	wchar_t *wcstok(wchar_t *__restrict <[source]>,
      			const wchar_t *__restrict <[delimiters]>,
			wchar_t **__restrict <[lasts]>)

TRAD_SYNOPSIS
	#include <wchar.h>
	wchar_t *wcstok(<[source]>, <[delimiters]>, <[lasts]>)
	wchar_t *__restrict <[source]>;
	wchar_t *__restrict <[delimiters]>;
	wchar_t **__restrict <[lasts]>;

DESCRIPTION
	The <<wcstok>> function is the wide-character equivalent of the
	<<strtok_r>> function (which in turn is the same as the <<strtok>>
	function with an added argument to make it thread-safe).

	The <<wcstok>> function is used to isolate (one at a time)
	sequential tokens in a null-terminated wide-character string,
	<<*<[source]>>>.  A token is defined as a substring not containing
	any wide-characters from <<*<[delimiters]>>>.

	The first time that <<wcstok>> is called, <<*<[source]>>> should
	be specified with the wide-character string to be searched, and
	<<*<[lasts]>>>--but not <<lasts>>, which must be non-NULL--may be
	random; subsequent calls, wishing to obtain further tokens from
	the same string, should pass a null pointer for <<*<[source]>>>
	instead but must supply <<*<[lasts]>>> unchanged from the last
	call.  The separator wide-character string, <<*<[delimiters]>>>,
	must be supplied each time and may change between calls.
	A pointer to placeholder <<*<[lasts]>>> must be supplied by
	the caller, and is set each time as needed to save the state
	by <<wcstok>>.	Every call to <<wcstok>> with <<*<[source]>>>
	== <<NULL>> must pass the value of <<*<[lasts]>>> as last set
	by <<wcstok>>.

	The <<wcstok>> function returns a pointer to the beginning of each 
	subsequent token in the string, after replacing the separator 
	wide-character itself with a null wide-character.  When no more tokens
	remain, a null pointer is returned.

RETURNS
	<<wcstok>> returns a pointer to the first wide character of a token, or
	<<NULL>> if there is no token.

NOTES
	<<wcstok>> is thread-safe (unlike <<strtok>>, but like <<strtok_r>>).
	<<wcstok>> writes into the string being searched.

PORTABILITY
<<wcstok>> is C99 and POSIX.1-2001.

<<wcstok>> requires no supporting OS subroutines.

QUICKREF
	strtok ansi pure
*/
/* wcstok for Newlib created by adapting strtok_r, 2008.  */
/*
 * Copyright (c) 1988 Regents of the University of California.
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
 * 3. Neither the name of the University nor the names of its contributors
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

#include <wchar.h>

wchar_t *
_DEFUN (wcstok, (s, delim, lasts),
	register wchar_t *__restrict s _AND
	register const wchar_t *__restrict delim _AND
	wchar_t **__restrict lasts)
{
	register const wchar_t *spanp;
	register int c, sc;
	wchar_t *tok;


	if (s == NULL && (s = *lasts) == NULL)
		return (NULL);

	/*
	 * Skip (span) leading delimiters (s += wcsspn(s, delim), sort of).
	 */
cont:
	c = *s++;
	for (spanp = delim; (sc = *spanp++) != L'\0';) {
		if (c == sc)  goto cont;
	}

	if (c == L'\0') {		/* no non-delimiter characters */
		*lasts = NULL;
		return (NULL);
	}
	tok = s - 1;

	/*
	 * Scan token (scan for delimiters: s += wcscspn(s, delim), sort of).
	 * Note that delim must have one NUL; we stop if we see that, too.
	 */
	for (;;) {
		c = *s++;
		spanp = delim;
		do {
			if ((sc = *spanp++) == c) {
				if (c == L'\0')
					s = NULL;
				else
					s[-1] = L'\0';
				*lasts = s;
				return (tok);
			}
		} while (sc != L'\0');
	}
	/* NOTREACHED */
}
 
/* The remainder of this file can serve as a regression test.  Compile
 *  with -D_REGRESSION_TEST.  */
#if defined(_REGRESSION_TEST)	/* [Test code:  example from C99 standard */
#include <stdio.h>
#include <wchar.h>
 
/* example from C99 standard with minor additions to be a test */
int
main(void)
{
int  errs=0;
static wchar_t str1[] = L"?a???b,,,#c";
static wchar_t str2[] = L"\t \t";
wchar_t *t, *ptr1, *ptr2;
 
t = wcstok(str1, L"?", &ptr1); // t points to the token L"a"
if(wcscmp(t,L"a")) errs++;
t = wcstok(NULL, L",", &ptr1); // t points to the token L"??b"
if(wcscmp(t,L"??b")) errs++;
t = wcstok(str2, L" \t", &ptr2); // t is a null pointer
if(t != NULL) errs++;
t = wcstok(NULL, L"#,", &ptr1); // t points to the token L"c"
if(wcscmp(t,L"c")) errs++;
t = wcstok(NULL, L"?", &ptr1); // t is a null pointer
if(t != NULL) errs++;
 
printf("wcstok() test ");
if(errs)  printf("FAILED %d test cases", errs);
  else    printf("passed");
printf(".\n");
 
return(errs);
}
#endif /* defined(_REGRESSION_TEST) ] */
