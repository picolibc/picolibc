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
