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
/* NOTE:  This file defines both strftime() and wcsftime().  Take care when
 * making changes.  See also wcsftime.c, and note the (small) overlap in the
 * manual description, taking care to edit both as needed.  */
/*
 * strftime.c
 * Original Author:	G. Haley
 * Additions from:	Eric Blake, Corinna Vinschen
 * Changes to allow dual use as wcstime, also:	Craig Howland
 *
 * Places characters into the array pointed to by s as controlled by the string
 * pointed to by format. If the total number of resulting characters including
 * the terminating null character is not more than maxsize, returns the number
 * of characters placed into the array pointed to by s (not including the
 * terminating null character); otherwise zero is returned and the contents of
 * the array indeterminate.
 */

#define MAKE_WCSFTIME
#include "test-strftime.c"
