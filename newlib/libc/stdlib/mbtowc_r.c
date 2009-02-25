#include <newlib.h>
#include <stdlib.h>
#include <locale.h>
#include "mbctype.h"
#include <wchar.h>
#include <string.h>

#ifdef _MB_CAPABLE
typedef enum { ESCAPE, DOLLAR, BRACKET, AT, B, J, 
               NUL, JIS_CHAR, OTHER, JIS_C_NUM } JIS_CHAR_TYPE;
typedef enum { ASCII, JIS, A_ESC, A_ESC_DL, JIS_1, J_ESC, J_ESC_BR,
               INV, JIS_S_NUM } JIS_STATE; 
typedef enum { COPY_A, COPY_J1, COPY_J2, MAKE_A, NOOP, EMPTY, ERROR } JIS_ACTION;

/************************************************************************************** 
 * state/action tables for processing JIS encoding
 * Where possible, switches to JIS are grouped with proceding JIS characters and switches
 * to ASCII are grouped with preceding JIS characters.  Thus, maximum returned length
 * is 2 (switch to JIS) + 2 (JIS characters) + 2 (switch back to ASCII) = 6.
 *************************************************************************************/

static JIS_STATE JIS_state_table[JIS_S_NUM][JIS_C_NUM] = {
/*              ESCAPE   DOLLAR    BRACKET   AT       B       J        NUL      JIS_CHAR  OTHER */
/* ASCII */   { A_ESC,   ASCII,    ASCII,    ASCII,   ASCII,  ASCII,   ASCII,   ASCII,    ASCII },
/* JIS */     { J_ESC,   JIS_1,    JIS_1,    JIS_1,   JIS_1,  JIS_1,   INV,     JIS_1,    INV },
/* A_ESC */   { ASCII,   A_ESC_DL, ASCII,    ASCII,   ASCII,  ASCII,   ASCII,   ASCII,    ASCII },
/* A_ESC_DL */{ ASCII,   ASCII,    ASCII,    JIS,     JIS,    ASCII,   ASCII,   ASCII,    ASCII }, 
/* JIS_1 */   { INV,     JIS,      JIS,      JIS,     JIS,    JIS,     INV,     JIS,      INV },
/* J_ESC */   { INV,     INV,      J_ESC_BR, INV,     INV,    INV,     INV,     INV,      INV },
/* J_ESC_BR */{ INV,     INV,      INV,      INV,     ASCII,  ASCII,   INV,     INV,      INV },
};

static JIS_ACTION JIS_action_table[JIS_S_NUM][JIS_C_NUM] = {
/*              ESCAPE   DOLLAR    BRACKET   AT       B        J        NUL      JIS_CHAR  OTHER */
/* ASCII */   { NOOP,    COPY_A,   COPY_A,   COPY_A,  COPY_A,  COPY_A,  EMPTY,   COPY_A,  COPY_A},
/* JIS */     { NOOP,    COPY_J1,  COPY_J1,  COPY_J1, COPY_J1, COPY_J1, ERROR,   COPY_J1, ERROR },
/* A_ESC */   { COPY_A,  NOOP,     COPY_A,   COPY_A,  COPY_A,  COPY_A,  COPY_A,  COPY_A,  COPY_A},
/* A_ESC_DL */{ COPY_A,  COPY_A,   COPY_A,   NOOP,    NOOP,    COPY_A,  COPY_A,  COPY_A,  COPY_A},
/* JIS_1 */   { ERROR,   COPY_J2,  COPY_J2,  COPY_J2, COPY_J2, COPY_J2, ERROR,   COPY_J2, ERROR },
/* J_ESC */   { ERROR,   ERROR,    NOOP,     ERROR,   ERROR,   ERROR,   ERROR,   ERROR,   ERROR },
/* J_ESC_BR */{ ERROR,   ERROR,    ERROR,    ERROR,   MAKE_A,  MAKE_A,  ERROR,   ERROR,   ERROR },
};
#endif /* _MB_CAPABLE */

/* we override the mbstate_t __count field for more complex encodings and use it store a state value */
#define __state __count

extern char __lc_ctype[12];

int
_DEFUN (_mbtowc_r, (r, pwc, s, n, state),
        struct _reent *r   _AND
        wchar_t       *pwc _AND 
        const char    *s   _AND        
        size_t         n   _AND
        mbstate_t      *state)
{
  wchar_t dummy;
  unsigned char *t = (unsigned char *)s;

  if (pwc == NULL)
    pwc = &dummy;

  if (s != NULL && n == 0)
    return -2;

#ifdef _MB_CAPABLE
  if (strlen (__lc_ctype) <= 1)
    { /* fall-through */ }
  else if (!strcmp (__lc_ctype, "C-UTF-8"))
    {
      int ch;
      int i = 0;

      if (s == NULL)
        return 0; /* UTF-8 character encodings are not state-dependent */

      if (state->__count == 4)
	{
	  /* Create the second half of the surrogate pair.  For a description
	     see the comment below. */
	  wint_t tmp = (wchar_t)((state->__value.__wchb[0] & 0x07) << 18)
	    |   (wchar_t)((state->__value.__wchb[1] & 0x3f) << 12)
	    |   (wchar_t)((state->__value.__wchb[2] & 0x3f) << 6)
	    |   (wchar_t)(state->__value.__wchb[3] & 0x3f);
	  state->__count = 0;
	  *pwc = 0xdc00 | ((tmp - 0x10000) & 0x3ff);
	  return 2;
	}
      if (state->__count == 0)
	ch = t[i++];
      else
	{
	  if (n < (size_t)-1)
	    ++n;
	  ch = state->__value.__wchb[0];
	}

      if (ch == '\0')
	{
	  *pwc = 0;
	  state->__count = 0;
	  return 0; /* s points to the null character */
	}

      if (ch >= 0x0 && ch <= 0x7f)
	{
	  /* single-byte sequence */
	  state->__count = 0;
	  *pwc = ch;
	  return 1;
	}
      else if (ch >= 0xc0 && ch <= 0xdf)
	{
	  /* two-byte sequence */
	  state->__value.__wchb[0] = ch;
	  state->__count = 1;
	  if (n < 2)
	    return -2;
	  ch = t[i++];
	  if (ch < 0x80 || ch > 0xbf)
	    return -1;
	  if (state->__value.__wchb[0] < 0xc2)
	    /* overlong UTF-8 sequence */
	    return -1;
	  state->__count = 0;
	  *pwc = (wchar_t)((state->__value.__wchb[0] & 0x1f) << 6)
	    |    (wchar_t)(ch & 0x3f);
	  return i;
	}
      else if (ch >= 0xe0 && ch <= 0xef)
	{
	  /* three-byte sequence */
	  wchar_t tmp;
	  state->__value.__wchb[0] = ch;
	  if (state->__count == 0)
	    state->__count = 1;
	  else if (n < (size_t)-1)
	    ++n;
	  if (n < 2)
	    return -2;
	  ch = (state->__count == 1) ? t[i++] : state->__value.__wchb[1];
	  if (state->__value.__wchb[0] == 0xe0 && ch < 0xa0)
	    /* overlong UTF-8 sequence */
	    return -1;
	  if (ch < 0x80 || ch > 0xbf)
	    return -1;
	  state->__value.__wchb[1] = ch;
	  state->__count = 2;
	  if (n < 3)
	    return -2;
	  ch = t[i++];
	  if (ch < 0x80 || ch > 0xbf)
	    return -1;
	  state->__count = 0;
	  tmp = (wchar_t)((state->__value.__wchb[0] & 0x0f) << 12)
	    |    (wchar_t)((state->__value.__wchb[1] & 0x3f) << 6)
	    |     (wchar_t)(ch & 0x3f);
	
	  if (tmp >= 0xd800 && tmp <= 0xdfff)
	    return -1;
	  *pwc = tmp;
	  return i;
	}
      else if (ch >= 0xf0 && ch <= 0xf7)
	{
	  /* four-byte sequence */
	  wint_t tmp;
	  state->__value.__wchb[0] = ch;
	  if (state->__count == 0)
	    state->__count = 1;
	  else if (n < (size_t)-1)
	    ++n;
	  if (n < 2)
	    return -2;
	  ch = (state->__count == 1) ? t[i++] : state->__value.__wchb[1];
	  if (state->__value.__wchb[0] == 0xf0 && ch < 0x90)
	    /* overlong UTF-8 sequence */
	    return -1;
	  if (ch < 0x80 || ch > 0xbf)
	    return -1;
	  state->__value.__wchb[1] = ch;
	  if (state->__count == 1)
	    state->__count = 2;
	  else if (n < (size_t)-1)
	    ++n;
	  if (n < 3)
	    return -2;
	  ch = (state->__count == 2) ? t[i++] : state->__value.__wchb[2];
	  if (ch < 0x80 || ch > 0xbf)
	    return -1;
	  state->__value.__wchb[2] = ch;
	  state->__count = 3;
	  if (n < 4)
	    return -2;
	  ch = t[i++];
	  if (ch < 0x80 || ch > 0xbf)
	    return -1;
	  tmp = (wint_t)((state->__value.__wchb[0] & 0x07) << 18)
	    |   (wint_t)((state->__value.__wchb[1] & 0x3f) << 12)
	    |   (wint_t)((state->__value.__wchb[2] & 0x3f) << 6)
	    |   (wint_t)(ch & 0x3f);
	  if (tmp > 0xffff && sizeof(wchar_t) == 2)
	    {
	      /* On systems which have wchar_t being UTF-16 values, the value
		 doesn't fit into a single wchar_t in this case.  So what we
		 do here is to store the state with a special value of __count
		 and return the first half of a surrogate pair.  As return
		 value we choose to return the half of the actual UTF-8 char.
		 The second half is returned in case we recognize the special
		 __count value above. */
	      state->__value.__wchb[3] = ch;
	      state->__count = 4;
	      *pwc = 0xd800 | (((tmp - 0x10000) >> 10) & 0x3ff);
	      return 2;
	    }
	  *pwc = tmp;
	  state->__count = 0;
	  return i;
	}
      else
	return -1;
    }      
  else if (!strcmp (__lc_ctype, "C-SJIS"))
    {
      int ch;
      int i = 0;
      if (s == NULL)
        return 0;  /* not state-dependent */
      ch = t[i++];
      if (state->__count == 0)
	{
	  if (_issjis1 (ch))
	    {
	      state->__value.__wchb[0] = ch;
	      state->__count = 1;
	      if (n <= 1)
		return -2;
	      ch = t[i++];
	    }
	}
      if (state->__count == 1)
	{
	  if (_issjis2 (ch))
	    {
	      *pwc = (((wchar_t)state->__value.__wchb[0]) << 8) + (wchar_t)ch;
	      state->__count = 0;
	      return i;
	    }
	  else  
	    return -1;
	}
    }
  else if (!strcmp (__lc_ctype, "C-EUCJP"))
    {
      int ch;
      int i = 0;
      if (s == NULL)
        return 0;  /* not state-dependent */
      ch = t[i++];
      if (state->__count == 0)
	{
	  if (_iseucjp (ch))
	    {
	      state->__value.__wchb[0] = ch;
	      state->__count = 1;
	      if (n <= 1)
		return -2;
	      ch = t[i++];
	    }
	}
      if (state->__count == 1)
	{
	  if (_iseucjp (ch))
	    {
	      *pwc = (((wchar_t)state->__value.__wchb[0]) << 8) + (wchar_t)ch;
	      state->__count = 0;
	      return i;
	    }
	  else
	    return -1;
	}
    }
  else if (!strcmp (__lc_ctype, "C-JIS"))
    {
      JIS_STATE curr_state;
      JIS_ACTION action;
      JIS_CHAR_TYPE ch;
      unsigned char *ptr;
      unsigned int i;
      int curr_ch;
 
      if (s == NULL)
        {
          state->__state = ASCII;
          return 1;  /* state-dependent */
        }

      curr_state = state->__state;
      ptr = t;

      for (i = 0; i < n; ++i)
        {
          curr_ch = t[i];
          switch (curr_ch)
            {
	    case ESC_CHAR:
              ch = ESCAPE;
              break;
	    case '$':
              ch = DOLLAR;
              break;
            case '@':
              ch = AT;
              break;
            case '(':
	      ch = BRACKET;
              break;
            case 'B':
              ch = B;
              break;
            case 'J':
              ch = J;
              break;
            case '\0':
              ch = NUL;
              break;
            default:
              if (_isjis (curr_ch))
                ch = JIS_CHAR;
              else
                ch = OTHER;
	    }

          action = JIS_action_table[curr_state][ch];
          curr_state = JIS_state_table[curr_state][ch];
        
          switch (action)
            {
            case NOOP:
              break;
            case EMPTY:
              state->__state = ASCII;
              *pwc = (wchar_t)0;
              return 0;
            case COPY_A:
	      state->__state = ASCII;
              *pwc = (wchar_t)*ptr;
              return (i + 1);
            case COPY_J1:
              state->__value.__wchb[0] = t[i];
	      break;
            case COPY_J2:
              state->__state = JIS;
              *pwc = (((wchar_t)state->__value.__wchb[0]) << 8) + (wchar_t)(t[i]);
              return (i + 1);
            case MAKE_A:
              ptr = (unsigned char *)(t + i + 1);
              break;
            case ERROR:
            default:
              return -1;
            }

        }

      state->__state = curr_state;
      return -2;  /* n < bytes needed */
    }
#endif /* _MB_CAPABLE */               

  /* otherwise this must be the "C" locale or unknown locale */
  if (s == NULL)
    return 0;  /* not state-dependent */

  *pwc = (wchar_t)*t;
  
  if (*t == '\0')
    return 0;

  return 1;
}
