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
#include <math.h>

static int compare_floats(float a, float b, float tolerance) {
    float diff = a - b;
    if (diff < 0)
        diff = -diff;
    return diff <= tolerance;
}

int main(void) {
    int res = -1;
    int ret1 = 0;
    int ret2 = 0;
    float var1 = 0.0F;
    float var2 = 0.0F;
    float tolerance = 0.0F;
    const float ref_out1 = 8.5F;
    const float ref_out2 = 5.5F;
    char strin[50];

    res = sprintf(strin, "0x1.1p+3 0x1.6p+2 ");

    /* Test Case 1 for %a */
    {
        res = sscanf(strin, "%a %a ", &var1, &var2);
        ret1 = compare_floats(var1, ref_out1, tolerance);
        ret2 = compare_floats(var2, ref_out2, tolerance);
        
        if ((res != 2) || (ret1 == 0) || (ret2 == 0)) {
            printf("Test Case 1 Failed: Failed to read the input string correctly using %%a\n");
            return 1;
        }
        else {
            printf("Test Case 1 Passed: %%a is handled correctly\n");
        }
    }

    /* Test Case 2 for %A */
    {
        res = sscanf(strin, "%A %A ", &var1, &var2);
        ret1 = compare_floats(var1, ref_out1, tolerance);
        ret2 = compare_floats(var2, ref_out2, tolerance);

        if ((res != 2) || (ret1 == 0) || (ret2 == 0)) {
            printf("Test Case 2 Failed: Failed to read the input string correctly using %%A\n");
            return 1;
        } 
        else {
            printf("Test Case 2 Passed: %%A is handled correctly\n");
        }  
    }

    return 0;
}
