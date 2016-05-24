/* tmpbuf.h

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _TMPBUF_H
#define _TMPBUF_H
class tmpbuf
{
  void *buf;
public:
  tmpbuf (size_t size = NT_MAX_PATH)
  {
    buf = calloc (1, size);
    if (!buf)
      api_fatal ("allocation of temporary buffer failed");
  }
  operator void * () {return buf;}
  operator char * () {return (char *) buf;}
  operator PSECURITY_DESCRIPTOR () {return (PSECURITY_DESCRIPTOR) buf;}
  PSECURITY_DESCRIPTOR operator -> () {return  (PSECURITY_DESCRIPTOR) buf;}
  ~tmpbuf () {free (buf);}
};
#endif /*_TMPBUF_H*/
