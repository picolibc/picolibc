/* Copyright (c) 2005, Dmitry Xmelkov
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
   * Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in
     the documentation and/or other materials provided with the
     distribution.
   * Neither the name of the copyright holders nor the names of
     contributors may be used to endorse or promote products derived
     from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE. */

/* $Id: ftoa_engine.h 1218 2007-02-18 13:18:41Z dmix $ */

#ifndef	_FTOA_ENGINE_H
#define	_FTOA_ENGINE_H

#include <stdlib.h>
#include <stdint.h>
#include <float.h>
#include <stdbool.h>

#define FTOA_MAX_DIG	9

struct ftoa {
	int32_t	exp;
	uint8_t flags;
	char	digits[FTOA_MAX_DIG + 1];
};

int __ftoa_engine (float val, struct ftoa *ftoa, int maxDigits, bool fmode, int maxDecimal);

/* '__ftoa_engine' flags return value */
#define	FTOA_MINUS	1
#define	FTOA_ZERO	2
#define	FTOA_INF	4
#define	FTOA_NAN	8
#define	FTOA_CARRY	16	/* Carry was to most significant position. */

#endif	/* !_FTOA_ENGINE_H */
