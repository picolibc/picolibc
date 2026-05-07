/*
Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
SPDX-License-Identifier: BSD-3-Clause-Clear

Redistribution and use in source and binary forms, with or without
modification, are permitted (subject to the limitations in the
disclaimer below) provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.

  * Redistributions in binary form must reproduce the above
    copyright notice, this list of conditions and the following
    disclaimer in the documentation and/or other materials provided
    with the distribution.

  * Neither the name of Qualcomm Technologies, Inc. nor the names of its
    contributors may be used to endorse or promote products derived
    from this software without specific prior written permission.

NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE
GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT
HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef _LINUX_STATFS_STRUCT_H_
#define _LINUX_STATFS_STRUCT_H_

/*
 * Kernel struct statfs for x86_64 (64-bit): all fields are 'long' (8 bytes).
 * Layout matches asm-generic/statfs.h with __statfs_word = __kernel_long_t.
 */
struct __kernel_statfs {
    __int64_t  f_type;
    __int64_t  f_bsize;
    __uint64_t f_blocks;
    __uint64_t f_bfree;
    __uint64_t f_bavail;
    __uint64_t f_files;
    __uint64_t f_ffree;
    __int32_t  f_fsid[2];
    __int64_t  f_namelen;
    __int64_t  f_frsize;
    __int64_t  f_flags;
    __int64_t  f_spare[4];
};

#define LINUX_SYS_statfs_CALL  LINUX_SYS_statfs
#define LINUX_SYS_fstatfs_CALL LINUX_SYS_fstatfs

#endif /* _LINUX_STATFS_STRUCT_H_ */
