/* Copyright (c) 2012-2015 Red Hat, Inc. All rights reserved.

   This copyrighted material is made available to anyone wishing to use, modify,
   copy, or redistribute it subject to the terms and conditions of the BSD
   License.   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY expressed or implied, including the implied warranties
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  A copy of this license
   is available at http://www.opensource.org/licenses. Any Red Hat trademarks that
   are incorporated in the source code or documentation are not subject to the BSD
   License and may only be used or replicated with the express permission of
   Red Hat, Inc.
*/

#include "memmodel.h"

	.section ".resetvec", "a"
__msp430_resetvec_hook:
	.word	__crt0_start

	;; Here we provide weak definitions of the symbols used in the
	;; init_highbss and move_highdata blocks, in case they are not
	;; provided by the linker script.  They are defined here because
	;; this block is always included in every executable, and because
	;; if there were defined in the blocks that need them their values
	;; would be used without giving the linker script a chance to
	;; override them.
	;; 
	;; The weak definitions are needed if the user targets an MCU
	;; without high memory - and hence uses a linker script without
	;; a definition of the .upper.bss or .upper.data sections - and
	;; they have compiled their code with the -mdata-region=either
	;; command line option.  That option causes the assembler to
	;; define the __crt0_move_highdata and/or crt0_init_highbss
	;; symbols, which in turn forces the inclusion of the
	;; move_highdata and/or init_highbss blocks in the startup code,
	;; regardless of the fact that the sections are not present in
	;; the linker script.

	WEAK_DEF __upper_data_init
	WEAK_DEF __rom_highdatacopysize
	WEAK_DEF __high_datastart
	WEAK_DEF __rom_highdatastart
	WEAK_DEF __high_bssstart
	WEAK_DEF __high_bsssize

START_CRT_FUNC 0000 start
	.refsym	__msp430_resetvec_hook
	.refsym	__crt0_call_main
	mov_	#__stack, R1

END_CRT_FUNC	start

;; The CRT functions will only be present in the final linked
;; executable if the assembler decides they are needed.  The assembler will
;; only define the symbol necessary to prevent them being garbage collected
;; by the linker if the file being assembled has a specific section,
;; or some other criteria is met.
;; The exception to this is __crt0_call_exit. GCC will include this function
;; if it detects that main() has an epilogue. For example, if main() has a
;; while(1) loop at the end, GCC will not generate an epilogue (since it won't
;; return) and __crt0_call_exit won't be included.
