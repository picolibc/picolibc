/* timer.c -- tick timer functions for OpenRISC 1000.
 *
 * Copyright (c) 2011, 2014 Authors
 *
 * Contributor Julius Baxter <juliusbaxter@gmail.com>
 * Contributor Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 *
 * The authors hereby grant permission to use, copy, modify, distribute,
 * and license this software and its documentation for any purpose, provided
 * that existing copyright notices are retained in all copies and that this
 * notice is included verbatim in any distributions. No written agreement,
 * license, or royalty fee is required for any of the authorized uses.
 * Modifications to this software may be copyrighted by their authors
 * and need not follow the licensing terms described here, provided that
 * the new terms are clearly indicated on the first page of each file where
 * they apply.
 */

#include "include/or1k-support.h"
#include "include/or1k-sprs.h"

#include "or1k-internals.h"
#include "board.h"

/* --------------------------------------------------------------------------*/
/*!Tick timer interrupt handler

   Increment timer ticks counter, reload TTMR
                                                                             */
/* --------------------------------------------------------------------------*/
void
_or1k_timer_interrupt_handler(void)
{
	OR1K_REENT.or1k_timer_ticks++;
	uint32_t ttmr = or1k_mfspr(OR1K_SPR_TICK_TTMR_ADDR);
	ttmr = OR1K_SPR_TICK_TTMR_IE_SET(ttmr, 1);
	ttmr = OR1K_SPR_TICK_TTMR_MODE_SET(ttmr, OR1K_SPR_TICK_TTMR_MODE_RESTART);
	ttmr = OR1K_SPR_TICK_TTMR_IE_SET(ttmr, 1);
	or1k_mtspr(OR1K_SPR_TICK_TTMR_ADDR, ttmr);
}

/* --------------------------------------------------------------------------*/
/*!Enable tick timer

   Install handler, calculate TTMR period, reset tick counter

   @param[in] hz     Rate at which to trigger timer ticks                    */
/* --------------------------------------------------------------------------*/
int
or1k_timer_init(unsigned int hz)
{
	uint32_t upr = or1k_mfspr(OR1K_SPR_SYS_UPR_ADDR);
	if (OR1K_SPR_SYS_UPR_TTP_GET(upr) == 0) {
		return -1;
	}

	/* Set this, for easy access when reloading */
	uint32_t period = (_or1k_board_clk_freq/hz) & OR1K_SPR_TICK_TTMR_TP_MASK;
	OR1K_REENT.or1k_timer_period = period;
	or1k_mtspr(OR1K_SPR_TICK_TTMR_ADDR, period);

	/* Reset timer tick counter */
	OR1K_REENT.or1k_timer_ticks = 0;

	/* Install handler */
	or1k_exception_handler_add(0x5, _or1k_timer_interrupt_handler);
	OR1K_REENT.or1k_timer_mode = OR1K_SPR_TICK_TTMR_MODE_RESTART;

	/* Reset counter register */
	or1k_mtspr(OR1K_SPR_TICK_TTCR_ADDR, 0);

	return 0;
}

void
or1k_timer_set_period(uint32_t hz)
{
	uint32_t period = (_or1k_board_clk_freq/hz) & OR1K_SPR_TICK_TTMR_TP_MASK;
	uint32_t ttmr = or1k_mfspr(OR1K_SPR_TICK_TTMR_ADDR);
	ttmr = OR1K_SPR_TICK_TTMR_TP_SET(ttmr, period);
	or1k_mtspr(OR1K_SPR_TICK_TTMR_ADDR, ttmr);
	OR1K_REENT.or1k_timer_period = period;
}

void
or1k_timer_set_handler(void (*handler)(void))
{
	or1k_exception_handler_add(0x5, handler);
}

void
or1k_timer_set_mode(uint32_t mode)
{
	// Store mode in variable
	OR1K_REENT.or1k_timer_mode = mode;

	uint32_t ttmr = or1k_mfspr(OR1K_SPR_TICK_TTMR_ADDR);
	// If the timer is currently running, we also change the mode
	if (OR1K_SPR_TICK_TTMR_MODE_GET(ttmr) != 0) {
		ttmr = OR1K_SPR_TICK_TTMR_MODE_SET(ttmr, mode);
		or1k_mtspr(OR1K_SPR_TICK_TTMR_ADDR, ttmr);
	}
}

/* --------------------------------------------------------------------------*/
/*!Enable tick timer

   Enable timer interrupt, install handler, load TTMR
                                                                             */
/* --------------------------------------------------------------------------*/
void
or1k_timer_enable(void)
{
	uint32_t ttmr = or1k_mfspr(OR1K_SPR_TICK_TTMR_ADDR);
	ttmr = OR1K_SPR_TICK_TTMR_IE_SET(ttmr, 1);
	ttmr = OR1K_SPR_TICK_TTMR_MODE_SET(ttmr, OR1K_REENT.or1k_timer_mode);
	or1k_mtspr(OR1K_SPR_TICK_TTMR_ADDR, ttmr);

	uint32_t sr = or1k_mfspr(OR1K_SPR_SYS_SR_ADDR);
	sr = OR1K_SPR_SYS_SR_TEE_SET(sr, 1);
	or1k_mtspr(OR1K_SPR_SYS_SR_ADDR, sr);
}

/* --------------------------------------------------------------------------*/
/*!Disable tick timer

   Disable timer interrupt in SR
                                                                             */
/* --------------------------------------------------------------------------*/
uint32_t
or1k_timer_disable(void)
{
	uint32_t oldsr = or1k_mfspr(OR1K_SPR_SYS_SR_ADDR);
	uint32_t sr = OR1K_SPR_SYS_SR_TEE_SET(oldsr, 0);
	or1k_mtspr(OR1K_SPR_SYS_SR_ADDR, sr);
	return OR1K_SPR_SYS_SR_TEE_GET(oldsr);
}

void
or1k_timer_restore(uint32_t sr_tee)
{
	uint32_t sr = or1k_mfspr(OR1K_SPR_SYS_SR_ADDR);
	sr = OR1K_SPR_SYS_SR_TEE_SET(sr, 1);
	or1k_mtspr(OR1K_SPR_SYS_SR_ADDR, sr);
}

void
or1k_timer_pause(void)
{
	uint32_t ttmr = or1k_mfspr(OR1K_SPR_TICK_TTMR_ADDR);
	ttmr = OR1K_SPR_TICK_TTMR_MODE_SET(ttmr, OR1K_SPR_TICK_TTMR_MODE_DISABLE);
	or1k_mtspr(OR1K_SPR_TICK_TTMR_ADDR, ttmr);
}

void
or1k_timer_reset(void)
{
	uint32_t ttmr = or1k_mfspr(OR1K_SPR_TICK_TTMR_ADDR);
	ttmr = OR1K_SPR_TICK_TTMR_IP_SET(ttmr, 0);
	or1k_mtspr(OR1K_SPR_TICK_TTMR_ADDR, ttmr);
	or1k_mtspr(OR1K_SPR_TICK_TTCR_ADDR, 0);
}

/* --------------------------------------------------------------------------*/
/*!Get tick timer

   Return value of tick timer
                                                                             */
/* --------------------------------------------------------------------------*/
unsigned long
or1k_timer_get_ticks(void)
{
	return OR1K_REENT.or1k_timer_ticks;
}

/* --------------------------------------------------------------------------*/
/*!Reset tick timer

   Clear value of tick timer
                                                                             */
/* --------------------------------------------------------------------------*/
void
or1k_timer_reset_ticks(void)
{
	OR1K_REENT.or1k_timer_ticks = 0;
}
