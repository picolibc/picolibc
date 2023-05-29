/*
 * Copyright (c) 2009 Corinna Vinschen <corinna@vinschen.de>
 */
/* ctype table definitions for Windows codepage charsets.
   Included by ctype_.c. */

#define _CTYPE_CP437_128_254 \
	_U,	_L,	_L,	_L,	_L,	_L,	_L,	_L, \
	_L,	_L,	_L,	_L,	_L,	_L,	_U,	_U, \
	_U,	_L,	_U,	_L,	_L,	_L,	_L,	_L, \
	_L,	_U,	_U,	_P,	_P,	_P,	_P,	_P, \
	_L,	_L,	_L,	_L,	_L,	_L,	_P,	_P, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_L,	_L,	_U,	_L,	_U,	_L,	_P,	_L, \
	_U,	_U,	_U,	_L,	_P,	_L,	_L,	_P, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P
#define _CTYPE_CP437_255 _S|_B
#define _CTYPE_CP720_128_254 \
	0,	0,	_L,	_L,	0,	_L,	0,	_L, \
	_L,	_L,	_L,	_L,	_L,	0,	0,	0,  \
	0,	_P,	_P,	_L,	_P,	_P,	_L,	_L, \
	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_P,	_U|_L,	_U|_L,	_U|_L, \
	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L, \
	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_P,	_P, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_P,	_U|_L, \
	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P
#define _CTYPE_CP720_255 _S|_B
#define _CTYPE_CP737_128_254 \
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, \
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, \
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, \
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, \
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, \
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, \
	_L,	_L,	_U,	_U,	_U,	_U,	_U,	_U, \
	_U,	_P,	_P,	_P,	_U,	_U,	_P,	_P, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P
#define _CTYPE_CP737_255 _S|_B
#define _CTYPE_CP775_128_254 \
	_U,	_L,	_L,	_L,	_L,	_L,	_L,	_L, \
	_L,	_L,	_U,	_L,	_L,	_U,	_U,	_U, \
	_U,	_L,	_U,	_L,	_L,	_U,	_P,	_U, \
	_L,	_U,	_U,	_L,	_P,	_U,	_P,	_P, \
	_U,	_U,	_L,	_U,	_L,	_L,	_P,	_P, \
	_P,	_P,	_P,	_P,	_P,	_U,	_P,	_P, \
	_P,	_P,	_P,	_P,	_P,	_U,	_U,	_U, \
	_U,	_P,	_P,	_P,	_P,	_U,	_U,	_P, \
	_P,	_P,	_P,	_P,	_P,	_P,	_U,	_U, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_U, \
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, \
	_L,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_U,	_L,	_U,	_U,	_L,	_U,	_P,	_L, \
	_U,	_L,	_U,	_L,	_L,	_U,	_U,	_P, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P
#define _CTYPE_CP775_255 _S|_B
#define _CTYPE_CP850_128_254 \
	_U,	_L,	_L,	_L,	_L,	_L,	_L,	_L, \
	_L,	_L,	_L,	_L,	_L,	_L,	_U,	_U, \
	_U,	_L,	_U,	_L,	_L,	_L,	_L,	_L, \
	_L,	_U,	_U,	_L,	_P,	_U,	_P,	_L, \
	_L,	_L,	_L,	_L,	_L,	_U,	_P,	_P, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_P,	_P,	_P,	_P,	_P,	_U,	_U,	_U, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_P,	_P,	_P,	_P,	_P,	_P,	_L,	_U, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_L,	_U,	_U,	_U,	_U,	_L,	_U,	_U, \
	_U,	_P,	_P,	_P,	_P,	_P,	_U,	_P, \
	_U,	_L,	_U,	_U,	_L,	_U,	_P,	_L, \
	_U,	_U,	_U,	_U,	_L,	_U,	_P,	_P, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P
#define _CTYPE_CP850_255 _S|_B
#define _CTYPE_CP852_128_254 \
	_U,	_L,	_L,	_L,	_L,	_L,	_L,	_L, \
	_L,	_L,	_U,	_L,	_L,	_U,	_U,	_U, \
	_U,	_U,	_L,	_L,	_L,	_U,	_L,	_U, \
	_L,	_U,	_U,	_U,	_L,	_U,	_P,	_L, \
	_L,	_L,	_L,	_L,	_U,	_L,	_U,	_L, \
	_U,	_L,	_P,	_L,	_U,	_L,	_P,	_P, \
	_P,	_P,	_P,	_P,	_P,	_U,	_U,	_U, \
	_U,	_P,	_P,	_P,	_P,	_U,	_L,	_P, \
	_P,	_P,	_P,	_P,	_P,	_P,	_U,	_L, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_L,	_U,	_U,	_U,	_L,	_U,	_U,	_U, \
	_L,	_P,	_P,	_P,	_P,	_U,	_U,	_P, \
	_U,	_L,	_U,	_U,	_L,	_L,	_U,	_L, \
	_U,	_U,	_L,	_U,	_L,	_U,	_L,	_P, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_P,	_P,	_P,	_L,	_U,	_L,	_P
#define _CTYPE_CP852_255 _S|_B
#define _CTYPE_CP855_128_254 \
	_L,	_U,	_L,	_U,	_L,	_U,	_L,	_U, \
	_L,	_U,	_L,	_U,	_L,	_U,	_L,	_U, \
	_L,	_U,	_L,	_U,	_L,	_U,	_L,	_U, \
	_L,	_U,	_L,	_U,	_L,	_U,	_L,	_U, \
	_L,	_U,	_L,	_U,	_L,	_U,	_L,	_U, \
	_L,	_U,	_L,	_U,	_L,	_U,	_P,	_P, \
	_P,	_P,	_P,	_P,	_P,	_L,	_U,	_L, \
	_U,	_P,	_P,	_P,	_P,	_L,	_U,	_P, \
	_P,	_P,	_P,	_P,	_P,	_P,	_L,	_U, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_L,	_U,	_L,	_U,	_L,	_U,	_L,	_U, \
	_L,	_P,	_P,	_P,	_P,	_U,	_L,	_P, \
	_U,	_L,	_U,	_L,	_U,	_L,	_U,	_L, \
	_U,	_L,	_U,	_L,	_U,	_L,	_U,	_P, \
	_P,	_L,	_U,	_L,	_U,	_L,	_U,	_L, \
	_U,	_L,	_U,	_L,	_U,	_P,	_P
#define _CTYPE_CP855_255 _S|_B
#define _CTYPE_CP857_128_254 \
	_U,	_L,	_L,	_L,	_L,	_L,	_L,	_L, \
	_L,	_L,	_L,	_L,	_L,	_L,	_U,	_U, \
	_U,	_L,	_U,	_L,	_L,	_L,	_L,	_L, \
	_U,	_U,	_U,	_L,	_P,	_U,	_U,	_L, \
	_L,	_L,	_L,	_L,	_L,	_U,	_U,	_L, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_P,	_P,	_P,	_P,	_U,	_U,	_U,	_P, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_P,	_P,	_P,	_P,	_P,	_P,	_L,	_U, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_P,	_P,	_U,	_U,	_U,	0,	_U,	_U, \
	_U,	_P,	_P,	_P,	_P,	_P,	_U,	_P, \
	_U,	_L,	_U,	_U,	_L,	_U,	_P,	0, \
	_P,	_U,	_U,	_U,	_L,	_L,	_P,	_P, \
	_P,	_P,	0,	_P,	_P,	_P,	_P,	_P, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P
#define _CTYPE_CP857_255 _S|_B
#define _CTYPE_CP858_128_254 \
	_U,	_L,	_L,	_L,	_L,	_L,	_L,	_L, \
	_L,	_L,	_L,	_L,	_L,	_L,	_U,	_U, \
	_U,	_L,	_U,	_L,	_L,	_L,	_L,	_L, \
	_L,	_U,	_U,	_L,	_P,	_U,	_P,	_L, \
	_L,	_L,	_L,	_L,	_L,	_U,	_P,	_P, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_P,	_P,	_P,	_P,	_P,	_U,	_U,	_U, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_P,	_P,	_P,	_P,	_P,	_P,	_L,	_U, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_L,	_U,	_U,	_U,	_U,	_P,	_U,	_U, \
	_U,	_P,	_P,	_P,	_P,	_P,	_U,	_P, \
	_U,	_L,	_U,	_U,	_L,	_U,	_P,	_L, \
	_U,	_U,	_U,	_U,	_L,	_U,	_P,	_P, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P
#define _CTYPE_CP858_255 _S|_B
#define _CTYPE_CP862_128_254 \
	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L, \
	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L, \
	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L, \
	_U|_L,	_U|_L,	_U|_L,	_P,	_P,	_P,	_P,	_L, \
	_L,	_L,	_L,	_L,	_L,	_U,	_P,	_P, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_L,	_L,	_U,	_L,	_U,	_L,	_P,	_L, \
	_U,	_U,	_U,	_L,	_P,	_L,	_L,	_P, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P
#define _CTYPE_CP862_255 _S|_B
#define _CTYPE_CP866_128_254 \
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, \
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, \
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, \
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, \
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, \
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, \
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, \
	_U,	_L,	_U,	_L,	_U,	_L,	_U,	_L, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P
#define _CTYPE_CP866_255 _S|_B
#define _CTYPE_CP874_128_254 \
	_P,	0,	0,	0,	0,	_P,	0,	0,  \
	0,	0,	0,	0,	0,	0,	0,	0,  \
	0,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	0,	0,	0,	0,	0,	0,	0,	0,  \
	_S|_B,	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L, \
	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L, \
	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L, \
	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L, \
	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L, \
	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L, \
	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L, \
	_U|_L,	_U|_L,	_U|_L,	0,	0,	0,	0,	_P, \
	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L, \
	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_P,	_P,	_U|_L,	_U|_L,	0,	0,	0
#define _CTYPE_CP874_255 0
#define _CTYPE_CP1125_128_254 \
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, \
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, \
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, \
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, \
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, \
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, \
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, \
	_U,	_L,	_U,	_L,	_U,	_L,	_U,	_L, \
	_U,	_L,	_P,	_P,	_P,	_P,	_P
#define _CTYPE_CP1125_255 _S|_B
#define _CTYPE_CP1250_128_254 \
	_P,	0,	_P,	0,	_P,	_P,	_P,	_P, \
	0,	_P,	_U,	_P,	_U,	_U,	_U,	_U, \
	0,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	0,	_P,	_L,	_P,	_L,	_L,	_L,	_L, \
	_S|_B,	_P,	_P,	_U,	_P,	_U,	_P,	_P, \
	_P,	_P,	_U,	_P,	_P,	_P,	_P,	_U, \
	_P,	_P,	_P,	_L,	_P,	_P,	_P,	_P, \
	_P,	_L,	_L,	_P,	_U,	_P,	_L,	_L, \
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, \
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, \
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_P, \
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_L, \
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, \
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, \
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_P, \
	_L,	_L,	_L,	_L,	_L,	_L,	_L
#define _CTYPE_CP1250_255 _P
#define _CTYPE_CP1251_128_254 \
	_U,	_U,	_P,	_L,	_P,	_P,	_P,	_P, \
	_P,	_P,	_U,	_P,	_U,	_U,	_U,	_U, \
	_L,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	0,	_P,	_L,	_P,	_L,	_L,	_L,	_L, \
	_S|_B,	_U,	_L,	_U,	_P,	_U,	_P,	_P, \
	_U,	_P,	_U,	_P,	_P,	_P,	_P,	_U, \
	_P,	_P,	_U,	_L,	_L,	_P,	_P,	_P, \
	_L,	_P,	_L,	_P,	_L,	_U,	_L,	_L, \
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, \
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, \
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, \
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, \
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, \
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, \
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, \
	_L,	_L,	_L,	_L,	_L,	_L,	_L
#define _CTYPE_CP1251_255 _L
#define _CTYPE_CP1252_128_254 \
	_P,	0,	_P,	_L,	_P,	_P,	_P,	_P, \
	_P,	_P,	_U,	_P,	_U,	_U,	0,	0,  \
	0,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_P,	_P,	_L,	_P,	_L,	0,	_L,	_U, \
	_S|_B,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, \
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, \
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_P, \
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_L, \
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, \
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, \
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_P, \
	_L,	_L,	_L,	_L,	_L,	_L,	_L
#define _CTYPE_CP1252_255 _L
#define _CTYPE_CP1253_128_254 \
	_P,	0,	_P,	_L,	_P,	_P,	_P,	_P, \
	0,	_P,	0,	_P,	0,	0,	0,	0,  \
	0,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	0,	_P,	0,	_P,	0,	0,	0,	0,  \
	_S|_B,	_P,	_U,	_P,	_P,	_P,	_P,	_P, \
	_P,	_P,	0,	_P,	_P,	_P,	_P,	_P, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_U,	_U,	_U,	_P,	_U,	_P,	_U,	_U, \
	_L,	_U,	_U,	_U,	_U,	_U,	_U,	_U, \
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, \
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, \
	_U,	_U,	_U,	_U,	_L,	_L,	_L,	_L, \
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, \
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, \
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, \
	_L,	_L,	_L,	_L,	_L,	_L,	_L
#define _CTYPE_CP1253_255 _L
#define _CTYPE_CP1254_128_254 \
	_P,	0,	_P,	_L,	_P,	_P,	_P,	_P, \
	_P,	_P,	_U,	_P,	_U,	0,	0,	0,  \
	0,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_P,	_P,	_L,	_P,	_L,	0,	0,	_U, \
	_S|_B,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, \
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, \
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_P, \
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_L, \
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, \
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, \
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_P, \
	_L,	_L,	_L,	_L,	_L,	_L,	_L
#define _CTYPE_CP1254_255 _L
#define _CTYPE_CP1255_128_254 \
	_P,	0,	_P,	_L,	_P,	_P,	_P,	_P, \
	_P,	_P,	0,	_P,	0,	0,	0,	0,  \
	0,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_P,	_P,	0,	_P,	0,	0,	0,	0,  \
	_S|_B,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_P,	_P,	_P,	_P,	_U|_L,	_U|_L,	_U|_L,	_P, \
	_P,	0,	0,	0,	0,	0,	0,	0,  \
	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L, \
	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L, \
	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L, \
	_U|_L,	_U|_L,	_U|_L,	0,	0,	_P,	_P
#define _CTYPE_CP1255_255 0
#define _CTYPE_CP1256_128_254 \
	_P,	_U|_L,	_P,	_L,	_P,	_P,	_P,	_P, \
	_P,	_P,	_U|_L,	_P,	_U,	_U|_L,	_U|_L,	_U|_L, \
	_U|_L,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_U|_L,	_P,	_U|_L,	_P,	_L,	_P,	_P,	_U|_L, \
	_S|_B,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_P,	_P,	_U|_L,	_P,	_P,	_P,	_P,	_P, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L, \
	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L, \
	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_P, \
	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_P,	_U|_L,	_U|_L,	_U|_L, \
	_L,	_U|_L,	_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_L, \
	_L,	_L,	_L,	_L,	_U|_L,	_U|_L,	_L,	_L, \
	_P,	_P,	_P,	_P,	_L,	_P,	_P,	_P, \
	_P,	_L,	_P,	_L,	_L,	_P,	_P
#define _CTYPE_CP1256_255 _U|_L
#define _CTYPE_CP1257_128_254 \
	_P,	0,	_P,	0,	_P,	_P,	_P,	_P, \
	0,	_P,	0,	_P,	0,	_P,	_P,	_P, \
	0,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	0,	_P,	0,	_P,	0,	_P,	_P,	0,  \
	_S|_B,	0,	_P,	_P,	_P,	0,	_P,	_P, \
	_U,	_P,	_U,	_P,	_P,	_P,	_P,	_U, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_L,	_P,	_L,	_P,	_P,	_P,	_P,	_L, \
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, \
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, \
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_P, \
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_L, \
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, \
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, \
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_P, \
	_L,	_L,	_L,	_L,	_L,	_L,	_L
#define _CTYPE_CP1257_255 _P
#define _CTYPE_CP1258_128_254 \
	_P,	0,	_P,	_L,	_P,	_P,	_P,	_P, \
	_P,	_P,	0,	_P,	_U,	0,	0,	0,  \
	0,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_P,	_P,	0,	_P,	_L,	0,	0,	_U, \
	_S|_B,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, \
	_U,	_U,	_U,	_U,	_P,	_U,	_U,	_U, \
	_U,	_U,	_P,	_U,	_U,	_U,	_U,	_P, \
	_U,	_U,	_U,	_U,	_U,	_U,	_P,	_L, \
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, \
	_L,	_L,	_L,	_L,	_P,	_L,	_L,	_L, \
	_L,	_L,	_P,	_L,	_L,	_L,	_L,	_P, \
	_L,	_L,	_L,	_L,	_L,	_L,	_P
#define _CTYPE_CP1258_255 _L
#define _CTYPE_CP20866_128_254 \
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_P,	_P,	_S|_B,	_P,	_P,	_P,	_P,	_P, \
	_P,	_P,	_P,	_L,	_P,	_P,	_P,	_P, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_P,	_P,	_P,	_U,	_P,	_P,	_P,	_P, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, \
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, \
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, \
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, \
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, \
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, \
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, \
	_U,	_U,	_U,	_U,	_U,	_U,	_U
#define _CTYPE_CP20866_255 _U
#define _CTYPE_CP21866_128_254 \
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_P,	_P,	_S|_B,	_P,	_P,	_P,	_P,	_P, \
	_P,	_P,	_P,	_L,	_L,	_P,	_L,	_L, \
	_P,	_P,	_P,	_P,	_P,	_L,	_P,	_P, \
	_P,	_P,	_P,	_U,	_U,	_P,	_U,	_U, \
	_P,	_P,	_P,	_P,	_P,	_U,	_P,	_P, \
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, \
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, \
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, \
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, \
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, \
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, \
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, \
	_U,	_U,	_U,	_U,	_U,	_U,	_U
#define _CTYPE_CP21866_255 _U
#define _CTYPE_GEORGIAN_PS_128_254 \
	_P,	0,	_P,	_L,	_P,	_P,	_P,	_P, \
	_P,	_P,	_U,	_P,	_U,	_U,	0,	0,  \
	0,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_P,	_P,	_L,	_P,	_L,	0,	_L,	_U, \
	_S|_B,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L, \
	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L, \
	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L, \
	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L, \
	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_U|_L,	_L,	_L, \
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, \
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_P, \
	_L,	_L,	_L,	_L,	_L,	_L,	_L
#define _CTYPE_GEORGIAN_PS_255 _L
#define _CTYPE_PT154_128_254 \
	_U,	_U,	_U,	_L,	_P,	_P,	_U,	_U, \
	_U,	_L,	_U,	_U,	_U,	_U,	_U,	_U, \
	_L,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, \
	_S|_B,	_U,	_L,	_U,	_U,	_U,	_U,	_P, \
	_U,	_P,	_U,	_P,	_P,	_L,	_P,	_U, \
	_P,	_L,	_U,	_L,	_L,	_L,	_P,	_P, \
	_L,	_P,	_L,	_P,	_L,	_U,	_L,	_L, \
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, \
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, \
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, \
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, \
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, \
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, \
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, \
	_L,	_L,	_L,	_L,	_L,	_L,	_L
#define _CTYPE_PT154_255 _L
#define _CTYPE_CP103_128_254 \
	_L,	_L,	_P,	_U,	_P,	_P,	_P,	_P, \
	0,	_P,	_L,	_P,	_U,	_L,	_U,	0,  \
	_U,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	0,	_P,	0,	_P,	0,	0,	0,	0,  \
	0,	_L,	_U,	_L,	_P,	_L,	_P,	_P, \
	0,	0,	0,	_P,	_P,	_P,	_P,	0,  \
	_P,	_P,	_P,	_U,	0,	_U,	_P,	_P, \
	0,	_P,	0,	_P,	0,	0,	0,	_P, \
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, \
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, \
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, \
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, \
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, \
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, \
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, \
	_U,	_U,	_U,	_U,	_U,	_U,	_U
#define _CTYPE_CP103_255 _U

#if defined(ALLOW_NEGATIVE_CTYPE_INDEX)

#ifndef __CYGWIN__
static const
#endif
char __ctype_cp[27][128 + 256] = {
  { _CTYPE_CP437_128_254,
    0,
    _CTYPE_DATA_0_127,
    _CTYPE_CP437_128_254,
    _CTYPE_CP437_255
  },
  { _CTYPE_CP720_128_254,
    0,
    _CTYPE_DATA_0_127,
    _CTYPE_CP720_128_254,
    _CTYPE_CP720_255
  },
  { _CTYPE_CP737_128_254,
    0,
    _CTYPE_DATA_0_127,
    _CTYPE_CP737_128_254,
    _CTYPE_CP737_255
  },
  { _CTYPE_CP775_128_254,
    0,
    _CTYPE_DATA_0_127,
    _CTYPE_CP775_128_254,
    _CTYPE_CP775_255
  },
  { _CTYPE_CP850_128_254,
    0,
    _CTYPE_DATA_0_127,
    _CTYPE_CP850_128_254,
    _CTYPE_CP850_255
  },
  { _CTYPE_CP852_128_254,
    0,
    _CTYPE_DATA_0_127,
    _CTYPE_CP852_128_254,
    _CTYPE_CP852_255
  },
  { _CTYPE_CP855_128_254,
    0,
    _CTYPE_DATA_0_127,
    _CTYPE_CP855_128_254,
    _CTYPE_CP855_255
  },
  { _CTYPE_CP857_128_254,
    0,
    _CTYPE_DATA_0_127,
    _CTYPE_CP857_128_254,
    _CTYPE_CP857_255
  },
  { _CTYPE_CP858_128_254,
    0,
    _CTYPE_DATA_0_127,
    _CTYPE_CP858_128_254,
    _CTYPE_CP858_255
  },
  { _CTYPE_CP862_128_254,
    0,
    _CTYPE_DATA_0_127,
    _CTYPE_CP862_128_254,
    _CTYPE_CP862_255
  },
  { _CTYPE_CP866_128_254,
    0,
    _CTYPE_DATA_0_127,
    _CTYPE_CP866_128_254,
    _CTYPE_CP866_255
  },
  { _CTYPE_CP874_128_254,
    0,
    _CTYPE_DATA_0_127,
    _CTYPE_CP874_128_254,
    _CTYPE_CP874_255
  },
  { _CTYPE_CP1125_128_254,
    0,
    _CTYPE_DATA_0_127,
    _CTYPE_CP1125_128_254,
    _CTYPE_CP1125_255
  },
  { _CTYPE_CP1250_128_254,
    0,
    _CTYPE_DATA_0_127,
    _CTYPE_CP1250_128_254,
    _CTYPE_CP1250_255
  },
  { _CTYPE_CP1251_128_254,
    0,
    _CTYPE_DATA_0_127,
    _CTYPE_CP1251_128_254,
    _CTYPE_CP1251_255
  },
  { _CTYPE_CP1252_128_254,
    0,
    _CTYPE_DATA_0_127,
    _CTYPE_CP1252_128_254,
    _CTYPE_CP1252_255
  },
  { _CTYPE_CP1253_128_254,
    0,
    _CTYPE_DATA_0_127,
    _CTYPE_CP1253_128_254,
    _CTYPE_CP1253_255
  },
  { _CTYPE_CP1254_128_254,
    0,
    _CTYPE_DATA_0_127,
    _CTYPE_CP1254_128_254,
    _CTYPE_CP1254_255
  },
  { _CTYPE_CP1255_128_254,
    0,
    _CTYPE_DATA_0_127,
    _CTYPE_CP1255_128_254,
    _CTYPE_CP1255_255
  },
  { _CTYPE_CP1256_128_254,
    0,
    _CTYPE_DATA_0_127,
    _CTYPE_CP1256_128_254,
    _CTYPE_CP1256_255
  },
  { _CTYPE_CP1257_128_254,
    0,
    _CTYPE_DATA_0_127,
    _CTYPE_CP1257_128_254,
    _CTYPE_CP1257_255
  },
  { _CTYPE_CP1258_128_254,
    0,
    _CTYPE_DATA_0_127,
    _CTYPE_CP1258_128_254,
    _CTYPE_CP1258_255
  },
  { _CTYPE_CP20866_128_254,
    0,
    _CTYPE_DATA_0_127,
    _CTYPE_CP20866_128_254,
    _CTYPE_CP20866_255
  },
  { _CTYPE_CP21866_128_254,
    0,
    _CTYPE_DATA_0_127,
    _CTYPE_CP21866_128_254,
    _CTYPE_CP21866_255
  },
  { _CTYPE_GEORGIAN_PS_128_254,
    0,
    _CTYPE_DATA_0_127,
    _CTYPE_GEORGIAN_PS_128_254,
    _CTYPE_GEORGIAN_PS_255
  },
  { _CTYPE_PT154_128_254,
    0,
    _CTYPE_DATA_0_127,
    _CTYPE_PT154_128_254,
    _CTYPE_PT154_255
  },
  { _CTYPE_CP103_128_254,
    0,
    _CTYPE_DATA_0_127,
    _CTYPE_CP103_128_254,
    _CTYPE_CP103_255
  },
};

#else /* !defined(ALLOW_NEGATIVE_CTYPE_INDEX) */

static const char __ctype_cp[27][1 + 256] = {
  { 0,
    _CTYPE_DATA_0_127,
    _CTYPE_CP437_128_254,
    _CTYPE_CP437_255
  },
  { 0,
    _CTYPE_DATA_0_127,
    _CTYPE_CP720_128_254,
    _CTYPE_CP720_255
  },
  { 0,
    _CTYPE_DATA_0_127,
    _CTYPE_CP737_128_254,
    _CTYPE_CP737_255
  },
  { 0,
    _CTYPE_DATA_0_127,
    _CTYPE_CP775_128_254,
    _CTYPE_CP775_255
  },
  { 0,
    _CTYPE_DATA_0_127,
    _CTYPE_CP850_128_254,
    _CTYPE_CP850_255
  },
  { 0,
    _CTYPE_DATA_0_127,
    _CTYPE_CP852_128_254,
    _CTYPE_CP852_255
  },
  { 0,
    _CTYPE_DATA_0_127,
    _CTYPE_CP855_128_254,
    _CTYPE_CP855_255
  },
  { 0,
    _CTYPE_DATA_0_127,
    _CTYPE_CP857_128_254,
    _CTYPE_CP857_255
  },
  { 0,
    _CTYPE_DATA_0_127,
    _CTYPE_CP858_128_254,
    _CTYPE_CP858_255
  },
  { 0,
    _CTYPE_DATA_0_127,
    _CTYPE_CP862_128_254,
    _CTYPE_CP862_255
  },
  { 0,
    _CTYPE_DATA_0_127,
    _CTYPE_CP866_128_254,
    _CTYPE_CP866_255
  },
  { 0,
    _CTYPE_DATA_0_127,
    _CTYPE_CP874_128_254,
    _CTYPE_CP874_255
  },
  { 0,
    _CTYPE_DATA_0_127,
    _CTYPE_CP1125_128_254,
    _CTYPE_CP1125_255
  },
  { 0,
    _CTYPE_DATA_0_127,
    _CTYPE_CP1250_128_254,
    _CTYPE_CP1250_255
  },
  { 0,
    _CTYPE_DATA_0_127,
    _CTYPE_CP1251_128_254,
    _CTYPE_CP1251_255
  },
  { 0,
    _CTYPE_DATA_0_127,
    _CTYPE_CP1252_128_254,
    _CTYPE_CP1252_255
  },
  { 0,
    _CTYPE_DATA_0_127,
    _CTYPE_CP1253_128_254,
    _CTYPE_CP1253_255
  },
  { 0,
    _CTYPE_DATA_0_127,
    _CTYPE_CP1254_128_254,
    _CTYPE_CP1254_255
  },
  { 0,
    _CTYPE_DATA_0_127,
    _CTYPE_CP1255_128_254,
    _CTYPE_CP1255_255
  },
  { 0,
    _CTYPE_DATA_0_127,
    _CTYPE_CP1256_128_254,
    _CTYPE_CP1256_255
  },
  { 0,
    _CTYPE_DATA_0_127,
    _CTYPE_CP1257_128_254,
    _CTYPE_CP1257_255
  },
  { 0,
    _CTYPE_DATA_0_127,
    _CTYPE_CP1258_128_254,
    _CTYPE_CP1258_255
  },
  { 0,
    _CTYPE_DATA_0_127,
    _CTYPE_CP20866_128_254,
    _CTYPE_CP20866_255
  },
  { 0,
    _CTYPE_DATA_0_127,
    _CTYPE_CP21866_128_254,
    _CTYPE_CP21866_255
  },
  { 0,
    _CTYPE_DATA_0_127,
    _CTYPE_GEORGIAN_PS_128_254,
    _CTYPE_GEORGIAN_PS_255
  },
  { 0,
    _CTYPE_DATA_0_127,
    _CTYPE_PT154_128_254,
    _CTYPE_PT154_255
  },
  { 0,
    _CTYPE_DATA_0_127,
    _CTYPE_CP103_128_254,
    _CTYPE_CP103_255
  },
};

#endif /* ALLOW_NEGATIVE_CTYPE_INDEX */
