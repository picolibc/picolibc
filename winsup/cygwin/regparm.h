/* regparm.h: Define macros for regparm functions and methods.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#pragma once

#if defined (__x86_64__) || defined (__CYGMAGIC__)
# define __reg1
# define __reg2
# define __reg3
#else
# define __reg1 __stdcall __attribute__ ((regparm (1)))
# define __reg2 __stdcall __attribute__ ((regparm (2)))
# define __reg3 __stdcall __attribute__ ((regparm (3)))
#endif
