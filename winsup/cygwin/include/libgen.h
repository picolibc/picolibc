/* libgen.h

   Copyright 2005 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _LIBGEN_H
#define _LIBGEN_H

#include <sys/cdefs.h>

__BEGIN_DECLS

extern char *basename (char *path);
extern char *dirname (char *path);

__END_DECLS

#endif /* _LIBGEN_H */
