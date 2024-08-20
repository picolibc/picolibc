/*
 * arc-specific.h -- provide ARC-specific definitions
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

#ifndef _ARC_SPECIFIC_H
#define _ARC_SPECIFIC_H

/* First check for MetaWare compiler as it also defines __GNUC__.  */
#if defined (__CCAC__)
	#define read_aux_reg(r)		_lr(r)
	#define write_aux_reg(r, v)	_sr((unsigned int)(v), r)
#elif defined (__GNUC__)
	#define read_aux_reg(r)		__builtin_arc_lr(r)
	#define write_aux_reg(v, r)	__builtin_arc_sr((unsigned int)(v), r)
#else
	#error "Unexpected compiler"
#endif

#endif /* _ARC_SPECIFIC_H */
