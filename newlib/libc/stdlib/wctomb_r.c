#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include "mbctype.h"

int
_DEFUN (_wctomb_r, (r, s, wchar, state),
        struct _reent *r     _AND 
        char          *s     _AND
        wchar_t        wchar _AND
        int           *state)
{
  if (strlen (r->_current_locale) <= 1)
    { /* fall-through */ }
  else if (!strcmp (r->_current_locale, "C-SJIS"))
    {
      unsigned char char2 = (unsigned char)wchar;
      unsigned char char1 = (unsigned char)(wchar >> 8);

      if (s == NULL)
        return 0;  /* not state-dependent */

      if (char1 != 0x00)
        {
        /* first byte is non-zero..validate multi-byte char */
          if (_issjis1(char1) && _issjis2(char2)) 
            {
              *s++ = (char)char1;
              *s = (char)char2;
              return 2;
            }
          else
            return -1;
        }
    }
  else if (!strcmp (r->_current_locale, "C-EUCJP"))
    {
      unsigned char char2 = (unsigned char)wchar;
      unsigned char char1 = (unsigned char)(wchar >> 8);

      if (s == NULL)
        return 0;  /* not state-dependent */

      if (char1 != 0x00)
        {
        /* first byte is non-zero..validate multi-byte char */
          if (_iseucjp (char1) && _iseucjp (char2)) 
            {
              *s++ = (char)char1;
              *s = (char)char2;
              return 2;
            }
          else
            return -1;
        }
    }
  else if (!strcmp (r->_current_locale, "C-JIS"))
    {
      int cnt = 0; 
      unsigned char char2 = (unsigned char)wchar;
      unsigned char char1 = (unsigned char)(wchar >> 8);

      if (s == NULL)
        return 1;  /* state-dependent */

      if (char1 != 0x00)
        {
        /* first byte is non-zero..validate multi-byte char */
          if (_isjis (char1) && _isjis (char2)) 
            {
              if (*state == 0)
                {
                  /* must switch from ASCII to JIS state */
                  *state = 1;
                  *s++ = ESC_CHAR;
                  *s++ = '$';
                  *s++ = 'B';
                  cnt = 3;
                }
              *s++ = (char)char1;
              *s = (char)char2;
              return cnt + 2;
            }
          else
            return -1;
        }
      else
        {
          if (*state != 0)
            {
              /* must switch from JIS to ASCII state */
              *state = 0;
              *s++ = ESC_CHAR;
              *s++ = '(';
              *s++ = 'B';
              cnt = 3;
            }
          *s = (char)char2;
          return cnt + 1;
        }
    }

  if (s == NULL)
    return 0;
 
  /* otherwise we are dealing with a single byte character */
  *s = (char) wchar;
  return 1;
}
    

