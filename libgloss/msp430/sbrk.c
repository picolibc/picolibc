/* Copyright (c) 2013 Red Hat, Inc. All rights reserved.

   This copyrighted material is made available to anyone wishing to use, modify,
   copy, or redistribute it subject to the terms and conditions of the BSD
   License.   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY expressed or implied, including the implied warranties
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  A copy of this license
   is available at http://www.opensource.org/licenses. Any Red Hat trademarks that
   are incorporated in the source code or documentation are not subject to the BSD
   License and may only be used or replicated with the express permission of
   Red Hat, Inc.
*/

char *
_sbrk (int adj)
{
  extern char    end[];		/* Defined in the linker script.  */
  static char *  heap = end;
  char *         rv = heap;
  char *        sp = (char *) & sp;	/* Stack grows down, heap grows up...  */

  if (heap + adj > sp)
    {
#define MESSAGE "Heap and stack collision\n"
      write (1, MESSAGE, sizeof MESSAGE);
      abort ();
    }

  heap += adj;
  return rv;
}

char * sbrk (int) __attribute__((weak, alias ("_sbrk")));
