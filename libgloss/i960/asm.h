#ifndef ASM_H
#define ASM_H

#define _C_LABEL(x)    _ ## x
#define _ASM_LABEL(x)   x

#define _ENTRY(name)	\
	.text; .align 4; .globl name; name:

#define ENTRY(name)	\
	_ENTRY(_C_LABEL(name))

#endif
