/*
Copyright (c) 1990 The Regents of the University of California.
All rights reserved.

Redistribution and use in source and binary forms are permitted
provided that the above copyright notice and this paragraph are
duplicated in all such forms and that any documentation,
and/or other materials related to such
distribution and use acknowledge that the software was developed
by the University of California, Berkeley.  The name of the
University may not be used to endorse or promote products derived
from this software without specific prior written permission.
THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */


int __ashrhi3(int a,int b)
{
  return a>>b;
}

int __ashlhi3(int a,int b)
{
  return a<<b;
}

unsigned __lshlhi3(unsigned int a,int b)
{
  return a<<b;
}

unsigned __lshrhi3(unsigned int a,int b)
{
  return a>>b;
}




long __ashrsi3(long a, int b)
{
  return a>>b;
}

long __ashlsi3(long a,int b)
{
  return a<<b;
}

unsigned __lshlsi3(unsigned long a,int b)
{
  return a<<b;
}

unsigned __lshrsi3(unsigned long a,int b)
{
  return a>>b;
}

