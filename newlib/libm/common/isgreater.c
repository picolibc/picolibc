/* isgreater.c:  This file contains no source code, but rather only the
 * man-page comments.  All of the documented "functions" are actually macros
 * defined in math.h (q.v.).  */
/*
FUNCTION
<<isgreater>>, <<isgreaterequal>>, <<isless>>, <<islessequal>>, <<islessgreater>>, and <<isunordered>>--comparison macros
INDEX
	isgreater
INDEX
	isgreaterequal
INDEX
	isless
INDEX
	islessequal
INDEX
	islessgreater
INDEX
	isunordered

ANSI_SYNOPSIS
	#include <math.h>
	int isgreater(real-floating <[x]>, real-floating <[y]>);
	int isgreaterequal(real-floating <[x]>, real-floating <[y]>);
	int isless(real-floating <[x]>, real-floating <[y]>);
	int islessequal(real-floating <[x]>, real-floating <[y]>);
	int islessgreater(real-floating <[x]>, real-floating <[y]>);
	int isunordered(real-floating <[x]>, real-floating <[y]>);

DESCRIPTION
<<isgreater>>, <<isgreaterequal>>, <<isless>>, <<islessequal>>,
<<islessgreater>>, and <<isunordered>> are macros defined for use in
comparing floating-point numbers without raising any floating-point
exceptions.

The relational operators (i.e. <, >, <=, and >=) support the usual mathematical
relationships between numeric values.  For any ordered pair of numeric
values exactly one of the relationships--less, greater, and equal--is
true.  Relational operators may raise the "invalid" floating-point
exception when argument values are NaNs.  For a NaN and a numeric value, or
for two NaNs, just the unordered relationship is true (i.e., if one or both
of the arguments a NaN, the relationship is called unordered).  The specified
macros are quiet (non floating-point exception raising) versions of the
relational operators, and other comparison macros that facilitate writing
efficient code that accounts for NaNs without suffering the "invalid"
floating-point exception.  In the synopses shown, "real-floating" indicates
that the argument is an expression of real floating type.

Please note that saying that the macros do not raise floating-point
exceptions, it is referring to the function that they are performing.  It
is certainly possible to give them an expression which causes an exception.
For example:
o+
o	NaN < 1.0
		causes an "invalid" exception,
o	isless(NaN, 1.0)
		does not, and
o	isless(NaN*0., 1.0)
		causes an exception due to the "NaN*0.", but not from the
resultant reduced comparison of isless(NaN, 1.0).
o-

RETURNS
@comment Formatting note:  "$@" forces a new line
No floating-point exceptions are raised for any of the macros.@*
The <<isgreater>> macro returns the value of (x) > (y).@*
The <<isgreaterequal>> macro returns the value of (x) >= (y).@*
The <<isless>> macro returns the value of (x) < (y).@*
The <<islessequal>> macro returns the value of (x) <= (y).@*
The <<islessgreater>> macro returns the value of (x) < (y) || (x) > (y).@*
The <<isunordered>> macro returns 1 if either of its arguments is NaN and 0 otherwise.

PORTABILITY
C99, POSIX.

*/
