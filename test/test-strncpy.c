/*
 * Copyright (c) 2025 Fredrik Gihl
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

static int testStr(char *dest, const char *src)
{
    char* tempStr = dest;
    size_t str_len;
    int ret = 1;

    strncpy(tempStr, src, 49);
    if(strcmp(tempStr, src) != 0)
    {
        printf("strncpy failed(len=%zd). String: %s, expected: %s\n",
               strlen(src),
               tempStr,
               src);
        ret = 0;
    }
    str_len = strlen(src);
    if (str_len > 49)
    {
        printf("strncpy wrote %zd to buffer 49 bytes long\n", str_len);
        ret = 0;
    }
    while (str_len < 49)
    {
        if (tempStr[str_len] != '\0')
        {
            printf("strncpy didn't clear byte at %zd\n", str_len);
            ret = 0;
        }
        str_len++;
    }
    return ret;
}

static char *fillStr(size_t len, char *src, char val)
{
    for (size_t i = 0; i < len; i++)
    {
        src[i] = val + (i % 10);
    }
    src[len] = '\0';
    return (char*)src;
}

// This function test the strncpy function so it copy a string correct.
// We copy string from 5 bytes to 40bytes, and verify that the string is copied correct.
static int test_strncpy(void)
{
    char str[100];
    int ret = 1;

    char src[49];

    for (size_t strLen = 9; strLen < 40; strLen++)
    {
        fillStr(99, str, 'a');
        if (!testStr(str, fillStr(strLen, src, '0')))
            ret = 0;
    }
    return ret;
}


int main(void)
{
    if (!test_strncpy())
        return 1;
    return 0;
}
