#include <fcntl.h>

/* Set default file mode to text */

/* Is this correct?  Default value of  _fmode in msvcrt.dll is 0. */

int _fmode = _O_TEXT; 
