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
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <reent.h>
#include <assert.h>
#include "local.h"

#ifdef _POSIX_MAPPED_FILES /* Should be defined in unistd.h if mmap/munmap */
#include <sys/mman.h>      /* are supported */
#endif

/*
 * _iconv_load_file - load CCS file into memory. 
 *
 * PARAMETERS:
 *     struct _reent *rptr - reent structure.
 *     _CONST char *fname - file name.
 *     _iconv_fd_t *desc - CCS file descriptor.
 *
 * DESCRIPTION:
 *    _iconv_load_file - is used to load charset file into memory. 
 *    Uses mmap() if possible. To close file - use _iconv_unload_file.
 *
 * RETURN: 0 if success, -1 if failure.
 */
int
_DEFUN(_iconv_load_file, (rptr, fname, desc),
       struct _reent *rptr _AND
       _CONST char *fname _AND
       _iconv_fd_t *desc)
{
    int fd;
#ifndef _POSIX_MAPPED_FILES
    off_t len;
#endif
    assert(desc != NULL);
    
    if ((fd = _open_r(rptr, fname, O_RDONLY, S_IRUSR)) < 0)
        return -1;
    
    if ((desc->len = (size_t)_lseek_r(rptr, fd, 0, SEEK_END)) == (off_t)-1 ||
        _lseek_r(rptr, fd, 0, SEEK_SET) == (off_t)-1)
        goto close_and_exit;

#ifdef _POSIX_MAPPED_FILES
    if ((desc->mem = _mmap_r(rptr, NULL, desc->len,
        PROT_READ, MAP_FIXED, fd, (off_t)0)) == MAP_FAILED)
        goto close_and_exit;
#else
   if ((desc->mem = _malloc_r(rptr, desc->len)) == NULL)
       goto close_and_exit;
           
   if ((len = _read_r(rptr, fd, desc->mem, desc->len)) < 0 || len != desc->len)
   {
       _free_r(rptr, desc->mem);
       goto close_and_exit;
   }
#endif /* #ifdef _POSIX_MAPPED_FILES */
   _close_r(rptr, fd);

   return 0;
close_and_exit:
   _close_r(rptr, fd);
   return -1;
}

/*
 * _iconv_unload_file - unload file loaded by _iconv_load_file.
 *
 * PARAMETERS:
 *    struct _reent *rptr - reent strucutre.
 *    _iconv_fd_t *desc - NLS file descriptor.
 *
 * DESCRIPTION:
 *    Unloads CCS file previously loaded with _iconv_load_file.
 *
 * RETURN: 0 if success, -1 if failure.
 */
int
_DEFUN(_iconv_unload_file, (rptr, desc), 
                        struct _reent *rptr _AND 
                        _iconv_fd_t *desc)
{
    assert(desc != NULL);
    assert(desc->mem != NULL);
#ifdef _POSIX_MAPPED_FILES
    assert(desc->len > 0);
    return _munmap_r(rptr, desc->mem, desc->len);
#else
    _free_r(rptr, desc->mem); 
    return 0;
#endif /* #ifdef _POSIX_MAPPED_FILES */
}

