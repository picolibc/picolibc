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
#ifdef ENABLE_ICONV
 
#include "deps.h"
#include <_ansi.h>
#include "local.h"

/*
 * Each known CES should be registered here
 */
#ifdef ICONV_CONVERTER_EUC_JP
extern _CONST struct iconv_ces_desc iconv_ces_euc_jp;
#endif 
#ifdef ICONV_CONVERTER_EUC_KR
extern _CONST struct iconv_ces_desc iconv_ces_euc_kr;
#endif 
#ifdef ICONV_CONVERTER_EUC_TW
extern _CONST struct iconv_ces_desc iconv_ces_euc_tw;
#endif 
#ifdef ICONV_CONVERTER_GB2312
extern _CONST struct iconv_ces_desc iconv_ces_gb2312;
#endif 
#ifdef ICONV_CONVERTER_ISO_10646_UCS_2
extern _CONST struct iconv_ces_desc iconv_ces_iso_10646_ucs_2;
#endif 
#ifdef ICONV_CONVERTER_ISO_10646_UCS_4
extern _CONST struct iconv_ces_desc iconv_ces_iso_10646_ucs_4;
#endif 
#ifdef ICONV_CONVERTER_UCS_2_INTERNAL
extern _CONST struct iconv_ces_desc iconv_ces_ucs_2_internal;
#endif 
#ifdef ICONV_CONVERTER_UCS_4_INTERNAL
extern _CONST struct iconv_ces_desc iconv_ces_ucs_4_internal;
#endif 
#ifdef ICONV_CONVERTER_UTF_16
extern _CONST struct iconv_ces_desc iconv_ces_utf_16;
#endif 
#ifdef ICONV_CONVERTER_UTF_8
extern _CONST struct iconv_ces_desc iconv_ces_utf_8;
#endif 

_CONST iconv_builtin_table iconv_builtin_ces[] =
{
#ifdef ICONV_CONVERTER_EUC_JP
    {(_VOID_PTR)"euc_jp", (_VOID_PTR)&iconv_ces_euc_jp},
#endif 
#ifdef ICONV_CONVERTER_EUC_KR
    {(_VOID_PTR)"euc_kr", (_VOID_PTR)&iconv_ces_euc_kr},
#endif 
#ifdef ICONV_CONVERTER_EUC_TW
    {(_VOID_PTR)"euc_tw", (_VOID_PTR)&iconv_ces_euc_tw},
#endif 
#ifdef ICONV_CONVERTER_GB2312
    {(_VOID_PTR)"gb_2312_80", (_VOID_PTR)&iconv_ces_gb2312},
#endif 
#ifdef ICONV_CONVERTER_ISO_10646_UCS_2
    {(_VOID_PTR)"iso_10646_ucs_2", (_VOID_PTR)&iconv_ces_iso_10646_ucs_2},
#endif 
#ifdef ICONV_CONVERTER_ISO_10646_UCS_4
    {(_VOID_PTR)"iso_10646_ucs_4", (_VOID_PTR)&iconv_ces_iso_10646_ucs_4},
#endif 
#ifdef ICONV_CONVERTER_UCS_2_INTERNAL
    {(_VOID_PTR)"ucs_2_internal", (_VOID_PTR)&iconv_ces_ucs_2_internal},
#endif 
#ifdef ICONV_CONVERTER_UCS_4_INTERNAL
    {(_VOID_PTR)"ucs_4_internal", (_VOID_PTR)&iconv_ces_ucs_4_internal},
#endif 
#ifdef ICONV_CONVERTER_UTF_16
    {(_VOID_PTR)"utf_16", (_VOID_PTR)&iconv_ces_utf_16},
#endif 
#ifdef ICONV_CONVERTER_UTF_8
    {(_VOID_PTR)"utf_8", (_VOID_PTR)&iconv_ces_utf_8},
#endif 
    {NULL, NULL}
};

#endif /* #ifdef ENABLE_ICONV */

