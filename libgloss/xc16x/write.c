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

/*int trap0(int file, unsigned long ptr,int len)
{
asm volatile("TRAP #3");
return len;
}
*/
int _write(int file, char *ptr,int len)
{
 return trap0(file,(unsigned long)ptr,len);
}
