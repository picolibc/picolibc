/*
 * Copyright (c) 2003, Artem B. Bityuckiy, SoftMine Corporation.
 * Rights transferred to Franklin Electronic Publishers.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
#include <_ansi.h>
#include "deps.h"

/* 
 * Built-in charset aliases table. The "official" name starts at the first 
 * position of a line, followed by zero or more aliases, separated by any 
 * whitespace character(s). Aliases list may continue on the next line if this 
 * line begins with a whitespace.
 */
_CONST char _iconv_builtin_aliases[] = 
{
#ifdef _ICONV_CONVERTER_US_ASCII
"us_ascii ansi_x3.4_1968 ansi_x3.4_1986 iso_646.irv:1991 ascii iso646_us us ibm367 cp367 csascii\n"
#endif
#ifdef _ICONV_CONVERTER_BIG5
"big5 csbig5 big_five bigfive cn_big5 cp950\n"
#endif
#ifdef _ICONV_CONVERTER_CNS11643_PLANE1
"cns11643_plane1\n"
#endif
#ifdef _ICONV_CONVERTER_CNS11643_PLANE2
"cns11643_plane2\n"
#endif
#ifdef _ICONV_CONVERTER_CNS11643_PLANE14
"cns11643_plane14\n"
#endif
#ifdef _ICONV_CONVERTER_SHIFT_JIS
"shift_jis sjis cp932 ms_kanji csshiftjis\n"
#endif
#ifdef _ICONV_CONVERTER_KSX_1001
"ksx1001 ks_x_1001\n"
#endif
#ifdef _ICONV_CONVERTER_JIS_X0212_1990
"jis_x0212_1990\n"
#endif
#ifdef _ICONV_CONVERTER_JIS_X0208_1983
"jis_x0208_1983 jis_c6226-1983 iso_ir_87 x0208\n"
#endif
#ifdef _ICONV_CONVERTER_JIS_X0201
"jis_x0201 x0201 cshalfwidthkatakana\n"
#endif
#ifdef _ICONV_CONVERTER_GB_2312_80
"gb_2312_80 gb2312 gb2312_80 euc_cn eucch cn_gb csgb2312 iso_ir_58 chinese\n"
#endif
#ifdef _ICONV_CONVERTER_CP866
"cp866 866 IBM866 CSIBM866\n"
#endif
#ifdef _ICONV_CONVERTER_CP855
"cp855 ibm855 855 csibm855\n"
#endif
#ifdef _ICONV_CONVERTER_CP852
"cp852 ibm852 852 cspcp852\n"
#endif
#ifdef _ICONV_CONVERTER_CP850
"cp850 ibm850 850 cspc850multilingual\n"
#endif
#ifdef _ICONV_CONVERTER_CP775
"cp775 ibm775 cspc775baltic\n"
#endif
#ifdef _ICONV_CONVERTER_KOI8_U
"koi8_u koi8u\n"
#endif
#ifdef _ICONV_CONVERTER_KOI8_R
"koi8_r cskoi8r koi8r koi8\n"
#endif
#ifdef _ICONV_CONVERTER_ISO_8859_1
"iso_8859_1 iso_8859_1 iso_88591:1987 iso_ir_100 latin1 l1 ibm819 cp819 csisolatin1\n"
#endif
#ifdef _ICONV_CONVERTER_ISO_8859_2
"iso_8859_2 iso8859_2 iso_88592 iso_8859_2:1987 iso_ir_101 latin2 l2 csisolatin2\n"
#endif
#ifdef _ICONV_CONVERTER_ISO_8859_4
"iso_8859_4 iso8859_4 iso_88594 iso_8859_4:1988 iso_ir_110 latin4 l4 csisolatin4\n"
#endif
#ifdef _ICONV_CONVERTER_ISO_8859_5
"iso_8859_5 iso8859_5 iso_88595 iso_8859_5:1988 iso_ir_144 cyrillic csisolatincyrillic\n"
#endif
#ifdef _ICONV_CONVERTER_ISO_8859_15
"iso_8859_15 iso8859_15 iso_885915 iso_8859_15:1998 iso8859_15 iso885915\n"
#endif
#ifdef _ICONV_CONVERTER_EUC_JP
"euc_jp eucjp\n"
#endif 
#ifdef _ICONV_CONVERTER_EUC_KR
"euc_kr euckr\n"
#endif 
#ifdef _ICONV_CONVERTER_EUC_TW
"euc_tw euctw\n"
#endif 
#ifdef _ICONV_CONVERTER_ISO_10646_UCS_2
"iso_10646_ucs_2 iso10646_ucs_2 iso_10646_ucs2 iso10646_ucs2 iso1064ucs2 ucs2 ucs_2\n"
#endif 
#ifdef _ICONV_CONVERTER_ISO_10646_UCS_4
"iso_10646_ucs_4 iso10646_ucs_4 iso_10646_ucs4 iso10646_ucs4 iso1064ucs4 ucs4 ucs_4\n"
#endif 
#ifdef _ICONV_CONVERTER_UCS_2_INTERNAL
"ucs_2_internal ucs2_internal ucs2internal\n"
#endif 
#ifdef _ICONV_CONVERTER_UCS_4_INTERNAL
"ucs_4_internal ucs4_internal ucs4internal\n"
#endif 
#ifdef _ICONV_CONVERTER_UTF_16
"utf_16 utf16\n"
#endif 
#ifdef _ICONV_CONVERTER_UTF_8
"utf_8 utf8\n"
#endif 
""
};

