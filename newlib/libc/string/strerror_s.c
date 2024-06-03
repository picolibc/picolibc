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
#define __STDC_WANT_LIB_EXT1__ 1
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "string_private.h"

const char *error_string(errno_t errnum)
{
    static const char * const _sys_errlist[] =
    {
            "No error",             /* 0 */
            "Not owner",
            "No such file or directory",
            "No such process",
            "Interrupted system call",
            "I/O error",
            "No such device or address",
            "Arg list too long",
            "Exec format error",
            "Bad file number",
            "No children",          /* 10 */
            "No more processes",
            "Not enough core",
            "Permission denied",
            "Bad address",
            "Block device required",
            "Mount device busy",
            "File exists",
            "Cross-device link",
            "No such device",
            "Not a directory",          /* 20 */
            "Is a directory",
            "Invalid argument",
            "File table overflow",
            "Too many open files",
            "Not a typewriter",
            "Text file busy",
            "File too large",
            "No space left on device",
            "Illegal seek",
            "Read-only file system",        /* 30 */
            "Too many links",
            "Broken pipe",
            "Mathematics argument out of domain of function",
            "Result too large",
            "No message of desired type",
            "Identifier removed",
            "Channel number out of range",
            "Level 2 not synchronized",
            "Level 3 halted",
            "Level 3 reset",                 /* 40 */
            "Link number out of range",
            "Protocol driver not attached",
            "No CSI structure available",
            "Level 2 halted",
            "Deadlock",
            "No lock",
            "unknown error",
            "unknown error",
            "unknown error",
            "Invalid exchange",          /* 50 */
            "Invalid request descriptor",
            "Exchange full",
            "No anode",
            "Invalid request code",
            "Invalid slot",
            "File locking deadlock error",
            "Bad font file fmt",
            "unknown error",
            "unknown error",
            "Not a stream",     /* 60 */
            "No data (for no delay io)",
            "Stream ioctl timeout",
            "No stream resources",
            "Machine is not on the network",
            "Package not installed",
            "The object is remote",
            "Virtual circuit is gone",
            "Advertise error",
            "Srmount error",
            "Communication error on send",       /* 70 */
            "Protocol error",
            "unknown error",
            "unknown error",
            "Multihop attempted",
            "Inode is remote (not really error)",
            "Cross mount point (not really error)",
            "Bad message",
            "unknown error",
            "Inappropriate file type or format",
            "Given log. name not unique",       /* 80 */
            "File descriptor in bad state",
            "Remote address changed",
            "Can't access a needed shared lib",
            "Accessing a corrupted shared lib",
            ".lib section in a.out corrupted",
            "Attempting to link in too many libs",
            "Attempting to exec a shared library",
            "Function not implemented",
            "No more files",
            "Directory not empty",       /* 90 */
            "File or path name too long",
            "Too many symbolic links",
            "unknown error",
            "unknown error",
            "Operation not supported on socket",
            "Protocol family not supported",
            "unknown error",
            "unknown error",
            "unknown error",
            "unknown error",       /* 100 */
            "unknown error",
            "unknown error",
            "unknown error", 
            "Connection reset by peer",
            "No buffer space available",
            "Address family not supported by protocol family",
            "Protocol wrong type for socket",
            "Socket operation on non-socket",
            "Protocol not available",
            "Can't send after socket shutdown",       /* 110 */
            "Connection refused",
            "Address already in use",
            "Software caused connection abort",
            "Network is unreachable",
            "Network interface is not configured",
            "Connection timed out",
            "Host is down",
            "Host is unreachable",
            "Connection already in progress",
            "Socket already connected",       /* 120 */
            "Destination address required",
            "Message too long",
            "Unknown protocol",
            "Socket type not supported",
            "Address not available",
            "Connection aborted by network",
            "Socket is already connected",
            "Socket is not connected",
            "Too many references: cannot splice",
            "Too many processes",       /* 130 */
            "Too many users",
            "Reserved",
            "Reserved",
            "Not supported",
            "No medium found",
            "unknown error",
            "unknown error",
            "Illegal byte sequence",
            "Value too large for defined data type",
            "Operation canceled",       /* 140 */
            "State not recoverable",
            "Previous owner died",
            "Streams pipe error",
            "Memory page has hardware error",
            "Is a named type file",
            "Key has expired",
            "Key was rejected by service",
            "Key has been revoked",
            NULL
    };

    const char *cp;
    if ((uint32_t)errnum  < SYS_NERR)
    {
        cp = SYS_ERRLIST(errnum);
    }
    else
    {
        /* Standard says any value of type int shall be mapped to a message */
        cp = "unknown error";
    }
    return cp;
}

/* C11 version; required by LLVM's C++11 library */
errno_t strerror_s(char *buf, rsize_t buflen, errno_t errnum)
{
    int32_t result = 0;
    bool constraint_failure = false;
    const char *msg = "";


    if (buf == NULL)
    {
        constraint_failure = true;
        msg = "strerror_s: dest is NULL";
    }

    if (constraint_failure == false)
    {
        if ((buflen == 0u) || (buflen > RSIZE_MAX))
        {
            constraint_failure = true;
            msg = "strerror_s: dest buffer size is 0 or exceeds RSIZE_MAX";
        }
    }

    if (constraint_failure == false)
    {
        const char *cp = error_string(errnum);
        uint32_t len = strnlen_s(cp, MAX_ERROR_MSG);

        if (len < buflen)
        {
            (void)strncpy(buf, cp, MAX_ERROR_MSG);
        }
        else
        {
            /* Standard allows truncation of error message with '...' to
               indicate truncation. */
            (void)memcpy(buf, cp, (buflen - 1u));
            buf[(buflen - 1u)] = '\0';

            if (buflen > 3u)
            {
                (void)strncpy(&buf[(buflen - 4u)], "...", 4u);
            }

            result = ERANGE;
        }

    }

    if (constraint_failure == true)
    {
        constraint_handler_t handler = set_constraint_handler_s(NULL);

        (void)set_constraint_handler_s(handler);
        (*handler)(msg, NULL, -1);

        result = -1;
    }

    return result;
}
