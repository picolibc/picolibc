/*
 * binmode.c
 *
 * Adding binmode.o as object file when building an app
 * sets the default file mode for user files to binary.
 * It does not affect stdin, stderr, or stdout which will
 * still be opened in text mode by default.
 *
 * 2002-10-19 Danny Smith  <dannysmith@users.sourceforge.net>
 */  	

#include <fcntl.h>

/* Set default file mode to binary */

int _fmode = _O_BINARY; 
