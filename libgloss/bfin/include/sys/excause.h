/*
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

/************************************************************************
 *
 * excause.h
 *
 * (c) Copyright 2001-2003 Analog Devices, Inc.  All rights reserved.
 *
 ************************************************************************/

/*
** Definitions of constants for the four user-level bits in EXCAUSE,
** the field from SYSSTAT that is set when the EXCPT instruction is
** invoked.
*/

#ifndef _EXCAUSE_H
#define _EXCAUSE_H

/*
** Value 0x0 - exit program. (halt)
** R0 => exit status.
*/

#define EX_EXIT_PROG	0x0

/*
** Value 0x1 - abnormal exit (abort)
*/

#define EX_ABORT_PROG	0x1

/*
** Value 0x2 - invoke system service.
** R0 => command.
** R1 => first arg
** R2 => second arg
*/

#define EX_SYS_REQ	0x2

/*
** Available commands:
*/

#define EX_SYSREQ_NONE		0x00	/* Do nothing */
#define EX_SYSREQ_REG_ISR	0x01	/* Register an interrupt handler.
					   R1==EVT entry, R2==func ptr
					   Returns previous entry in R0. */
#define EX_SYSREQ_RAISE_INT	0x02	/* Cause an interrupt
					   R1 = int number */
/*
** Values 0x3 to 0x4 currently undefined.
*/

/*
** Value 0x5 - File I/O
** R0 => first arg
** R1 => second arg
** R2 => third arg
** R4 => command
** result => R0
*/

#define EX_FILE_IO 0x5

/*
** Available commands:
** XXX stdout/stderr are handled separately for writing.
*/

#define EX_FILEIO_OPEN		0x00	/* R0 => dev, R1=> path, R2=>mode */
#define EX_FILEIO_CLOSE		0x01	/* R0=> fid */
#define EX_FILEIO_WRITE		0x02	/* R0=>fid, R1=>data, R2=>length */
#define EX_FILEIO_READ		0x03	/* R0=>fid, R1=>data, R2=>length */
#define EX_FILEIO_SEEK		0x04	/* R0=>fid, R1=>offset, R2=>mode */
#define EX_FILEIO_DUP		0x05	/* R0=>fid */

/*
** Values 0x6 to 0xF currently undefined.
*/

#endif /*  _EXCAUSE_H */
