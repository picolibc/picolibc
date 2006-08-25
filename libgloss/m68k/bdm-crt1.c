/* Initialization code for coldfire boards.
 *
 * Copyright (c) 2006 CodeSourcery Inc
 *
 * The authors hereby grant permission to use, copy, modify, distribute,
 * and license this software and its documentation for any purpose, provided
 * that existing copyright notices are retained in all copies and that this
 * notice is included verbatim in any distributions. No written agreement,
 * license, or royalty fee is required for any of the authorized uses.
 * Modifications to this software may be copyrighted by their authors
 * and need not follow the licensing terms described here, provided that
 * the new terms are clearly indicated on the first page of each file where
 * they apply.
 */

#include <stdlib.h>

extern const int __interrupt_vector[];
extern void __reset (void);

extern const char __data_load[] __attribute__ ((aligned (4)));
extern char __data_start[] __attribute__ ((aligned (4)));
extern char __bss_start[] __attribute__ ((aligned (4)));
extern char __end[] __attribute__ ((aligned (4)));

extern void software_init_hook (void) __attribute__ ((weak));
extern void hardware_init_hook (void) __attribute__ ((weak));
extern void __INIT_SECTION__ (void);
extern void __FINI_SECTION__ (void);

extern int main (int, char **, char **);

/* This is called from a tiny assembly stub that just initializes the
   stack pointer.  */
void __start1 (void)
{
  unsigned ix;
  
  /* Set the VBR. */
  __asm__ __volatile__ ("movec.l %0,%/vbr" :: "r" (__interrupt_vector));

  /* Initialize memory */
  if (__data_load != __data_start)
    memcpy (__data_start, __data_load, __bss_start - __data_start);
  memset (__bss_start, 0, __end - __bss_start);

  if (hardware_init_hook)
    hardware_init_hook ();
  if (software_init_hook)
    software_init_hook ();

  __INIT_SECTION__ ();

  /* I'm not sure how useful it is to have a fini_section in an
     embedded system.  */
  atexit (__FINI_SECTION__);
  
  ix = main (0, NULL, NULL);
  exit (ix);
  
  while (1)
    __reset ();
}
