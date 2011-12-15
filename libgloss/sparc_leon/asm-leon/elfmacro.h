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


#ifndef _INCLUDE_LEON_ELFMACRO_h
#define _INCLUDE_LEON_ELFMACRO_h

#ifdef __ASSEMBLER__
#define _TEXT_SEG_ALIGN 4
#define _LIBLEONBARE_TEXT_SEG_START \
        .text ; .balign _TEXT_SEG_ALIGN
#define	FUNC_BEGIN(func)	func:
#define	FUNC_END(func)		.size	func, . - func

#define GTEXT(sym) sym  ;  .type   sym,@function
#define GDATA(sym) sym  ;  .type   sym,@object

#define	FUNC_EXPORT(func)	.globl	GTEXT(func)
#define	DATA_EXPORT(var)	.globl	GDATA(var)

#define	FUNC_IMPORT(func)	.extern	FUNC(func)
#define	DATA_IMPORT(var)	.extern	var
#endif

#ifndef weak_alias
/* Define ALIASNAME as a weak alias for NAME. */
#  define weak_alias(name, aliasname) _weak_alias (name, aliasname)
#  define _weak_alias(name, aliasname) \
      extern __typeof (name) aliasname __attribute__ ((weak, alias (#name)));
#endif

#ifndef strong_alias
/* Define ALIASNAME as a strong alias for NAME.  */
# define strong_alias(name, aliasname) _strong_alias(name, aliasname)
# define _strong_alias(name, aliasname) \
  extern __typeof (name) aliasname __attribute__ ((alias (#name)));
#endif

#ifndef __ASSEMBLER__
typedef int (*initcall_t) (void);
extern initcall_t __leonbare_initcall_start;
extern initcall_t __leonbare_initcall_end;

#endif

#if __GNUC_MINOR__ >= 3
# define __attribute_used__	__attribute__((__used__))
#else
# define __attribute_used__	__attribute__((__unused__))
#endif

#define __define_initcall(level,fn) \
	static initcall_t __initcall_##fn __attribute_used__ \
	__attribute__((__section__(".initcall" level ".init"))) = fn

#define libc_initcall(fn)		__define_initcall("1",fn)

#endif /* !_INCLUDE_LEON_STACK_h */
