/*
 * Copyright (c) 1990, 2007 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * and/or other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*
FUNCTION
<<swprintf>>, <<fwprintf>>, <<wprintf>>---wide character format output

INDEX
	fwprintf
INDEX
	_fwprintf_r
INDEX
	wprintf
INDEX
	_wprintf_r
INDEX
	swprintf
INDEX
	_swprintf_r

SYNOPSIS
        #include <wchar.h>

        int wprintf(const wchar_t *<[format]>, ...);
        int fwprintf(FILE *__restrict <[fd]>,
        	const wchar_t *__restrict <[format]>, ...);
        int swprintf(wchar_t *__restrict <[str]>, size_t <[size]>,
                     const wchar_t *__restrict <[format]>, ...);

        int wprintf( const wchar_t *<[format]>, ...);
        int fwprintf( FILE *<[fd]>,
                        const wchar_t *<[format]>, ...);
        int swprintf( wchar_t *<[str]>,
                        size_t <[size]>, const wchar_t *<[format]>, ...);

DESCRIPTION
        <<wprintf>> accepts a series of arguments, applies to each a
        format specifier from <<*<[format]>>>, and writes the
        formatted data to <<stdout>>, without a terminating NUL
        wide character.  The behavior of <<wprintf>> is undefined if there
        are not enough arguments for the format or if any argument is not the
	right type for the corresponding conversion specifier.  <<wprintf>>
	returns when it reaches the end of the format string.  If there are
        more arguments than the format requires, excess arguments are
        ignored.

        <<fwprintf>> is like <<wprintf>>, except that output is directed
        to the stream <[fd]> rather than <<stdout>>.

        <<swprintf>> is like <<wprintf>>, except that output is directed
        to the buffer <[str]> with a terminating wide <<NUL>>, and the
	resulting string length is limited to at most <[size]> wide characters,
	including the terminating <<NUL>>.  It is considered an error if the
	output (including the terminating wide-<<NULL>>) does not fit into
	<[size]> wide characters.  (This error behavior is not the same as for
	<<snprintf>>, which <<swprintf>> is otherwise completely analogous to.
	While <<snprintf>> allows the needed size to be known simply by giving
	<[size]>=0, <<swprintf>> does not, giving an error instead.)

        For <<swprintf>> the behavior is undefined if the output
	<<*<[str]>>> overlaps with one of the arguments.  Behavior is also
	undefined if the argument for <<%n>> within <<*<[format]>>>
	overlaps another argument.

        <[format]> is a pointer to a wide character string containing two
	types of objects: ordinary characters (other than <<%>>),
	which are copied unchanged to the output, and conversion
	specifications, each of which is introduced by <<%>>. (To
	include <<%>> in the output, use <<%%>> in the format string.)
	A conversion specification has the following form:

.       %[<[pos]>][<[flags]>][<[width]>][.<[prec]>][<[size]>]<[type]>

        The fields of the conversion specification have the following
        meanings:

        O+
	o <[pos]>

        Conversions normally consume arguments in the order that they
        are presented.  However, it is possible to consume arguments
        out of order, and reuse an argument for more than one
        conversion specification (although the behavior is undefined
        if the same argument is requested with different types), by
        specifying <[pos]>, which is a decimal integer followed by
        '$'.  The integer must be between 1 and <NL_ARGMAX> from
        limits.h, and if argument <<%n$>> is requested, all earlier
        arguments must be requested somewhere within <[format]>.  If
        positional parameters are used, then all conversion
        specifications except for <<%%>> must specify a position.
	This positional parameters method is a POSIX extension to the C
	standard definition for the functions.

	o <[flags]>

	<[flags]> is an optional sequence of characters which control
	output justification, numeric signs, decimal points, trailing
	zeros, and octal and hex prefixes.  The flag characters are
	minus (<<->>), plus (<<+>>), space ( ), zero (<<0>>), sharp
	(<<#>>), and quote (<<'>>).  They can appear in any
	combination, although not all flags can be used for all
	conversion specification types.

		o+
		o '
			A POSIX extension to the C standard.  However, this
			implementation presently treats it as a no-op, which
			is the default behavior for the C locale, anyway.  (If
			it did what it is supposed to, when <[type]> were <<i>>,
			<<d>>, <<u>>, <<f>>, <<F>>, <<g>>, or <<G>>, the
			integer portion of the conversion would be formatted
			with thousands' grouping wide characters.)

		o -
			The result of the conversion is left
			justified, and the right is padded with
			blanks.  If you do not use this flag, the
			result is right justified, and padded on the
			left.

	        o +
			The result of a signed conversion (as
			determined by <[type]> of <<d>>, <<i>>, <<a>>,
			<<A>>, <<e>>, <<E>>, <<f>>, <<F>>, <<g>>, or
			<<G>>) will always begin with a plus or minus
			sign.  (If you do not use this flag, positive
			values do not begin with a plus sign.)

		o " " (space)
			If the first character of a signed conversion
		        specification is not a sign, or if a signed
		        conversion results in no characters, the
		        result will begin with a space.  If the space
		        ( ) flag and the plus (<<+>>) flag both
		        appear, the space flag is ignored.

	        o 0
			If the <[type]> character is <<d>>, <<i>>,
			<<o>>, <<u>>, <<x>>, <<X>>, <<a>>, <<A>>,
			<<e>>, <<E>>, <<f>>, <<F>>, <<g>>, or <<G>>:  leading
			zeros are used to pad the field width
			(following any indication of sign or base); no
			spaces are used for padding.  If the zero
			(<<0>>) and minus (<<->>) flags both appear,
			the zero (<<0>>) flag will be ignored.  For
			<<d>>, <<i>>, <<o>>, <<u>>, <<x>>, and <<X>>
			conversions, if a precision <[prec]> is
			specified, the zero (<<0>>) flag is ignored.

			Note that <<0>> is interpreted as a flag, not
		        as the beginning of a field width.

	        o #
			The result is to be converted to an
			alternative form, according to the <[type]>
			character.
		o-

	The alternative form output with the # flag depends on the <[type]>
	character:

		o+
		o o
			Increases precision to force the first
			digit of the result to be a zero.

		o x
			A non-zero result will have a <<0x>>
			prefix.

		o X
			A non-zero result will have a <<0X>>
			prefix.

		o a, A, e, E, f, or F
			The result will always contain a
			decimal point even if no digits follow
			the point.  (Normally, a decimal point
			appears only if a digit follows it.)
			Trailing zeros are removed.

		o g or G
			The result will always contain a
			decimal point even if no digits follow
			the point.  Trailing zeros are not
			removed.

		o all others
			Undefined.

		o-


	o <[width]>

		<[width]> is an optional minimum field width.  You can
		either specify it directly as a decimal integer, or
		indirectly by using instead an asterisk (<<*>>), in
		which case an <<int>> argument is used as the field
		width.  If positional arguments are used, then the
		width must also be specified positionally as <<*m$>>,
		with m as a decimal integer.  Negative field widths
		are treated as specifying the minus (<<->>) flag for
		left justfication, along with a positive field width.
		The resulting format may be wider than the specified
		width.

	o <[prec]>

		<[prec]> is an optional field; if present, it is
		introduced with `<<.>>' (a period). You can specify
		the precision either directly as a decimal integer or
		indirectly by using an asterisk (<<*>>), in which case
		an <<int>> argument is used as the precision.  If
		positional arguments are used, then the precision must
		also be specified positionally as <<*m$>>, with m as a
		decimal integer.  Supplying a negative precision is
		equivalent to omitting the precision.  If only a
		period is specified the precision is zero. The effect
		depends on the conversion <[type]>.

		o+
		o d, i, o, u, x, or X
			Minimum number of digits to appear.  If no
			precision is given, defaults to 1.

		o a or A
			Number of digits to appear after the decimal
			point.  If no precision is given, the
			precision defaults to the minimum needed for
			an exact representation.

		o e, E, f or F
			Number of digits to appear after the decimal
			point.  If no precision is given, the
			precision defaults to 6.

		o g or G
			Maximum number of significant digits.  A
			precision of 0 is treated the same as a
			precision of 1.  If no precision is given, the
			precision defaults to 6.

		o s or S
			Maximum number of characters to print from the
			string.  If no precision is given, the entire
			string is printed.

		o all others
			undefined.

		o-

	o <[size]>

		<[size]> is an optional modifier that changes the data
		type that the corresponding argument has.  Behavior is
		unspecified if a size is given that does not match the
		<[type]>.

		o+
		o hh
			With <<d>>, <<i>>, <<o>>, <<u>>, <<x>>, or
			<<X>>, specifies that the argument should be
			converted to a <<signed char>> or <<unsigned
			char>> before printing.

			With <<n>>, specifies that the argument is a
			pointer to a <<signed char>>.

		o h
			With <<d>>, <<i>>, <<o>>, <<u>>, <<x>>, or
			<<X>>, specifies that the argument should be
			converted to a <<short>> or <<unsigned short>>
			before printing.

			With <<n>>, specifies that the argument is a
			pointer to a <<short>>.

		o l
			With <<d>>, <<i>>, <<o>>, <<u>>, <<x>>, or
			<<X>>, specifies that the argument is a
			<<long>> or <<unsigned long>>.

			With <<c>>, specifies that the argument has
			type <<wint_t>>.

			With <<s>>, specifies that the argument is a
			pointer to <<wchar_t>>.

			With <<n>>, specifies that the argument is a
			pointer to a <<long>>.

			With <<a>>, <<A>>, <<e>>, <<E>>, <<f>>, <<F>>,
			<<g>>, or <<G>>, has no effect (because of
			vararg promotion rules, there is no need to
			distinguish between <<float>> and <<double>>).

		o ll
			With <<d>>, <<i>>, <<o>>, <<u>>, <<x>>, or
			<<X>>, specifies that the argument is a
			<<long long>> or <<unsigned long long>>.

			With <<n>>, specifies that the argument is a
			pointer to a <<long long>>.

		o j
			With <<d>>, <<i>>, <<o>>, <<u>>, <<x>>, or
			<<X>>, specifies that the argument is an
			<<intmax_t>> or <<uintmax_t>>.

			With <<n>>, specifies that the argument is a
			pointer to an <<intmax_t>>.

		o z
			With <<d>>, <<i>>, <<o>>, <<u>>, <<x>>, or
			<<X>>, specifies that the argument is a <<size_t>>.

			With <<n>>, specifies that the argument is a
			pointer to a <<size_t>>.

		o t
			With <<d>>, <<i>>, <<o>>, <<u>>, <<x>>, or
			<<X>>, specifies that the argument is a
			<<ptrdiff_t>>.

			With <<n>>, specifies that the argument is a
			pointer to a <<ptrdiff_t>>.

		o L
			With <<a>>, <<A>>, <<e>>, <<E>>, <<f>>, <<F>>,
			<<g>>, or <<G>>, specifies that the argument
			is a <<long double>>.

		o-

	o   <[type]>

		<[type]> specifies what kind of conversion <<wprintf>>
		performs.  Here is a table of these:

		o+
		o %
			Prints the percent character (<<%>>).

		o c
			If no <<l>> qualifier is present, the int argument shall
			be converted to a wide character as if by calling
			the btowc() function and the resulting wide character
			shall be written.  Otherwise, the wint_t argument
			shall be converted to wchar_t, and written.

		o C
			Short for <<%lc>>.  A POSIX extension to the C standard.

		o s
			If no <<l>> qualifier is present, the application
			shall ensure that the argument is a pointer to a
			character array containing a character sequence
			beginning in the initial shift state.  Characters
			from the array shall be converted as if by repeated
			calls to the mbrtowc() function, with the conversion
			state described by an mbstate_t object initialized to
			zero before the first character is converted, and
			written up to (but not including) the terminating
			null wide character. If the precision is specified,
			no more than that many wide characters shall be
			written.  If the precision is not specified, or is
			greater than the size of the array, the application
			shall ensure that the array contains a null wide
			character.

			If an <<l>> qualifier is present, the application
			shall ensure that the argument is a pointer to an
			array of type wchar_t. Wide characters from the array
			shall be written up to (but not including) a
			terminating null wide character. If no precision is
			specified, or is greater than the size of the array,
			the application shall ensure that the array contains
			a null wide character. If a precision is specified,
			no more than that many wide characters shall be
			written.

		o S
			Short for <<%ls>>.  A POSIX extension to the C standard.

		o d or i
			Prints a signed decimal integer; takes an
			<<int>>.  Leading zeros are inserted as
			necessary to reach the precision.  A value of 0 with
			a precision of 0 produces an empty string.

		o o
			Prints an unsigned octal integer; takes an
			<<unsigned>>.  Leading zeros are inserted as
			necessary to reach the precision.  A value of 0 with
			a precision of 0 produces an empty string.

		o u
			Prints an unsigned decimal integer; takes an
			<<unsigned>>.  Leading zeros are inserted as
			necessary to reach the precision.  A value of 0 with
			a precision of 0 produces an empty string.

		o x
			Prints an unsigned hexadecimal integer (using
			<<abcdef>> as digits beyond <<9>>); takes an
			<<unsigned>>.  Leading zeros are inserted as
			necessary to reach the precision.  A value of 0 with
			a precision of 0 produces an empty string.

		o X
			Like <<x>>, but uses <<ABCDEF>> as digits
			beyond <<9>>.

		o f
			Prints a signed value of the form
			<<[-]9999.9999>>, with the precision
			determining how many digits follow the decimal
			point; takes a <<double>> (remember that
			<<float>> promotes to <<double>> as a vararg).
			The low order digit is rounded to even.  If
			the precision results in at most DECIMAL_DIG
			digits, the result is rounded correctly; if
			more than DECIMAL_DIG digits are printed, the
			result is only guaranteed to round back to the
			original value.

			If the value is infinite, the result is
			<<inf>>, and no zero padding is performed.  If
			the value is not a number, the result is
			<<nan>>, and no zero padding is performed.

		o F
			Like <<f>>, but uses <<INF>> and <<NAN>> for
			non-finite numbers.

		o e
			Prints a signed value of the form
			<<[-]9.9999e[+|-]999>>; takes a <<double>>.
			The digit before the decimal point is non-zero
			if the value is non-zero.  The precision
			determines how many digits appear between
			<<.>> and <<e>>, and the exponent always
			contains at least two digits.  The value zero
			has an exponent of zero.  If the value is not
			finite, it is printed like <<f>>.

		o E
			Like <<e>>, but using <<E>> to introduce the
			exponent, and like <<F>> for non-finite
			values.

		o g
			Prints a signed value in either <<f>> or <<e>>
			form, based on the given value and
			precision---an exponent less than -4 or
			greater than the precision selects the <<e>>
			form.  Trailing zeros and the decimal point
			are printed only if necessary; takes a
			<<double>>.

		o G
			Like <<g>>, except use <<F>> or <<E>> form.

		o a
			Prints a signed value of the form
			<<[-]0x1.ffffp[+|-]9>>; takes a <<double>>.
			The letters <<abcdef>> are used for digits
			beyond <<9>>.  The precision determines how
			many digits appear after the decimal point.
			The exponent contains at least one digit, and
			is a decimal value representing the power of
			2; a value of 0 has an exponent of 0.
			Non-finite values are printed like <<f>>.

		o A
			Like <<a>>, except uses <<X>>, <<P>>, and
			<<ABCDEF>> instead of lower case.

		o n
			Takes a pointer to <<int>>, and stores a count
			of the number of bytes written so far.  No
			output is created.

		o p
			Takes a pointer to <<void>>, and prints it in
			an implementation-defined format.  This
			implementation is similar to <<%#tx>>), except
			that <<0x>> appears even for the NULL pointer.

		o m
			Prints the output of <<strerror(errno)>>; no
			argument is required.  A GNU extension.

		o-
	O-

        <<_wprintf_r>>, <<_fwprintf_r>>, <<_swprintf_r>>, are simply
        reentrant versions of the functions above.

RETURNS
On success, <<swprintf>> return the number of wide characters in
the output string, except the concluding <<NUL>> is not counted.
<<wprintf>> and <<fwprintf>> return the number of characters transmitted.

If an error occurs, the result of <<wprintf>>, <<fwprintf>>, and
<<swprintf>> is a negative value.  For <<wprintf>> and <<fwprintf>>,
<<errno>> may be set according to <<fputwc>>.  For <<swprintf>>, <<errno>>
may be set to EOVERFLOW if <[size]> is greater than INT_MAX / sizeof (wchar_t),
or when the output does not fit into <[size]> wide characters (including the
terminating wide <<NULL>>).

BUGS
The ``''' (quote) flag does not work when locale's thousands_sep is not empty.

PORTABILITY
POSIX-1.2008 with extensions; C99 (compliant except for POSIX extensions).

Depending on how newlib was configured, not all format specifiers are
supported.

Supporting OS subroutines required: <<close>>, <<fstat>>, <<isatty>>,
<<lseek>>, <<read>>, <<sbrk>>, <<write>>.
*/


#define _DEFAULT_SOURCE
#include <_ansi.h>
#include <stdio.h>
#include <wchar.h>
#include <stdarg.h>
#include <limits.h>
#include <errno.h>
#include "local.h"

int
swprintf (wchar_t *__restrict str,
       size_t size,
       const wchar_t *__restrict fmt, ...)
{
  int ret;
  va_list ap;
  FILE f;

  if (size > INT_MAX / sizeof (wchar_t))
    {
      _REENT_ERRNO(ptr) = EOVERFLOW;	/* POSIX extension */
      return EOF;
    }
  f._flags = __SWR | __SSTR;
  f._flags2 = 0;
  f._bf._base = f._p = (unsigned char *) str;
  f._bf._size = f._w = (size > 0 ? (size - 1) * sizeof (wchar_t) : 0);
  f._file = -1;  /* No file. */
  va_start (ap, fmt);
  ret = svfwprintf ( &f, fmt, ap);
  va_end (ap);
  /* svfwprintf( so add one if
   * appropriate, which is whenever size is > 0.  _svfwprintf_r() stops
   * after n-1, so always just put at the end.  */
  if (size > 0)  {
    *(wchar_t *)f._p = L'\0';	/* terminate the string */
  }
  if(ret >= 0 && (size_t) ret >= size)  {
    /* _svfwprintf_r() returns how many wide characters it would have printed
     * if there were enough space.  Return an error if too big to fit in str,
     * unlike snprintf, which returns the size needed.  */
    _REENT_ERRNO(ptr) = EOVERFLOW;	/* POSIX extension */
    ret = -1;
  }
  return (ret);
}
