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

#pragma once
#ifndef __NO_BUILTIN
#pragma system_header /* cplbtab.h */
#endif
/************************************************************************
 *
 * cplbtab.h
 *
 * Copyright (C) 2008 Analog Devices, Inc.
 *
 ************************************************************************/

/* Define structures for the CPLB tables. */

#ifndef _CPLBTAB_H
#define _CPLBTAB_H

#include <cplb.h>

#ifdef _MISRA_RULES
#pragma diag(push)
#pragma diag(suppress:misra_rule_6_3)
#pragma diag(suppress:misra_rule_8_12)
#endif /* _MISRA_RULES */

typedef struct {
  unsigned long addr;
  unsigned long flags;
} cplb_entry;

extern cplb_entry dcplbs_table[];
extern cplb_entry icplbs_table[];
extern int __cplb_ctrl;

#ifdef __cplusplus
 extern "C" {
#endif

void cplb_init(int _enable_cpls_caches);
int cplb_mgr(int _is_data_miss, int _enable_cache);
void cplb_hdr(void);
void cache_invalidate(int _caches);
void icache_invalidate(void);
void dcache_invalidate(int _caches);
void dcache_invalidate_both(void);
void flush_data_cache(void);
void flush_data_buffer(void *_start, void *_end, int _invalidate);
void disable_data_cache(void);
void enable_data_cache(int _cplb_ctrl);

#ifdef __cplusplus
 }
#endif

#ifdef _MISRA_RULES
#pragma diag(pop)
#endif /* _MISRA_RULES */

#endif /* _CPLBTAB_H */

