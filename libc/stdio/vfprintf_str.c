/* Copyright (c) 2002, Alexander Popov (sasho@vip.bg)
   Copyright (c) 2002,2004,2005 Joerg Wunsch
   Copyright (c) 2005, Helmut Wallner
   Copyright (c) 2007, Dmitry Xmelkov
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
   * Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in
     the documentation and/or other materials provided with the
     distribution.
   * Neither the name of the copyright holders nor the names of
     contributors may be used to endorse or promote products derived
     from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
*/

{
#ifdef _NEED_IO_WCHAR
    if (flags & FL_LONG) {
        wstr = va_arg(ap, wchar_t *);
        if (!wstr)
            goto str_null;
    wstr_lpad:
        size = (flags & FL_PREC) ? (size_t)prec : SIZE_MAX;
#ifdef _NEED_IO_WIDETOMB
        size = _mbslen(wstr, size);
        if (size == (size_t)-1)
            goto ret;
#else
        size = wcsnlen(wstr, size);
#endif
        goto str_lpad;
    } else
#endif
        pnt = va_arg(ap, char *);
    if (!pnt) {
#ifdef _NEED_IO_WCHAR
    str_null:
#endif
#ifdef VFPRINTF_S
        msg = "arg corresponding to '%s' is null";
        goto handle_error;
#endif
        pnt = "(null)";
    }
#ifdef _NEED_IO_SHRINK
    char c;
    while ((c = *pnt++))
        my_putc(c, stream);
#else
    size = (flags & FL_PREC) ? (size_t)prec : SIZE_MAX;
#ifdef _NEED_IO_MBTOWIDE
    size = _wcslen(pnt, size);
#else
    size = strnlen(pnt, size);
#endif
str_lpad:
    if (!(flags & FL_LPAD)) {
        while ((size_t)width > size) {
            my_putc(' ', stream);
            width--;
        }
    }
    width -= size;
#ifdef _NEED_IO_WCHAR
    if (wstr) {
#ifdef _NEED_IO_WIDETOMB
        mbstate_t ps = { 0 };
        while (size) {
            wchar_t c = *wstr++;
            char   *m = u.mb;
            int     mb_len = __WCTOMB(m, c, &ps);
            while (size && mb_len) {
                my_putc(*m++, stream);
                size--;
                mb_len--;
            }
        }
#else
        while (size--)
            my_putc(*wstr++, stream);
#endif
    } else
#endif
    {
#ifdef _NEED_IO_MBTOWIDE
        mbstate_t ps = { 0 };
        while (size--) {
            wchar_t c;
            size_t  mb_len = mbrtowc(&c, pnt, MB_LEN_MAX, &ps);
            my_putc(c, stream);
            pnt += mb_len;
        }
#else
        while (size--)
            my_putc(*pnt++, stream);
#endif
    }
#endif
}
