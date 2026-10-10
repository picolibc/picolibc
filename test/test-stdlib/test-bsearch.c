/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2026 Keith Packard
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

#define _ISOC23_SOURCE
#include <stdlib.h>
#include <stdio.h>

static int
int_compare(const void *key_v, const void *array_v)
{
    int key = *(int *)key_v;
    int array = *(int *)array_v;

    if (key < array)
        return -1;
    if (key > array)
        return 1;
    return 0;
}

#define NVALUES 1075

static int       values[NVALUES];

static const int const_values[] = {
    158,  190,  245,  262,  443,  507,  659,  714,  737,  742,  837,  996,  999,  1014, 1105, 1190,
    1219, 1397, 1509, 1520, 1552, 1581, 1870, 2192, 2229, 2267, 2297, 2299, 2395, 2478, 2614, 2637,
    2690, 2706, 2748, 2754, 2758, 2821, 2830, 2904, 3112, 3142, 3172, 3194, 3223, 3241, 3249, 3260,
    3273, 3283, 3392, 3407, 3618, 3634, 3635, 3649, 3929, 4041, 4076, 4105, 4141, 4157, 4220, 4317,
    4386, 4423, 4772, 4896, 4917, 5190, 5219, 5258, 5462, 5480, 5534, 5884, 5898, 5905, 5933, 6095,
    6114, 6135, 6289, 6347, 6376, 6435, 6460, 6820, 7066, 7108, 7113, 7165, 7210, 7232, 7308, 7500,
    7569, 7572, 7587, 7656, 7700, 7803, 7861, 7890, 8024, 8093, 8119, 8292, 8307, 8367, 8748, 8782,
    8796, 9099, 9099, 9174, 9191, 9237, 9304, 9521, 9582, 9605, 9636, 9914, 9976,
};

#define NCONST (sizeof(const_values) / sizeof(const_values[0]))

int
main(void)
{
    size_t     s;
    int        key;
    int        ret = 0;
    int       *found;
    const int *found_const;

    /* Fill with random data */
    for (s = 0; s < NVALUES; s++)
        values[s] = rand();
    /* Remember our search term */
    key = values[0];
    /* Sort */
    qsort(values, NVALUES, sizeof(int), int_compare);
    /* Verify */
    for (s = 0; s < NVALUES - 1; s++)
        if (values[s] > values[s + 1]) {
            printf("values[%zd] = %d. values[%zd] = %d.\n", s, values[s], s + 1, values[s + 1]);
            ret = 1;
        }
    /* Search */
    found = bsearch(&key, values, NVALUES, sizeof(int), int_compare);
    if (*found != key) {
        printf("found %d, expected %d\n", *found, key);
        ret = 1;
    }

    s = rand() % NCONST;
    key = const_values[s];

    found_const = bsearch(&key, const_values, NCONST, sizeof(int), int_compare);
    if (found_const != &const_values[s]) {
        printf("found %td, expected %zd\n", found - const_values, s);
        ret = 1;
    }
    return ret;
}
