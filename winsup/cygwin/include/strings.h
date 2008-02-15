/* strings.h

   Copyright 2007 Red Hat, Inc.

 This file is part of Cygwin.

 This software is a copyrighted work licensed under the terms of the
 Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
 details. */

#ifndef _STRINGS_H_
#define _STRINGS_H_

/* newlib's string.h already declares these functions. */
#ifndef _STRING_H_

#include "_ansi.h"

#define __need_size_t
#include <stddef.h>

_BEGIN_STD_C

int   _EXFUN(bcmp,(const void *, const void *, size_t));
void  _EXFUN(bcopy,(const void *, void *, size_t));
void  _EXFUN(bzero,(void *, size_t));
int   _EXFUN(ffs,(int));
char *_EXFUN(index,(const char *, int));
char *_EXFUN(rindex,(const char *, int));
int   _EXFUN(strcasecmp,(const char *, const char *));
int   _EXFUN(strncasecmp,(const char *, const char *, size_t));

_END_STD_C

#endif /* _STRING_H_ */

#endif /* _STRINGS_H_ */
