/* sys/syslimits.h

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _SYS_SYSLIMITS_H
#define _SYS_SYSLIMITS_H

#ifdef _COMPILING_NEWLIB
# include <limits.h>
#else
# error "Do not include sys/syslimits.h from applications directly."
#endif

#endif /*_SYS_SYSLIMITS_H */
