#ifndef _CTYPE_H_
#define _CTYPE_H_

#include <_ansi.h>

#ifdef __cplusplus
extern "C" {
#endif

int __cdecl isalnum(int);
int __cdecl isalpha(int);
int __cdecl iscntrl(int);
int __cdecl isdigit(int);
int __cdecl isgraph(int);
int __cdecl islower(int);
int __cdecl isprint(int);
int __cdecl ispunct(int);
int __cdecl isspace(int);
int __cdecl isupper(int);
int __cdecl isxdigit(int);
int __cdecl tolower(int);
int __cdecl toupper(int);

#ifndef __STRICT_ANSI__
int __cdecl isblank(int);
int __cdecl isascii(int);
int __cdecl toascii(int);
int __cdecl _tolower(int);
int __cdecl _toupper(int);
#endif

#define	_U	01
#define	_L	02
#define	_N	04
#define	_S	010
#define _P	020
#define _C	040
#define _X	0100
#define	_B	0200

#if defined (__INSIDE_CYGWIN__) || defined (_COMPILING_NEWLIB)
extern const char _ctype_[];
#ifdef _COMPILING_NEWLIB
extern const char *__ctype_ptr__;
#endif
#else
extern const __declspec(dllimport) char _ctype_[];
#endif

#if !defined(__cplusplus) || defined(__INSIDE_CYGWIN__)
#define	isalpha(c)	((_ctype_+1)[(unsigned)(c)]&(_U|_L))
#define isblank(c)	((c) == ' ' || (c) == '\t')
#define	isupper(c)	((_ctype_+1)[(unsigned)(c)]&_U)
#define	islower(c)	((_ctype_+1)[(unsigned)(c)]&_L)
#define	isdigit(c)	((_ctype_+1)[(unsigned)(c)]&_N)
#define	isxdigit(c)	((_ctype_+1)[(unsigned)(c)]&(_X|_N))
#define	isspace(c)	((_ctype_+1)[(unsigned)(c)]&_S)
#define ispunct(c)	((_ctype_+1)[(unsigned)(c)]&_P)
#define isalnum(c)	((_ctype_+1)[(unsigned)(c)]&(_U|_L|_N))
#define isprint(c)	((_ctype_+1)[(unsigned)(c)]&(_P|_U|_L|_N|_B))
#define	isgraph(c)	((_ctype_+1)[(unsigned)(c)]&(_P|_U|_L|_N))
#define iscntrl(c)	((_ctype_+1)[(unsigned)(c)]&_C)
/* Non-gcc versions will get the library versions, and will be
   slightly slower */
# define toupper(c) \
	__extension__ ({ int __x = (c); islower(__x) ? (__x - 'a' + 'A') : __x;})
# define tolower(c) \
	__extension__ ({ int __x = (c); isupper(__x) ? (__x - 'A' + 'a') : __x;})
#endif /* !__cplusplus */

#if !defined(__STRICT_ANSI__) || defined(__INSIDE_CYGWIN__)
#define isascii(c)	((unsigned)(c)<=0177)
#define toascii(c)	((c)&0177)
#endif

#ifdef __cplusplus
}
#endif
#endif /* _CTYPE_H_ */
