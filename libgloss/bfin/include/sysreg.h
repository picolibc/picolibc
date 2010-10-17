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

/* This file must be used with compiler version 8.0.8.1 */

#ifdef __VERSIONNUM__
#if __VERSIONNUM__ != 0x08000801
#error The compiler version does not match the version of the sysreg.h include
#endif
#endif

/************************************************************************
 *
 * sysreg.h
 *
 * (c) Copyright 2001-2006 Analog Devices, Inc.  All rights reserved.
 *
 ***********************************************************************/

#pragma once
#ifndef __NO_BUILTIN
#pragma system_header /* sysreg.h */
#endif

/* sysreg definitions for use in sysreg_read and sysreg_write calls. */

#ifndef _SYSREG_H
#define _SYSREG_H

#ifdef _MISRA_RULES
#pragma diag(push)
#pragma diag(suppress:misra_rule_2_4)
#pragma diag(suppress:misra_rule_6_3)
#pragma diag(suppress:misra_rule_19_4)
#pragma diag(suppress:misra_rule_19_7)
#pragma diag(suppress:misra_rule_19_10)
#endif /* _MISRA_RULES */

enum {
  /* the following can be used in word-sized sysreg reads and writes */
  reg_I0,
  reg_I1,
  reg_I2,
  reg_I3,
  reg_M0,
  reg_M1,
  reg_M2,
  reg_M3,
  reg_B0,
  reg_B1,
  reg_B2,
  reg_B3,
  reg_L0,
  reg_L1,
  reg_L2,
  reg_L3,
  reg_LC0,
  reg_LC1,
  reg_LT0,
  reg_LT1,
  reg_LB0,
  reg_LB1,
  reg_RETS,
  reg_RETI,
  reg_RETX,
  reg_RETN,
  reg_RETE,
  reg_SEQSTAT,
  reg_SYSCFG,
  reg_CYCLES,
  reg_CYCLES2,
  reg_A0W,
  reg_A0X,
  reg_A1W,
  reg_A1X,
  reg_FP,
  reg_SP,
  reg_ASTAT,

  /* the following can be used in double-word sysreg reads and writes */
  reg_A0,
  reg_A1,
  __num_SysRegs
};

#define STACKPOINTER reg_SP
#define FRAMEPOINTER reg_FP

#ifdef _MISRA_RULES
#pragma diag(pop)
#endif /* _MISRA_RULES */

#endif /* _SYSREG_H */
