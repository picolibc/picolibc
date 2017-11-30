/*
FUNCTION
	<<strverscmp>>---version string compare

INDEX
	strverscmp

SYNOPSIS
	#define _GNU_SOURCE
	#include <string.h>
	int strverscmp(const char *<[a]>, const char *<[b]>);

DESCRIPTION
	<<strverscmp>> compares the string at <[a]> to
	the string at <[b]> in a version-logical order.

RETURNS

	If <<*<[a]>>> version-sorts after <<*<[b]>>>, <<strverscmp>> returns
	a number greater than zero.  If the two strings match, <<strverscmp>>
	returns zero.  If <<*<[a]>>> version-sorts before <<*<[b]>>>,
	<<strverscmp>> returns a number less than zero.

PORTABILITY
<<strverscmp>> is a GNU extension.

<<strverscmp>> requires no supporting OS subroutines. It uses
isdigit() from elsewhere in this library.

QUICKREF
	strverscmp
*/

/*
From musl src/string/strverscmp.c

Copyright © 2005-2014 Rich Felker, et al.

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define _GNU_SOURCE
#include <ctype.h>
#include <string.h>

int strverscmp(const char *l0, const char *r0)
{
	const unsigned char *l = (const void *)l0;
	const unsigned char *r = (const void *)r0;
	size_t i, dp, j;
	int z = 1;

	/* Find maximal matching prefix and track its maximal digit
	 * suffix and whether those digits are all zeros. */
	for (dp=i=0; l[i]==r[i]; i++) {
		int c = l[i];
		if (!c) return 0;
		if (!isdigit(c)) dp=i+1, z=1;
		else if (c!='0') z=0;
	}

	if (l[dp]!='0' && r[dp]!='0') {
		/* If we're not looking at a digit sequence that began
		 * with a zero, longest digit string is greater. */
		for (j=i; isdigit(l[j]); j++)
			if (!isdigit(r[j])) return 1;
		if (isdigit(r[j])) return -1;
	} else if (z && dp<i && (isdigit(l[i]) || isdigit(r[i]))) {
		/* Otherwise, if common prefix of digit sequence is
		 * all zeros, digits order less than non-digits. */
		return (unsigned char)(l[i]-'0') - (unsigned char)(r[i]-'0');
	}

	return l[i] - r[i];
}
