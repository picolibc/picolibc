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
 * cplb.h
 *
 * (c) Copyright 2002-2007 Analog Devices, Inc.  All rights reserved.
 *
 ************************************************************************/

/* Defines necessary for cplb initialisation routines. */

#ifndef _CPLB_H
#define _CPLB_H

#include <sys/platform.h>

#ifdef _MISRA_RULES
#pragma diag(push)
#pragma diag(suppress:misra_rule_19_4)
#endif /* _MISRA_RULES */

#define CPLB_ENABLE_ICACHE_P  0
#define CPLB_ENABLE_DCACHE_P  1
#define CPLB_ENABLE_DCACHE2_P 2
#define CPLB_ENABLE_CPLBS_P   3  /* Deprecated! */
#define CPLB_ENABLE_ICPLBS_P  4
#define CPLB_ENABLE_DCPLBS_P  5
#define CPLB_SET_DCBS_P       6
#define CPLB_INVALIDATE_B_P   23

/* ___cplb_ctrl bitmasks */
#define CPLB_ENABLE_ICACHE   (1<<CPLB_ENABLE_ICACHE_P)
#define CPLB_ENABLE_DCACHE   (1<<CPLB_ENABLE_DCACHE_P)
#define CPLB_ENABLE_DCACHE2  (1<<CPLB_ENABLE_DCACHE2_P)
#define CPLB_ENABLE_CPLBS    (1<<CPLB_ENABLE_CPLBS_P)
#define CPLB_ENABLE_ICPLBS   (1<<CPLB_ENABLE_ICPLBS_P)
#define CPLB_ENABLE_DCPLBS   (1<<CPLB_ENABLE_DCPLBS_P)
#define CPLB_ENABLE_ANY_CPLBS  \
           ( CPLB_ENABLE_CPLBS | CPLB_ENABLE_ICPLBS | CPLB_ENABLE_DCPLBS )
#define CPLB_SET_DCBS        (1<<CPLB_SET_DCBS_P)

/* Bitmasks for dcache_invalidate routine parameters */
#define CPLB_INVALIDATE_A  0
#define CPLB_INVALIDATE_B  (1<<CPLB_INVALIDATE_B_P)

/* _cplb_mgr return values */
#define CPLB_RELOADED      0x0000
#define CPLB_NO_UNLOCKED   0x0001
#define CPLB_NO_ADDR_MATCH 0x0002
#define CPLB_PROT_VIOL     0x0003
#define CPLB_NO_ACTION     0x0004

/* CPLB configurations */
#define CPLB_DEF_CACHE_WT  ( CPLB_L1_CHBL | CPLB_WT )
#define CPLB_DEF_CACHE_WB  ( CPLB_L1_CHBL )
#define CPLB_CACHE_ENABLED ( CPLB_L1_CHBL | CPLB_DIRTY )

#define CPLB_DEF_CACHE     ( CPLB_L1_CHBL | CPLB_WT )
#define CPLB_ALL_ACCESS    ( CPLB_SUPV_WR | CPLB_USER_RD | CPLB_USER_WR )

#define CPLB_I_PAGE_MGMT   ( CPLB_LOCK | CPLB_VALID )
#define CPLB_D_PAGE_MGMT   ( CPLB_LOCK | CPLB_ALL_ACCESS | CPLB_VALID )
#define CPLB_DNOCACHE      ( CPLB_ALL_ACCESS | CPLB_VALID )
#define CPLB_DDOCACHE      ( CPLB_DNOCACHE | CPLB_DEF_CACHE )
#define CPLB_INOCACHE      ( CPLB_USER_RD | CPLB_VALID )
#define CPLB_IDOCACHE      ( CPLB_INOCACHE | CPLB_L1_CHBL )

#define CPLB_DDOCACHE_WT   ( CPLB_DNOCACHE | CPLB_DEF_CACHE_WT )
#define CPLB_DDOCACHE_WB   ( CPLB_DNOCACHE | CPLB_DEF_CACHE_WB )

/* Event type parameter for replacement manager _cplb_mgr */
#define CPLB_EVT_ICPLB_MISS   0
#define CPLB_EVT_DCPLB_MISS   1
#define CPLB_EVT_DCPLB_WRITE  2

/* size of cplb tables */
#define __CPLB_TABLE_SIZE    16

#ifdef _MISRA_RULES
#pragma diag(pop)
#endif /* _MISRA_RULES */

#endif /* _CPLB_H */
