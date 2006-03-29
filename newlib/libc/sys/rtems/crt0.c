void rtems_provides_crt0( void ) {}
/*
 *  RTEMS Fake crt0
 *
 *  Each RTEMS BSP provides its own crt0 and linker script.  Unfortunately
 *  this means that crt0 and the linker script are not available as
 *  each tool is configured.  Without a crt0 and linker script, some
 *  targets do not successfully link "conftest.c" during the configuration 
 *  process.  So this fake crt0.c provides all the symbols required to
 *  successfully link a program.  The resulting program will not run
 *  but this is enough to satisfy the autoconf macro AC_PROG_CC.
 */

/* RTEMS provides some of its own routines including a Malloc family */

void *malloc() { return 0; }
void *realloc() { return 0; }
void free() { ; }
void abort() { ; }
int raise() { return -1; }

/* gcc 2.8.1 implicitly can generate references to these for at
 * least sparc-elf */
#if (__GNUC__ == 2) && (__GNUC_MINOR__ == 8)
strcmp() {}
strcpy() {}
strlen() {}
memcmp() {}
memcpy() {}
memset() {}
#endif

/* The PowerPC expects certain symbols to be defined in the linker script. */

#if defined(__PPC__)
  int __SDATA_START__;  int __SDATA2_START__;
  int __GOT_START__;    int __GOT_END__;
  int __GOT2_START__;   int __GOT2_END__;
  int __SBSS_END__;     int __SBSS2_END__;
  int __FIXUP_START__;  int __FIXUP_END__;
  int __EXCEPT_START__; int __EXCEPT_END__;
  int __init;           int __fini;
#endif

/*  The hppa expects this to be defined in the real crt0.s. 
 *  Also for some reason, the hppa1.1 does not find atexit()
 *  during the AC_PROG_CC tests.
 */

#if defined(__hppa__)
/*
  asm ( ".subspa \$GLOBAL\$,QUAD=1,ALIGN=8,ACCESS=0x1f,SORT=40");
  asm ( ".export \$global\$" );
  asm ( "\$global\$:");
*/

  int atexit(void (*function)(void)) { return 0; }
#endif


/*
 *  The AMD a29k generates code expecting the following.
 */

#if defined(_AM29000) || defined(_AM29K)
asm (".global V_SPILL, V_FILL" );
asm (".global V_EPI_OS, V_BSD_OS" );

asm (".equ    V_SPILL, 64" );
asm (".equ    V_FILL, 65" );

asm (".equ    V_BSD_OS, 66" );
asm (".equ    V_EPI_OS, 69" );
#endif
