/*
 * Copyright (C) 2011 by ARM Ltd. All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software
 * is freely granted, provided that this notice is preserved.
 */

#include <stdio.h>
#include <newlib.h>
#include <stdlib.h>
#include <wchar.h>
#include "check.h"

int main()
{
#if defined(INTEGER_ONLY) || defined(NO_FLOATING_POINT)

#else
  char cbuf[512];
  wchar_t wcbuf[512], wcbuf2[512];
  double val = 1E+30;
  snprintf(cbuf, 512, "%.*f", 3, val);
  swprintf(wcbuf, 512, L"%.*f", 3, val);
  mbstowcs(wcbuf2, cbuf, 512);

  CHECK (wcscmp(wcbuf, wcbuf2) == 0);
#endif

  exit (0);
}
