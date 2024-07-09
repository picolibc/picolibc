/*
* SPDX-License-Identifier: BSD-3-Clause
* 
* Copyright © 2024, Synopsys Inc.
* Copyright © 2024, Solid Sands B.V.
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

#ifndef TEST_FILE_NAME
#define TEST_FILE_NAME "DTOXENGINEROUND.TXT"
#endif

int main() {
    int res = 0;
    int ret = 0;
    size_t len = 0;
    char* pchar;
    char line[32];

    FILE *file = NULL;

    file = fopen(TEST_FILE_NAME, "w" );
    if( file == NULL )
        return 0;

    res = fprintf(file, "%.A\n", 24.00);

    fclose(file);

    file = fopen(TEST_FILE_NAME, "r" );
    if( file == NULL )
        return 0;

    const char* ref_out = "0X2P+4\n";
    const int ref_len = 7;
    
    pchar = fgets(line, 32, file);
    ret = strcmp(line, ref_out);
    len = strlen(line);

    if ((res != ref_len) || (ret != 0) || (len != ref_len)) {
        printf("Test Failed: Failed to round the significant figure\n");
    }
    else {
        printf("Test Passed\n");
    }

    return 0;
}