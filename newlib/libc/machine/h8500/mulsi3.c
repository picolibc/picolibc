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

typedef union {
  struct {
  unsigned int msw;
  unsigned int lsw;
} s;
  long v;
} u;

long __mulsi3(u a, u b)
{
  int s;
  long pp1;
  long pp2;
  long r;

  if (a.s.msw == 0 &&
      b.s.msw == 0)
    {
      return (long)a.s.lsw * b.s.lsw;
    }

  s = 0;
  if (a.v < 0)
    {
      s = 1;
      a.v = - a.v;
    }
  if (b.v < 0)
    { 
      s = 1-s;
      b.v = - b.v;
    }

  pp1 = (long)a.s.lsw * b.s.lsw ;
  pp2 = (long)a.s.lsw * b.s.msw + (long)a.s.msw * b.s.lsw;

  pp1 += pp2 << 16;

  if (s)
    {
      pp1 = -pp1;
    }
  return pp1;
}
long __mulpsi3(long a, long b)
{
 return a*b;
}


short 
__mulhi3(short a, short b)
{
  int r;

  r = 0;
  while (a) 
    {
      if (a & 1) 
	{
	  r += b;

	}
      b<<=1;
      a>>=1;

    }
  return r;
}


