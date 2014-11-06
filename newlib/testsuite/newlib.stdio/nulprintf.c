/*
 * Copyright (C) 2014 by ARM Ltd. All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software
 * is freely granted, provided that this notice is preserved.
 */

#include <stdio.h>
#include "check.h"

const char m[8] = {'M','M','M','M','M','M','M','M'};

int main()
{
  printf ("%.*s\n", 8, m);
  exit (0);
}
