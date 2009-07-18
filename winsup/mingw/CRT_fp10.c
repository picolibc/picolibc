/*
 * CRT_FP10.c
 *
 * This defines _fpreset as asm ("fnint"). Calls to _fpreset
 * will set default floating point precesion to 64-bit mantissa
 * at app startup.
 *
 * Linking in CRT_FP10.o before libmingw.a will override the definition
 * set in CRT_FP8.o.
 */

/* Override library  _fpreset() with asm fninit */
void _fpreset (void)
  { __asm__ ( "fninit" ) ;}

#if defined(__PCC__)
void _Pragma("alias _fpreset") fpreset(void);
#else
void __attribute__ ((alias ("_fpreset"))) fpreset(void);
#endif
