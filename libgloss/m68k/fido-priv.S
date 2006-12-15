/*
 * fido-priv.S -- 
 *
 * Copyright (c) 2006 CodeSourcery Inc
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

/* Older releases of Sourcery G++ for fido misspelled this handler.
   Provide a thunk from the correct name to the old name, in case
   the user overrode the old name.  */

	.extern _PriviligeViolationHandler
	.globl _PrivilegeViolationHandler
_PrivilegeViolationHandler:
	jmp _PriviligeViolationHandler
