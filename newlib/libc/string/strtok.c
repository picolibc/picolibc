/*
FUNCTION
	<<strtok>>---get next token from a string

INDEX
	strtok

INDEX
	strtok_r

ANSI_SYNOPSIS
	#include <string.h>
      	char *strtok(char *<[source]>, const char *<[delimiters]>)
      	char *strtok_r(char *<[source]>, const char *<[delimiters]>,
			char **<[lasts]>)

TRAD_SYNOPSIS
	#include <string.h>
	char *strtok(<[source]>, <[delimiters]>)
	char *<[source]>;
	char *<[delimiters]>;

	char *strtok_r(<[source]>, <[delimiters]>, <[lasts]>)
	char *<[source]>;
	char *<[delimiters]>;
	char **<[lasts]>;

DESCRIPTION
	The <<strtok>> function is used to isolate sequential tokens in a 
	null-terminated string, <<*<[source]>>>. These tokens are delimited 
	in the string by at least one of the characters in <<*<[delimiters]>>>.
	The first time that <<strtok>> is called, <<*<[source]>>> should be
	specified; subsequent calls, wishing to obtain further tokens from
	the same string, should pass a null pointer instead.  The separator
	string, <<*<[delimiters]>>>, must be supplied each time, and may 
	change between calls.

	The <<strtok>> function returns a pointer to the beginning of each 
	subsequent token in the string, after replacing the separator 
	character itself with a NUL character.  When no more tokens remain, 
	a null pointer is returned.

	The <<strtok_r>> function has the same behavior as <<strtok>>, except
	a pointer to placeholder <<*[lasts]>> must be supplied by the caller.

RETURNS
	<<strtok>> returns a pointer to the next token, or <<NULL>> if
	no more tokens can be found.

NOTES
	<<strtok>> is unsafe for multi-thread applications.  <<strtok_r>>
	is MT-Safe and should be used instead.

PORTABILITY
<<strtok>> is ANSI C.

<<strtok>> requires no supporting OS subroutines.

QUICKREF
	strtok ansi impure
*/

/* undef STRICT_ANSI so that strtok_r prototype will be defined */
#undef  __STRICT_ANSI__
#include <string.h>
#include <_ansi.h>
#include <reent.h>

#ifndef _REENT_ONLY

char *
_DEFUN (strtok, (s, delim),
	register char *s _AND
	register const char *delim)
{
	return strtok_r (s, delim, &(_REENT->_new._reent._strtok_last));
}
#endif
