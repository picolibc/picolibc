/*
 * Copyright (c) 2003-2004, Artem B. Bityuckiy
 * Copyright (c) 1999,2000, Konstantin Chuguev. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
#include <_ansi.h>
#include <reent.h>
#include <sys/types.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "local.h"
#include "conv.h"

static int null_conversion_dummy_data;


static void *
null_conversion_open (struct _reent *rptr,
                             const char *to,
                             const char *from)
{
  return (void *)&null_conversion_dummy_data;
}


static size_t
null_conversion_close (struct _reent *rptr,
                              void *data)
{
  return 0;
}


static size_t
null_conversion_convert (struct _reent *rptr,
                     void *data,
                     const unsigned char **inbuf,
                     size_t *inbytesleft,
                     unsigned char **outbuf,
                     size_t *outbytesleft,
                     int flags)
{
  size_t result;
  size_t len;
  
  if (*inbytesleft < *outbytesleft)
    {
      result = 0;
      len = *inbytesleft;
    }
  else
    {
      result = (size_t)-1;
      len = *outbytesleft;
      __errno_r (rptr) = E2BIG;
    }
  
  if ((flags & 1) == 0)
    memcpy (*outbuf, *inbuf, len);

  *inbuf        += len;
  *outbuf       += len;
  *inbytesleft  -= len;
  *outbytesleft -= len;

  return result;
}


static int
null_conversion_get_mb_cur_max (void *data,
                                       int direction)
{
  return ICONV_MB_LEN_MAX;
}


static void
null_conversion_get_state (void *data,
                                  mbstate_t *state,
                                  int direction)
{
  return;
}


static int
null_conversion_set_state (void *data,
                                  mbstate_t *state,
                                  int direction)
{
  return 0;
}

static int
null_conversion_is_stateful (void *data,
                                    int direction)
{
  return 0;
}

/* Null conversion definition object */
const iconv_conversion_handlers_t
_iconv_null_conversion_handlers =
{
  null_conversion_open,
  null_conversion_close,
  null_conversion_convert,
  null_conversion_get_state,
  null_conversion_set_state,
  null_conversion_get_mb_cur_max,
  null_conversion_is_stateful
};

