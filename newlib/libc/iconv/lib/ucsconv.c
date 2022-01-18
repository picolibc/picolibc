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
#include "ucsconv.h"

static int fake_data;

static int 
find_encoding_name (const char *searchee,
                            const char **names);


/*
 * UCS-based conversion interface functions implementation.
 */

static void *
ucs_based_conversion_open (struct _reent *rptr,
                                  const char *to,
                                  const char *from)
{
  iconv_ucs_conversion_t *uc;
  const iconv_to_ucs_ces_t   *to_ucs_bices;
  const iconv_from_ucs_ces_t *from_ucs_bices;
  
  uc = (iconv_ucs_conversion_t *)
             _calloc_r (rptr, 1, sizeof (iconv_ucs_conversion_t));
  if (uc == NULL)
    return NULL;

  /* 
   * Find CES converter for "from" encoding ("from" source encoding corresponds
   * to "to_ucs" CES converter).
   */
  for (to_ucs_bices = &_iconv_to_ucs_ces[0];
       to_ucs_bices->names != NULL;
       to_ucs_bices++)
    {
      if (find_encoding_name (from, to_ucs_bices->names) == 0)
        break;
    }
  
  /* 
   * Find CES converter for "to" encoding ("to" source encoding corresponds
   * to "from_ucs" CES converter).
   */
  for (from_ucs_bices = &_iconv_from_ucs_ces[0];
       from_ucs_bices->names != NULL;
       from_ucs_bices++)
    {
      if (find_encoding_name (to, from_ucs_bices->names) == 0)
        break;
    }

  if (to_ucs_bices->names == NULL || from_ucs_bices->names == NULL)
    goto error;

  uc->to_ucs.handlers = to_ucs_bices->handlers;
  uc->from_ucs.handlers = from_ucs_bices->handlers;
  
  /* Initialize "to UCS" CES converter */
  if (to_ucs_bices->handlers->init != NULL)
    {
      uc->to_ucs.data = to_ucs_bices->handlers->init (rptr, from);
      if (uc->to_ucs.data == NULL)
        goto error;
    }
  else
    uc->to_ucs.data = (void *)&fake_data;
    

  /* Initialize "from UCS" CES converter */
  if (from_ucs_bices->handlers->init != NULL)
    {
      uc->from_ucs.data = from_ucs_bices->handlers->init (rptr, to);
      if (uc->from_ucs.data == NULL)
        goto error;
    }
  else
    uc->from_ucs.data = (void *)&fake_data;

  return uc;

error:
  if (uc->to_ucs.data != NULL && uc->to_ucs.handlers->close != NULL)
    uc->to_ucs.handlers->close (rptr, uc->to_ucs.data);

  _free_r (rptr, (void *)uc);

  return NULL;
}


static size_t
ucs_based_conversion_close (struct _reent *rptr,
                                   void *data)
{
  iconv_ucs_conversion_t *uc;
  size_t res = 0;

  uc = (iconv_ucs_conversion_t *)data;

  if (uc->from_ucs.handlers->close != NULL)  
    res = uc->from_ucs.handlers->close (rptr, uc->from_ucs.data);
  if (uc->to_ucs.handlers->close != NULL)
    res |= uc->to_ucs.handlers->close (rptr, uc->to_ucs.data);

  _free_r (rptr, (void *)data);

  return res;
}


static size_t
ucs_based_conversion_convert (struct _reent *rptr,
                 void *data,
                 const unsigned char **inbuf,
                 size_t *inbytesleft,
                 unsigned char **outbuf,
                 size_t *outbytesleft,
                 int flags)
{
  unsigned char outbuf1[ICONV_MB_LEN_MAX];
  unsigned char *poutbuf1;
  size_t res = 0;
  iconv_ucs_conversion_t *uc = (iconv_ucs_conversion_t *)data;

  while (*inbytesleft > 0)
    {
      register size_t bytes;
      register ucs4_t ch;
      const unsigned char *inbuf_save = *inbuf;
      size_t inbyteslef_save = *inbytesleft;

      if (*outbytesleft == 0)
        {
          _REENT_ERRNO (rptr) = E2BIG;
          return (size_t)-1;
        }

      ch = uc->to_ucs.handlers->convert_to_ucs (uc->to_ucs.data,
                                                inbuf, inbytesleft);

      if (ch == (ucs4_t)ICONV_CES_BAD_SEQUENCE)
        {
          _REENT_ERRNO (rptr) = EINVAL;
          return (size_t)-1;
        }

      if (ch == (ucs4_t)ICONV_CES_INVALID_CHARACTER)
        {
          _REENT_ERRNO (rptr) = EILSEQ;
          return (size_t)-1;
        }

      if (flags & ICONV_DONT_SAVE_BIT)
        {
          poutbuf1 = &outbuf1[0];
          outbuf = &poutbuf1;
        }

      bytes = uc->from_ucs.handlers->convert_from_ucs (uc->from_ucs.data, ch,
                                                       outbuf, outbytesleft); 

      if (bytes == (size_t)ICONV_CES_NOSPACE)
        {
          *inbuf = inbuf_save;
          *inbytesleft = inbyteslef_save;
          _REENT_ERRNO (rptr) = E2BIG;
          return (size_t)-1;
        }
      else if (bytes == (size_t)ICONV_CES_INVALID_CHARACTER)
        {
          if (flags & ICONV_FAIL_BIT)
            {
              /* Generate error */
              _REENT_ERRNO (rptr) = EILSEQ;
              return (size_t)-1;
            }
          /*
           * For this case SUSv3 stands: "if iconv() encounters a character in the
           * input buffer that is valid, but for which an identical character does
           * not exist in the target encoding, iconv() shall perform an
           * implementation-defined conversion on this character".
           * Don't generate error, just write default character.
           */
          bytes = uc->from_ucs.handlers->convert_from_ucs (
                                         uc->from_ucs.data,
                                         (ucs4_t)DEFAULT_CHARACTER,
                                         outbuf,
                                         outbytesleft);
          if ((__int32_t)bytes < 0)
            {
              _REENT_ERRNO (rptr) = E2BIG;
              return (size_t)-1;
            }
      
          res += 1;
        }
    }

  return res;
}


static int
ucs_based_conversion_get_mb_cur_max (void *data,
                                            int direction)
{
  iconv_ucs_conversion_t *uc = (iconv_ucs_conversion_t *)data;
  
  if (direction == 0)
    return uc->to_ucs.handlers->get_mb_cur_max (uc->to_ucs.data);
  else
    return uc->from_ucs.handlers->get_mb_cur_max (uc->from_ucs.data);
}


static void
ucs_based_conversion_get_state (void *data,
                                       mbstate_t *state,
                                       int direction)
{
  iconv_ucs_conversion_t *uc = (iconv_ucs_conversion_t *)data;
  mbstate_t nullstate = ICONV_ZERO_MB_STATE_T;
 
  if (direction == 0)
    {
      if (uc->to_ucs.handlers->get_state != NULL)
        uc->to_ucs.handlers->get_state (uc->to_ucs.data, state);
      else
        *state = nullstate; /* internal copy */
    }
  else
    {
      if (uc->from_ucs.handlers->get_state != NULL)
        uc->from_ucs.handlers->get_state (uc->from_ucs.data, state);
      else
        *state = nullstate; /* internal copy */
    }

  return;
}


static int
ucs_based_conversion_set_state (void *data,
                                       mbstate_t *state,
                                       int direction)
{
  iconv_ucs_conversion_t *uc = (iconv_ucs_conversion_t *)data;

  if (direction == 0)
    {
      if (uc->to_ucs.handlers->set_state != NULL)
        return uc->to_ucs.handlers->set_state (uc->to_ucs.data, state);
    }
  else
    {
      if (uc->from_ucs.handlers->set_state != NULL)
        return uc->from_ucs.handlers->set_state (uc->from_ucs.data, state);
    }

  return 0;
}

static int
ucs_based_conversion_is_stateful (void *data,
                                         int direction)
{
  iconv_ucs_conversion_t *uc = (iconv_ucs_conversion_t *)data;

  if (direction == 0)
    {
      if (uc->to_ucs.handlers->is_stateful != NULL)
        return uc->to_ucs.handlers->is_stateful (uc->to_ucs.data);
    }
  else
    {
      if (uc->from_ucs.handlers->is_stateful != NULL)
        return uc->from_ucs.handlers->is_stateful (uc->from_ucs.data);
    }

  return 0;
}


/* UCS-based conversion definition object */
const iconv_conversion_handlers_t 
_iconv_ucs_conversion_handlers =
{
  ucs_based_conversion_open,
  ucs_based_conversion_close,
  ucs_based_conversion_convert,
  ucs_based_conversion_get_state,
  ucs_based_conversion_set_state,
  ucs_based_conversion_get_mb_cur_max,
  ucs_based_conversion_is_stateful
};


/*
 * Supplementary functions.
 */

static int
find_encoding_name (const char *searchee,
                           const char **names)
{
  const char *p;

  for (p = *names; p != NULL; p = *(names++))
    if (strcmp (p, searchee) == 0)
      return 0;

  return -1;
}

