#include "winsup.h"
extern "C" {
#include <ctype.h>
#include "../locale/setlocale.h"
#include <stdlib.h>
#include <wctype.h>

extern char *__ctype_ptr__;
extern char _ctype_b[128 + 256];

/* Called from newlib's setlocale().  What we do here is to copy the
   128 bytes of charset specific ctype data into the array at _ctype_b.
   Given that the functionality is usually implemented locally in the
   application, that's the only backward compatible way to do it.
   Setlocale is usually only called once in an application, so this isn't
   time-critical anyway. */
extern int __iso_8859_index (const char *charset_ext);	/* Newlib */
extern int __cp_index (const char *charset_ext);	/* Newlib */
extern const char __ctype_cp[27][128 + 256];		/* Newlib */
extern const char __ctype_iso[15][128 + 256];		/* Newlib */

void
__set_ctype (struct __locale_t *loc, const char *charset)
{
  int idx;
  char *ctype_ptr = NULL;

  switch (*charset)
    {
    case 'I':
      idx = __iso_8859_index (charset + 9);
      /* Our ctype table has a leading ISO-8859-1 element. */
      if (idx < 0)
	idx = 0;
      else
	++idx;
      ctype_ptr = (char *) __ctype_iso[idx];
      break;
    case 'C':
      idx = __cp_index (charset + 2);
      if (idx < 0)
	break;
      ctype_ptr = (char *) __ctype_cp[idx];
      break;
    default:
      break;
    }
  if (!ctype_ptr)
    {
      ctype_ptr = (char *) _ctype_b;
    }
  loc->ctype_ptr = ctype_ptr + 127;
  /* For backward compatibilty */
  if (loc == __get_global_locale ())
    __ctype_ptr__ = loc->ctype_ptr;
}

} /* extern "C" */

/*
 * Copyright (c) 1989 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

