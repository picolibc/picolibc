/* Copyright (c) 2013 Red Hat, Inc. All rights reserved.

   This copyrighted material is made available to anyone wishing to use, modify,
   copy, or redistribute it subject to the terms and conditions of the BSD
   License.   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY expressed or implied, including the implied warranties
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  A copy of this license
   is available at http://www.opensource.org/licenses. Any Red Hat trademarks that
   are incorporated in the source code or documentation are not subject to the BSD
   License and may only be used or replicated with the express permission of
   Red Hat, Inc.
*/

#include "memmodel.h"

	.section	.init,"ax"
	call_	#_msp430_run_preinit_array
	call_	#_msp430_run_init_array
	ret_
	.global	__msp430_init_end
__msp430_init_end:

	.section	.fini,"ax"

	ret_
	.global	__msp430_fini_end
__msp430_fini_end:

	.text

