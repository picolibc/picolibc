/*
 * Support file for nvptx in newlib.
 * Copyright (c) 2014-2018 Mentor Graphics.
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

#include <unistd.h>
#include <stdlib.h>

/* Sadly, PTX doesn't support weak declarations, only weak
   definitions.  Weakly define it here in case we're not using crt0
   (for instance in offloading).  You probably shouldn't be calling
   'exit' in an offloaded region anyway, but that'd be a runtime
   error, not a link error.  */
int *__attribute((weak)) __exitval_ptr;

void __attribute__((noreturn))
_exit (int status)
{
  if (__exitval_ptr)
    {
      *__exitval_ptr = status;
      for (;;)
	asm ("exit;" ::: "memory");
    }
  else /* offloading */
    {
      /* Map to 'abort'; see <https://gcc.gnu.org/PR85463>
	 '[nvptx] "exit" in offloaded region doesn't terminate process'.  */
      abort ();
    }
}
