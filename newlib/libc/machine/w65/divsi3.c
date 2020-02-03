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

#define divnorm(num, den, sign) 		\
{						\
  if (num < 0) 					\
    {						\
      num = -num;				\
      sign = 1;					\
    }						\
  else 						\
    {						\
      sign = 0;					\
    }						\
						\
  if (den < 0) 					\
    {						\
      den = - den;				\
      sign = 1 - sign;				\
    } 						\
}





static unsigned long 
divmodsi4(int modwanted, unsigned long num, unsigned long den)	
{						
  long int bit = 1;				
  long int res = 0;				
  long prevden;
  while (den < num && bit && !(den & (1L<<31)))			       
    {
      den <<=1;					
      bit <<=1;					
    }						
  while (bit)
    {					
      if (num >= den)
	{				
	  num -= den;				
	  res |= bit;				
	}						
      bit >>=1;					
      den >>=1;					
    }						
  if (modwanted) return num;
  return res;
}


#define exitdiv(sign, res) if (sign) { res = - res;} return res;

long 
__modsi3 (long numerator, long denominator)
{
  int sign = 0;
  long dividend;
  long modul;


  if (numerator < 0) 
    {
      numerator = -numerator;
      sign = 1;
    }
  if (denominator < 0)
    {
      denominator = -denominator;
    }  
  
  modul =  divmodsi4 (1, numerator, denominator);
  if (sign)
    return - modul;
  return modul;
}


long 
__divsi3 (long numerator, long denominator)
{
  int sign;
  long dividend;
  long modul;
  divnorm (numerator, denominator, sign);

  dividend = divmodsi4 (0,  numerator, denominator);
  exitdiv (sign, dividend);
}

long 
__umodsi3 (unsigned long numerator, unsigned long denominator)
{
  long dividend;
  long modul;

modul= divmodsi4 (1,  numerator, denominator);
  return modul;
}

long 
__udivsi3 (unsigned long numerator, unsigned long denominator)
{
  int sign;
  long dividend;
  long modul;
  dividend =   divmodsi4 (0, numerator, denominator);
  return dividend;
}






#ifdef TEST



main ()
{
  long int i, j, k, m;
  for (i = -10000; i < 10000; i += 8)
    {
      for (j = -10000; j < 10000; j += 11)
	{
	  k = i / j;
	  m = __divsi3 (i, j);
	  if (k != m)
	    printf ("fail %d %d %d %d\n", i, j, k, m);
	}
    }
}

#endif
