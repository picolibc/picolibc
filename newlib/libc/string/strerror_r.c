/* GNU variant of strerror_r. */
/*
FUNCTION
	<<strerror_r>>---convert error number to string and copy to buffer

INDEX
	strerror_r

SYNOPSIS
	#include <string.h>
	#ifdef _GNU_SOURCE
	char *strerror_r(int <[errnum]>, char *<[buffer]>, size_t <[n]>);
	#else
	int strerror_r(int <[errnum]>, char *<[buffer]>, size_t <[n]>);
	#endif

DESCRIPTION
<<strerror_r>> converts the error number <[errnum]> into a
string and copies the result into the supplied <[buffer]> for
a length up to <[n]>, including the NUL terminator. The value of
<[errnum]> is usually a copy of <<errno>>.  If <<errnum>> is not a known
error number, the result is the empty string.

See <<strerror>> for how strings are mapped to <<errnum>>.

RETURNS
There are two variants: the GNU version always returns a NUL-terminated
string, which is <[buffer]> if all went well, but which is another
pointer if <[n]> was too small (leaving <[buffer]> untouched).  If the
return is not <[buffer]>, your application must not modify that string.
The POSIX version returns 0 on success, <[EINVAL]> if <<errnum>> was not
recognized, and <[ERANGE]> if <[n]> was too small.  The variant chosen
depends on macros that you define before inclusion of <<string.h>>.

PORTABILITY
<<strerror_r>> with a <[char *]> result is a GNU extension.
<<strerror_r>> with an <[int]> result is required by POSIX 2001.
This function is compliant only if <<_user_strerror>> is not provided,
or if it is thread-safe and uses separate storage according to whether
the second argument of that function is non-zero.  For more details
on <<_user_strerror>>, see the <<strerror>> documentation.

POSIX states that the contents of <[buf]> are unspecified on error,
although this implementation guarantees a NUL-terminated string for
all except <[n]> of 0.

POSIX recommends that unknown <[errnum]> result in a message including
that value, however it is not a requirement and this implementation
provides only an empty string (unless you provide <<_user_strerror>>).
POSIX also recommends that unknown <[errnum]> fail with EINVAL even
when providing such a message, however it is not a requirement and
this implementation will return success if <<_user_strerror>> provided
a non-empty alternate string without assigning into its third argument.

<<strerror_r>> requires no supporting OS subroutines.

*/

#undef __STRICT_ANSI__
#define _GNU_SOURCE
#include <errno.h>
#include <string.h>
#undef strerror_r

/* For backwards-compatible linking, this must be the GNU signature;
   see xpg_strerror_r.c for the POSIX version.  */
char *
strerror_r (int errnum,
	char *buffer,
	size_t n)
{
  char *error = _strerror_r (_REENT, errnum, 1, NULL);

  if (strlen (error) >= n)
    return error;
  return strcpy (buffer, error);
}
