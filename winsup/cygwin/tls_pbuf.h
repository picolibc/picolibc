/* tls_pbuf.h

   Copyright 2008, 2014 Red Hat, Inc.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

class tmp_pathbuf
{
  unsigned c_buf_old;
  unsigned w_buf_old;
public:
  tmp_pathbuf ();
  ~tmp_pathbuf ();

  inline bool check_usage (unsigned c_need, unsigned w_need)
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
