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
#include "deps.h"
#include "local.h"

#ifdef _ICONV_CONVERTER_BIG5
extern _CONST unsigned char _iconv_ccs_table_big5[];
#endif
#ifdef _ICONV_CONVERTER_CNS11643_PLANE1
extern _CONST unsigned char _iconv_ccs_table_cns11643_plane1[];
#endif
#ifdef _ICONV_CONVERTER_CNS11643_PLANE2
extern _CONST unsigned char _iconv_ccs_table_cns11643_plane2[];
#endif
#ifdef _ICONV_CONVERTER_CNS11643_PLANE14
extern _CONST unsigned char _iconv_ccs_table_cns11643_plane14[];
#endif
#ifdef _ICONV_CONVERTER_CP775
extern _CONST unsigned char _iconv_ccs_table_cp775[];
#endif
#ifdef _ICONV_CONVERTER_CP850
extern _CONST unsigned char _iconv_ccs_table_cp850[];
#endif
#ifdef _ICONV_CONVERTER_CP852
extern _CONST unsigned char _iconv_ccs_table_cp852[];
#endif
#ifdef _ICONV_CONVERTER_CP855
extern _CONST unsigned char _iconv_ccs_table_cp855[];
#endif
#ifdef _ICONV_CONVERTER_CP866
extern _CONST unsigned char _iconv_ccs_table_cp866[];
#endif
#ifdef _ICONV_CONVERTER_GB_2312_80
extern _CONST unsigned char _iconv_ccs_table_gb_2312_80[];
#endif
#ifdef _ICONV_CONVERTER_ISO_8859_1
extern _CONST unsigned char _iconv_ccs_table_iso_8859_1[];
#endif
#ifdef _ICONV_CONVERTER_ISO_8859_15
extern _CONST unsigned char _iconv_ccs_table_iso_8859_15[];
#endif
#ifdef _ICONV_CONVERTER_ISO_8859_2
extern _CONST unsigned char _iconv_ccs_table_iso_8859_2[];
#endif
#ifdef _ICONV_CONVERTER_ISO_8859_4
extern _CONST unsigned char _iconv_ccs_table_iso_8859_4[];
#endif
#ifdef _ICONV_CONVERTER_ISO_8859_5
extern _CONST unsigned char _iconv_ccs_table_iso_8859_5[];
#endif
#ifdef _ICONV_CONVERTER_JIS_X0201
extern _CONST unsigned char _iconv_ccs_table_jis_x0201[];
#endif
#ifdef _ICONV_CONVERTER_JIS_X0208_1983
extern _CONST unsigned char _iconv_ccs_table_jis_x0208_1983[];
#endif
#ifdef _ICONV_CONVERTER_JIS_X0212_1990
extern _CONST unsigned char _iconv_ccs_table_jis_x0212_1990[];
#endif
#ifdef _ICONV_CONVERTER_KOI8_R
extern _CONST unsigned char _iconv_ccs_table_koi8_r[];
#endif
#ifdef _ICONV_CONVERTER_KOI8_U
extern _CONST unsigned char _iconv_ccs_table_koi8_u[];
#endif
#ifdef _ICONV_CONVERTER_KSX1001
extern _CONST unsigned char _iconv_ccs_table_ksx1001[];
#endif
#ifdef _ICONV_CONVERTER_SHIFT_JIS
extern _CONST unsigned char _iconv_ccs_table_shift_jis[];
#endif
#ifdef _ICONV_CONVERTER_US_ASCII
extern _CONST unsigned char _iconv_ccs_table_us_ascii[];
#endif

_CONST iconv_builtin_table_t _iconv_builtin_ccs[] =
{
#ifdef _ICONV_CONVERTER_BIG5
  {"big5", (_CONST _VOID_PTR)&_iconv_ccs_table_big5},
#endif
#ifdef _ICONV_CONVERTER_CNS11643_PLANE1
  {"cns11643_plane1", (_CONST _VOID_PTR)&_iconv_ccs_table_cns11643_plane1},
#endif
#ifdef _ICONV_CONVERTER_CNS11643_PLANE2
  {"cns11643_plane2", (_CONST _VOID_PTR)&_iconv_ccs_table_cns11643_plane2},
#endif
#ifdef _ICONV_CONVERTER_CNS11643_PLANE14
  {"cns11643_plane14", (_CONST _VOID_PTR)&_iconv_ccs_table_cns11643_plane14},
#endif
#ifdef _ICONV_CONVERTER_CP775
  {"cp775", (_CONST _VOID_PTR)&_iconv_ccs_table_cp775},
#endif
#ifdef _ICONV_CONVERTER_CP850
  {"cp850", (_CONST _VOID_PTR)&_iconv_ccs_table_cp850},
#endif
#ifdef _ICONV_CONVERTER_CP852
  {"cp852", (_CONST _VOID_PTR)&_iconv_ccs_table_cp852},
#endif
#ifdef _ICONV_CONVERTER_CP855
  {"cp855", (_CONST _VOID_PTR)&_iconv_ccs_table_cp855},
#endif
#ifdef _ICONV_CONVERTER_CP866
  {"cp866", (_CONST _VOID_PTR)&_iconv_ccs_table_cp866},
#endif
#ifdef _ICONV_CONVERTER_GB_2312_80
  {"gb_2312_80", (_CONST _VOID_PTR)&_iconv_ccs_table_gb_2312_80},
#endif
#ifdef _ICONV_CONVERTER_ISO_8859_1
  {"iso_8859_1", (_CONST _VOID_PTR)&_iconv_ccs_table_iso_8859_1},
#endif
#ifdef _ICONV_CONVERTER_ISO_8859_15
  {"iso_8859_15", (_CONST _VOID_PTR)&_iconv_ccs_table_iso_8859_15},
#endif
#ifdef _ICONV_CONVERTER_ISO_8859_2
  {"iso_8859_2", (_CONST _VOID_PTR)&_iconv_ccs_table_iso_8859_2},
#endif
#ifdef _ICONV_CONVERTER_ISO_8859_4
  {"iso_8859_4", (_CONST _VOID_PTR)&_iconv_ccs_table_iso_8859_4},
#endif
#ifdef _ICONV_CONVERTER_ISO_8859_5
  {"iso_8859_5", (_CONST _VOID_PTR)&_iconv_ccs_table_iso_8859_5},
#endif
#ifdef _ICONV_CONVERTER_JIS_X0201
  {"jis_x0201", (_CONST _VOID_PTR)&_iconv_ccs_table_jis_x0201},
#endif
#ifdef _ICONV_CONVERTER_JIS_X0208_1983
  {"jis_x0208_1983", (_CONST _VOID_PTR)&_iconv_ccs_table_jis_x0208_1983},
#endif
#ifdef _ICONV_CONVERTER_JIS_X0212_1990
  {"jis_x0212_1990", (_CONST _VOID_PTR)&_iconv_ccs_table_jis_x0212_1990},
#endif
#ifdef _ICONV_CONVERTER_KOI8_R
  {"koi8_r", (_CONST _VOID_PTR)&_iconv_ccs_table_koi8_r},
#endif
#ifdef _ICONV_CONVERTER_KOI8_U
  {"koi8_u", (_CONST _VOID_PTR)&_iconv_ccs_table_koi8_u},
#endif
#ifdef _ICONV_CONVERTER_KSX1001
  {"ksx1001", (_CONST _VOID_PTR)&_iconv_ccs_table_ksx1001},
#endif
#ifdef _ICONV_CONVERTER_SHIFT_JIS
  {"shift_jis", (_CONST _VOID_PTR)&_iconv_ccs_table_shift_jis},
#endif
#ifdef _ICONV_CONVERTER_US_ASCII
  {"us_ascii", (_CONST _VOID_PTR)&_iconv_ccs_table_us_ascii},
#endif
  {NULL, (_CONST _VOID_PTR)NULL}
};

