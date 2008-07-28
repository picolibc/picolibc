/* ofmt_stub.s
 *
 * $Id$
 *
 * A trivial stub, to replace the _get_output_format() function.
 *
 * _pformat() requires this function, which is provided by MSVCRT runtimes
 * from msvcr80.dll onwards; add this stub to the import libraries for earlier
 * versions of MSVCRT, (those which do not already advertise availability of
 * any exported _get_output_format() function); this will permit _pformat()
 * to transparently interoperate with all supported versions of MSVCRT.
 * (Likewise for CRTDLL).
 *
 * Written by Keith Marshall  <keithmarshall@users.sourceforge.net>
 * Contributed to the MinGW Project, and hereby assigned to the public domain.
 *
 * This is free software.  It is provided AS IS, in the hope that it may be
 * useful.  There is NO WARRANTY OF ANY KIND, not even an implied warranty of
 * merchantability, nor of fitness for any particular purpose.
 *
 */
	.text
	.p2align 1,,4

.globl __get_output_format
	.def	__get_output_format;	.scl	2;	.type	32;	.endef

__get_output_format:
/*
 * int _get_output_format( void );
 *
 * Implementation is trivial: we immediately return zero, thus matching the
 * default behaviour of Microsoft's own implementation, in the absence of any
 * preceding call to _set_output_format(); (if we are using this stub, then
 * that entire API is unsupported, so no such prior call is possible).
 */
	xorl	%eax, %eax
	ret

/* $RCSfile$Revision$: end of file */
