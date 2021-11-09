#include <ctype.h>

/* Define some short names to keep internal files shorter.  */
#define _U _ISupper
#define _L _ISlower
#define _N _ISdigit
#define _S _ISspace
#define _P _ISpunct
#define _C _IScntrl
#define _X _ISxdigit
#define _B _ISblank

#if (defined(__GNUC__) && !defined(__CHAR_UNSIGNED__) && !defined(COMPACT_CTYPE)) || defined (__CYGWIN__)
#define ALLOW_NEGATIVE_CTYPE_INDEX
#endif

#ifdef ALLOW_NEGATIVE_CTYPE_INDEX

#ifndef __CYGWIN__
  extern const char _ctype_b[];
#else
  extern char _ctype_b[];
#endif
# define DEFAULT_CTYPE_PTR	((char *) _ctype_b + 127)

#else	/* !ALLOW_NEGATIVE_CTYPE_INDEX */

  /* _ctype_ is declared in <ctype.h>. */
# define DEFAULT_CTYPE_PTR	((char *) _ctype_)

#endif	/* !ALLOW_NEGATIVE_CTYPE_INDEX */

