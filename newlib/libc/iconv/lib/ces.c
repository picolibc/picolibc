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
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <reent.h>
#include "local.h"

static int 
_DEFUN(close_builtin, (rptr, ces),
                      struct _reent *rptr _AND
                      _VOID_PTR ces)
{
    return ((struct iconv_ces *)ces)->desc->close(rptr,
            ((struct iconv_ces *)ces)->data);
}

static int
_DEFUN(ces_instance_init, (rptr, ces, name, desc),
                          struct _reent *rptr _AND
                          struct iconv_ces *ces _AND
                          _CONST char *name _AND
                          _CONST struct iconv_ces_desc *desc)
{
    int res = desc->init(rptr, &(ces->data), name, desc->data);

    if (res)
        return __errno_r(rptr) = res;
    ces->desc = desc;
    ces->close = close_builtin;
    return 0;
}

static int
_DEFUN(ces_init_builtin, (rptr, ces, name),
                         struct _reent *rptr   _AND
                         struct iconv_ces *ces _AND
                         _CONST char *name)
{
    _CONST iconv_builtin_table_t *ces_ptr;
    for (ces_ptr = _iconv_builtin_ces; ces_ptr->key != NULL; ces_ptr ++)
        if (strcmp(ces_ptr->key, name) == 0)
            return ces_instance_init(rptr, ces, name,
                   (_CONST struct iconv_ces_desc *)(ces_ptr->value));
    return __errno_r(rptr) = EINVAL;
}

int
_DEFUN(_iconv_ces_init, (rptr, ces, name),
                        struct _reent *rptr   _AND
                        struct iconv_ces *ces _AND
                        _CONST char *name)
{
    return ces_init_builtin(rptr, ces, name)
           && ces_instance_init(rptr, ces, name, &_iconv_ces_table_driven);
}

int
_DEFUN(_iconv_ces_init_state, (rptr, res, name, data),
                              struct _reent *rptr _AND
                              _VOID_PTR *res      _AND
                              _CONST char *name   _AND
                              _CONST _VOID_PTR data)
{
        if ((*res = _malloc_r(rptr, sizeof(int))) == NULL)
            return __errno_r(rptr);
        memset(*res, '\0', sizeof(int));
        return 0;
}

int
_DEFUN(_iconv_ces_close_state, (rptr, data),
                              struct _reent *rptr _AND
                              _VOID_PTR data)
{
    if (data != NULL)
        _free_r(rptr, data);
    return 0;
}

_VOID
_DEFUN(_iconv_ces_reset_state, (data), _VOID_PTR data)
{
    *(int *)data = 0;
}

int
_DEFUN(_iconv_ces_init_null, (rptr, res, name, data),
                             struct _reent *rptr _AND
                             _VOID_PTR *res      _AND
                             _CONST char *name   _AND
                             _CONST _VOID_PTR data)
{
    return 0;
}

int
_DEFUN(_iconv_ces_close_null, (rptr, data),
                             struct _reent *rptr _AND
                             _VOID_PTR data)
{
    return 0;
}

_VOID
_DEFUN(_iconv_ces_reset_null, (data), _VOID_PTR data)
{
    return;
}

