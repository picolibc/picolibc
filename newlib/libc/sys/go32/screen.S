/* This is file SCREEN.S */
/*
** Copyright (C) 1993 DJ Delorie
**
** This file is distributed under the terms listed in the document
** "copying.dj".
** A copy of "copying.dj" should accompany this file; if not, a copy
** should be available from where this file was obtained.  This file
** may not be distributed without a verbatim copy of "copying.dj".
**
** This file is distributed WITHOUT ANY WARRANTY; without even the implied
** warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
**
** Modified by J. Alan Eldridge, Liberty Brokerage, 77 Water St, NYC 10005
**
** added global char ScreenAttrib -- this is the attribute
** used by ScreenClear(): it defaults to 0x07 so as not to
** break existing code.
**
** Modified by C. Sandmann (sandmann@clio.rice.edu) for DPMI support
** Combined SCREEN2.S and SCREEN.C into SCREEN.S
*/

	.data

	.globl	_ScreenAttrib
_ScreenAttrib:
	.byte 0x07

	.text

	.align 2
	.globl	_ScreenSetCursor
		/* row, col */
_ScreenSetCursor:
	pushl	%ebp
	movl	%esp,%ebp
	pushl	%ebx
	pushl	%esi
	pushl	%edi
	movb	$0x02,%ah
	movb	$0,%bh
	movb	8(%ebp),%dh
	movb	12(%ebp),%dl
	int	$0x10
	popl	%edi
	popl	%esi
	popl	%ebx
	popl	%ebp
	ret

	.align 2
	.globl	_ScreenGetCursor
		 /* &row, &col */
_ScreenGetCursor:
	pushl	%ebp
	movl	%esp,%ebp
	pushl	%ebx
	pushl	%esi
	pushl	%edi
	movb	$0x03,%ah
	movb	$0,%bh
	pushl	%ebp
	int	$0x10
	popl	%ebp
	movl	8(%ebp),%esi
	movzbl	%dh,%eax
	movl	%eax,(%esi)
	movl	12(%ebp),%esi
	movzbl	%dl,%eax
	movl	%eax,(%esi)
	popl	%edi
	popl	%esi
	popl	%ebx
	popl	%ebp
	ret

	.align 2
	.globl	_ScreenClear
_ScreenClear:
	pushl	%edi
	call	_ScreenCols
	movl	%eax,%ecx
	call	_ScreenRows
		/* ECX is total words to store */
	imull	%eax,%ecx
	movl	_ScreenPrimary,%eax
	call	dosmemsetup
	movl	%eax,%edi
	push	%es
	push	%gs
	pop	%es
	movb	$0x20,%al
	movb	_ScreenAttrib,%ah
	rep
	stosw
	pop	%es
	popl	%edi
	ret
	
	.align 2
	.globl _ScreenRows
_ScreenRows:
	movl	$0x484,%eax
	call	dosmemsetup
	movzbl	%gs:(%eax),%eax
	incl	%eax
	ret

	.align 2
	.globl _ScreenCols
_ScreenCols:
	movl	$0x44a,%eax
	call	dosmemsetup
	movzwl	%gs:(%eax),%eax
	ret

	.align 2
	.globl _ScreenMode
_ScreenMode:
	movl	$0x449,%eax
	call	dosmemsetup
	movzbl	%gs:(%eax),%eax
	ret

	.align 2
	.globl _ScreenPutChar
_ScreenPutChar:
		/* int ch, int attr, unsigned x, unsigned y */
/*
  if ( (x >= ScreenCols()) || (y >= ScreenRows())) return;
  ch &= 0xff;
  attr = (attr & 0xff) << 8;
  *(unsigned short *)(ScreenPrimary+x+y*ScreenCols()) = ch | attr;
*/
	call	_ScreenRows
	cmpl	%eax,16(%esp)
	jae	L1
	call	_ScreenCols
	cmpl	%eax,12(%esp)
	jae	L1
				/* Out of range */
	movl	%eax,%edx
	imull	16(%esp),%edx
	addl	12(%esp),%edx
			/* EDX = x + y*ScreenCols */

	movb	4(%esp),%cl
	movb	8(%esp),%ch

	movl	_ScreenPrimary,%eax
	call	dosmemsetup
	movw	%cx,%gs:(%eax,%edx,2)	
L1:
	ret	

/* A quick way to update the screen from a logical video buffer, used 
   primarily for DPMI full screen management */
	.align 2
	.globl _ScreenUpdate
		/* (void *)screenbuf */
_ScreenUpdate:
	call	_ScreenRows
	movl	%eax,%ecx
	call	_ScreenCols
	imull	%eax,%ecx
	sarl	$1,%ecx
			/* Number of long words in screen */

	movl	_ScreenPrimary,%eax
	call	dosmemsetup
	movl	4(%esp),%edx
		/* screenbuf */

	pushl	%esi
	pushl	%edi
	movl	%eax,%edi
	movl	%edx,%esi
	push	%es
	push	%gs
	pop	%es
	rep
	movsl
					/* move ECX bytes to Real area */
	pop	%es
	popl	%edi
	popl	%esi
	ret

/* A quick way to update a screen line from a logical video buffer, used 
   primarily for DPMI full screen management */
	.align 2
	.globl _ScreenUpdateLine
		/* (void *)screenline, int row */
_ScreenUpdateLine:
	call	_ScreenCols
	movl	%eax,%ecx
	sarl	$1,%ecx
			/* Number of long words in screen line */
	shll	$1,%eax
	imull	8(%esp),%eax
	addl	_ScreenPrimary,%eax
	call	dosmemsetup
	movl	4(%esp),%edx
		/* screenbuf */

	pushl	%esi
	pushl	%edi
	movl	%eax,%edi
	movl	%edx,%esi
	push	%es
	push	%gs
	pop	%es
	rep
	movsl
					/* move ECX bytes to Real area */
	pop	%es
	popl	%edi
	popl	%esi
	ret


/* A quick way to update the screen from a logical video buffer, used 
   primarily for DPMI full screen management */
	.align 2
	.globl _ScreenRetrieve
		/* (void *)screenbuf */
_ScreenRetrieve:
	call	_ScreenRows
	movl	%eax,%ecx
	call	_ScreenCols
	imull	%eax,%ecx
	sarl	$1,%ecx
			/* Number of long words in screen */

	movl	_ScreenPrimary,%eax
	call	dosmemsetup
	movl	4(%esp),%edx
		/* screenbuf */

	pushl	%esi
	pushl	%edi
	movl	%eax,%esi
	movl	%edx,%edi
	push	%ds
	push	%gs
	pop	%ds
	rep
	movsl
					/* move ECX bytes to Real area */
	pop	%ds
	popl	%edi
	popl	%esi
	ret


