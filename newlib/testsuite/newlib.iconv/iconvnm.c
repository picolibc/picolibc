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
#include <stdio.h>
#include <stdlib.h>
#include <iconv.h>
#include <errno.h>
#include "check.h"

static const char * const good_names[] = {
    "us_ascii", "US_ASCII", "us-ASCII", "US-ASCII",
#ifdef __MB_CAPABLE
    "utf_8", "UTF_8", "uTf-8", "UTF-8",
#endif
#ifdef MB_EXTENDED_CHARSETS_ISO
    "iso_8859_5", "iso-8859-5", "iso-8859_5", "IsO-8859_5",
#endif
#ifdef __MB_EXTENDED_CHARSETS_JIS
    "euc-jp", "EUC_JP", "euc-JP", "EUC-JP",
#endif
};

static const char * const bad_names[] =
{" ", "iso", "8", "iso_8859_5 ", " iso_8859_5", "csisolatincyrillic ",
 " csisolatincyrillic", "euc-", "p", "euc_jp ", "euc-jp-",
 "us_as", "us_", "us_ascii ", " us_ascii",
 "CCCP", "", "-1", "-", "_", "---", "___", "-_-_-", "_-_-_", NULL};


int main(void)
{
    unsigned i, failed = 0;
    iconv_t cd;

    puts("iconv names test");

    for (i = 0; i < sizeof(good_names)/sizeof(char *); i++)
    {
        printf("Trying iconv(%s, %s)", good_names[0], good_names[i]);
        fflush(stdout);

        cd = iconv_open(good_names[0], good_names[i]);

        if (cd == (iconv_t)-1)
        {
            puts(" ... FAILED");
            failed += 1;
        }
        else
        {
            puts(" ... PASSED");
            CHECK(iconv_close(cd) != -1);
        }
    }

    for (i = 0; i < sizeof(bad_names)/sizeof(char *); i++)
    {
        printf("Trying iconv(%s, \"%s\")", good_names[0], bad_names[i]);
        fflush(stdout);

        cd = iconv_open(good_names[0], bad_names[i]);

        if (cd != (iconv_t)-1)
        {
            puts(" ... FAILED");
            failed += 1;
        }
        else
            puts(" ... PASSED");
    }

    if (failed)
    {
        printf("%d FAILTURES\n", failed);
        abort();
    }

    exit(0);
}
