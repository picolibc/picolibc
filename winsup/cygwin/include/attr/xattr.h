/* attr/xattr.h

   Copyright 2008 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _ATTR_XATTR_H
#define _ATTR_XATTR_H

#include "_ansi.h"
#if 0
/* Per man pages you have to include <sys/types.h> explicitely before
   including <attr/xattr.h>.  That's how it works on Linux, too. */
#include <sys/types.h>
#endif
#include <sys/errno.h>

#define XATTR_CREATE	1
#define XATTR_REPLACE	2

#ifndef ENOATTR
#define ENOATTR ENODATA
#endif

_BEGIN_STD_C

ssize_t _EXFUN(getxattr,(const char *, const char *, void *, size_t ));
ssize_t _EXFUN(lgetxattr,(const char *, const char *, void *, size_t ));
ssize_t _EXFUN(fgetxattr,(int , const char *, void *, size_t ));
ssize_t _EXFUN(listxattr,(const char *, char *, size_t ));
ssize_t _EXFUN(llistxattr,(const char *, char *, size_t ));
ssize_t _EXFUN(flistxattr,(int , char *, size_t ));
int _EXFUN(setxattr,(const char *, const char *, const void *, size_t , int ));
int _EXFUN(lsetxattr,(const char *, const char *, const void *, size_t , int ));
int _EXFUN(fsetxattr,(int , const char *, const void *, size_t , int ));
int _EXFUN(removexattr,(const char *, const char *));
int _EXFUN(lremovexattr,(const char *, const char *));
int _EXFUN(fremovexattr,(int ,   const char *));

_END_STD_C

#endif	/* _ATTR_XATTR_H */
