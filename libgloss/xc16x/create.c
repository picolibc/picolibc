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
#include<sys/stat.h>
#include<sys/types.h>
/*volatile int creatsys(char *name,int perms)
{
 #ifndef __xc16xL__
        asm volatile("push r10\n"
		     " mov r10,r9  \n"
                     " mov r9,#0x300 \n"
		     );
                                                                                
  #endif

asm volatile("trap #7");
#ifndef __xc16xL__
asm volatile("pop r10");
#endif
}
*/
volatile int _creat(char *name,int perms)
{
 int temp;

  temp=creatsys(name,perms);    
// putchar((char)temp);
//printf("%d\n",temp);					
return temp;  
}
