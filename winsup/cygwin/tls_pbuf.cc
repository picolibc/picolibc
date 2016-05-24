/* tls_pbuf.cc

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include <winsup.h>
#include <malloc.h>
#include "tls_pbuf.h"

#define tls_pbuf	_my_tls.locals.pathbufs

void
tls_pathbuf::destroy ()
{
  for (uint32_t i = 0; i < TP_NUM_C_BUFS && c_buf[i]; ++i)
    free (c_buf[i]);
  for (uint32_t i = 0; i < TP_NUM_W_BUFS && w_buf[i]; ++i)
    free (w_buf[i]);
}

char *
tmp_pathbuf::c_get ()
{
  if (tls_pbuf.c_cnt >= TP_NUM_C_BUFS)
    api_fatal ("Internal error: TP_NUM_C_BUFS too small: %u", TP_NUM_C_BUFS);
  if (!tls_pbuf.c_buf[tls_pbuf.c_cnt]
      && !(tls_pbuf.c_buf[tls_pbuf.c_cnt] = (char *) malloc (NT_MAX_PATH)))
    api_fatal ("Internal error: Out of memory for new path buf.");
  return tls_pbuf.c_buf[tls_pbuf.c_cnt++];
}

PWCHAR
tmp_pathbuf::w_get ()
{
  if (tls_pbuf.w_cnt >= TP_NUM_W_BUFS)
    api_fatal ("Internal error: TP_NUM_W_BUFS too small: %u.", TP_NUM_W_BUFS);
  if (!tls_pbuf.w_buf[tls_pbuf.w_cnt]
      && !(tls_pbuf.w_buf[tls_pbuf.w_cnt]
	   = (PWCHAR) malloc (NT_MAX_PATH * sizeof (WCHAR))))
    api_fatal ("Internal error: Out of memory for new wide path buf.");
  return tls_pbuf.w_buf[tls_pbuf.w_cnt++];
}
