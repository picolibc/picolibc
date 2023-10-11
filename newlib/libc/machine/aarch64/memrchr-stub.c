/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (C) 2023 embedded brains GmbH & Co. KG
 */

#if defined (__OPTIMIZE_SIZE__) || defined (PREFER_SIZE_OVER_SPEED)
#include "../../string/memrchr.c"
#else
/* See memrchr.S */
#endif
