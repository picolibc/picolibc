/*
 * Copyright (c) 2003-2004, Artem B. Bityuckiy
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
#include <newlib.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <sys/iconvnls.h>
#ifdef _MB_CAPABLE
#include <wchar.h>
#include <iconv.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "local.h"
#include "conv.h"
#include "ucsconv.h"
#include "iconvnls.h"
#endif

/*
 * _iconv_nls_construct_filename -- constructs full file name. 
 *
 * PARAMETERS:
 *   const char *file   - the name of file.
 *   const char *dir    - the name of subdirectory;
 *   const char *ext    - file extension.
 *
 * DESCRIPTION:
 *   Function constructs patch to icionv-related file.
 *   'file' shouldn't be NULL. Doesn't use extension if 'ext' is NULL.
 *
 * RETURN:
 *   The pointer to file name if success, In case of error returns NULL
 *   and sets current thread's/process's errno.
 */
const char *
_iconv_nls_construct_filename (
                                      const char *file,
                                      const char *dir,
                                      const char *ext)
{
  int len1, len2, len3;
  char *path;
  char *p;
  int dirlen = strlen (dir);
    
  if ((path = getenv (NLS_ENVVAR_NAME)) == NULL || *path == '\0')
    path = ICONV_DEFAULT_NLSPATH;

  len1 = strlen (path);
  len2 = strlen (file);
  len3 = ext ? strlen (ext) : 0;

  if ((p = malloc (len1 + dirlen + len2 + len3 + 3)) == NULL)
    return (const char *)NULL;

  memcpy (p, path, len1);
  if (p[len1 - 1] != '/')
    p[len1++] = '/';
  memcpy (p + len1, dir, dirlen);
  len1 += dirlen;
  p[len1++] = '/';
  memcpy (p + len1, file, len2);
  len1 += len2;
  if (ext != NULL)
  {
    memcpy (p + len1, ext, len3);
    len1 += len3;
  }
  p[len1] = '\0';
 
  return (const char *)p;
}


#ifdef _MB_CAPABLE
/*
 * _iconv_nls_get_mb_cur_max -- return encoding's maximum length
 *                              of a multi-byte character.
 *
 * PARAMETERS:
 *    iconv_t cd - opened iconv conversion descriptor;
 *    int direction - "from encoding" or "to encoding" direction.
 *
 * DESCRIPTION:
 *    Return maximum  length  of a multi-byte character in one of 'cd's
 *    encoding. Return "from" encoding's value if 'direction' is 0 and
 *    "to" encoding's value if 'direction' isn't 0.
 */
int
_iconv_nls_get_mb_cur_max (iconv_t cd,
                                  int direction)
{
  iconv_conversion_t *ic = (iconv_conversion_t *)cd;
  
  return ic->handlers->get_mb_cur_max (ic->data, direction);
}

/*
 * _iconv_nls_is_stateful -- is encoding stateful?
 *
 * PARAMETERS:
 *    iconv_t cd - opened iconv conversion descriptor;
 *    int direction - "from encoding" or "to encoding" direction.
 *
 * DESCRIPTION:
 *    Returns 0 if encoding is stateless or 1 if stateful.
 *    Tests "from" encoding if 'direction' is 0 and
 *    "to" encoding's value if 'direction' isn't 0.

 */
int
_iconv_nls_is_stateful (iconv_t cd,
                               int direction)
{
  iconv_conversion_t *ic = (iconv_conversion_t *)cd;
  
  return ic->handlers->is_stateful (ic->data, direction);
}

/*
 * _iconv_nls_conv - special version of iconv for NLS.
 *
 * PARAMETERS:
 *    Same as _iconv_r.
 *
 * DESCRIPTION:
 *    Function behaves as _iconv_r but:
 *    1.  Don't handle reset/return shift states queries
 *        (like iconv does when 'inbuf' == NULL, etc);
 *    2. Don't save result if 'outbuf' == NULL or
 *       '*outbuf' == NULL;
 *    3. Don't perform default conversion if there is no character
 *       in "to" encoding that corresponds to character from "from"
 *       encoding.
 *
 * RETURN:
 *    Same as _iconv_r.
 */
size_t
_iconv_nls_conv (
                        iconv_t cd,
                        const char **inbuf,
                        size_t *inbytesleft,
                        char **outbuf,
                        size_t *outbytesleft)
{
  iconv_conversion_t *ic = (iconv_conversion_t *)cd;
  int flags = ICONV_FAIL_BIT;

  if ((void *)cd == NULL || cd == (iconv_t)-1 || ic->data == NULL
       || (ic->handlers != &_iconv_null_conversion_handlers
           && ic->handlers != &_iconv_ucs_conversion_handlers))
    {
      _REENT_ERRNO (rptr) = EBADF;
      return (size_t)-1;
    }
  
  if (inbytesleft == NULL || *inbytesleft == 0)
    return (size_t)0;
  
  if (outbuf == NULL || *outbuf == NULL)
    flags |= ICONV_DONT_SAVE_BIT;
  
  if (outbytesleft == NULL || *outbytesleft == 0)
    {
      _REENT_ERRNO (rptr) = E2BIG;
      return (size_t)-1;
    }

  return ic->handlers->convert (
                                ic->data,
                                (const unsigned char**)inbuf,
                                inbytesleft,
                                (unsigned char**)outbuf,
                                outbytesleft,
                                flags);
}

/*
 * _iconv_nls_get_state -- get encoding's current shift state value.
 *
 * PARAMETERS:
 *    iconv_t cd - iconv descriptor; 
 *    mbstate_t *ps - where to save shift state;
 *    int direction - "from" encoding if 0, "to" encoding if 1.
 *
 * DESCRIPTION:
 *    Save encoding's current shift state to 'ps'. Save "from" encoding's
 *    shift state if 'direction' is 0 and "to" encodings's shift state
 *    if 'direction' isn't 0.
 */
void
_iconv_nls_get_state (iconv_t cd,
                             mbstate_t *ps,
                             int direction)
{
  iconv_conversion_t *ic = (iconv_conversion_t *)cd;
  
  ic->handlers->get_state (ic->data, ps, direction);

  return;
}

/*
 * _iconv_nls_set_state -- set encoding's current shift state value.
 *
 * PARAMETERS:
 *    iconv_t cd    - iconv descriptor; 
 *    mbstate_t *ps - where to save shift state.
 *    int direction - "from" encoding if 0, "to" encoding if 1.
 *
 * DESCRIPTION:
 *    Set encoding's current shift state.
 *
 * RETURN:
 *    0 if success, -1 if failure.
 */
int
_iconv_nls_set_state (iconv_t cd,
                             mbstate_t *ps,
                             int direction)
{
  iconv_conversion_t *ic = (iconv_conversion_t *)cd;
  
  return ic->handlers->set_state (ic->data, ps, direction);
}

/* Same as iconv_open() but don't perform name resolving */
static iconv_t
iconv_open1 (
                     const char *to,
                     const char *from)
{
  iconv_conversion_t *ic;
    
  if (to == NULL || from == NULL || *to == '\0' || *from == '\0')
    return (iconv_t)-1;

  ic = (iconv_conversion_t *)malloc (sizeof (iconv_conversion_t));
  if (ic == NULL)
    return (iconv_t)-1;

  /* Select which conversion type to use */
  if (strcmp (from, to) == 0)
    {
      /* Use null conversion */
      ic->handlers = &_iconv_null_conversion_handlers;
      ic->data = ic->handlers->open (to, from);
    }
  else  
    {
      /* Use UCS-based conversion */
      ic->handlers = &_iconv_ucs_conversion_handlers;
      ic->data = ic->handlers->open (to, from);
    }

  if (ic->data == NULL)
    {
      free ((void *)ic);
      return (iconv_t)-1;
    }

  return (void *)ic;
}

/*
 * _iconv_nls_open - open iconv descriptors for NLS.
 *
 * PARAMETERS:
 *     const char *encoding - encoding name;
 *     iconv_t *tomb - wchar -> encoding iconv descriptor pointer;
 *     iconv_t *towc - encoding -> wchar iconv descriptor pointer;
 *     int flag - perform encoding name resolving flag.
 *
 * DESCRIPTION:
 *     Opens two iconv descriptors for 'encoding' -> wchar and 
 *     wchar -> 'encoding' iconv conversions. Function is used when locale or
 *     wide-oriented stream is opened. If 'flag' is 0, don't perform encoding
 *     name resolving ('encoding' must not be alias in this case).
 *
 * RETURN:
 *     If successful - return 0, else set errno and return -1.
 */
int
_iconv_nls_open (
                        const char *encoding,
                        iconv_t *tomb,
                        iconv_t *towc,
                        int flag)
{
  const char *wchar_encoding;

  if (sizeof (wchar_t) > 2 && WCHAR_MAX > 0xFFFF)
    wchar_encoding = "ucs_4_internal";
  else if (sizeof (wchar_t) > 1 && WCHAR_MAX > 0xFF)
    wchar_encoding = "ucs_2_internal";
  else
    wchar_encoding = ""; /* This shuldn't happen */

  if (flag)
    {
      if ((*towc = iconv_open (wchar_encoding, encoding)) == (iconv_t)-1)
        return -1;
    
      if ((*tomb = iconv_open (encoding, wchar_encoding)) == (iconv_t)-1)
      {
        iconv_close (*towc);
        return -1;
      }
    }
  else
    {
      if ((*towc = iconv_open1 (wchar_encoding, encoding)) == (iconv_t)-1)
        return -1;
    
      if ((*tomb = iconv_open1 (encoding, wchar_encoding)) == (iconv_t)-1)
      {
        iconv_close (*towc);
        return -1;
      }
    }

  return 0;
}

#endif /* _MB_CAPABLE */

