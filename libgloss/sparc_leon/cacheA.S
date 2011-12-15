/*
 * Copyright (c) 2011 Aeroflex Gaisler
 *
 * BSD license:
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */


#include <asm-leon/leon.h>
#include <asm-leon/leonstack.h>
        
        .seg    "data"
	.global sparc_leon23_cache_flush, sparc_leon23_icache_flush, sparc_leon23_dcache_flush
	
	.global _leon_version
	
	.seg    "text"

/* =============================================== */
	
/* use only %o7 */	
sparc_leon23_icache_flush:
sparc_leon3_icache_flush:
	retl
sparc_leon23_cache_flush:	
sparc_leon3_cache_flush:
	sta %g0, [%g0] ASI_LEON3_IFLUSH
sparc_leon23_dcache_flush:	
sparc_leon3_dcache_flush:
	retl
	 sta %g0, [%g0] ASI_LEON3_DFLUSH


	
