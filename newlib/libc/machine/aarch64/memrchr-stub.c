/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (C) 2023 embedded brains GmbH & Co. KG
 */

#include <picolibc.h>

#if (defined (__OPTIMIZE_SIZE__) || defined (__PREFER_SIZE_OVER_SPEED)) || !defined(__LP64__) || !defined(__ARM_NEON)
#include "../../string/memrchr.c"
#else
/* See memrchr.S */
#endif
