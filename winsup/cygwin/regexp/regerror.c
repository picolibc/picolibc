#include "winsup.h"
#include "regexp.h"
#include <stdio.h>

void
regerror(const char *s __attribute__ ((unused)))
{
#ifdef ERRAVAIL
	error("regexp: %s", s);
#else
/*
	fprintf(stderr, "regexp(3): %s\n", s);
	exit(1);
*/
	return;	  /* let std. egrep handle errors */
#endif
	/* NOTREACHED */
}
