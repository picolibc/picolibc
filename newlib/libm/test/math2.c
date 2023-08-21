/*
 * Copyright (c) 1994 Cygnus Support.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * and/or other materials related to such
 * distribution and use acknowledge that the software was developed
 * at Cygnus Support, Inc.  Cygnus Support, Inc. may not be used to
 * endorse or promote products derived from this software without
 * specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "test.h"
#include <errno.h>


int
randi (void)
{
  static int32_t next;
  next = (next * 1103515245) + 12345;
  return ((next >> 16) & 0xffff);
}

#ifndef __FLOAT_WORD_ORDER__
#define __FLOAT_WORD_ORDER__ __BYTE_ORDER__
#endif

double randx (void)
{
  double res;

  do
  {
    union {
	short parts[4];
	double res;
      } u;

#if __FLOAT_WORD_ORDER__ == __ORDER_LITTLE_ENDIAN__
    u.parts[0] = randi();
    u.parts[1] = randi();
    u.parts[2] = randi();
    u.parts[3] = randi();
#else
    u.parts[3] = randi();
    u.parts[2] = randi();
    u.parts[1] = randi();
    u.parts[0] = randi();
#endif
    res = u.res;

  } while (!finite(res));

  return res ;
}

/* Return a random double, but bias for numbers closer to 0 */
double randy (void)
{
  int pow;
  double r= randx();
  r = frexp(r, &pow);
  return ldexp(r, randi() & 0x1f);
}

void
test_frexp (void)
{
  int i;
  double r;
  int t;

  float xf;
  double gives;

  int pow;


  /* Frexp of x return a and n, where a * 2**n == x, so test this with a
     set of random numbers */
  for (t = 0; t < 2; t++)
  {
    for (i = 0; i < 1000; i++)
    {

      double x = randx();
      line(i);
      switch (t)
      {
      case 0:
	newfunc("frexp/ldexp");
	r = frexp(x, &pow);
	if (r > 1.0 || r < -1.0)
	{
	  /* Answer can never be > 1 or < 1 */
	  test_iok(0,1);
	}

	gives = ldexp(r ,pow);
	test_mok(gives,x,62);
	break;
      case 1:
	newfunc("frexpf/ldexpf");
	if (x > (double) FLT_MIN && x < (double) FLT_MAX)
	{
	  /* test floats too, but they have a smaller range so make sure x
	     isn't too big. Also x can get smaller than a float can
	     represent to make sure that doesn't happen too */
	  xf = x;
	  r = (double) frexpf(xf, &pow);
	  if (r > 1.0 || r < -1.0)
	  {
	    /* Answer can never be > 1 or < -1 */
	    test_iok(0,1);
	  }

	  gives = (double) ldexpf(r ,pow);
	  test_mok(gives,x, 32);

	}
      }

    }

  }

  /* test a few numbers manually to make sure frexp/ldexp are not
     testing as ok because both are broken */

  r = frexp(64.0, &i);

  test_mok(r, 0.5,64);
  test_iok(i, 7);

  r = frexp(96.0, &i);

  test_mok(r, 0.75, 64);
  test_iok(i, 7);

}

/* Test mod - this is given a real hammering by the strtod type
   routines, here are some more tests.

   By definition

   modf = func(value, &iptr)

      (*iptr + modf) == value

   we test this

*/
void
test_mod (void)
{
  int i;

  newfunc("modf");


  for (i = 0; i < 1000; i++)
  {
    double intpart;
    double n;
    line(i);
    n  = randx();
    if (finite(n) && n != 0.0 )
    {
      volatile double r = modf(n, &intpart);
      line(i);
      test_mok(intpart + r, n, 63);
    }

  }
  newfunc("modff");

  for (i = 0; i < 1000; i++)
  {
    float intpart;
    double nd;
    line(i);
    nd  = randx() ;
    if (fabs(nd) < (double) FLT_MAX && finitef(nd) && nd != 0.0)
    {
      volatile float n = nd;
      volatile double r = (double) modff(n, &intpart);
      line(i);
      test_mok((double) intpart + r, (double) n, 32);
    }
  }


}

/*
Test pow by multiplying logs
*/
void
test_pow (void)
{
  unsigned int i;
  newfunc("pow");

  for (i = 0; i < 1000; i++)
  {
    double n1;
    double n2;
    double res;
    double shouldbe;

    line(i);
    n1 = fabs(randy());
    n2 = fabs(randy()/100.0);
    res = pow(n1, n2);
    shouldbe = exp(log(n1) * n2);
    test_mok(shouldbe, res,55);
  }

  newfunc("powf");

  for (i = 0; i < 1000; i++)
  {
    float n1;
    float n2;
    float res;
    float shouldbe;

    errno = 0;

    line(i);
    n1 = fabs(randy());
    n2 = fabs(randy()/100.0);
    res = powf(n1, n2);
    shouldbe = expf(logf(n1) * n2);
    if (!errno)
      test_mok((double) shouldbe, (double) res,28);
  }




}



void
test_math2 (void)
{
  test_mod();
  test_frexp();
  test_pow();
}
