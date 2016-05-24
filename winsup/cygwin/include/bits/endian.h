/* bits/endian.h

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _BITS_ENDIAN_H_
#define _BITS_ENDIAN_H_

#ifndef __BIG_ENDIAN
# define __BIG_ENDIAN 4321
#endif
#ifndef __LITTLE_ENDIAN
# define __LITTLE_ENDIAN 1234
#endif

#ifndef __BYTE_ORDER
# define __BYTE_ORDER __LITTLE_ENDIAN
#endif

#endif /* _BITS_ENDIAN_H_ */
