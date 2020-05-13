/* Copyright (c) 2009 Corinna Vinschen <corinna@vinschen.de> */
/*
FUNCTION
	<<wcsdup>>---wide character string duplicate
	
INDEX
	wcsdup
INDEX
	_wcsdup_r

SYNOPSIS
	#include <wchar.h>
	wchar_t *wcsdup(const wchar_t *<[str]>);

	#include <wchar.h>
	wchar_t *_wcsdup_r(struct _reent *<[ptr]>, const wchar_t *<[str]>);

DESCRIPTION
	<<wcsdup>> allocates a new wide character string using <<malloc>>,
	and copies the content of the argument <[str]> into the newly
	allocated string, thus making a copy of <[str]>.

RETURNS 
	<<wcsdup>> returns a pointer to the copy of <[str]> if enough
	memory for the copy was available.  Otherwise it returns NULL
	and errno is set to ENOMEM.

PORTABILITY
POSIX-1.2008

QUICKREF
	wcsdup 
*/

#include <stdlib.h>
#include <wchar.h>

#ifndef _REENT_ONLY

wchar_t *
wcsdup (const wchar_t *str)
{
  size_t len = wcslen (str) + 1;
  wchar_t *copy = malloc (len * sizeof (wchar_t));
  if (copy)
    wmemcpy (copy, str, len);
  return copy;
}

#endif /* !_REENT_ONLY */
