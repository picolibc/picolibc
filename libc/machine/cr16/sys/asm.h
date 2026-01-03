/* asm.h -- CR16 architecture intrinsic functions
 *
 * Copyright (c) 2012 National Semiconductor Corporation
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
 */

#ifndef	_ASM
#define _ASM

/* Note that immediate input values are not checked for validity. It is 
   the user's responsibility to use the intrinsic functions with appropriate
   immediate values. */

/* Addition Instructions */
#define _addb_(src, dest)	__asm__("addb %1, %0" : "=r" (dest) : \
					"ri" ((unsigned char)src), "0" (dest) : "cc")
#define _addub_(src, dest)	__asm__("addub	%1, %0" : "=r" (dest) : \
					"ri" ((unsigned char)src), "0" (dest) : "cc")
#define _addw_(src, dest)	__asm__("addw %1, %0" : "=r" (dest) : \
					"ri" ((unsigned short)src), "0" (dest) : "cc")
#define _adduw_(src, dest)	__asm__("adduw	%1, %0" : "=r" (dest) : \
					"ri" ((unsigned short)src), "0" (dest) : "cc")
#define _addd_(src, dest)	__asm__("addd %1, %0" : "=r" (dest) : \
					"ri" ((unsigned long)src), "0" (dest) : "cc")
					
/* Add with Carry */
#define _addcb_(src, dest)	__asm__("addcb	%1, %0" : "=r" (dest) : \
					"ri" ((unsigned char)src), "0" (dest) : "cc")
#define _addcw_(src, dest)	__asm__("addcw	%1, %0" : "=r" (dest) : \
					"ri" ((unsigned short)src), "0" (dest) : "cc")
					
/* Bitwise Logical AND */
#define _andb_(src, dest)	__asm__("andb %1,%0" : "=r" (dest) : \
					"ri" ((unsigned char)src) , "0" (dest))
#define _andw_(src, dest)	__asm__("andw %1,%0" : "=r" (dest) : \
					"ri" ((unsigned short)src) , "0" (dest))
#define _andd_(src, dest)	__asm__("andd %1,%0" : "=r" (dest) : \
		 			"ri" ((unsigned long)src) , "0" (dest))

/* Arithmetic shift Instructions */								
#define _ashub_(count, dest)	__asm__("ashub %1,%0" : "=r" (dest) : \
					"ri" ((char)count) , "0" (dest) )
#define _ashuw_(count, dest)	__asm__("ashuw %1,%0" : "=r" (dest) : \
					"ri" ((char)count) , "0" (dest) )
#define _ashud_(count, dest)	__asm__("ashud %1,%0" : "=r" (dest) : \
					"ri" ((char)count) , "0" (dest) )

/* cbit (clear bit) Instructions */
#define _cbitb_(pos, dest) 	__asm__("cbitb %1,%0" : "=mr" (dest) : \
					"i" ((unsigned char)pos) , "0" (dest) : "cc")
#define _cbitw_(pos, dest) 	__asm__("cbitw %1,%0" : "=mr" (dest) : \
					"i" ((unsigned char)pos) , "0" (dest) : "cc")

/* Compare Instructions */
#define _cmpb_(src1, src2)	__asm__("cmpb %0,%1" : /* no output */ : \
					"ri" ((unsigned char)src1) , "r" (src2) : "cc")
#define _cmpw_(src1, src2)	__asm__("cmpw %0,%1" : /* no output */ : \
					"ri" ((unsigned short)src1) , "r" (src2) : "cc")
#define _cmpd_(src1, src2)	__asm__("cmpd %0,%1" : /* no output */ : \
					"ri" ((unsigned long)src1) , "r" (src2) : "cc")

/* Disable Inerrupts instructions */
#define _di_()		__asm__ volatile ("di\n" :  :  : "cc")
#define _disable_()	__asm__ volatile ("di\n" :  :  : "cc")
#define _disable_interrupt_()	_di_

/* Enable Inerrupts instructions */
#define _ei_()		__asm__ volatile ("ei\n" :  :  : "cc")
#define _enable_()	__asm__ volatile ("ei\n" :  :  : "cc")
#define _enable_interrupt_()	_ei_	

/* Enable Inerrupts instructions and Wait */
#define _eiwait_()	__asm__ volatile ("eiwait" :  :  : "cc")

/* excp Instructions */
#define _excp_(vector)	__asm__ volatile ("excp " # vector)
	
/* lpr and lprd Instructions */					
#define _lpr_(procreg, src)	__asm__("lpr\t%0," procreg : \
				  	/* no output */ : "r" (src) : "cc")	
#define _lprd_(procregd, src)	__asm__("lprd\t%0," procregd : \
					/* no output */ : "r" (src) : "cc")				  
/* Left Shift Instructions */		
#define _lshb_(count, dest)	__asm__("lshb %1,%0" : "=r" (dest) : \
					"ri" ((char)count) , "0" (dest) )
#define _lshw_(count, dest)	__asm__("lshw %1,%0" : "=r" (dest) : \
					"ri" ((char)count) , "0" (dest) )
#define _lshd_(count, dest)	__asm__("lshd %1,%0" : "=r" (dest) : \
					"ri" ((char)count) , "0" (dest) )
		   
/* Load Instructions */
#define _loadb_(base, dest)	__asm__("loadb %1,%0" : "=r" (dest) : \
					"m" (base) , "0" (dest))
#define _loadw_(base, dest)	__asm__("loadw %1,%0" : "=r" (dest) : \
					"m" (base) , "0" (dest))
#define _loadd_(base, dest)	__asm__("loadd %1,%0" : "=r" (dest) : \
					"m" (base) , "0" (dest))

/* Load Multiple Instructions */
#define _loadm_(src, mask) 	__asm__("loadm %0,%1" : /* No output */ : \
					"r" ((unsigned int)src) , "i" (mask))
#define _loadmp_(src, mask) 	__asm__("loadmp %0,%1" : /* No output */ : \
					"r" ((unsigned int)src) , "i" (mask))

/* Multiply Accumulate Instrutions */
#define _macsw_(hi, lo, src1, src2)  	__asm__("macsw %1,%0" 				\
						: =l (lo), =h (hi) 			\
						: "r" ((short)src1) , "r" (src2))
#define _macuw_(hi, lo, src1, src2)  	__asm__("macuw %1,%0" 				\
  						: =l (lo), =h (hi) 			\
						: "r" ((unsigned short)src1) , "r" (src2))
#define _macqw_(src1, src2)  		__asm__("macqw %1,%0" 				\
  						: =l (lo), =h (hi) 			\
						:"r" ((short)src1) , "r" (src2))

/* Move Instructions */
#define _movb_(src, dest)  	__asm__("movb %1,%0" : "=r" (dest) : \
					"ri" ((unsigned char)src) , "0" (dest))
#define _movw_(src, dest)  	__asm__("movw %1,%0" : "=r" (dest) : \
					"ri" ((unsigned short)src) , "0" (dest))
#define _movd_(src, dest)  	__asm__("movd %1,%0" : "=r" (dest)  : \
					"ri" ((unsigned int)src) , "0" (dest))
#define _movxb_(src, dest)	__asm__("movxb %1,%0" : "=r" (dest) : \
					"r" (src), "0" (dest) )
#define _movzb_(src, dest)	__asm__("movzb %1,%0" : "=r" (dest) : \
					"r" (src), "0" (dest) )
#define _movxw_(src, dest)	__asm__("movxw %1,%0" : "=r" (dest) : \
					"r" (src), "0" (dest) )
#define _movzw_(src, dest)	__asm__("movzw %1,%0" : "=r" (dest) : \
					"r" (src), "0" (dest) )		   					
   
/* Multiplication Instructions */
#define _mulb_(src, dest)  	__asm__("mulb %1,%0" : "=r" (dest) : \
					"ri" ((char)src) , "0" (dest))
#define _mulw_(src, dest)  	__asm__("mulw %1,%0" : "=r" (dest) : \
					"ri" ((short)src) , "0" (dest))
#define _mulsb_(src, dest)	__asm__("mulsb %1,%0" : "=r" (dest) : \
					"r" ((char)src) , "0" (dest))
#define _mulsw_(src, dest)	__asm__("mulsw %1,%0" : "=r" (dest) : \
					"r" ((short)src) , "0" (dest))
#define _muluw_(src, dest)	__asm__("muluw %1,%0" : "=r" (dest) : \
					"r" ((unsigned short)src) , "0" (dest))

/* nop Instruction */
#define _nop_()  		__asm__("nop")

/* or Instructions */
#define _orb_(src, dest)  	__asm__("orb %1,%0" : "=r" (dest) : \
                   			"ri" ((unsigned char)src) , "0" (dest))
#define _orw_(src, dest)  	__asm__("orw %1,%0" : "=r" (dest) : \
					"ri" ((unsigned short)src) , "0" (dest))
#define _ord_(src, dest)  	__asm__("ord %1,%0" : "=r" (dest)  : \
					"ri" ((unsigned int)src) , "0" (dest))

/* retx Instruction */
#define _retx_()  		__asm__("retx")

/* Set Bit Instructions */
#define _sbitb_(pos, dest)	__asm__("sbitb %1,%0" : "=mr" (dest) : \
					"i" ((unsigned char)pos) , "0" (dest) : "cc")
#define _sbitw_(pos, dest)	__asm__("sbitw %1,%0" : "=mr" (dest) : \
					"i" ((unsigned char)pos) , "0" (dest) : "cc")

/* spr and sprd Instructions */
#define _spr_(procreg, dest)	__asm__("spr\t" procreg ",%0" : \
				        "=r" (dest) : /* no input */ "0" (dest) : "cc")
#define _sprd_(procregd, dest)	__asm__("sprd\t" procregd ",%0" : \
				        "=r" (dest) : /* no input */ "0" (dest) : "cc")

/* Store Instructions */
#define _storb_(src, address)  	__asm__("storb %1,%0" : "=m" (address) : \
					"ri" ((unsigned int)src))
#define _storw_(src, address)  	__asm__("storw %1,%0" : "=m" (address) : \
					"ri" ((unsigned int)src))
#define _stord_(src, address)  	__asm__("stord %1,%0" : "=m" (address) : \
					"ri" ((unsigned int)src))

/* Store Multiple Instructions */
#define _storm_(mask, src)  	__asm__("storm %1,%0" : /* No output here */ : \
					"i" (mask) , "r" ((unsigned int)src))
#define _stormp_(mask, src)  	__asm__("stormp %1,%0" : /* No output here */ : \
					"i" (mask) , "r" ((unsigned int)src))

/* Substruct Instructions */
#define _subb_(src, dest)	__asm__("subb %1, %0" : "=r" (dest) : \
					"ri" ((unsigned char)src), "0" (dest) : "cc")
#define _subw_(src, dest)	__asm__("subw %1, %0" : "=r" (dest) : \
					"ri" ((unsigned short)src), "0" (dest) : "cc")
#define _subd_(src, dest)	__asm__("subd %1, %0" : "=r" (dest) : \
					"ri" ((unsigned long)src), "0" (dest) : "cc")

/* Substruct with Carry Instructions */
#define _subcb_(src, dest)	__asm__("subcb %1, %0" : "=r" (dest) : \
					"ri" ((unsigned char)src), "0" (dest) : "cc")
#define _subcw_(src, dest)	__asm__("subcw %1, %0" : "=r" (dest) : \
					"ri" ((unsigned short)src), "0" (dest) : "cc")
					   
/* Test Bit Instructions */
#define _tbit_(offset, base)	__asm__("tbit %0,%1" : /* no output */ : \
					"ri" ((unsigned char)offset) , "r" (base) : "cc")
#define _tbitb_(pos, dest)	__asm__("tbitb %0,%1" : /* No output */ : \
					"i" ((unsigned char)pos) , "m" (dest) : "cc")
#define _tbitw_(pos, dest)	__asm__("tbitw %0,%1" : /* No output */ : \
					"i" ((unsigned char)pos) , "m" (dest) : "cc")

/* wait Instruction*/
#define _wait_()  		__asm__ volatile ("wait" :  :  : "cc")

/* xor Instructions */
#define _xorb_(src, dest)  	__asm__("xorb %1,%0" : "=r" (dest) : \
					"ri" ((unsigned char)src) , "0" (dest))
#define _xorw_(src, dest)  	__asm__("xorw %1,%0" : "=r" (dest) : \
					"ri" ((unsigned short)src) , "0" (dest))
#define _xord_(src, dest)  	__asm__("xord %1,%0" : "=r" (dest)  : \
					"ri" ((unsigned long)src) , "0" (dest))

#if !defined (__CR16C__)
#define _di_()       		__asm__ volatile ("di\n" :  :  : "cc")
#else
/* In CR16C architecture the "nop" instruction is required after the di instruction
   in order to be sure the interrupts are indeed disabled.
   For details, refer the the CR16C Programmers Reference Manual. */
#define _di_()			__asm__ volatile ("di\n\tnop" :  :  : "cc")
#endif
		
/* mtgpr Instruction */		
#define _mtgpr_(src, gpr) 				  \
__asm__ volatile ("movd\t%[_src], " gpr : /* no output */ \
		  : [_src] "ri" (src) 			  \
		  : gpr )

/* mfgpr Instruction */	
#define _mfgpr_(gpr, dest) 					  \
__asm__ volatile ("movd\t" gpr ", %[_dest]" : [_dest] "=r" (dest) \
		  : /* no inputs */ )

/* set_i_bit Operation Definition */	
#define set_i_bit() 		\
  do 				\
  { 				\
    unsigned short tpsr; 	\
    _spr_("psr", tpsr); 	\
    tpsr |= 0x0800;    		\
    _lpr_("psr",tpsr); 		\
  } while(0)

/* set_i_bit Macro Definition */  
#define _enable_global_interrupt_	set_i_bit

/* clear_i_bit Operation Definition */ 
#define clear_i_bit() 		\
  do 				\
  { 				\
    unsigned short tpsr; 	\
    _spr_("psr", tpsr); 	\
    tpsr &= 0xf7ff; 		\
    _lpr_("psr",tpsr); 		\
  } while(0)

/* clear_i_bit Macro Definition */  
#define _disbale_global_interrupt_	clear_i_bit

#define _save_asm_(x) 						\
  __asm__ volatile (x ::: "memory","cc", 			\
		    "r0","r1","r2","r3","r4","r5","r6","r7", 	\
		    "r8","r9","r10","r11","r12","r13")

#endif  /* _ASM */



