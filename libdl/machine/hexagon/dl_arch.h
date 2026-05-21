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

#ifndef _DL_ARCH_H_
#define _DL_ARCH_H_

/* Hexagon ELF machine type */
#define EM_HEXAGON 164

/* Hexagon relocation types present in picolibc libc.so */
#define R_HEX_32       6  /* word32: S + A                */
#define R_HEX_GLOB_DAT 33 /* word32: S + A (GOT entry)    */
#define R_HEX_JMP_SLOT 34 /* word32: S    (PLT slot)      */
#define R_HEX_RELATIVE 35 /* word32: B + A (base-relative)*/

/* Global-dynamic TLS relocation types (appear when thread-local-storage=true) */
#define R_HEX_DTPMOD_32 44 /* GOT TLS descriptor: module ID        */
#define R_HEX_DTPREL_32 47 /* GOT TLS descriptor: offset in block  */

#endif /* _DL_ARCH_H_ */
