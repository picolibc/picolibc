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

/*
 * Each known CES should be registered here
 */
#ifdef _ICONV_CONVERTER_EUC_JP
extern _CONST struct iconv_ces_desc _iconv_ces_module_euc_jp;
#endif 
#ifdef _ICONV_CONVERTER_EUC_KR
extern _CONST struct iconv_ces_desc _iconv_ces_module_euc_kr;
#endif 
#ifdef _ICONV_CONVERTER_EUC_TW
extern _CONST struct iconv_ces_desc _iconv_ces_module_euc_tw;
#endif 
#ifdef _ICONV_CONVERTER_GB2312
extern _CONST struct iconv_ces_desc _iconv_ces_module_gb2312;
#endif 
#ifdef _ICONV_CONVERTER_ISO_10646_UCS_2
extern _CONST struct iconv_ces_desc _iconv_ces_module_iso_10646_ucs_2;
#endif 
#ifdef _ICONV_CONVERTER_ISO_10646_UCS_4
extern _CONST struct iconv_ces_desc _iconv_ces_module_iso_10646_ucs_4;
#endif 
#ifdef _ICONV_CONVERTER_UCS_2_INTERNAL
extern _CONST struct iconv_ces_desc _iconv_ces_module_ucs_2_internal;
#endif 
#ifdef _ICONV_CONVERTER_UCS_4_INTERNAL
extern _CONST struct iconv_ces_desc _iconv_ces_module_ucs_4_internal;
#endif 
#ifdef _ICONV_CONVERTER_UTF_16
extern _CONST struct iconv_ces_desc _iconv_ces_module_utf_16;
#endif 
#ifdef _ICONV_CONVERTER_UTF_8
extern _CONST struct iconv_ces_desc _iconv_ces_module_utf_8;
#endif 

_CONST iconv_builtin_table_t _iconv_builtin_ces[] =
{
#ifdef _ICONV_CONVERTER_EUC_JP
    {"euc_jp", (_CONST _VOID_PTR)&_iconv_ces_module_euc_jp},
#endif 
#ifdef _ICONV_CONVERTER_EUC_KR
    {"euc_kr", (_CONST _VOID_PTR)&_iconv_ces_module_euc_kr},
#endif 
#ifdef _ICONV_CONVERTER_EUC_TW
    {"euc_tw", (_CONST _VOID_PTR)&_iconv_ces_module_euc_tw},
#endif 
#ifdef _ICONV_CONVERTER_GB2312
    {"gb_2312_80", (_CONST _VOID_PTR)&_iconv_ces_module_gb2312},
#endif 
#ifdef _ICONV_CONVERTER_ISO_10646_UCS_2
    {"iso_10646_ucs_2", (_CONST _VOID_PTR)&_iconv_ces_module_iso_10646_ucs_2},
#endif 
#ifdef _ICONV_CONVERTER_ISO_10646_UCS_4
    {"iso_10646_ucs_4", (_CONST _VOID_PTR)&_iconv_ces_module_iso_10646_ucs_4},
#endif 
#ifdef _ICONV_CONVERTER_UCS_2_INTERNAL
    {"ucs_2_internal", (_CONST _VOID_PTR)&_iconv_ces_module_ucs_2_internal},
#endif 
#ifdef _ICONV_CONVERTER_UCS_4_INTERNAL
    {"ucs_4_internal", (_CONST _VOID_PTR)&_iconv_ces_module_ucs_4_internal},
#endif 
#ifdef _ICONV_CONVERTER_UTF_16
    {"utf_16", (_CONST _VOID_PTR)&_iconv_ces_module_utf_16},
#endif 
#ifdef _ICONV_CONVERTER_UTF_8
    {"utf_8", (_CONST _VOID_PTR)&_iconv_ces_module_utf_8},
#endif 
    {NULL, (_CONST _VOID_PTR)NULL}
};

