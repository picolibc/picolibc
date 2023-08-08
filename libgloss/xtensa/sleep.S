/*
 * Copyright (C) 2023 Espressif Systems (Shanghai) CO LTD
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <soc/cpu.h>

#define USECONDS_IN_SECOND 1000000

	.type	sleep, @function
	.align	4
	.global sleep
sleep:
	entry	a1, 0x30
	mov.n	a7, a1
	s32i.n	a2, a7, 0

	mov	a3, a2
.Lsleep_delay_second:
	addi	a3, a3, -1
	movi	a10, USECONDS_IN_SECOND
	call8	usleep
	bnez	a3, .Lsleep_delay_second

.Lsleep_exit:
	s32i.n	a2, a7, 0
	retw


	.type	usleep, @function
	.align	4
	.global usleep
usleep:
	entry	a1, 0x30
	mov.n	a7, a1
	s32i.n	a2, a7, 0

	/* convert time to CLOCK ticks per 1us */
	movi	a6, CPU_FREQUENCY_MHZ // CPU_FREQUENCY_MHZ == ticks per us
	mull	a2, a2, a6

	/*
	 * Registers purpose:
	 *   a3 - start CCOUNT value
	 *   a4 - current CCOUNT value
	 *   a5 - target CCOUNT value
	 */

	rsr	a3, CCOUNT
	add	a5, a3, a2
	bgeu	a5, a3, .Lusleep_waitloop

.Lusleep_waitoverflow:
	/* you are here because a5 (target CCOUNT) was overflown
	 * it means that we need to wait CCOUNT register overflow
	 * before do main loop
	 */
	rsr	a4, CCOUNT
	bgeu	a4, a3, .Lusleep_waitoverflow

.Lusleep_waitloop:
	/* wait until we reach target CCOUNT value */
	rsr	a4, CCOUNT
	bgeu	a4, a5, .Lusleep_exit
	j	.Lusleep_waitloop
.Lusleep_exit:
	s32i.n	a2, a7, 0
	retw

