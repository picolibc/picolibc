/*
Copyright (c) 1994 Cygnus Support.
All rights reserved.

Redistribution and use in source and binary forms are permitted
provided that the above copyright notice and this paragraph are
duplicated in all such forms and that any documentation,
and/or other materials related to such
distribution and use acknowledge that the software was developed
at Cygnus Support, Inc.  Cygnus Support, Inc. may not be used to
endorse or promote products derived from this software without
specific prior written permission.
THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
/*
FUNCTION
	<<strtok>>, <<strtok_r>>, <<strsep>>---get next token from a string

INDEX
	strtok

INDEX
	strtok_r

INDEX
	strsep

SYNOPSIS
	#include <string.h>
      	char *strtok(char *restrict <[source]>,
                     const char *restrict <[delimiters]>);
      	char *strtok_r(char *restrict <[source]>,
                       const char *restrict <[delimiters]>,
                       char **<[lasts]>);
	char *strsep(char **<[source_ptr]>, const char *<[delimiters]>);

DESCRIPTION
	The <<strtok>> function is used to isolate sequential tokens in a 
	null-terminated string, <<*<[source]>>>. These tokens are delimited 
	in the string by at least one of the characters in <<*<[delimiters]>>>.
	The first time that <<strtok>> is called, <<*<[source]>>> should be
	specified; subsequent calls, wishing to obtain further tokens from
	the same string, should pass a null pointer instead.  The separator
	string, <<*<[delimiters]>>>, must be supplied each time and may 
	change between calls.

	The <<strtok>> function returns a pointer to the beginning of each 
	subsequent token in the string, after replacing the separator 
	character itself with a null character.  When no more tokens remain, 
	a null pointer is returned.

	The <<strtok_r>> function has the same behavior as <<strtok>>, except
	a pointer to placeholder <<*<[lasts]>>> must be supplied by the caller.

	The <<strsep>> function is similar in behavior to <<strtok>>, except
	a pointer to the string pointer must be supplied <<<[source_ptr]>>> and
	the function does not skip leading delimiters.  When the string starts
	with a delimiter, the delimiter is changed to the null character and
	the empty string is returned.  Like <<strtok_r>> and <<strtok>>, the
	<<*<[source_ptr]>>> is updated to the next character following the
	last delimiter found or NULL if the end of string is reached with
	no more delimiters.

RETURNS
	<<strtok>>, <<strtok_r>>, and <<strsep>> all return a pointer to the 
	next token, or <<NULL>> if no more tokens can be found.  For
	<<strsep>>, a token may be the empty string.

NOTES
	<<strtok>> is unsafe for multi-threaded applications.  <<strtok_r>>
	and <<strsep>> are thread-safe and should be used instead.

PORTABILITY
<<strtok>> is ANSI C.
<<strtok_r>> is POSIX.
<<strsep>> is a BSD extension.

<<strtok>>, <<strtok_r>>, and <<strsep>> require no supporting OS subroutines.

QUICKREF
	strtok ansi impure
*/

/* undef STRICT_ANSI so that strtok_r prototype will be defined */
#undef  __STRICT_ANSI__
#include <string.h>
#include <stdlib.h>
#include <_ansi.h>

#ifndef _REENT_ONLY

static NEWLIB_THREAD_LOCAL char *_strtok_last;

extern char *__strtok_r (char *, const char *, char **, int);

char *
strtok (register char *__restrict s,
	register const char *__restrict delim)
{
	return __strtok_r (s, delim, &_strtok_last, 1);
}
#endif
