/*-
 * Copyright (c) 1999,2000
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
#include <stdlib.h>
#include <string.h>
#include "local.h"

typedef struct {
    _CONST char *sequence;
    size_t      length;
    int         prefix_type;
} iconv_ces_iso2022_shift_t;

enum { ICONV_PREFIX_STATE = 0, ICONV_PREFIX_LINE, ICONV_PREFIX_CHAR };

static _CONST iconv_ces_iso2022_shift_t iso_shift[] = {
    { "\x0f",  1, ICONV_PREFIX_STATE },
    { "\x0e",  1, ICONV_PREFIX_LINE },
    { "\x1bN", 2, ICONV_PREFIX_CHAR },
    { "\x1bO", 2, ICONV_PREFIX_CHAR }
};

#define shift_num (sizeof(iso_shift) / sizeof(iconv_ces_iso2022_shift_t))

typedef struct {
    int     nccs;
    ucs_t   previous_char;
    int     shift_index;
    int     shift_tab[shift_num];
    char    prefix_cache[128];
    struct  iconv_ccs ccs[1];
} iconv_ces_iso2022_state_t;

int
_DEFUN(_iconv_iso2022_init, (rptr, data, desc_data, num),
                            struct _reent *rptr        _AND
                            _VOID_PTR *data            _AND
                            _CONST _VOID_PTR desc_data _AND
                            size_t num)
{
    size_t stsz = sizeof(iconv_ces_iso2022_state_t) +
                  sizeof(struct iconv_ccs) * (num - 1);
    int i;
    iconv_ces_iso2022_state_t *state
        = (iconv_ces_iso2022_state_t *)_malloc_r(rptr, stsz);

    if (state == NULL)
        return __errno_r(rptr);
    bzero(state->prefix_cache, sizeof(state->prefix_cache));
    for (i = 0; i < num; i++) {
        _CONST iconv_ces_iso2022_ccs_t *ccsattr = 
                         &(((_CONST iconv_ces_iso2022_ccs_t *)desc_data)[i]);
        int res = _iconv_ccs_init(rptr, &(state->ccs[i]), ccsattr->name);
        if (res) {
            while (--i >= 0)
                state->ccs[i].close(rptr, &(state->ccs[i]));
            _free_r(rptr, state);
            return res;
        }
        if (ccsattr->designatorlen)
            state->prefix_cache[(int)ccsattr->designator[0]] = 1;
        if (ccsattr->shift >= 0)
            state->prefix_cache[(int)iso_shift[ccsattr->shift].sequence[0]] = 1;
    }
    state->nccs = num;
    iconv_iso2022_reset(state);
    (iconv_ces_iso2022_state_t *)*data = state;
    return 0;
}

#define state ((iconv_ces_iso2022_state_t *)data)

int
_DEFUN(_iconv_iso2022_close, (rptr, data),
                             struct _reent *rptr _AND
                             _VOID_PTR data)
{
    int i, res = 0;

    for (i = 0; i < state->nccs; i++)
        res = state->ccs[i].close(rptr, &(state->ccs[i])) || res;
    _free_r(rptr, data);
    return res;
}

_VOID
_DEFUN(_iconv_iso2022_reset, (data), _VOID_PTR data)
{
    size_t i;

    state->shift_index = 0;
    state->shift_tab[0] = 0;
    for (i = 1; i < shift_num; i++)
        state->shift_tab[i] = -1;
    state->previous_char = UCS_CHAR_NONE;
}

#undef state

#define CES_STATE(ces)     ((iconv_ces_iso2022_state_t *)((ces)->data))
#define CES_CCSATTR(ces)   ((_CONST iconv_ces_iso2022_ccs_t *) \
                           (((struct iconv_ces_desc *)((ces)->desc))->data))

static _VOID 
_DEFUN(update_shift_state, (ces, ch),
                           _CONST struct iconv_ces *ces _AND
                           ucs_t ch)
{
    iconv_ces_iso2022_state_t *iso_state = CES_STATE(ces);
    size_t i;

    if (ch == '\n' && iso_state->previous_char == '\r') {
        for (i = 0; i < shift_num; i ++) {
            if (iso_shift[i].prefix_type != ICONV_PREFIX_STATE)
                iso_state->shift_tab[i] = -1;
        }
    }
    iso_state->previous_char = ch;
}

#define is_7_14bit(ccs) ((ccs)->nbits & 7)

static ssize_t
_DEFUN(cvt_ucs2iso, (ces, in, outbuf, outbytesleft, cs),
                    _CONST struct iconv_ces *ces _AND
                    ucs_t in                     _AND
                    unsigned char **outbuf       _AND
                    size_t *outbytesleft         _AND
                    int cs)
{
    iconv_ces_iso2022_state_t *iso_state = CES_STATE(ces);
    _CONST iconv_ces_iso2022_ccs_t *ccsattr;
    _CONST struct iconv_ccs *ccs;
    ucs_t res;
    size_t len = 0;
    int need_designator, need_shift;

    ccs = &(iso_state->ccs[cs]);
    res = (in == UCS_CHAR_NONE) ?
        in : ICONV_CCS_CONVERT_FROM_UCS(ccs, in);
    if (in != UCS_CHAR_NONE) {
        if (iso_shift[cs].prefix_type == ICONV_PREFIX_CHAR &&
                                                 !is_7_14bit(ccs)) {
            if ((res & 0x8080) == 0)
                return -1;
            res &= 0x7F7F;
        } else if (res & 0x8080)
            return -1; /* Invalid/missing character in the output charset */
    }
    ccsattr = &(CES_CCSATTR(ces)[cs]);
    if ((need_shift = (ccsattr->shift != iso_state->shift_index)))
        len += iso_shift[ccsattr->shift].length;
    if ((need_designator = (cs != iso_state->shift_tab[ccsattr->shift])))
        len += ccsattr->designatorlen;
    if (in != UCS_CHAR_NONE)
        len += res & 0xFF00 ? 2 : 1;
    if (len > *outbytesleft)
        return 0;    /* No space in output buffer */
    if (need_designator && (len = ccsattr->designatorlen)) {
        memcpy(*outbuf, ccsattr->designator, len);
        (*outbuf) += len;
        (*outbytesleft) -= len;
        iso_state->shift_tab[ccsattr->shift] = cs;
    }
    if (need_shift && (len = iso_shift[ccsattr->shift].length)) {
        memcpy(*outbuf, iso_shift[ccsattr->shift].sequence, len);
        (*outbuf) += len;
        (*outbytesleft) -= len;
        if (iso_shift[ccsattr->shift].prefix_type != ICONV_PREFIX_CHAR)
            iso_state->shift_index = ccsattr->shift;
    }
    if (in == UCS_CHAR_NONE)
        return 1;
    if (res & 0xFF00) {
        *(unsigned char *)(*outbuf) ++ = res >> 8;
        (*outbytesleft)--;
    }
    *(unsigned char *)(*outbuf) ++ = res;
    (*outbytesleft) --;
    update_shift_state(ces, res);
    return 1;
}

ssize_t
_DEFUN(_iconv_iso2022_convert_from_ucs, (ces, in, outbuf, outbytesleft),
                                        struct iconv_ces *ces  _AND
                                        ucs_t in               _AND
                                        unsigned char **outbuf _AND
                                        size_t *outbytesleft)
{
    iconv_ces_iso2022_state_t *iso_state = CES_STATE(ces);
    ssize_t res;
    int cs, i;

    if (in == UCS_CHAR_NONE)
        return cvt_ucs2iso(ces, in, outbuf, outbytesleft, 0);
    if (iconv_char32bit(in))
        return -1;
    cs = iso_state->shift_tab[iso_state->shift_index];
    if ((res = cvt_ucs2iso(ces, in, outbuf, outbytesleft, cs)) >= 0)
        return res;
    for (i = 0; i < iso_state->nccs; i++) {
        if (i == cs)
            continue;
        if ((res = cvt_ucs2iso(ces, in, outbuf, outbytesleft, i)) >= 0)
            return res;
    }
    (*outbuf) ++;
    (*outbytesleft) --;
    return -1;    /* No character in output charset */
}

static ucs_t
_DEFUN(cvt_iso2ucs, (ccs, inbuf, inbytesleft, prefix_type),
                    _CONST struct iconv_ccs *ccs _AND
                    _CONST unsigned char **inbuf _AND
                    size_t *inbytesleft          _AND
                    int prefix_type)
{
    size_t bytes = ccs->nbits > 8 ? 2 : 1;
    ucs_t ch = **inbuf;

    if (*inbytesleft < bytes)
        return UCS_CHAR_NONE;    /* Not enough bytes in the input buffer */
    if (bytes == 2)
        ch = (ch << 8) | *(++(*inbuf));
    (*inbuf)++;
    (*inbytesleft) -= bytes;
    if (ch & 0x8080)
        return UCS_CHAR_INVALID;
    if (prefix_type == ICONV_PREFIX_CHAR && !is_7_14bit(ccs))
        ch |= (bytes == 2) ? 0x8080 : 0x80;
    return ICONV_CCS_CONVERT_TO_UCS(ccs, ch);
}

ucs_t
_DEFUN(_iconv_iso2022_convert_to_ucs, (ces, inbuf, inbytesleft),
                                      struct iconv_ces *ces        _AND
                                      _CONST unsigned char **inbuf _AND
                                      size_t *inbytesleft)
{
    iconv_ces_iso2022_state_t *iso_state = CES_STATE(ces);
    _CONST iconv_ces_iso2022_ccs_t *ccsattr;
    ucs_t res;
    _CONST unsigned char *ptr = *inbuf;
    unsigned char byte;
    size_t len, left = *inbytesleft;
    int i;

    while (left) {
        byte = *ptr;
        if (byte & 0x80) {
            (*inbuf)++;
            (*inbytesleft) --;
            return UCS_CHAR_INVALID;
        }
        if (!iso_state->prefix_cache[byte])
            break;
        for (i = 0; i < iso_state->nccs; i++) {
            ccsattr = &(CES_CCSATTR(ces)[i]);
            len = ccsattr->designatorlen;
            if (len) {
                if (len + 1 > left)
                    return UCS_CHAR_NONE;
                if (memcmp(ptr, ccsattr->designator, len) == 0) {
                    iso_state->shift_tab[ccsattr->shift] = i;
                    ptr += len;
                    left -= len;
                    break;
                }
            }
            len = iso_shift[ccsattr->shift].length;
            if (len) {
                if (len + 1 > left)
                    return UCS_CHAR_NONE;
                if (memcmp(ptr,
                    iso_shift[ccsattr->shift].sequence, len) == 0) {
                    if (iso_shift[ccsattr->shift].prefix_type != ICONV_PREFIX_CHAR)
                        iso_state->shift_index = ccsattr->shift;
                    ptr += len;
                    left -= len;
                    break;
                }
            }
        }
    }
    i = iso_state->shift_tab[iso_state->shift_index];
    if (i < 0) {
        (*inbuf) ++;
        (*inbytesleft) --;
        return UCS_CHAR_INVALID;
    }
    res = cvt_iso2ucs(&(iso_state->ccs[i]), &ptr, &left,
                      iso_shift[i].prefix_type);
    if (res != UCS_CHAR_NONE) {
        *inbuf = (_CONST char*)ptr;
        *inbytesleft = left;
        update_shift_state(ces, res);
    }
    return res;
}

