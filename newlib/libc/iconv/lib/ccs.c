/*-
 * Copyright (c) 1999, 2000
 *    Konstantin Chuguev.  All rights reserved.
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
 *
 *    iconv (Charset Conversion Library) v2.0
 */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <reent.h>
#include <endian.h>
#include <sys/param.h>
#include <sys/types.h>
#include "local.h"

static __uint16_t __inline
_DEFUN(betohs, (s), __uint16_t s)
{
#if (BYTE_ORDER == LITTLE_ENDIAN)
    return ((s << 8) & 0xFF00) | ((s >> 8) & 0x00FF);
#elif (BYTE_ORDER == BIG_ENDIAN)
    return s;
#else
#error "Unknown byte order."
#endif
}

static __uint32_t __inline
_DEFUN(betohl, (l), __uint32_t l)
{
#if (BYTE_ORDER == LITTLE_ENDIAN)
    return (((l << 24) & 0xFF000000) |
            ((l <<  8) & 0x00FF0000) |
            ((l >>  8) & 0x0000FF00) |
            ((l >> 24) & 0x000000FF));
#elif (BYTE_ORDER == BIG_ENDIAN)
    return l;
#else
#error "Unknown byte order."
#endif
} 

static __uint16_t __inline
_DEFUN(letohs, (s), __uint16_t s)
{
#if (BYTE_ORDER == LITTLE_ENDIAN)
    return s;
#elif (BYTE_ORDER == BIG_ENDIAN)
    return ((s << 8) & 0xFF00) | ((s >> 8) & 0x00FF);
#else
#error "Unknown byte order."
#endif
}

static __uint32_t __inline
_DEFUN(letohl, (s), __uint32_t l)
{
#if (BYTE_ORDER == LITTLE_ENDIAN)
    return l;
#elif (BYTE_ORDER == BIG_ENDIAN)
    return (((l << 24) & 0xFF000000) |
            ((l <<  8) & 0x00FF0000) |
            ((l >>  8) & 0x0000FF00) |
            ((l >> 24) & 0x000000FF));
#else
#error "Unknown byte order."
#endif
} 

/* Generic coded character set conversion table */
typedef struct {
    unsigned char label[8];  /* CSconvT<N>; N=[0-3] */
    __uint32_t tables[2];     /* offsets to 2 unidirectional tables */
} iconv_ccs_convtable_t;

#define ICONV_TBL_LABEL             "\003CSCT"
#define ICONV_TBL_LABEL_SIZE        5
#define ICONV_TBL_BYTE_ORDER(table) (((table)->label[5]) & 1)
#define ICONV_TBL_NBITS(table)      ((table)->label[6])
#define ICONV_TBL_VERSION(table)    ((table)->label[7])

/* Indices for unidirectional conversion tables */
enum { _iconv_ccs_to_ucs = 0, _iconv_ccs_from_ucs = 1 };


/* Unidirectional conversion table types */

/* one-level tables */
typedef struct {
    ucs2_t data[128];
} iconv_ccs_table_7bit_t; /* 7-bit charset to Unicode */

typedef struct {
    ucs2_t data[256];
} iconv_ccs_table_8bit_t; /* 8-bit charset to Unicode */

/* two-level tables */
typedef struct {
    __uint32_t data[128];
} iconv_ccs_table_14bit_t; /* 14-bit charset to Unicode */

typedef struct {
    __uint32_t data[256];
} iconv_ccs_table_16bit_t; /* 16-bit charset to Unicode;
                            * Unicode to any charset */

/* abstract table */
typedef union {
    iconv_ccs_table_7bit_t  _7bit;
    iconv_ccs_table_8bit_t  _8bit;
    iconv_ccs_table_14bit_t _14bit;
    iconv_ccs_table_16bit_t _16bit;
} iconv_ccs_table_abstract_t;

/* host and network byte order array element macros */
#define iconv_table_elt_le(base, i, type)    \
    ((type *)(((char *)(base)) + letohl(((__uint32_t *)(base))[(i)])))

#define iconv_table_elt_be(base, i, type)    \
    ((type *)(((char *)(base)) + betohl(((__int32_t *)(base))[(i)])))

#define abstable ((_CONST iconv_ccs_table_abstract_t *)table)

/* Functions for little endian byte order tables */
static ucs2_t
_DEFUN(cvt_7bit_le, (table, ch), 
                     _CONST _VOID_PTR table _AND 
                     ucs2_t ch)
{
    return ch & 0x80 ? UCS_CHAR_INVALID : letohs(abstable->_7bit.data[ch]);
}

static ucs2_t
_DEFUN(cvt_8bit_le, (table, ch),
                     _CONST _VOID_PTR table _AND
                     ucs2_t ch)
{
    return letohs(abstable->_8bit.data[ch]);
}

static ucs2_t
_DEFUN(cvt_14bit_le, (table, ch),
                     _CONST _VOID_PTR table _AND
                     ucs2_t ch)
{
    _CONST iconv_ccs_table_7bit_t *sub_table;

    if (ch & 0x8080)
        return UCS_CHAR_INVALID;
    sub_table =  iconv_table_elt_le(table, ch >> 8, iconv_ccs_table_7bit_t);
    return sub_table == &(abstable->_7bit) ? UCS_CHAR_INVALID
                                           : letohs(sub_table->data[ch & 0x7F]);
}

static ucs2_t
_DEFUN(cvt_16bit_le, (table, ch),
                     _CONST _VOID_PTR table _AND
                     ucs2_t ch)
{
    _CONST iconv_ccs_table_8bit_t *sub_table =
        iconv_table_elt_le(table, ch >> 8, iconv_ccs_table_8bit_t);
    return sub_table == &(abstable->_8bit) ? UCS_CHAR_INVALID
                                           : letohs(sub_table->data[ch & 0xFF]);
}

static iconv_ccs_convert_t * _CONST converters_le[] = {
    cvt_7bit_le, cvt_8bit_le, cvt_14bit_le, cvt_16bit_le
};


/* Functions for network byte order tables */
static ucs2_t
_DEFUN(cvt_7bit_be, (table, ch),
                     _CONST _VOID_PTR table _AND
                     ucs2_t ch)
{
    return ch & 0x80 ? UCS_CHAR_INVALID : betohs(abstable->_7bit.data[ch]);
}

static ucs2_t
_DEFUN(cvt_8bit_be, (table, ch),
                     _CONST _VOID_PTR table _AND
                     ucs2_t ch)
{
    return betohs(abstable->_8bit.data[ch]);
}

static ucs2_t
_DEFUN(cvt_14bit_be, (table, ch),
                     _CONST _VOID_PTR table _AND
                     ucs2_t ch)
{
    _CONST iconv_ccs_table_7bit_t *sub_table;

    if (ch & 0x8080)
        return UCS_CHAR_INVALID;
    sub_table =  iconv_table_elt_be(table, ch >> 8, iconv_ccs_table_7bit_t);
    return sub_table == &(abstable->_7bit) ? UCS_CHAR_INVALID
                                           : betohs(sub_table->data[ch & 0x7F]);
}

static ucs2_t
_DEFUN(cvt_16bit_be, (table, ch),
                     _CONST _VOID_PTR table _AND
                     ucs2_t ch)
{
    _CONST iconv_ccs_table_8bit_t *sub_table =
        iconv_table_elt_be(table, ch >> 8, iconv_ccs_table_8bit_t);
    return sub_table == &(abstable->_8bit) ? UCS_CHAR_INVALID
                                           : betohs(sub_table->data[ch & 0xFF]);
}

static iconv_ccs_convert_t * _CONST converters_be[] = {
    cvt_7bit_be, cvt_8bit_be, cvt_14bit_be, cvt_16bit_be
};

#undef abstable

/* Generic coded character set initialisation function */
static int
_DEFUN(ccs_init, (ccs, table),
                 struct iconv_ccs *ccs _AND
                 _CONST iconv_ccs_convtable_t *table)
{
    if (strncmp(table->label, ICONV_TBL_LABEL, ICONV_TBL_LABEL_SIZE))
        return EINVAL;
    if (ICONV_TBL_VERSION(table) > 3)
        return EINVAL;
    ccs->nbits = ICONV_TBL_NBITS(table);

    if (ICONV_TBL_BYTE_ORDER(table) == ICONV_CCT_LE) {
        /* Little Endian */
        ccs->from_ucs = iconv_table_elt_le(table->tables,
                                           _iconv_ccs_from_ucs,
                                           _CONST iconv_ccs_convtable_t);
        ccs->to_ucs = iconv_table_elt_le(table->tables,
                                         _iconv_ccs_to_ucs,
                                         _CONST iconv_ccs_convtable_t);
        ccs->convert_from_ucs = cvt_16bit_le;
        ccs->convert_to_ucs = converters_le[ICONV_TBL_VERSION(table)];
    } else {
        /* Big Endian (Network Byte Order) */
        ccs->from_ucs = iconv_table_elt_be(table->tables,
                                             _iconv_ccs_from_ucs,
                                             _CONST iconv_ccs_convtable_t);
        ccs->to_ucs = iconv_table_elt_be(table->tables,
                                           _iconv_ccs_to_ucs,
                                           _CONST iconv_ccs_convtable_t);
        ccs->convert_from_ucs = cvt_16bit_be;
        ccs->convert_to_ucs = converters_be[ICONV_TBL_VERSION(table)];
    }
    return 0;
}


static int
_DEFUN(close_builtin, (rptr, desc),
                      struct _reent *rptr _AND
                      struct iconv_ccs *desc)
{
    return 0;
}

static int
_DEFUN(iconv_ccs_init_builtin, (ccs, name),
                               struct iconv_ccs *ccs _AND
                               _CONST char *name)
{
    _CONST iconv_builtin_table_t *ccs_ptr;
    for (ccs_ptr = _iconv_builtin_ccs; ccs_ptr->key != NULL; ccs_ptr ++) {
        if (strcmp(ccs_ptr->key, name) == 0) {
            int res = ccs_init(ccs, (_CONST iconv_ccs_convtable_t *)
                                    (ccs_ptr->value));
            if (res == 0)
                ccs->close = close_builtin;
            return res;
        }
    } 
    return EINVAL;
}

/* External CCS initialisation */
struct external_extra {
    _CONST iconv_ccs_convtable_t *ptr;
    off_t size;
};

static int
_DEFUN(close_external, (rptr, desc),
                       struct _reent *rptr _AND
                       struct iconv_ccs *desc)
{
    _iconv_unload_file(rptr, (_iconv_fd_t *)desc->extra);
    _free_r(rptr, desc->extra);
    return 0;
}

static int
_DEFUN(iconv_ccs_init_external, (rptr, ccs, name),
                                struct _reent *rptr   _AND
                                struct iconv_ccs *ccs _AND
                                _CONST char *name)
{
    char *file;
    _CONST iconv_ccs_convtable_t *ccs_ptr;
    _CONST char *datapath;
    _iconv_fd_t *extra;


    if ((datapath = _getenv_r(rptr, NLS_ENVVAR_NAME)) == NULL || 
                                                 *datapath == '\0')
        datapath = NLS_DEFAULT_NLSPATH;

    if ((file = _malloc_r(rptr, strlen(name) + sizeof(ICONV_DATA_EXT) + 1)) 
                                                                    == NULL)
        return EINVAL;
    
    _sprintf_r(rptr, file, "%s"ICONV_DATA_EXT, name);
        
    name = (_CONST char *)_iconv_construct_filename(rptr, datapath, file);
    _free_r(rptr, (_VOID_PTR)file);
    if (name == NULL)
        return EINVAL;
    
    if ((extra = (_iconv_fd_t *)_malloc_r(rptr, sizeof(_iconv_fd_t))) == NULL) {
        _free_r(rptr, (_VOID_PTR)name);
        return EINVAL;
    }
    
    if (_iconv_load_file(rptr, name, extra) != 0) {
        _free_r(rptr, (_VOID_PTR)name);
        return EINVAL;
    }
    _free_r(rptr, (_VOID_PTR)name);
    
    ccs_ptr = (_CONST iconv_ccs_convtable_t *)extra->mem;
    if (ccs_init(ccs, ccs_ptr) != 0) {
        _iconv_unload_file(rptr, extra);
        _free_r(rptr, (_VOID_PTR)extra);
        return __errno_r(rptr);
    }

    ccs->extra = (_VOID_PTR)extra;
    ccs->close = close_external;
    return 0;
}

int
_DEFUN(_iconv_ccs_init, (rptr, ccs, name),
                        struct _reent *rptr   _AND
                        struct iconv_ccs *ccs _AND
                        _CONST char *name)
{
    int res = iconv_ccs_init_builtin(ccs, name);
    if (res)
        res = iconv_ccs_init_external(rptr, ccs, name);
    if (res)
        __errno_r(rptr) = res;
    return res;
}

