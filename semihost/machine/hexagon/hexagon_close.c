/*
Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
SPDX-License-Identifier: BSD-3-Clause-Clear
*/

#include "hexagon_semihost.h"
#include <unistd.h>

int
close(int fd)
{
    int arg[] = { fd };
    int retval = hexagon_semihost(SYS_CLOSE, arg);
    return retval;
}
