/*
 * crtmt.c
 *
 * This object file defines _CRT_MT to have a value of 1, which will
 * turn on MT support in GCC runtime. This is only linked in when
 * you specify -mthreads when linking with gcc. The Mingw support
 * library, libmingw32.a, contains the complement, crtst.o, which
 * sets this variable to 0. 
 *
 * Mumit Khan  <khan@nanotech.wisc.edu>
 *
 */

int _CRT_MT = 1;
