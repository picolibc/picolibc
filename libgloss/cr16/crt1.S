 ##############################################################################
 # crt0.s -- CR16 default start-up routine                                    #
 #                                                                            #
 # Copyright (c) 2004 National Semiconductor Corporation                      #
 #                                                                            #
 # The authors hereby grant permission to use, copy, modify, distribute,      #
 # and license this software and its documentation for any purpose, provided  #
 # that existing copyright notices are retained in all copies and that this   #
 # notice is included verbatim in any distributions. No written agreement,    #
 # license, or royalty fee is required for any of the authorized uses.        #
 # Modifications to this software may be copyrighted by their authors         #
 # and need not follow the licensing terms described here, provided that      #
 # the new terms are clearly indicated on the first page of each file where   #
 # they apply.                                                                #
 #                                                                            #
 # This is the  start routine of your CR16 program.                           #
 # It is linked with your application automatically. You can use              #
 # this routine as a template and modify it to your needs, yet this           #
 # file must be supplied for the compiler.                                    #
 # It is assumed that the following symbols are defined in your linker        #
 # script: __STACK_START, __ISTACK_START                                      #
 ##############################################################################

	.text
#ifdef __CR16CP__
	.align 4
#else
	.align 2
#endif
	.global	_main
	.global	_atexit
	.global	_exit
	.global	_start
	.global	__fini
	.global	__init
	.global	__STACK_START
	.global	__ISTACK_START

_start:

 #----------------------------------------------------------------------------#
 # Initialize the stack pointers. The constants __STACK_START and             #
 # __ISTACK_START should be defined in the linker script.                     #

	movd	$__STACK_START, (sp)

	movd	$__ISTACK_START, (r1,r0)
	lprd	(r1,r0), isp

 #----------------------------------------------------------------------------#
 # Initialize the default sections according to the linker script.            #

#	bal 	(ra), __init_bss_data

 #----------------------------------------------------------------------#
 # Set the Extended Dispatch bit in the CFG register. This is the       #
 # default configuration for CR16C.                                     #

	spr	cfg, r0			# Set dispatch table width
	orw	$0x100, r0
	lpr	r0, cfg

 #----------------------------------------------------------------------------#


 #----------------------------------------------------------------------------#
 # Handle global and static constructurs execution and setup                  #
 # destructors to be called from exit.                                        #

	bal	(ra),__init
	movd	$__fini@c, (r3,r2)
	bal     (ra), _atexit

 #----------------------------------------------------------------------------#
 # Jump to the main function in your application.                             #

#ifdef __INT32__
	movd    $0, (r3,r2) # Number of arguments
	movd    $0, (r5,r4) # conatins pointer to argument string.
#else
	movw    $0, r2      # Number of arguments
	movd    $0, (r4,r3) # conatins pointer to argument string.
#endif
	bal		(ra), _main

 #----------------------------------------------------------------------------#
 # Upon returning from the main function (if it isn't an infinite loop),      #
 # jump to the exit function. The exit function is located in the             #
 # library 'libc.a'.                                                          #

#ifdef __INT32__
	movd	(r1,r0), (r3,r2) # _main return value gets forwarded.
#else
	movw	r0, r2		# _main return value gets forwarded.
#endif
	br		_exit		# returns control to the functional simulator.


