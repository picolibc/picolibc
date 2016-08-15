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

/*
FUNCTION
	<<towlower>>, <<towlower_l>>---translate wide characters to lowercase

INDEX
	towlower

INDEX
	towlower_l

ANSI_SYNOPSIS
	#include <wctype.h>
	wint_t towlower(wint_t <[c]>);

	#include <wctype.h>
	wint_t towlower_l(wint_t <[c]>, locale_t <[locale]>);

TRAD_SYNOPSIS
	#include <wctype.h>
	wint_t towlower(<[c]>)
	wint_t <[c]>;


DESCRIPTION
<<towlower>> is a function which converts uppercase wide characters to
lowercase, leaving all other characters unchanged.

<<towlower_l>> is like <<towlower>> but performs the function based on the
locale specified by the locale object locale.  If <[locale]> is
LC_GLOBAL_LOCALE or not a valid locale object, the behaviour is undefined.

RETURNS
<<towlower>>, <<towlower_l>> return the lowercase equivalent of <[c]> when it is a
uppercase wide character; otherwise, it returns the input character.

PORTABILITY
<<towlower>> is C99.
<<towlower_l>> is POSIX-1.2008.

No supporting OS subroutines are required.
*/

#include <_ansi.h>
#include <newlib.h>
#include <string.h>
#include <reent.h>
#include <ctype.h>
#include <wctype.h>
#include "local.h"

wint_t
_DEFUN(towlower,(c), wint_t c)
{
#ifdef _MB_CAPABLE
  c = _jp2uc (c);
  /* Based on and tested against Unicode 5.2 */

  /* Expression used to filter out the characters for the below code:

     awk -F\; '{ if ( $14 != "" ) print $1; }' UnicodeData.txt
  */
  if (c < 0x100)
    {
      if ((c >= 0x0041 && c <= 0x005a) ||
	  (c >= 0x00c0 && c <= 0x00d6) ||
	  (c >= 0x00d8 && c <= 0x00de))
	return (c + 0x20);

      return c;
    }
  else if (c < 0x300)
    {
      if ((c >= 0x0100 && c <= 0x012e) ||
	  (c >= 0x0132 && c <= 0x0136) ||
	  (c >= 0x014a && c <= 0x0176) ||
	  (c >= 0x01de && c <= 0x01ee) ||
	  (c >= 0x01f8 && c <= 0x021e) ||
	  (c >= 0x0222 && c <= 0x0232))
	{
	  if (!(c & 0x01))
	    return (c + 1);
	  return c;
	}

      if (c == 0x0130)
	return 0x0069;

      if ((c >= 0x0139 && c <= 0x0147) ||
	  (c >= 0x01cd && c <= 0x01db))
	{
	  if (c & 0x01)
	    return (c + 1);
	  return c;
	}
      
      if (c >= 0x178 && c <= 0x01f7)
	{
	  wint_t k;
	  switch (c)
	    {
	    case 0x0178:
	      k = 0x00ff;
	      break;
	    case 0x0179:
	    case 0x017b:
	    case 0x017d:
	    case 0x0182:
	    case 0x0184:
	    case 0x0187:
	    case 0x018b:
	    case 0x0191:
	    case 0x0198:
	    case 0x01a0:
	    case 0x01a2:
	    case 0x01a4:
	    case 0x01a7:
	    case 0x01ac:
	    case 0x01af:
	    case 0x01b3:
	    case 0x01b5:
	    case 0x01b8:
	    case 0x01bc:
	    case 0x01c5:
	    case 0x01c8:
	    case 0x01cb:
	    case 0x01cd:
	    case 0x01cf:
	    case 0x01d1:
	    case 0x01d3:
	    case 0x01d5:
	    case 0x01d7:
	    case 0x01d9:
	    case 0x01db:
	    case 0x01f2:
	    case 0x01f4:
	      k = c + 1;
	      break;
	    case 0x0181:
	      k = 0x0253;
	      break;
	    case 0x0186:
	      k = 0x0254;
	      break;
	    case 0x0189:
	      k = 0x0256;
	      break;
	    case 0x018a:
	      k = 0x0257;
	      break;
	    case 0x018e:
	      k = 0x01dd;
	      break;
	    case 0x018f:
	      k = 0x0259;
	      break;
	    case 0x0190:
	      k = 0x025b;
	      break;
	    case 0x0193:
	      k = 0x0260;
	      break;
	    case 0x0194:
	      k = 0x0263;
	      break;
	    case 0x0196:
	      k = 0x0269;
	      break;
	    case 0x0197:
	      k = 0x0268;
	      break;
	    case 0x019c:
	      k = 0x026f;
	      break;
	    case 0x019d:
	      k = 0x0272;
	      break;
	    case 0x019f:
	      k = 0x0275;
	      break;
	    case 0x01a6:
	      k = 0x0280;
	      break;
	    case 0x01a9:
	      k = 0x0283;
	      break;
	    case 0x01ae:
	      k = 0x0288;
	      break;
	    case 0x01b1:
	      k = 0x028a;
	      break;
	    case 0x01b2:
	      k = 0x028b;
	      break;
	    case 0x01b7:
	      k = 0x0292;
	      break;
	    case 0x01c4:
	    case 0x01c7:
	    case 0x01ca:
	    case 0x01f1:
	      k = c + 2;
	      break;
	    case 0x01f6:
	      k = 0x0195;
	      break;
	    case 0x01f7:
	      k = 0x01bf;
	      break;
	    default:
	      k = 0;
	    }
	  if (k != 0)
	    return k;
	}
      else if (c == 0x0220)
      	return 0x019e;
      else if (c >= 0x023a && c <= 0x024e)
      	{
	  wint_t k;
	  switch (c)
	    {
	    case 0x023a:
	      k = 0x2c65;
	      break;
	    case 0x023b:
	    case 0x0241:
	    case 0x0246:
	    case 0x0248:
	    case 0x024a:
	    case 0x024c:
	    case 0x024e:
	      k = c + 1;
	      break;
	    case 0x023d:
	      k = 0x019a;
	      break;
	    case 0x023e:
	      k = 0x2c66;
	      break;
	    case 0x0243:
	      k = 0x0180;
	      break;
	    case 0x0244:
	      k = 0x0289;
	      break;
	    case 0x0245:
	      k = 0x028c;
	      break;
	    default:
	      k = 0;
	    }
	  if (k != 0)
	    return k;
	}
    }
  else if (c < 0x0400)
    {
      if (c == 0x0370 || c == 0x0372 || c == 0x0376)
      	return (c + 1);
      if (c >= 0x0391 && c <= 0x03ab && c != 0x03a2)
	return (c + 0x20);
      if (c >= 0x03d8 && c <= 0x03ee && !(c & 0x01))
	return (c + 1);
      if (c >= 0x0386 && c <= 0x03ff)
	{
	  wint_t k;
	  switch (c)
	    {
	    case 0x0386:
	      k = 0x03ac;
	      break;
	    case 0x0388:
	      k = 0x03ad;
	      break;
	    case 0x0389:
	      k = 0x03ae;
	      break;
	    case 0x038a:
	      k = 0x03af;
	      break;
	    case 0x038c:
	      k = 0x03cc;
	      break;
	    case 0x038e:
	      k = 0x03cd;
	      break;
	    case 0x038f:
	      k = 0x03ce;
	      break;
	    case 0x03cf:
	      k = 0x03d7;
	      break;
	    case 0x03f4:
	      k = 0x03b8;
	      break;
	    case 0x03f7:
	      k = 0x03f8;
	      break;
	    case 0x03f9:
	      k = 0x03f2;
	      break;
	    case 0x03fa:
	      k = 0x03fb;
	      break;
	    case 0x03fd:
	      k = 0x037b;
	      break;
	    case 0x03fe:
	      k = 0x037c;
	      break;
	    case 0x03ff:
	      k = 0x037d;
	      break;
	    default:
	      k = 0;
	    }
	  if (k != 0)
	    return k;
	}
    }
  else if (c < 0x500)
    {
      if (c >= 0x0400 && c <= 0x040f)
	return (c + 0x50);
      
      if (c >= 0x0410 && c <= 0x042f)
	return (c + 0x20);
      
      if ((c >= 0x0460 && c <= 0x0480) ||
	  (c >= 0x048a && c <= 0x04be) ||
	  (c >= 0x04d0 && c <= 0x04fe))
	{
	  if (!(c & 0x01))
	    return (c + 1);
	  return c;
	}
      
      if (c == 0x04c0)
	return 0x04cf;

      if (c >= 0x04c1 && c <= 0x04cd)
	{
	  if (c & 0x01)
	    return (c + 1);
	  return c;
	}
    }
  else if (c < 0x1f00)
    {
      if ((c >= 0x0500 && c <= 0x050e) ||
	  (c >= 0x0510 && c <= 0x0524) ||
	  (c >= 0x1e00 && c <= 0x1e94) ||
	  (c >= 0x1ea0 && c <= 0x1ef8))
	{
	  if (!(c & 0x01))
	    return (c + 1);
	  return c;
	}
      
      if (c >= 0x0531 && c <= 0x0556)
	return (c + 0x30);

      if (c >= 0x10a0 && c <= 0x10c5)
	return (c + 0x1c60);

      if (c == 0x1e9e)
	return 0x00df;

      if (c >= 0x1efa && c <= 0x1efe && !(c & 0x01))
	return (c + 1);
    }
  else if (c < 0x2000)
    {
      if ((c >= 0x1f08 && c <= 0x1f0f) ||
	  (c >= 0x1f18 && c <= 0x1f1d) ||
	  (c >= 0x1f28 && c <= 0x1f2f) ||
	  (c >= 0x1f38 && c <= 0x1f3f) ||
	  (c >= 0x1f48 && c <= 0x1f4d) ||
	  (c >= 0x1f68 && c <= 0x1f6f) ||
	  (c >= 0x1f88 && c <= 0x1f8f) ||
	  (c >= 0x1f98 && c <= 0x1f9f) ||
	  (c >= 0x1fa8 && c <= 0x1faf))
	return (c - 0x08);

      if (c >= 0x1f59 && c <= 0x1f5f)
	{
	  if (c & 0x01)
	    return (c - 0x08);
	  return c;
	}
    
      if (c >= 0x1fb8 && c <= 0x1ffc)
	{
	  wint_t k;
	  switch (c)
	    {
	    case 0x1fb8:
	    case 0x1fb9:
	    case 0x1fd8:
	    case 0x1fd9:
	    case 0x1fe8:
	    case 0x1fe9:
	      k = c - 0x08;
	      break;
	    case 0x1fba:
	    case 0x1fbb:
	      k = c - 0x4a;
	      break;
	    case 0x1fbc:
	      k = 0x1fb3;
	      break;
	    case 0x1fc8:
	    case 0x1fc9:
	    case 0x1fca:
	    case 0x1fcb:
	      k = c - 0x56;
	      break;
	    case 0x1fcc:
	      k = 0x1fc3;
	      break;
	    case 0x1fda:
	    case 0x1fdb:
	      k = c - 0x64;
	      break;
	    case 0x1fea:
	    case 0x1feb:
	      k = c - 0x70;
	      break;
	    case 0x1fec:
	      k = 0x1fe5;
	      break;
	    case 0x1ff8:
	    case 0x1ff9:
	      k = c - 0x80;
	      break;
	    case 0x1ffa:
	    case 0x1ffb:
	      k = c - 0x7e;
	      break;
	    case 0x1ffc:
	      k = 0x1ff3;
	      break;
	    default:
	      k = 0;
	    }
	  if (k != 0)
	    return k;
	}
    }
  else if (c < 0x2c00)
    {
      if (c >= 0x2160 && c <= 0x216f)
	return (c + 0x10);

      if (c >= 0x24b6 && c <= 0x24cf)
	return (c + 0x1a);
      
      switch (c)
      	{
	case 0x2126:
	  return 0x03c9;
	case 0x212a:
	  return 0x006b;
	case 0x212b:
	  return 0x00e5;
	case 0x2132:
	  return 0x214e;
	case 0x2183:
	  return 0x2184;
	}
    }
  else if (c < 0x2d00)
    {
      if (c >= 0x2c00 && c <= 0x2c2e)
	return (c + 0x30);

      if (c >= 0x2c80 && c <= 0x2ce2 && !(c & 0x01))
	return (c + 1);

      switch (c)
      	{
	case 0x2c60:
	  return 0x2c61;
	case 0x2c62:
	  return 0x026b;
	case 0x2c63:
	  return 0x1d7d;
	case 0x2c64:
	  return 0x027d;
	case 0x2c67:
	case 0x2c69:
	case 0x2c6b:
	case 0x2c72:
	case 0x2c75:
	case 0x2ceb:
	case 0x2ced:
	  return c + 1;
	case 0x2c6d:
	  return 0x0251;
	case 0x2c6e:
	  return 0x0271;
	case 0x2c6f:
	  return 0x0250;
	case 0x2c70:
	  return 0x0252;
	case 0x2c7e:
	  return 0x023f;
	case 0x2c7f:
	  return 0x0240;
	}
    }
  else if (c >= 0xa600 && c < 0xa800)
    {
      if ((c >= 0xa640 && c <= 0xa65e) ||
	  (c >= 0xa662 && c <= 0xa66c) ||
	  (c >= 0xa680 && c <= 0xa696) ||
	  (c >= 0xa722 && c <= 0xa72e) ||
	  (c >= 0xa732 && c <= 0xa76e) ||
	  (c >= 0xa77f && c <= 0xa786))
	{
	  if (!(c & 1))
	    return (c + 1);
	  return c;
	}

      switch (c)
      	{
	case 0xa779:
	case 0xa77b:
	case 0xa77e:
	case 0xa78b:
	  return (c + 1);
	case 0xa77d:
	  return 0x1d79;
	}
    }
  else
    {
      if (c >= 0xff21 && c <= 0xff3a)
	return (c + 0x20);
      
      if (c >= 0x10400 && c <= 0x10427)
	return (c + 0x28);
    }
  return c;
#else
  return (c < 0x00ff ? (wint_t)(tolower ((int)c)) : c);
#endif /* _MB_CAPABLE */
}

