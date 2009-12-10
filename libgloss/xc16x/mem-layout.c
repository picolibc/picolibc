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
/* Ideally this kind of stuff is specified in a linker script.  It's not clear
   what the default linker script should do, so for now we have this.  */

#ifndef STACK_SIZE
/* Cache lines recycle at 4096 I think, and 4096 is listed as the page size,
   so we make the stack size a multiple of it.  Not that it's relevant or
   anything, but why not base it on *something*?  */
#define STACK_SIZE (4096 * 4)
#endif

int stack_size asm ("stack_size") = STACK_SIZE;

#ifndef SBRK_SIZE
#define SBRK_SIZE (4096 * 32)
#endif

int sbrk_size asm ("sbrk_size") = SBRK_SIZE;
