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
#include <stdlib.h>
#include <signal.h>
#include  "test.h"
#include <math.h>
#include <string.h>
#include <stdint.h>
#include <malloc.h>
int verbose;
static int count;
int inacc;

int redo = 0;
extern int calc;

#ifdef MALLOC_DEBUG
extern char **environ;
extern int _malloc_test_fail;
#endif

int
main (int ac,
      char **av)
{
#ifdef MALLOC_DEBUG
  environ = en;
#endif
  int i;
  int math2 = 1;
  int string= 1;
  int is = 1;
  int math= 1;
  int cvt = 1;
#ifdef _HAVE_IEEEFP_FUNCS
  int ieee= 1;
#endif
  int vector=0;
  for (i = 1; i < ac; i++) 
  {
    if (strcmp(av[i],"-v")==0) 
     verbose ++;
    if (strcmp(av[i],"-nomath2") == 0)
     math2 = 0;
    (void) math2;
    if (strcmp(av[i],"-nostrin") == 0)
     string= 0;
    (void) string;
    if (strcmp(av[i],"-nois") == 0)
     is = 0;
    (void) is;
    if (strcmp(av[i],"-nomath") == 0)
     math= 0;
    (void) math;
    if (strcmp(av[i],"-nocvt") == 0)
     cvt = 0;
#ifdef _HAVE_IEEEFP_FUNCS
    if (strcmp(av[i],"-noiee") == 0)
     ieee= 0;
  (void) ieee;
#endif
    if (strcmp(av[i],"-generate") == 0) {
     vector = 1;
     calc = 1;
    }
  }
  if (cvt)
   test_cvt();

#if TEST_PART == 0 || TEST_PART == -1
  if (math2)
   test_math2();
  if (string)
   test_string();
#endif
  if (math)
   test_math(vector);
#if TEST_PART == 0 || TEST_PART == -1
  if (is)
   test_is();
#ifdef _HAVE_IEEEFP_FUNCS
  if (ieee)
   test_ieee();
#endif
#endif
  printf("Tested %d functions, %d errors detected\n", count, inacc);
  exit(inacc != 0);
}

static const char *iname = "foo";
void newfunc (const char *string)
{
  if (strcmp(iname, string)) 
  {
#ifdef MALLOC_DEBUG
    char *memcheck = getenv("CHECK_NAME");
    if (memcheck && !strcmp(string, memcheck))
      _malloc_test_fail = atoi(getenv("CHECK_COUNT"));
    if (memcheck && !strcmp(iname, memcheck)) {
      if (_malloc_test_fail) {
	printf("malloc test fail remain %d\n", _malloc_test_fail);
	_malloc_test_fail = 0;
      }
    }
#endif
    printf("testing %s\n", string);
    fflush(stdout);
    iname = string;
  }
  
}


static int theline;

void line(int li)
{
  if (verbose)  
  {
    printf("  %d\n", li);
  }
  theline = li;
  
  count++;
}



int reduce = 0;

int strtod_vector = 0;

int 
bigger (__ieee_double_shape_type *a,
	   __ieee_double_shape_type *b)
{

  if (a->parts.msw > b->parts.msw) 
    {

      return 1;
    } 
  else if (a->parts.msw == b->parts.msw) 
    {
      if (a->parts.lsw > b->parts.lsw) 
	{
	  return 1;
	}
    }
  return 0;
}

int 
fbigger (__ieee_float_shape_type *a,
	   __ieee_float_shape_type *b)
{

  if (a->p1 > b->p1)
    {
      return 1;
    }
  return 0;
}



/* Return the first bit different between two double numbers */
int 
mag_of_error (double is,
       double shouldbe)
{
  __ieee_double_shape_type a,b;
  int i;
  int a_big;
  uint32_t mask;
  uint32_t __x;
  uint32_t msw, lsw;
  a.value = is;
  
  b.value = shouldbe;
  
  if (a.parts.msw == b.parts.msw 
      && a.parts.lsw== b.parts.lsw) return 64;


  /* Subtract the larger from the smaller number */

  a_big = bigger(&a, &b);

  if (!a_big) {
    uint32_t t;
    t = a.parts.msw;
    a.parts.msw = b.parts.msw;
    b.parts.msw = t;

    t = a.parts.lsw;
    a.parts.lsw = b.parts.lsw;
    b.parts.lsw = t;
  }



  __x = (a.parts.lsw) - (b.parts.lsw);							
  msw = (a.parts.msw) - (b.parts.msw) - (__x > (a.parts.lsw));
  lsw = __x;								

  


  /* Find out which bit the difference is in */
  mask = 0x80000000UL;
  for (i = 0; i < 32; i++)
  {
    if (((msw) & mask)!=0) return i;
    mask >>=1;
  }
  
  mask = 0x80000000UL;
  for (i = 0; i < 32; i++)
  {
    
    if (((lsw) & mask)!=0) return i+32;
    mask >>=1;
  }
  
  return 64;
  
}

/* Return the first bit different between two double numbers */
int 
fmag_of_error (float is,
       float shouldbe)
{
  __ieee_float_shape_type a,b;
  int i;
  int a_big;
  uint32_t mask;
  uint32_t sw;
  a.value = is;
  
  b.value = shouldbe;
  
  if (a.p1 == b.p1) return 32;

  /* Subtract the larger from the smaller number */

  a_big = fbigger(&a, &b);

  if (!a_big) {
    uint32_t t;
    t = a.p1;
    a.p1 = b.p1;
    b.p1 = t;
  }

  sw = (a.p1) - (b.p1);

  mask = 0x80000000UL;
  for (i = 0; i < 32; i++)
  {
	  if (((sw) & mask)!=0) return i;
	  mask >>=1;
  }
  return 32;
}

 int ok_mag;



void
test_sok (char *is,
       char *shouldbe)
{
  if (strcmp(is,shouldbe))
    {
    printf("%s:%d, inacurate answer: (%s should be %s)\n",
	   iname, 
	   theline,
	   is, shouldbe);
    inacc++;
  }
}
void
test_iok (int is,
       int shouldbe)
{
  if (is != shouldbe){
    printf("%s:%d, inacurate answer: (%08x should be %08x)\n",
	   iname, 
	   theline,
	   is, shouldbe);
    inacc++;
  }
}


/* Compare counted strings upto a certain length - useful to test single
   prec float conversions against double results
*/
void 
test_scok (char *is,
       char *shouldbe,
       int count)
{
  if (strncmp(is,shouldbe, count))
    {
    printf("%s:%d, inacurate answer: (%s should be %s)\n",
	   iname, 
	   theline,
	   is, shouldbe);
    inacc++;
  }
}

/* Compare counted strings upto a certain length, allowing two forms - useful to test single
   prec float conversions against double results
*/
void 
test_scok2 (char *is,
       char *maybe1,
       char *maybe2,
       int count)
{
  if (strncmp(is,maybe1, count) && (maybe2 == NULL || strncmp(is,maybe2,count)))
    {
    printf("%s:%d, inacurate answer: (%s may be %s or %s)\n",
	   iname, 
	   theline,
	   is, maybe1, maybe2 ? maybe2 : "(nothing)");
    inacc++;
  }
}

void
test_eok (int is,
       int shouldbe)
{
  if (is != shouldbe){
    printf("%s:%d, bad errno answer: (%d should be %d)\n",
	   iname, 
	   theline,
	   is, shouldbe);
    inacc++;
  }
}

void
test_mok (double value,
       double shouldbe,
       int okmag)
{
  __ieee_double_shape_type a,b;
  int mag = mag_of_error(value, shouldbe);
  if (mag == 0) 
  {
    /* error in the first bit is ok if the numbers are both 0 */
    if (value == 0.0 && shouldbe == 0.0)
     return;
    
  }
  a.value = shouldbe;
  b.value = value;
  
  if (mag < okmag) 
  {
    printf("%s:%d, wrong answer: bit %d ",
	   iname, 
	   theline,
	   mag);
     printf("%08lx%08lx %08lx%08lx) ",
	    (unsigned long) a.parts.msw,	     (unsigned long) a.parts.lsw,
	    (unsigned long) b.parts.msw,	     (unsigned long) b.parts.lsw);
    printf("(%g %g)\n",   a.value, b.value);
    inacc++;
  }
}

void
test_mfok (float value,
	   float shouldbe,
	   int okmag)
{
  __ieee_float_shape_type a,b;
  int mag = fmag_of_error(value, shouldbe);
  if (mag == 0) 
  {
    /* error in the first bit is ok if the numbers are both 0 */
    if (value == 0.0f && shouldbe == 0.0f)
     return;
    
  }
  a.value = shouldbe;
  b.value = value;
  
  if (mag < okmag) 
  {
    printf("%s:%d, wrong answer: bit %d ",
	   iname, 
	   theline,
	   mag);
     printf("%08lx %08lx) ",
	    (unsigned long) a.p1,
	    (unsigned long) b.p1);
     printf("(%g %g)\n",   (double) a.value, (double) b.value);
    inacc++;
  }
}

#ifdef __PCCNECV70__
kill() {}
getpid() {}
#endif

void bt(void){

  double f1,f2;
  f1 = 0.0;
  f2 = 0.0/f1;
  printf("(%g)\n", f2);

}
