/* Routine to translate from Japanese characters to Unicode */

/* Copyright (c) 2002 Red Hat Incorporated.
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

     Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.

     Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.

     The name of Red Hat Incorporated may not be used to endorse
     or promote products derived from this software without specific
     prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED.  IN NO EVENT SHALL RED HAT INCORPORATED BE LIABLE FOR ANY
   DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS   
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifdef MB_CAPABLE

#include <_ansi.h>
#include <wctype.h>
#include "local.h"
#include "jp2uc.h"

wint_t
_DEFUN (__jp2uc, (c, type), wint_t c _AND int type)
{
  int index, adj;
  unsigned char byte1, byte2, adj_byte1, adj_byte2;

  /* we actually use a table of JIS to Unicode.  For SJIS, we simply
     note that SJIS is essentially JIS with the top bits on in each
     byte.  For EUCJP, we essentially do a translation to JIS before
     accessing the table. */
  switch (type)
    {
    case JP_JIS:
      index = ((c >> 8) - 0x21) * 0xfe + ((c & 0xff) - 0x21);
      break;
    case JP_SJIS:
      index = ((c >> 8) - 0xa1) * 0xfe + ((c & 0xff) - 0xa1);
      break;
    case JP_EUCJP:
      byte1 = c >> 8;
      byte2 = c & 0xff;
      if (byte2 <= 0x7e || (byte2 & 0x1))
        {
          adj = -0x22;
          adj_byte2 = (byte2 & 0xfe) - 31;
        }
      else
        {
          adj = -0x21;
          adj_byte2 = byte2 - (0x7e + 0x21);
        }
      if (byte1 <= 0x9f)
        adj_byte1 = ((byte1 - 112) >> 1) + adj;
      else
        adj_byte1 = ((byte1 - 112) >> 1) + adj;
      index = adj_byte1 * 0xfe + adj_byte2;
      break;
    default:
      return WEOF;
    }

  if (index < 0 || index > (sizeof(jp2uc) / sizeof(unsigned short)))
    return WEOF;
  
  return (wint_t)jp2uc[index];
}

#endif /* MB_CAPABLE */
