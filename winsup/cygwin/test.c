/* test.c: misc Cygwin testing code

   Copyright 1996, 1998, 1999, 2000, 2001 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include <stdio.h>
#include <windows.h>

char a[] ="This is static data";

void
test1()
{
  int depth = 0;
  while (depth < 5) 
    {
      int  r;
      printf ("about to fork %d\n", depth);

      r = fork ();

      if (r == 0)
	{
	  int res;
	  depth++;
	  printf ("************Depth is %d\n", depth);
	  sleep (1);
	}
      else 
	{
	  printf ("This is the parent, quitting %d\n", depth);
	  sleep (1);
	  exit (1);
	}
      printf ("done loop, depth %d\n", depth);
    }
}

#define N 10
int v[N];
startup ()
{
  int i;
  for (i = 0; i < N; i++)
    {
      int r;
      fflush (stdout);
      r = fork ();
      if (r) 
	{
	  v[i] = r;
	  printf ("started %d, were'id %d\n", v[i], GetCurrentProcessId ());
	  fflush (stdout);
	}
      else
	{
	  /* running the child, sleep a bit and exit. */
	  printf ("the fork said 0, were %d\n", GetCurrentProcessId ());
	  fflush (stdout);
  sleep (2);
	  printf ("Running, and exiting %d\n", i);
	  fflush (stdout);
	  _exit (i + 0x30);
	}
    }
}

test2()
{
  int i;
  startup ();
  sleep (1);
  /* Wait for them one by one */
  for (i = 0; i < N; i++) 
    {
      int res;
      
      waitpid (v[i], &res, 0);
      printf ("Process %d gave res %x\n", v[i], res);
      if (res != (0x30 + i) << 8)
	printf ("***** BAD *** Process %d gave res %x\n", v[i], res);
    }
}

test3()
{
  int i;
  startup ();
  /* Wait for them all at the same time */
  for (i = 0; i < N; i++) 
    {
      int res;
      wait (&res);
      printf ("Got res %x\n", res);
    }
}

test5()
{
  char *c = strdup ("HI STEVE");
  printf ("c is %s\n", c);
  free (c);
}

int count;

main (int ac, char **av)
{
  int r;
  int done;
  int test;
  fprintf (stderr,"TO STDERR\n");
  if (ac < 2) {
		printf ("usage: test <n>\n");
		exit (2);
	      }
  test = atoi (av[1]);

  printf ("%d %d Hi steve, about to start fork test %d %d.\n",getpid (), count++, test,
	 GetCurrentProcessId ());
fflush (stdout);
  switch (test) 
    {
    case 1:
      test1();
      break;
    case 2:
      test2();
      break;
    case 3:
      test3();
      break;
    case 4:
SetConsoleTextAttribute (GetStdHandle (STD_OUTPUT_HANDLE), FOREGROUND_RED);
break;
    case 5:
      test5();
break;
    }

}

free ()
{
  printf ("MY FREE!\n");
}

char b[100000];
int i;

malloc (x)
{
char *r = b + i;
i += x;
return r;
}

realloc ()
{
}
