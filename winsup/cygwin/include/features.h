/* features.h

   Copyright 2001, 2002, 2003, 2004 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _FEATURES_H
#define _FEATURES_H

#include <sys/cdefs.h>
#include <sys/features.h>

/* Various options should be defined here, but the framework to do this
   is not laid down so far.  Especially notable are the following defines,
   which can be used by the application to switch on or off various
   datatypes and function prototypes:

     _BSD_SOURCE   to include pure BSD functions which are not defined
		   under POSIX.

     _POSIX_SOURCE if the application requests a POSIX compatible system.

     _XOPEN_SOURCE if X/Open functions and datatypes are requested.  This
		   option includes _POSIX_SOURCE.

     _GNU_SOURCE   to turn on GNU extensions which might collide with defines
		   used in application or library headers.  This option
		   includes _BSD_SOURCE, _XOPEN_SOURCE and _POSIX_SOURCE.
*/

#endif /* _FEATURES_H */
