/* ctype table definitions for ISO-8859-x charsets. 
   Included by ctype_.c. */

#define _CTYPE_ISO_8859_1_128_256 \
        _C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, \
        _C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, \
        _C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, \
        _C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, \
        _S|_B,  _P,     _P,     _P,     _P,     _P,     _P,     _P, \
        _P,     _P,     _P,     _P,     _P,     _P,     _P,     _P, \
        _P,     _P,     _P,     _P,     _P,     _P,     _P,     _P, \
        _P,     _P,     _P,     _P,     _P,     _P,     _P,     _P, \
        _U,     _U,     _U,     _U,     _U,     _U,     _U,     _U, \
        _U,     _U,     _U,     _U,     _U,     _U,     _U,     _U, \
        _U,     _U,     _U,     _U,     _U,     _U,     _U,     _P, \
        _U,     _U,     _U,     _U,     _U,     _U,     _U,     _L, \
        _L,     _L,     _L,     _L,     _L,     _L,     _L,     _L, \
        _L,     _L,     _L,     _L,     _L,     _L,     _L,     _L, \
        _L,     _L,     _L,     _L,     _L,     _L,     _L,     _P, \
        _L,     _L,     _L,     _L,     _L,     _L,     _L,     _L
#define _CTYPE_ISO_8859_2_128_256 \
   	_C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, \
        _C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, \
        _C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, \
        _C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, \
	_S|_B,	_U,	_P,	_U,	_P,	_U,	_U,	_P, \
	_P,	_U,	_U,	_U,	_U,	_P,	_U,	_U, \
	_P,	_L,	_P,	_L,	_P,	_L,	_L,	_P, \
	_P,	_L,	_L,	_L,	_L,	_P,	_L,	_L, \
        _U,     _U,     _U,     _U,     _U,     _U,     _U,     _U, \
        _U,     _U,     _U,     _U,     _U,     _U,     _U,     _U, \
        _U,     _U,     _U,     _U,     _U,     _U,     _U,     _P, \
        _U,     _U,     _U,     _U,     _U,     _U,     _U,     _L, \
        _L,     _L,     _L,     _L,     _L,     _L,     _L,     _L, \
        _L,     _L,     _L,     _L,     _L,     _L,     _L,     _L, \
        _L,     _L,     _L,     _L,     _L,     _L,     _L,     _P, \
        _L,     _L,     _L,     _L,     _L,     _L,     _L,     _L
#define _CTYPE_ISO_8859_3_128_256 \
   	_C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, \
        _C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, \
        _C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, \
        _C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, \
	_S|_B,	_U,	_P,	_P,	_P,	0,	_U,	_P, \
	_P,	_U,	_U,	_U,	_U,	_P,	0,	_U, \
	_P,	_L,	_P,	_P,	_P,	_L,	_L,	_P, \
	_P,	_L,	_L,	_L,	_L,	_P,	0,	_L, \
	_U,	_U,	_U,	0,	_U,	_U,	_U,	_U, \
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, \
	0,	_U,	_U,	_U,	_U,	_U,	_U,	_P, \
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_L, \
	_L,	_L,	_L,	0,	_L,	_L,	_L,	_L, \
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, \
	0,	_L,	_L,	_L,	_L,	_L,	_L,	_P, \
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_P
#define _CTYPE_ISO_8859_4_128_256 \
   	_C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, \
        _C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, \
        _C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, \
        _C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, \
	_S|_B,	_U,	_L,	_U,	_P,	_U,	_U,	_P, \
	_P,	_U,	_U,	_U,	_U,	_P,	_U,	_P, \
	_P,	_L,	_P,	_L,	_P,	_L,	_L,	_P, \
	_P,	_L,	_L,	_L,	_L,	_P,	_L,	_L, \
        _U,     _U,     _U,     _U,     _U,     _U,     _U,     _U, \
        _U,     _U,     _U,     _U,     _U,     _U,     _U,     _U, \
        _U,     _U,     _U,     _U,     _U,     _U,     _U,     _P, \
        _U,     _U,     _U,     _U,     _U,     _U,     _U,     _L, \
        _L,     _L,     _L,     _L,     _L,     _L,     _L,     _L, \
        _L,     _L,     _L,     _L,     _L,     _L,     _L,     _L, \
        _L,     _L,     _L,     _L,     _L,     _L,     _L,     _P, \
        _L,     _L,     _L,     _L,     _L,     _L,     _L,     _L
#define _CTYPE_ISO_8859_5_128_256 \
   	_C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, \
        _C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, \
        _C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, \
        _C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, \
	_S|_B,	_U,	_U,	_U,	_U,	_U,	_U,	_U, \
	_U,	_U,	_U,	_U,	_U,	_P,	_U,	_U, \
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, \
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, \
        _U,     _U,     _U,     _U,     _U,     _U,     _U,     _U, \
        _U,     _U,     _U,     _U,     _U,     _U,     _U,     _U, \
        _L,     _L,     _L,     _L,     _L,     _L,     _L,     _L, \
        _L,     _L,     _L,     _L,     _L,     _L,     _L,     _L, \
        _L,     _L,     _L,     _L,     _L,     _L,     _L,     _L, \
        _L,     _L,     _L,     _L,     _L,     _L,     _L,     _L, \
        _P,     _L,     _L,     _L,     _L,     _L,     _L,     _L, \
        _L,     _L,     _L,     _L,     _L,     _P,     _L,     _L
#define _CTYPE_ISO_8859_6_128_256 \
   	_C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, \
        _C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, \
        _C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, \
        _C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, \
	_S|_B,	0,	0,	0,	_P,	0,	0,	0,  \
	0,	0,	0,	0,	_P,	_P,	0,	0,  \
	0,	0,	0,	0,	0,	0,	0,	0,  \
	0,	0,	0,	_P,	0,	0,	0,	_P, \
	0,	_L,	_L,	_L,	_L,	_L,	_L,	_L, \
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, \
	0,	_L,	_L,	_L,	_L,	_L,	_L,	_L, \
	_L,	_L,	_L,	0,	0,	0,	0,	0,  \
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, \
	_L,	_L,	_L,	_P,	_P,	_P,	_P,	_P, \
	_P,	_P,	_P,	0,	0,	0,	0,	0,  \
	0,	0,	0,	0,	0,	0,	0,	0
#define _CTYPE_ISO_8859_7_128_256 \
   	_C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, \
        _C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, \
        _C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, \
        _C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, \
	_S|_B,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_P,	_P,	_P,	_P,	_P,	_P,	_U,	_P, \
	_U,	_U,	_U,	_P,	_U,	_P,	_U,	_U, \
	_L,	_U,	_U,	_U,	_U,	_U,	_U,	_U, \
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, \
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, \
	_U,	_U,	_U,	_U,	_L,	_L,	_L,	_L, \
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, \
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, \
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, \
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_P
#define _CTYPE_ISO_8859_8_128_256 \
   	_C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, \
        _C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, \
        _C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, \
        _C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, \
	_S|_B,	0,	_P,	_P,	_P,	_P,	_P,	_P, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	0,  \
	0,	0,	0,	0,	0,	0,	0,	0,  \
	0,	0,	0,	0,	0,	0,	0,	0,  \
	0,	0,	0,	0,	0,	0,	0,	0,  \
	0,	0,	0,	0,	0,	0,	0,	_L, \
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, \
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, \
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, \
	_L,	_L,	_L,	0,	0,	_P,	_P,	0
#define _CTYPE_ISO_8859_9_128_256 \
   	_C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, \
        _C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, \
        _C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, \
        _C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, \
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
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L
#define _CTYPE_ISO_8859_10_128_256 \
   	_C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, \
        _C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, \
        _C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, \
        _C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, \
	_S|_B,	_U,	_U,	_U,	_U,	_U,	_U,	_P, \
	_U,	_U,	_U,	_U,	_U,	_P,	_U,	_U, \
	_P,	_L,	_L,	_L,	_L,	_L,	_L,	_P, \
	_L,	_L,	_L,	_L,	_L,	_P,	_L,	_L, \
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, \
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, \
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, \
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_L, \
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, \
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, \
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, \
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L
#define _CTYPE_ISO_8859_11_128_256 \
   	_C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, \
        _C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, \
        _C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, \
        _C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, \
	_S|_B,	_L,	_L,	_L,	_L,	_L,	_L,	_L, \
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, \
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, \
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, \
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, \
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, \
	_L,	_P,	_L,	_L,	_P,	_P,	_P,	_P, \
	_P,	_P,	_P,	0,	0,	0,	0,	_P, \
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_P, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_L, \
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, \
	_L,	_L,	_L,	_L,	0,	0,	0,	0
#define _CTYPE_ISO_8859_13_128_256 \
   	_C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, \
        _C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, \
        _C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, \
        _C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, \
	_S|_B,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_P,	_P,	_U,	_P,	_P,	_P,	_P,	_P, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_P,	_P,	_L,	_P,	_P,	_P,	_P,	_P, \
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, \
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, \
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_P, \
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_L, \
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, \
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, \
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_P, \
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_P
#define _CTYPE_ISO_8859_14_128_256 \
   	_C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, \
        _C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, \
        _C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, \
        _C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, \
	_S|_B,	_U,	_L,	_P,	_U,	_L,	_U,	_P, \
	_U,	_P,	_U,	_L,	_U,	_P,	_P,	_U, \
	_U,	_L,	_U,	_L,	_U,	_L,	_P,	_U, \
	_L,	_L,	_L,	_U,	_L,	_U,	_L,	_L, \
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, \
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, \
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, \
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_L, \
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, \
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, \
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, \
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L
#define _CTYPE_ISO_8859_15_128_256 \
   	_C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, \
        _C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, \
        _C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, \
        _C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, \
        _S|_B,  _P,     _P,     _P,     _P,     _P,     _P,     _P, \
        _P,     _P,     _P,     _P,     _P,     _P,     _P,     _P, \
        _P,     _P,     _P,     _P,     _P,     _P,     _P,     _P, \
        _P,     _P,     _P,     _P,     _P,     _P,     _P,     _P, \
        _U,     _U,     _U,     _U,     _U,     _U,     _U,     _U, \
        _U,     _U,     _U,     _U,     _U,     _U,     _U,     _U, \
        _U,     _U,     _U,     _U,     _U,     _U,     _U,     _P, \
        _U,     _U,     _U,     _U,     _U,     _U,     _U,     _L, \
        _L,     _L,     _L,     _L,     _L,     _L,     _L,     _L, \
        _L,     _L,     _L,     _L,     _L,     _L,     _L,     _L, \
        _L,     _L,     _L,     _L,     _L,     _L,     _L,     _P, \
        _L,     _L,     _L,     _L,     _L,     _L,     _L,     _L
#define _CTYPE_ISO_8859_16_128_256 \
   	_C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, \
        _C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, \
        _C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, \
        _C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, \
	_S|_B,	_U,	_L,	_U,	_P,	_P,	_U,	_P, \
	_L,	_P,	_U,	_P,	_U,	_P,	_L,	_U, \
	_P,	_P,	_U,	_U,	_U,	_P,	_P,	_P, \
	_L,	_L,	_L,	_P,	_U,	_L,	_U,	_L, \
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, \
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, \
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, \
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_L, \
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, \
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, \
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, \
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L

extern int __iso_8859_index (const char *charset_ext);

#if defined(ALLOW_NEGATIVE_CTYPE_INDEX)

#ifndef __CYGWIN__
static _CONST
#endif
char __ctype_iso[15][128 + 256] = {
  { _CTYPE_ISO_8859_1_128_256,
    _CTYPE_DATA_0_127,
    _CTYPE_ISO_8859_1_128_256
  },
  { _CTYPE_ISO_8859_2_128_256,
    _CTYPE_DATA_0_127,
    _CTYPE_ISO_8859_2_128_256
  },
  { _CTYPE_ISO_8859_3_128_256,
    _CTYPE_DATA_0_127,
    _CTYPE_ISO_8859_3_128_256
  },
  { _CTYPE_ISO_8859_4_128_256,
    _CTYPE_DATA_0_127,
    _CTYPE_ISO_8859_4_128_256
  },
  { _CTYPE_ISO_8859_5_128_256,
    _CTYPE_DATA_0_127,
    _CTYPE_ISO_8859_5_128_256
  },
  { _CTYPE_ISO_8859_6_128_256,
    _CTYPE_DATA_0_127,
    _CTYPE_ISO_8859_6_128_256
  },
  { _CTYPE_ISO_8859_7_128_256,
    _CTYPE_DATA_0_127,
    _CTYPE_ISO_8859_7_128_256
  },
  { _CTYPE_ISO_8859_8_128_256,
    _CTYPE_DATA_0_127,
    _CTYPE_ISO_8859_8_128_256
  },
  { _CTYPE_ISO_8859_9_128_256,
    _CTYPE_DATA_0_127,
    _CTYPE_ISO_8859_9_128_256
  },
  { _CTYPE_ISO_8859_10_128_256,
    _CTYPE_DATA_0_127,
    _CTYPE_ISO_8859_10_128_256
  },
  { _CTYPE_ISO_8859_11_128_256,
    _CTYPE_DATA_0_127,
    _CTYPE_ISO_8859_11_128_256
  },
  { _CTYPE_ISO_8859_13_128_256,
    _CTYPE_DATA_0_127,
    _CTYPE_ISO_8859_13_128_256
  },
  { _CTYPE_ISO_8859_14_128_256,
    _CTYPE_DATA_0_127,
    _CTYPE_ISO_8859_14_128_256
  },
  { _CTYPE_ISO_8859_15_128_256,
    _CTYPE_DATA_0_127,
    _CTYPE_ISO_8859_15_128_256
  },
  { _CTYPE_ISO_8859_16_128_256,
    _CTYPE_DATA_0_127,
    _CTYPE_ISO_8859_16_128_256
  },
};

#else /* !defined(ALLOW_NEGATIVE_CTYPE_INDEX) */

static _CONST char __ctype_iso[15][1 + 256] = {
  { 0,
    _CTYPE_DATA_0_127,
    _CTYPE_ISO_8859_1_128_256
  },
  { 0,
    _CTYPE_DATA_0_127,
    _CTYPE_ISO_8859_2_128_256
  },
  { 0,
    _CTYPE_DATA_0_127,
    _CTYPE_ISO_8859_3_128_256
  },
  { 0,
    _CTYPE_DATA_0_127,
    _CTYPE_ISO_8859_4_128_256
  },
  { 0,
    _CTYPE_DATA_0_127,
    _CTYPE_ISO_8859_5_128_256
  },
  { 0,
    _CTYPE_DATA_0_127,
    _CTYPE_ISO_8859_6_128_256
  },
  { 0,
    _CTYPE_DATA_0_127,
    _CTYPE_ISO_8859_7_128_256
  },
  { 0,
    _CTYPE_DATA_0_127,
    _CTYPE_ISO_8859_8_128_256
  },
  { 0,
    _CTYPE_DATA_0_127,
    _CTYPE_ISO_8859_9_128_256
  },
  { 0,
    _CTYPE_DATA_0_127,
    _CTYPE_ISO_8859_10_128_256
  },
  { 0,
    _CTYPE_DATA_0_127,
    _CTYPE_ISO_8859_11_128_256
  },
  { 0,
    _CTYPE_DATA_0_127,
    _CTYPE_ISO_8859_13_128_256
  },
  { 0,
    _CTYPE_DATA_0_127,
    _CTYPE_ISO_8859_14_128_256
  },
  { 0,
    _CTYPE_DATA_0_127,
    _CTYPE_ISO_8859_15_128_256
  },
  { 0,
    _CTYPE_DATA_0_127,
    _CTYPE_ISO_8859_16_128_256
  },
};

#endif /* ALLOW_NEGATIVE_CTYPE_INDEX */
