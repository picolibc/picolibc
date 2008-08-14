/*
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

/************************************************************************
 *
 * pll.h
 *
 * Copyright (C) 2008 Analog Devices, Inc.
 *
 ************************************************************************/

#ifdef __ASSEMBLY__
#pragma once
#pragma system_header
#endif

#ifndef _PLL_H
#define _PLL_H

#ifdef _MISRA_RULES
#pragma diag(push)
#pragma diag(suppress:misra_rule_6_3)
#endif /* _MISRA_RULES */

#define NO_STARTUP_SET 0
#define MAX_IN_STARTUP 1

#ifdef __ASSEMBLY__

enum clkctrl_t {
    /* no modification of PLL rates in CRT startup - default */
   no_startup_set=NO_STARTUP_SET,

    /* CRT startup sets PLL rates to suitable maximum values */
   max_in_startup=MAX_IN_STARTUP
};

/*
** Define __clk_ctrl to 1 to cause startup to set PLL rates for maximum
** speed performance rates.  The default version defined in the runtime-
** libraries defines __clk_ctrl to 0 which disables the feature.
*/
extern enum clkctrl_t __clk_ctrl;

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__ADSPLPBLACKFIN__)

/* Sets SSEL and CSEL bits in PLL_DIV to passed values.
** Returns -1 on failure.
*/
int pll_set_system_clocks(int _csel, int _ssel);

/*
** Sets MSEL and DF bits in PLL_CTL and LOCKCNT in PLL_LOCKCNT.
** Returns -1 on failure.
*/
int pll_set_system_vco(int _msel, int _df, int _lockcnt);

#endif /* __ADSPLPBLACKFIN__ */

#ifdef __cplusplus
}
#endif

#endif /* __ASSEMBLY__ */

#ifdef _MISRA_RULES
#pragma diag(pop)
#endif /* _MISRA_RULES */

#endif /* _PLL_H */

