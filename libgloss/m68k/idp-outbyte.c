/* idp-outbyte.c
 * Copyright (c) 1995 Cygnus Support
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

#include <_ansi.h>
#include "mc68681reg.h"

/* 
 * The DUART is mapped into the IDP address space in an unusual 
 * manner.  The mc68681 is an 8 bit device located on the least
 * significant byte (byte0) of the data bus.  Bytes 3, 2, and 
 * one have nothing in them and writes to these locations are
 * not valid.
 */

#define DUART_ADDR	0x00B00000
#define READREG(x)	(*((volatile char *) DUART_ADDR + (x * 4) + 3))
#define WRITEREG(x, y)	(*((char *) DUART_ADDR + (x * 4) + 3) = y)

/*
 * outbyte -- send a byte to the DUART buffer. This only sends
 *           to channel A.
 */
void
_DEFUN (outbyte, (byte),
	char byte)
{
  while ((READREG (DUART_SRA) & 0x04) == 0x00)
    ;

  WRITEREG (DUART_TBA, byte);		/* write the byte */
}
