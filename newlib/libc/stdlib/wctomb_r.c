#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <locale.h>
#include "mbctype.h"

/* for some conversions, we use the __count field as a place to store a state value */
#define __state __count

extern char __lc_ctype[12];

int
_DEFUN (_wctomb_r, (r, s, wchar, state),
        struct _reent *r     _AND 
        char          *s     _AND
        wchar_t        _wchar _AND
        mbstate_t     *state)
{
  /* Avoids compiler warnings about comparisons that are always false
     due to limited range when sizeof(wchar_t) is 2 but sizeof(wint_t)
     is 4, as is the case on cygwin.  */
  wint_t wchar = _wchar;

  if (strlen (__lc_ctype) <= 1)
    { /* fall-through */ }
  else if (!strcmp (__lc_ctype, "C-UTF-8"))
    {
      if (s == NULL)
        return 0; /* UTF-8 encoding is not state-dependent */

      if (state->__count == -4 && (wchar < 0xdc00 || wchar >= 0xdfff))
	{
	  /* At this point only the second half of a surrogate pair is valid. */
	  r->_errno = EILSEQ;
	  return -1;
	}
      if (wchar <= 0x7f)
        {
          *s = wchar;
          return 1;
        }
      else if (wchar >= 0x80 && wchar <= 0x7ff)
        {
          *s++ = 0xc0 | ((wchar & 0x7c0) >> 6);
          *s   = 0x80 |  (wchar &  0x3f);
          return 2;
        }
      else if (wchar >= 0x800 && wchar <= 0xffff)
        {
          if (wchar >= 0xd800 && wchar <= 0xdfff)
	    {
	      wint_t tmp;
	      /* UTF-16 surrogates -- must not occur in normal UCS-4 data */
	      if (sizeof (wchar_t) != 2)
		{
		  r->_errno = EILSEQ;
		  return -1;
		}
	      if (wchar >= 0xdc00)
		{
		  /* Second half of a surrogate pair. It's not valid if
		     we don't have already read a first half of a surrogate
		     before. */
		  if (state->__count != -4)
		    {
		      r->_errno = EILSEQ;
		      return -1;
		    }
		  /* If it's valid, reconstruct the full Unicode value and
		     return the trailing three bytes of the UTF-8 char. */
		  tmp = (state->__value.__wchb[0] << 16)
			| (state->__value.__wchb[1] << 8)
			| (wchar & 0x3ff);
		  state->__count = 0;
		  *s++ = 0x80 | ((tmp &  0x3f000) >> 12);
		  *s++ = 0x80 | ((tmp &    0xfc0) >> 6);
		  *s   = 0x80 |  (tmp &     0x3f);
		  return 3;
	      	}
	      /* First half of a surrogate pair.  Store the state and return
	         the first byte of the UTF-8 char. */
	      tmp = ((wchar & 0x3ff) << 10) + 0x10000;
	      state->__value.__wchb[0] = (tmp >> 16) & 0xff;
	      state->__value.__wchb[1] = (tmp >> 8) & 0xff;
	      state->__count = -4;
	      *s = (0xf0 | ((tmp & 0x1c0000) >> 18));
	      return 1;
	    }
          *s++ = 0xe0 | ((wchar & 0xf000) >> 12);
          *s++ = 0x80 | ((wchar &  0xfc0) >> 6);
          *s   = 0x80 |  (wchar &   0x3f);
          return 3;
        }
      else if (wchar >= 0x10000 && wchar <= 0x10ffff)
        {
          *s++ = 0xf0 | ((wchar & 0x1c0000) >> 18);
          *s++ = 0x80 | ((wchar &  0x3f000) >> 12);
          *s++ = 0x80 | ((wchar &    0xfc0) >> 6);
          *s   = 0x80 |  (wchar &     0x3f);
          return 4;
        }
      else
	{
	  r->_errno = EILSEQ;
	  return -1;
	}
    }
  else if (!strcmp (__lc_ctype, "C-SJIS"))
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
	    {
	      r->_errno = EILSEQ;
	      return -1;
	    }
        }
    }
  else if (!strcmp (__lc_ctype, "C-EUCJP"))
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
	    {
	      r->_errno = EILSEQ;
	      return -1;
	    }
        }
    }
  else if (!strcmp (__lc_ctype, "C-JIS"))
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
              if (state->__state == 0)
                {
                  /* must switch from ASCII to JIS state */
                  state->__state = 1;
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
	    {
	      r->_errno = EILSEQ;
	      return -1;
	    }
        }
      else
        {
          if (state->__state != 0)
            {
              /* must switch from JIS to ASCII state */
              state->__state = 0;
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
  if (wchar >= 0x100)
    {
      r->_errno = EILSEQ;
      return -1;
    }

  *s = (char) wchar;
  return 1;
}
    
