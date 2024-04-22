/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2024 HanaMAshour
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

#include <stdio.h>
#include <string.h>

const char *string = "123456789";

int main(void)
{
    char *pchar;
    char line[12];
    int compare_result;
    FILE *file;
    int ret = 0;

    file = fopen("testfile.dat", "w");
    if(file == NULL) return 1;
    fputs(string, file);
    fclose(file);

    file = fopen( "testfile.dat", "r" );
    if(file == NULL) return 1;

    /*Calling fgets to reach EOF and check on the returned value*/
    pchar = fgets( line, 12, file);
    if(pchar == line)
        printf("\nfgets return is correct\n");
    else {
        printf("\nfgets return is wrong!!\n");
        ret = 1;
    }

    compare_result = strcmp( line, string );
    if(compare_result == 0)
        printf("\ncompare results passed\n");
    else {
        printf("\ncompare results failed!!\n");
        ret = 1;
    }

    fclose( file );

    return ret;
}
