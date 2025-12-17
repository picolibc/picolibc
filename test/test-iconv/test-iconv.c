/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2025 Keith Packard
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <iconv.h>
#include <locale.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <wchar.h>
#include <errno.h>
#include <inttypes.h>
#include <assert.h>
#include <limits.h>

#ifndef __PICOLIBC__
#ifdef HAVE_UTF_CHARSETS
#define __MB_CAPABLE
#define __MB_EXTENDED_CHARSETS_UCS
#ifdef HAVE_ISO_CHARSETS
#define __MB_EXTENDED_CHARSETS_ISO
#endif
#ifdef HAVE_WINDOWS_CHARSETS
#define __MB_EXTENDED_CHARSETS_WINDOWS
#endif
#ifdef HAVE_JIS_CHARSETS
#define __MB_EXTENDED_CHARSETS_JIS
#endif
#endif
#endif

#ifdef __MB_CAPABLE
#define MID_CODE "UTF-8"
#else
#define MID_CODE "ASCII"
#endif

static const char *encodings[] = {
    "ASCII",
#ifdef __MB_CAPABLE
    "UTF-8",
#ifdef __MB_EXTENDED_CHARSETS_ISO
    "ISO-8859-1",  "ISO-8859-2",  "ISO-8859-3",  "ISO-8859-4",  "ISO-8859-5",  "ISO-8859-6",
    "ISO-8859-7",  "ISO-8859-8",  "ISO-8859-9",  "ISO-8859-10", "ISO-8859-11", "ISO-8859-13",
    "ISO-8859-14", "ISO-8859-15", "ISO-8859-16",
#endif
#ifdef __MB_EXTENDED_CHARSETS_WINDOWS
    "CP437",
#ifndef __GLIBC__
    "CP720",
#endif
    "CP737",       "CP775",       "CP850",       "CP852",       "CP855",       "CP857",
    "CP858",       "CP862",       "CP866",       "CP874",       "CP1125",      "CP1250",
    "CP1251",      "CP1252",      "CP1253",      "CP1254",
#ifndef __GLIBC__
    "CP1255",
#endif
    "CP1256",      "CP1257",
#ifndef __GLIBC__
    "CP1258",
#endif
    "KOI8-R",      "KOI8-U",      "GEORGIAN-PS", "PT154",       "KOI8-T",
#endif
#ifdef __MB_EXTENDED_CHARSETS_JIS
    "EUC-JP",      "SHIFT-JIS",
#endif
#endif
};

#define NENCODING (sizeof(encodings) / sizeof(encodings[0]))

#ifdef __MSP430__
#define INBUF_SIZE 512
#else
#define INBUF_SIZE 1024
#endif

#if __SIZEOF_WCHAR_T__ == 2
#define LAST_CHAR 0xffff
#else
#define LAST_CHAR 0xe01ef
#endif

#if ((__GNUC__ == 4 && __GNUC_MINOR__ >= 2) || __GNUC__ > 4)
#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Wunknown-warning-option"
#pragma GCC diagnostic ignored "-Wanalyzer-malloc-leak"
#endif

/* Test every valid char in every picolibc charset */
static int
test_iconv_valid(void)
{
    size_t      e;
    char        locale[64];
    char       *locale_ret;
    static char inbuf[INBUF_SIZE];
    static char utf8buf[INBUF_SIZE * MB_LEN_MAX];
    static char outbuf[INBUF_SIZE * MB_LEN_MAX];
    char       *inptr;
    char       *utf8ptr;
    char       *outptr;
    int         err = 0;
    int         wctomb_ret;
    size_t      incount;
    wchar_t     wc, wc_start;
    size_t      inexact_to_utf;
    size_t      inexact_from_utf;
    iconv_t     to_utf;
    iconv_t     from_utf;
    size_t      tocount;
    size_t      utf8count;
    size_t      utftotal;
    size_t      fromcount;
    size_t      to_utf_ret;
    size_t      from_utf_ret;
    size_t      inbytes_thistime;
    size_t      inbytes_extra;
    size_t      inbytes_left;
    size_t      outbytes_thistime;
    size_t      outbytes_left;

    for (e = 0; e < NENCODING; e++) {

        printf("Encoding %s\n", encodings[e]);

        /*
         * Switch to a locale which uses the desired encoding so
         * we can use wctomb
         */
        if (!strcmp(encodings[e], "ASCII"))
            sprintf(locale, "C");
        else
            sprintf(locale, "C.%s", encodings[e]);
        locale_ret = setlocale(LC_CTYPE, locale);
        if (locale_ret == NULL) {
            printf("setlocale failed for %s\n", locale);
            err = 1;
            continue;
        }

        /* Create the two iconv contexts */
        to_utf = iconv_open(MID_CODE, encodings[e]);
        if (to_utf == (iconv_t)-1) {
            printf("iconv_open(\"%s\", \"%s\") failed: %s\n", MID_CODE, encodings[e],
                   strerror(errno));
            err = 1;
            continue;
        }

        from_utf = iconv_open(encodings[e], MID_CODE);
        if (from_utf == (iconv_t)-1) {
            printf("iconv_open(\"%s\", \"%s\") failed: %s\n", encodings[e], MID_CODE,
                   strerror(errno));
            iconv_close(to_utf);
            err = 1;
            continue;
        }

        incount = 0;
        wc_start = 0;

        /* Fill a buffer in the target encoding with valid chars */
        for (wc = 1;; wc++) {
            wctomb_ret = wctomb(&inbuf[incount], wc);
            if (wctomb_ret <= 0) {
                if (wc != LAST_CHAR)
                    continue;
            } else {
                if (wc_start == 0)
                    wc_start = wc;
                incount += wctomb_ret;
            }

            /* When the buffer is full, convert it all to MID_CODE and back */
            if (sizeof(inbuf) - incount <= (MB_CUR_MAX + 1) || wc == LAST_CHAR) {
                inexact_to_utf = 0;
                tocount = 0;
                utf8count = 0;
                inptr = inbuf;
                utf8ptr = utf8buf;

                /* Reset the conversion state */
                to_utf_ret = iconv(to_utf, NULL, NULL, NULL, NULL);
                if (to_utf_ret == (size_t)-1) {
                    printf("iconv reset failed: %s\n", strerror(errno));
                    err = 1;
                    break;
                }

                /* Convert the whole input buffer */
                while (inptr < &inbuf[incount]) {

                    /*
                     * Provide a limited subset of the input and
                     * output buffers to test boundary conditions
                     */
                    inbytes_thistime = (tocount % 9) + 1;
                    inbytes_extra = 0;
                retry_to_utf:
                    if (inbytes_thistime > (incount - tocount))
                        inbytes_thistime = incount - tocount;
#ifdef __GLIBC__
                    /* glibc returns E2BIG if output overflows, so avoid that */
                    outbytes_thistime = inbytes_thistime * MB_LEN_MAX;
#else
                    /* picolibc handles this correctly */
                    outbytes_thistime = (tocount % 8) + 1;
#endif

                    /* Convert some bytes */
                    inbytes_left = inbytes_thistime;
                    outbytes_left = outbytes_thistime;
                    to_utf_ret = iconv(to_utf, &inptr, &inbytes_left, &utf8ptr, &outbytes_left);
                    /*
                     * When we get an error without converting any input, check to see if we need to
                     * extend the input to build a complete character
                     */
                    if (to_utf_ret == (size_t)-1 && inbytes_left == inbytes_thistime
                        && outbytes_left == outbytes_thistime) {
                        if (errno == EINVAL || errno == EILSEQ) {
                            if (inbytes_thistime < (incount - tocount)
                                && inbytes_extra < MB_LEN_MAX) {
                                inbytes_thistime++;
                                inbytes_extra++;
                                goto retry_to_utf;
                            }
                        }
                        printf("encoding %s(%zu) start %lx. iconv to %s failed: %s\n", encodings[e],
                               e, (unsigned long)wc_start, MID_CODE, strerror(errno));
                        err = 1;
                        break;
                    }
                    if (to_utf_ret != (size_t)-1)
                        inexact_to_utf += to_utf_ret;
                    tocount = tocount + (inbytes_thistime - inbytes_left);
                    utf8count = utf8count + (outbytes_thistime - outbytes_left);
                }
#ifdef __PICOLIBC__
                outbytes_left = MB_LEN_MAX;
                inbytes_left = 0;
                to_utf_ret = iconv(to_utf, &inptr, &inbytes_left, &utf8ptr, &outbytes_left);
                if (to_utf_ret == (size_t)-1) {
                    printf("iconv to %s failed: %s\n", MID_CODE, strerror(errno));
                    err = 1;
                } else {
                    inexact_to_utf += to_utf_ret;
                    utf8count = utf8count + (MB_LEN_MAX - outbytes_left);
                }
                if (inexact_to_utf != 0) {
                    printf("encoding %s(%zu) start %lx. %zd chars inexactly converted to %s\n",
                           encodings[e], e, (unsigned long)wc_start, inexact_to_utf, MID_CODE);
                    err = 1;
                }
#endif

                utftotal = utf8ptr - utf8buf;
                utf8count = 0;
                inexact_from_utf = 0;
                fromcount = 0;
                outptr = outbuf;
                utf8ptr = utf8buf;
                from_utf_ret = iconv(from_utf, NULL, NULL, NULL, NULL);
                if (from_utf_ret == (size_t)-1) {
                    printf("iconv reset failed: %s\n", strerror(errno));
                    err = 1;
                    break;
                }

                /* Convert the whole utf8 buffer back to the target encoding */
                while (utf8ptr < &utf8buf[utftotal]) {
                    inbytes_thistime = (utf8count % 7) + 1;
                    inbytes_extra = 0;
                retry_from_utf:
                    if (inbytes_thistime > utftotal - utf8count)
                        inbytes_thistime = utftotal - utf8count;
#ifdef __GLIBC__
                    /* glibc returns E2BIG if output overflows, so avoid that */
                    outbytes_thistime = inbytes_thistime * MB_LEN_MAX;
#else
                    outbytes_thistime = (utf8count % 6) + 1;
#endif

                    /* Perform the conversion */
                    inbytes_left = inbytes_thistime;
                    outbytes_left = outbytes_thistime;
                    from_utf_ret
                        = iconv(from_utf, &utf8ptr, &inbytes_left, &outptr, &outbytes_left);

                    /*
                     * When we get an error without converting any input, check to see if we need to
                     * extend the input to build a complete character
                     */
                    if (from_utf_ret == (size_t)-1 && inbytes_left == inbytes_thistime
                        && outbytes_left == outbytes_thistime) {
                        if (errno == EINVAL || errno == EILSEQ) {
                            if (inbytes_thistime < utftotal - utf8count
                                && inbytes_extra < MB_LEN_MAX) {
                                inbytes_thistime++;
                                inbytes_extra++;
                                goto retry_from_utf;
                            }
                        }
                        printf("encoding %s(%zu) start %lx. iconv from %s failed: %s\n",
                               encodings[e], e, (unsigned long)wc_start, MID_CODE, strerror(errno));
                        err = 1;
                        break;
                    }
                    if (from_utf_ret != (size_t)-1)
                        inexact_from_utf += from_utf_ret;
                    fromcount = fromcount + (outbytes_thistime - outbytes_left);
                    utf8count = utf8count + (inbytes_thistime - inbytes_left);
                }

#ifdef __PICOLIBC__
                /* picolibc can leave bits stuck in the iconv buffer */
                inbytes_left = 0;
                outbytes_left = MB_LEN_MAX;
                from_utf_ret = iconv(from_utf, &utf8ptr, &inbytes_left, &outptr, &outbytes_left);
                if (from_utf_ret == (size_t)-1) {
                    printf("iconv from %s failed: %s\n", MID_CODE, strerror(errno));
                    err = 1;
                } else {
                    inexact_from_utf += from_utf_ret;
                    fromcount = fromcount + (MB_LEN_MAX - outbytes_left);
                }
                if (inexact_from_utf != 0) {
                    printf("encoding %s(%zu) start %lx. %zd chars inexactly converted from %s\n",
                           encodings[e], e, (unsigned long)wc_start, inexact_from_utf, MID_CODE);
                    err = 1;
                }
#endif

                /* Make sure the round trip is successful */
                if (tocount != fromcount) {
                    printf("encoding %s(%zu) start %lx. converted %zd encoded to %zd %s to %zd "
                           "encoded\n",
                           encodings[e], e, (unsigned long)wc_start, tocount, utf8count, MID_CODE,
                           fromcount);
                    err = 1;
                }
                size_t o;
                for (o = 0; o < tocount && o < fromcount; o++) {
                    if (inbuf[o] != outbuf[o]) {
                        printf("encoding %s(%zu) start %lx. at %zd, input %02x != output %02x\n",
                               encodings[e], e, (unsigned long)wc_start, o, (unsigned char)inbuf[o],
                               (unsigned char)outbuf[o]);
                        err = 1;
                    }
                }
                incount = 0;
                wc_start = 0;
            }
            if (wc == LAST_CHAR)
                break;
        }
        iconv_close(from_utf);
        iconv_close(to_utf);
    }
    return err;
}

int
main(void)
{
    if (test_iconv_valid())
        return 1;
    return 0;
}
