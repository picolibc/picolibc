/*
Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
SPDX-License-Identifier: BSD-3-Clause-Clear
*/

#include "hexagon_semihost.h"

int
sys_get_cmdline(char *buffer, int count)
{
    /* read a buffer from file */
    int args[] = { (int)buffer, count };
    int retval = hexagon_semihost(SYS_GET_CMDLINE, args);
    return retval;
}
