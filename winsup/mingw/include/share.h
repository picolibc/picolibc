/*
 * share.h
 *
 * Constants for file sharing functions.
 *
 * This file is part of the Mingw32 package.
 *
 * Contributors:
 *  Created by Colin Peters <colin@bird.fu.is.saga-u.ac.jp>
 *
 *  THIS SOFTWARE IS NOT COPYRIGHTED
 *
 *  This source code is offered for use in the public domain. You may
 *  use, modify or distribute it freely.
 *
 *  This code is distributed in the hope that it will be useful but
 *  WITHOUT ANY WARRANTY. ALL WARRANTIES, EXPRESS OR IMPLIED ARE HEREBY
 *  DISCLAIMED. This includes but is not limited to warranties of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * $Revision$
 * $Author$
 * $Date$
 *
 */

#ifndef	_SHARE_H_
#define	_SHARE_H_

/* All the headers include this file. */
#include <_mingw.h>

#define _SH_COMPAT	0x00	/* Compatibility */
#define	_SH_DENYRW	0x10	/* Deny read/write */
#define	_SH_DENYWR	0x20	/* Deny write */
#define	_SH_DENYRD	0x30	/* Deny read */
#define	_SH_DENYNO	0x40	/* Deny nothing */

#ifndef _NO_OLDNAMES

/* Non ANSI names */
#define SH_DENYRW _SH_DENYRW
#define SH_DENYWR _SH_DENYWR
#define SH_DENYRD _SH_DENYRD
#define SH_DENYNO _SH_DENYNO

#endif	/* Not _NO_OLDNAMES */

#endif	/* Not _SHARE_H_ */
