/*
 * txtmode.c
 *
 * txtmode.o is included in libmingwex.a and sets default
 * file mode to text. The value of this statically linked
 * _fmode is used to set the dll version (using __p__fmode()),   
 * in crt1.c. 
 *
 * 2002-10-19 Danny Smith  <dannysmith@users.sourceforge.net>
 */  	

#include <fcntl.h>

/* Set default file mode to text */

/* Is this correct?  Default value of  _fmode in msvcrt.dll is 0. */

int _fmode = _O_TEXT; 
