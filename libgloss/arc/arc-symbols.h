/*
 * arc-symbols.h -- provide ARC-specific symbols
 *
 * Copyright (c) 2024 Synopsys Inc.
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
 *
 */

#ifndef _ARC_SYMBOLS_H
#define _ARC_SYMBOLS_H

/* First check for MetaWare compiler as it also defines __GNUC__.  */
#if defined (__CCAC__)
	#define STACK_TOP		_estack
	#define SMALL_DATA_BASE		_SDA_BASE_
	#define SMALL_DATA_BSS_START	_fsbss
	#define SMALL_DATA_BSS_END	_esbss
	#define START_HEAP		_fheap
	#define END_HEAP		_eheap
#elif defined (__GNUC__)
	#define STACK_TOP		__stack_top
	#define SMALL_DATA_BASE		__SDATA_BEGIN__
	#define SMALL_DATA_BSS_START	__sbss_start
	#define SMALL_DATA_BSS_END	_end
	#define START_HEAP		__start_heap
	#define END_HEAP		__end_heap
#else
	#error "Unexpected compiler"
#endif

#endif /* _ARC_SYMBOLS_H */