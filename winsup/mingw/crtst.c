/*
 * crtst.c
 *
 * This object file defines _CRT_MT to have a value of 0, which will
 * turn off MT support in GCC runtime. This is linked by default unless
 * you specify -mthreads when linking with gcc. 
 *
 * Mumit Khan  <khan@nanotech.wisc.edu>
 *
 */

int _CRT_MT = 0;
