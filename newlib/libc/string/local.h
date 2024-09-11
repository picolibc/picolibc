/* Copyright (c) 2009 Corinna Vinschen <corinna@vinschen.de> */
#include "../ctype/local.h"

/* internal function to compute width of wide char. */
int __wcwidth (wint_t);

/* Reentrant version of strerror.  */
char *	_strerror_r (int, int, int *);
