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

union u {
  struct {
  short int msw;
  unsigned short lsw;
} w;
long l;
};

int
__cmpsi2(long arga,
	 short int msw_b, unsigned short int lsw_b)
{
  union u u;
  u.l = arga;

  if (u.w.msw != msw_b)
    {
      if (u.w.msw < msw_b) return 0;
      return 2;
    }
  if (u.w.lsw != lsw_b) 
    {
      if (u.w.lsw < lsw_b) return 0;
      return 2;
    }
  return 1;
}
