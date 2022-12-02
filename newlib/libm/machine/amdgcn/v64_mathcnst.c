/*
 * Copyright 2023 Siemens
 *
 * The authors hereby grant permission to use, copy, modify, distribute,
 * and license this software and its documentation for any purpose, provided
 * that existing copyright notices are retained in all copies and that this
 * notice is included verbatim in any distributions.  No written agreement,
 * license, or royalty fee is required for any of the authorized uses.
 * Modifications to this software may be copyrighted by their authors
 * and need not follow the licensing terms described here, provided that
 * the new terms are clearly indicated on the first page of each file where
 * they apply.
 */

/*
 * Copyright (c) 1994-2009  Red Hat, Inc. All rights reserved.
 *
 * This copyrighted material is made available to anyone wishing to use,
 * modify, copy, or redistribute it subject to the terms and conditions
 * of the BSD License.   This program is distributed in the hope that
 * it will be useful, but WITHOUT ANY WARRANTY expressed or implied,
 * including the implied warranties of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  A copy of this license is available at
 * http://www.opensource.org/licenses. Any Red Hat trademarks that are
 * incorporated in the source code or documentation are not subject to
 * the BSD License and may only be used or replicated with the express
 * permission of Red Hat, Inc.
 */

/* Based on newlib/libm/mathfp/s_mathcnst.c in Newlib.  */

#include "amdgcnmach.h"

double BIGX = 7.09782712893383973096e+02;
double SMALLX = -7.45133219101941108420e+02;
double z_rooteps = 7.4505859692e-9;
float  z_rooteps_f = 1.7263349182589107e-4;

ufloat z_hugeval_f  = { 0x7f800000 };
ufloat z_infinity_f = { 0x7f800000 };
ufloat z_notanum_f  = { 0x7fd00000 };

#ifdef __IEEE_BIG_ENDIAN
udouble z_hugeval  = { 0x7ff00000, 0 };
udouble z_infinity = { 0x7ff00000, 0 };
udouble z_notanum  = { 0xeff80000, 0 };
#else /* __IEEE_LITTLE_ENDIAN  */
udouble z_hugeval  = { 0, 0x7ff00000 };
udouble z_infinity = { 0, 0x7ff00000 };
udouble z_notanum  = { 0, 0x7ff80000 };
#endif /* __IEEE_LITTLE_ENDIAN */

