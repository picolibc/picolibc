#include "mb_wc_common.h"
#include <wchar.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>


static int __MINGW_ATTRIB_NONNULL(1)
 __wcrtomb_cp (char *dst, wchar_t wc, const unsigned int cp,
	       const unsigned int mb_max)
{       
 if (wc > 255)
    {
      errno = EILSEQ;
      return -1;
    }

  if (cp == 0)
    {
      *dst = (char) wc;
      return 1;
    }
  else
    {
      int invalid_char = 0;
   
      int size = WideCharToMultiByte(get_cp_from_locale(),
				     0 /* Is this correct flag? */,
				     &wc, 1, dst, mb_max,
				     NULL, &invalid_char);
      if (size == 0 || invalid_char)  
        {
          errno = EILSEQ;
          return -1;
        }
      return size;
    }
}

size_t
wcrtomb (char *dst, wchar_t wc, mbstate_t * __UNUSED_PARAM (ps))
{
  char byte_bucket [MB_LEN_MAX];
  char* tmp_dst = dst ? dst : byte_bucket;      
  return (size_t)__wcrtomb_cp (tmp_dst, wc, get_cp_from_locale (),
			       MB_CUR_MAX);
}

size_t wcsrtombs (char *dst, const wchar_t **src, size_t len,
		  mbstate_t * __UNUSED_PARAM (ps))
{
  int ret = 0;
  size_t n = 0;
  const unsigned int cp = get_cp_from_locale();
  const unsigned int mb_max = MB_CUR_MAX;

  if (src == NULL || *src == NULL) /* undefined behavior */
     return 0;

  if (dst != NULL)
    {
      const wchar_t ** saved_src = src;
      while (n < len)
        {
          if ((ret = __wcrtomb_cp (dst, **src, cp, mb_max)) <= 0)
	     return (size_t) -1;
  	  n += ret;
   	  dst += ret;
          if (*(dst - 1) == '\0')
	    {
	      *saved_src = (wchar_t) NULL;;
	      return (n  - 1);
	    }
	  *src++;
        }
    }
  else
    {
      char byte_bucket [MB_LEN_MAX];
      while (n < len)
        {
	  if ((ret = __wcrtomb_cp (byte_bucket, **src, cp, mb_max))
		 <= 0)
 	    return (size_t) -1;    
	  n += ret;
	  if (byte_bucket [ret - 1] == '\0')
	    return (n - 1);
          *src++;
        }
    }
  return n;
}
