/*
Copyright (C) 2002 by Red Hat, Incorporated. All rights reserved.

Permission to use, copy, modify, and distribute this software
is freely granted, provided that this notice is preserved.
 */
#include <stdlib.h>
#include <stdio.h>

void a(void);
void b(void);
void c(int, void *);
static void newline(void);

void a (void)
{
  printf("a");
}

void b (void)
{
  printf("b");
}

void c (int code, void *k)
{
  char *x = (char *)k;
  printf("%d%c",code,x[0]);
}

static void newline (void)
{
  printf("\n");
}

int main(void)
{
  if (atexit(newline) != 0)
    abort();

  if (atexit(a) != 0)
    abort();

  if (atexit(b) != 0)
    abort();

  if (on_exit(c,(void *)"c") != 0)
    abort();

  if (atexit(a) != 0)
    abort();

  exit(0);
}
