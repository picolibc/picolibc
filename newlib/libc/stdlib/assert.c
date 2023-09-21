/*
Copyright (c) 1990 Regents of the University of California.
All rights reserved.
 */
/*
FUNCTION
<<assert>>---macro for debugging diagnostics

INDEX
	assert

SYNOPSIS
	#include <assert.h>
	void assert(int <[expression]>);

DESCRIPTION
	Use this macro to embed debuggging diagnostic statements in
	your programs.  The argument <[expression]> should be an
	expression which evaluates to true (nonzero) when your program
	is working as you intended.

	When <[expression]> evaluates to false (zero), <<assert>>
	calls <<abort>>, after first printing a message showing what
	failed and where:

. Assertion failed: <[expression]>, file <[filename]>, line <[lineno]>, function: <[func]>

	If the name of the current function is not known (for example,
	when using a C89 compiler that does not understand __func__),
	the function location is omitted.

	The macro is defined to permit you to turn off all uses of
	<<assert>> at compile time by defining <<NDEBUG>> as a
	preprocessor variable.   If you do this, the <<assert>> macro
	expands to

. (void(0))

RETURNS
	<<assert>> does not return a value.

PORTABILITY
	The <<assert>> macro is required by ANSI, as is the behavior
	when <<NDEBUG>> is defined.

Supporting OS subroutines required (only if enabled): <<close>>, <<fstat>>,
<<getpid>>, <<isatty>>, <<kill>>, <<lseek>>, <<read>>, <<sbrk>>, <<write>>.
*/

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

void
__assert (const char *failedexpr,
	const char *file,
	int line)
{
   __assert_func (file, line, NULL, failedexpr);
  /* NOTREACHED */
}
