/*
 * Copyright (c) 1987 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that: (1) source distributions retain this entire copyright
 * notice and comment, and (2) distributions including binaries display
 * the following acknowledgement:  ``This product includes software
 * developed by the University of California, Berkeley and its contributors''
 * in the documentation or other materials provided with the distribution
 * and in all advertising materials mentioning features or use of this
 * software. Neither the name of the University nor the names of its
 * contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef _REENT_ONLY

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

extern int _unsetenv_r (struct _reent *, const char *);

/*
 * setenv --
 *	Set the value of the environmental variable "name" to be
 *	"value".  If rewrite is set, replace any current value.
 */

int
setenv (const char *name,
	const char *value,
	int rewrite)
{
  return _setenv_r (_REENT, name, value, rewrite);
}

/*
 * unsetenv(name) --
 *	Delete environmental variable "name".
 */
int
unsetenv (const char *name)
{
  return _unsetenv_r (_REENT, name);
}

#endif /* !_REENT_ONLY */
