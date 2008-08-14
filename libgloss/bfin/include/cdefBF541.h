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

/*
** cdefBF541.h
**
** Copyright (C) 2008 Analog Devices, Inc.
**
************************************************************************************
**
** This include file contains a list of macro "defines" to enable the programmer
** to use symbolic names for the ADSP-BF541 peripherals.
**
************************************************************************************
** System MMR Register Map
************************************************************************************/

#ifndef _CDEF_BF541_H
#define _CDEF_BF541_H

/* include all Core registers and bit definitions */
#include <defBF541.h>

/* include core specific register pointer definitions */
#include <cdef_LPBlackfin.h>

/** ADSP-BF541 is a non-existent processor -- no additional #defines **/

#define                           pCHIPID ((volatile unsigned long *)CHIPID)

#endif /* _CDEF_BF541_H */
