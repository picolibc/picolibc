/* BSD strsep function */

/* Copyright 2002, Red Hat Inc. */

#define _DEFAULT_SOURCE
#include <string.h>
#include "strtok_r.h"

char *
strsep (register char **source_ptr,
	register const char *delim)
{
	return __strtok_r (*source_ptr, delim, source_ptr, 0);
}
