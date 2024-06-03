/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2024, Synopsys Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#define __STDC_WANT_LIB_EXT1__  1
#include "stdio_private.h"
#include <errno.h>
#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

int sprintf_s(char *restrict s, rsize_t bufsize,
        const char * restrict fmt, ...) {
	bool constraint_failure = false;
	bool write_null = true;
	const char *msg = "";
	va_list args;
	int rc;

    if (s == NULL)
	{
		constraint_failure = true;
		write_null = false;
		msg = "sprintf_s: dest buffer is null";
	}
	else if (bufsize == 0 || bufsize > RSIZE_MAX)
	{
		constraint_failure = true;
		write_null = false;
		msg = "sprintf_s: invalid buffer size";
	}
	else if (fmt == NULL)
	{
		constraint_failure = true;
		msg = "sprintf_s: null format string";
	}
	else if (strstr(fmt, " %n") != NULL)
	{
		constraint_failure = true;
		msg = "sprintf_s: format string contains percent-n";
	}
	else
	{
		va_start(args, fmt);

		va_list args_copy;
    	va_copy(args_copy, args);

		const char *check_ptr = fmt;
		uint8_t null_str = 0;
		while (*check_ptr && null_str == 0) 
		{
			if (check_ptr[0] == '%')
			{
				switch(check_ptr[1])
				{
					case 's':
					{
						char *str_arg = va_arg(args_copy, char *);
						if (str_arg == NULL)
						{
							constraint_failure = true;
							msg = "sprintf_s: null string argument";
							va_end(args_copy);
							va_end(args);
							null_str = 1;
						}
						break;
					}
					case 'd': case 'i': case 'u': case 'o': case 'x': case 'X':
					case 'c': case 'h': case 'l': case 'L': case 'z': case 't':
						va_arg(args_copy, int);
						break;
					case 'f': case 'F': case 'e': case 'E': case 'g': case 'G': case 'a': case 'A':
						va_arg(args_copy, double);
						break;
					case 'p':
						va_arg(args_copy, void *);
						break;
					default:
						break;
				}
				
			}
			check_ptr++;
		}

		if (constraint_failure == false)
		{
			rc = vsnprintf(s, bufsize, fmt, args);
			va_end(args_copy);
			va_end(args);
		}
		
	}

	if (constraint_failure == false)
	{
		if (rc < 0 || rc >= (int)bufsize)
		{
			constraint_failure = true;
			msg = "sprintf_s: dest buffer overflow";
		}
		else
		{
			s[rc] = 0;
		}
	}

	if (constraint_failure == true)
	{
		constraint_handler_t handler = set_constraint_handler_s(NULL);
		(void) set_constraint_handler_s(handler);
		(*handler)(msg, NULL, -1);
		rc = 0; /* standard stipulates this */
		if (write_null)
	    	s[0] = '\0';  /* again, standard requires this */
	}

	return(rc);
}