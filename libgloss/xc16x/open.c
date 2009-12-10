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
#include<sys/types.h>
#include<sys/stat.h>
/*volatile int opensys(char *name,int flags,int perms)
{
 #ifndef __xc16xL__
        asm volatile("push r11\n"
		     "mov r11,r10 \n"
                     " mov r10,r9  \n"
                     " mov r9,#0x300 \n"
		     );
                                                                                
  #endif

asm volatile("trap #5");
#ifndef __xc16xL__
asm volatile("pop r11");
#endif
}*/

int _open(char *name,int flags,int perms)
{
 int temp;

  temp=opensys(name,flags,perms);    
return temp;  
}
