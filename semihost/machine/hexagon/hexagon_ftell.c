/*
Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
SPDX-License-Identifier: BSD-3-Clause-Clear
*/

#include "hexagon_semihost.h"

int
hexagon_ftell(int fd)
{
    int args[] = { fd };
    int retval = hexagon_semihost(SYS_FTELL, args);
    return retval;
}
