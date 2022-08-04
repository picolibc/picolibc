/* tls_pbuf.h

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#pragma once

class tmp_pathbuf
{
  uint32_t c_buf_old;
  uint32_t w_buf_old;
public:
  tmp_pathbuf () __attribute__ ((always_inline))
  : c_buf_old (_my_tls.locals.pathbufs.c_cnt),
    w_buf_old (_my_tls.locals.pathbufs.w_cnt)
  {}
  ~tmp_pathbuf () __attribute__ ((always_inline))
  {
    _my_tls.locals.pathbufs.c_cnt = c_buf_old;
    _my_tls.locals.pathbufs.w_cnt = w_buf_old;
  }

  inline bool check_usage (uint32_t c_need, uint32_t w_need)
    {
      return c_need + c_buf_old < TP_NUM_C_BUFS
	     && w_need + w_buf_old < TP_NUM_W_BUFS;
    }
  char *c_get ();  /* Create temporary TLS path buf of size NT_MAX_PATH. */
  PWCHAR w_get (); /* Create temporary TLS path buf of size 2 * NT_MAX_PATH. */
  inline char *t_get () { return (char *) w_get (); }
  inline PUNICODE_STRING u_get (PUNICODE_STRING up)
    {
      up->Length = 0;
      up->MaximumLength = (NT_MAX_PATH - 1) * sizeof (WCHAR);
      up->Buffer = w_get ();
      return up;
    }
};
