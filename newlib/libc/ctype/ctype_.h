/* Copyright (c) 2016 Corinna Vinschen <corinna@vinschen.de> */
#include <ctype.h>

# define DEFAULT_CTYPE_PTR	((char *) _ctype_)

void
__set_ctype (struct __locale_t *loc, const char *charset);
