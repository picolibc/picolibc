/*
 * CRT_FP8.c
 *
 * This forces calls of _fpreset to the MSVCRT function
 * exported from dll.  Effectively it make default
 * precison same as apps built with MSVC (53-bit mantissa).

 *
 * To change to 64-bit mantissa, link in CRT_FP10.o before libmingw.a. 
 */

/* Link against the _fpreset visible in import lib */

extern void (*_imp___fpreset)(void) ;
void _fpreset (void)
{  (*_imp___fpreset)(); }

#if defined(__PCC__)
void _Pragma("alias _fpreset") fpreset(void);
#else
void __attribute__ ((alias ("_fpreset"))) fpreset(void);
#endif
