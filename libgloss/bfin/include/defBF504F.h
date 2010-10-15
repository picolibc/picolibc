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

/*
** This include file contains a list of macro "defines" to enable programs
** to use symbolic names for register-access and bit-manipulation for the
** ADSP-BF504F processor.
**
** Copyright (C) 2009 Analog Devices Inc., All Rights Reserved.
*/

#ifndef _DEF_BF504F_H
#define _DEF_BF504F_H

#ifdef _MISRA_RULES
#pragma diag(push)
#pragma diag(suppress:misra_rule_5_1:"ADI header disables rule 5.1 which bars use of long identifiers (>31 chars).")
#endif /* _MISRA_RULES */

/* Include all Core registers and bit definitions */
#include <def_LPBlackfin.h>

/* Include defBF50x_base.h for the set of #defines that are
** common to all ADSP-BF50x processors
*/
#include <defBF50x_base.h>

/* Define the set of macros that are specific to the ADSP-BF504F processor */


/* Flash commands */
#define FLASH_CMD_BLOCK_LOCK_CONFIRM     0x01
#define FLASH_CMD_SET_CONFIG_CONFIRM     0x03
#define FLASH_CMD_ALT_PROGRAM_SETUP      0x10
#define FLASH_CMD_BLOCK_ERASE_SETUP      0x20
#define FLASH_CMD_BLOCK_LOCKDOWN_CONFIRM 0x2F
#define FLASH_CMD_ENH_FACT_PROG_SETUP    0x30
#define FLASH_CMD_DOUBLE_WORD_PROG_SETUP 0x35
#define FLASH_CMD_PROGRAM_SETUP          0x40
#define FLASH_CMD_CLEAR_STATUS           0x50
#define FLASH_CMD_QUAD_WORD_PROG_SETUP   0x56
#define FLASH_CMD_BLOCK_LOCK_SETUP       0x60
#define FLASH_CMD_BLOCK_UNLOCK_SETUP     0x60
#define FLASH_CMD_BLOCK_LOCKDOWN_SETUP   0x60
#define FLASH_CMD_SET_CONFIG_SETUP       0x60
#define FLASH_CMD_READ_STATUS            0x70
#define FLASH_CMD_QUAD_ENH_PROG_SETUP    0x75
#define FLASH_CMD_READ_ELECTRONIC_SIG    0x90
#define FLASH_CMD_READ_CFI_QUERY         0x98
#define FLASH_CMD_PROGRAM_SUSPEND        0xB0
#define FLASH_CMD_ERASE_SUSPEND          0xB0
#define FLASH_CMD_PROTECTION_REG_PROGRAM 0xC0
#define FLASH_CMD_PROGRAM_RESUME         0xD0
#define FLASH_CMD_ERASE_RESUME           0xD0
#define FLASH_CMD_BLOCK_ERASE_CONFIRM    0xD0
#define FLASH_CMD_BLOCK_UNLOCK_CONFIRM   0xD0
#define FLASH_CMD_ENH_FACT_PROG_CONFIRM  0xD0
#define FLASH_CMD_READ_ARRAY             0xFF

/* Bit definitions for Flash Configuration Register */
#define FLASH_CR_ASYNC_READ            0x8000
#define FLASH_CR_XLAT_5                0x2800
#define FLASH_CR_XLAT_4                0x2000
#define FLASH_CR_XLAT_3                0x1800
#define FLASH_CR_XLAT_2                0x1000
#define FLASH_CR_WAIT_HIGH             0x0400
#define FLASH_CR_DATA_HOLD             0x0200
#define FLASH_CR_WAIT_ONE              0x0100
#define FLASH_CR_BURST_SEQ             0x0080
#define FLASH_CR_CLK_RISE              0x0040
#define FLASH_CR_WRAP_DIS              0x0008
#define FLASH_CR_BURST_CONT            0x0007
#define FLASH_CR_BURST_16              0x0003
#define FLASH_CR_BURST_8               0x0002
#define FLASH_CR_BURST_4               0x0001

/* Bit definitions for Flash Status Register */
#define FLASH_SR_PRG_ERASE_READY       0x0080
#define FLASH_SR_ERASE_SUSPENDED       0x0040
#define FLASH_SR_ERASE_ERROR           0x0020
#define FLASH_SR_PRG_ERROR             0x0010
#define FLASH_SR_VPP_INVALID_ABORT     0x0008
#define FLASH_SR_PRG_SUSPENDED         0x0004
#define FLASH_SR_BLK_PROT_ABORT        0x0002
#define FLASH_SR_BNK_WRITE             0x0001

#ifdef _MISRA_RULES
#pragma diag(pop)
#endif /* _MISRA_RULES */

#endif /* _DEF_BF504F_H */
