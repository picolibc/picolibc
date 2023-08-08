/*
 * Copyright (C) 2023 Espressif Systems (Shanghai) CO LTD
 * SPDX-License-Identifier: BSD-2-Clause
 */

/* crt0.S
 * Dummy startup file which calls main() function.
 * It can be used for compilation tests only.
 */

	.text

	/*
	 *  Simple jump to main().
	 */

	.global	_start
	_start:
	movi	a4, main
	jx	a4		/* jump to main */
