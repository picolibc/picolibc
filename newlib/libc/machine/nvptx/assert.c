/*
 * Support file for nvptx in newlib.
 * Copyright (c) 2016-2018 Mentor Graphics.
 *
 * The authors hereby grant permission to use, copy, modify, distribute,
 * and license this software and its documentation for any purpose, provided
 * that existing copyright notices are retained in all copies and that this
 * notice is included verbatim in any distributions. No written agreement,
 * license, or royalty fee is required for any of the authorized uses.
 * Modifications to this software may be copyrighted by their authors
 * and need not follow the licensing terms described here, provided that
 * the new terms are clearly indicated on the first page of each file where
 * they apply.
 */

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#ifndef HAVE_ASSERT_FUNC
/* func can be NULL, in which case no function information is given.  */
void
__assert_func (const char *file,
	int line,
	const char *func,
	const char *failedexpr)
{
  abort();
  /* NOTREACHED */
}
#endif /* HAVE_ASSERT_FUNC */

void
__assert (const char *file,
	int line,
	const char *failedexpr)
{
   __assert_func (file, line, NULL, failedexpr);
  /* NOTREACHED */
}
