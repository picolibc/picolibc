/*
 * Copyright (C) 2006 KPIT Cummins
 * Copyright (C) 2009 Conny Marco Menebröcker
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms is permitted
 * provided that the above copyright notice and following paragraph are
 * duplicated in all such forms.
 *
 * This file is distributed WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <_ansi.h>
#include <sys/types.h>
#include <sys/stat.h>

 char *stack_ptr ;

caddr_t
  _sbrk(incr)
     int incr;
{
  extern char end;		/* Defined by the linker */
  static char *heap_end=&end;
  char *prev_heap_end;



  prev_heap_end = heap_end;


  heap_end += incr;
  return (caddr_t)prev_heap_end;
}

