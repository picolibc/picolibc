#include <alloca.h>
#include "libc-symbols.h"

# define extend_alloca(buf, len, newlen) \
  (__typeof (buf)) ({ size_t __newlen = (newlen);                             \
                      char *__newbuf = alloca (__newlen);                     \
                      if (__newbuf > (char *)buf)			      \
			if ((char *)buf + len == __newbuf) {	              \
                          len += __newlen;				      \
                          __newbuf = buf;				      \
                        }						      \
                      else {                                                  \
			if (__newbuf + newlen == (char *)buf) 	              \
                          len += __newlen;				      \
                        else						      \
                          len = __newlen;                                     \
                      }			                                      \
                      __newbuf; })

#define __fsetlocking(fp, x) fp

extern const char *_res_opcodes[];
libresolv_hidden_proto (_res_opcodes)

