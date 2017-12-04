/*
FUNCTION
<<ecvt>>, <<ecvtf>>, <<fcvt>>, <<fcvtf>>---double or float to string

INDEX
	ecvt
INDEX
	ecvtf
INDEX
	fcvt
INDEX
	fcvtf

SYNOPSIS
	#include <stdlib.h>

	char *ecvt(double <[val]>, int <[chars]>, int *<[decpt]>, int *<[sgn]>);
	char *ecvtf(float <[val]>, int <[chars]>, int *<[decpt]>, int *<[sgn]>);

	char *fcvt(double <[val]>, int <[decimals]>, 
                   int *<[decpt]>, int *<[sgn]>);
	char *fcvtf(float <[val]>, int <[decimals]>, 
                    int *<[decpt]>, int *<[sgn]>);

DESCRIPTION
<<ecvt>> and <<fcvt>> produce (null-terminated) strings of digits
representating the <<double>> number <[val]>.
<<ecvtf>> and <<fcvtf>> produce the corresponding character
representations of <<float>> numbers.

(The <<stdlib>> functions <<ecvtbuf>> and <<fcvtbuf>> are reentrant
versions of <<ecvt>> and <<fcvt>>.)

The only difference between <<ecvt>> and <<fcvt>> is the
interpretation of the second argument (<[chars]> or <[decimals]>).
For <<ecvt>>, the second argument <[chars]> specifies the total number
of characters to write (which is also the number of significant digits
in the formatted string, since these two functions write only digits).
For <<fcvt>>, the second argument <[decimals]> specifies the number of
characters to write after the decimal point; all digits for the integer
part of <[val]> are always included.

Since <<ecvt>> and <<fcvt>> write only digits in the output string,
they record the location of the decimal point in <<*<[decpt]>>>, and
the sign of the number in <<*<[sgn]>>>.  After formatting a number,
<<*<[decpt]>>> contains the number of digits to the left of the
decimal point.  <<*<[sgn]>>> contains <<0>> if the number is positive,
and <<1>> if it is negative.

RETURNS
All four functions return a pointer to the new string containing a
character representation of <[val]>.

PORTABILITY
None of these functions are ANSI C.

Supporting OS subroutines required: <<close>>, <<fstat>>, <<isatty>>,
<<lseek>>, <<read>>, <<sbrk>>, <<write>>.

NEWPAGE
FUNCTION
<<gcvt>>, <<gcvtf>>---format double or float as string

INDEX
	gcvt
INDEX
	gcvtf

SYNOPSIS
	#include <stdlib.h>

	char *gcvt(double <[val]>, int <[precision]>, char *<[buf]>);
	char *gcvtf(float <[val]>, int <[precision]>, char *<[buf]>);

DESCRIPTION
<<gcvt>> writes a fully formatted number as a null-terminated
string in the buffer <<*<[buf]>>>.  <<gcvtf>> produces corresponding
character representations of <<float>> numbers.

<<gcvt>> uses the same rules as the <<printf>> format
`<<%.<[precision]>g>>'---only negative values are signed (with
`<<->>'), and either exponential or ordinary decimal-fraction format
is chosen depending on the number of significant digits (specified by
<[precision]>).

RETURNS
The result is a pointer to the formatted representation of <[val]>
(the same as the argument <[buf]>).

PORTABILITY
Neither function is ANSI C.

Supporting OS subroutines required: <<close>>, <<fstat>>, <<isatty>>,
<<lseek>>, <<read>>, <<sbrk>>, <<write>>.
*/

#define _XOPEN_SOURCE
#define _XOPEN_SOURCE_EXTENDED
#include <_ansi.h>
#include <reent.h>
#include <stdio.h>
#include <stdlib.h>
#include "local.h"

char *
fcvt (double d,
	int ndigit,
	int *decpt,
	int *sign)
{
  return fcvtbuf (d, ndigit, decpt, sign, NULL);
}

char *
fcvtf (float d,
	int ndigit,
	int *decpt,
	int *sign)
{
  return fcvt ((float) d, ndigit, decpt, sign);
}


char *
gcvtf (float d,
	int ndigit,
	char *buf)
{
  double asd = d;
  return gcvt (asd, ndigit, buf);
}


char *
ecvt (double d,
	int ndigit,
	int *decpt,
	int *sign)
{
  return ecvtbuf (d, ndigit, decpt, sign, NULL);
}

char *
ecvtf (float d,
	int ndigit,
	int *decpt,
	int *sign)
{
  return ecvt ((double) d, ndigit, decpt, sign);
}


char *
gcvt (double d,
	int ndigit,
	char *buf)
{
  char *tbuf = buf;
  if (d < 0) {
    *buf = '-';
    buf++;
    ndigit--;
  }
  return (_gcvt (_REENT, d, ndigit, buf, 'g', 0) ? tbuf : 0);
}
