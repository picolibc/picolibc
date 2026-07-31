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

/*
 * testmod.c - source for the shared object loaded by the libdl tests.
 *
 * Built as a freestanding ET_DYN (-fPIC -shared -nostdlib) so it can be
 * loaded by libdl's dlopen()/dlsym().  It deliberately references no libc
 * symbols so that loading succeeds regardless of the built-in symbol
 * table: it exercises the loader (segment mapping + relocations) and
 * symbol resolution only.
 *
 * Exports:
 *   testmod_add        - a simple exported function (call after dlsym)
 *   testmod_value      - an exported initialised data symbol (data reloc)
 *   testmod_name       - an exported function returning a static string
 *                        (exercises a base-relative/absolute relocation)
 *
 * Thread-local storage accessors (see below):
 *   testmod_tdataN_get/set  - several initialised .tdata vars
 *   testmod_tbssN_get/set   - several zero-initialised .tbss vars
 *   testmod_wide_get/set    - 64-bit .tdata var (aligned -> non-zero offset)
 *   testmod_tdata1_addr     - address of the first .tdata var
 *
 * The thread-local variables exercise the dynamic-TLS path in the loader:
 * the module is built with -ftls-model=global-dynamic, so accesses go
 * through __tls_get_addr(descriptor) and the DTPMOD/DTPREL GOT
 * relocations, with the loader pointing the thread pointer (UGP on
 * Hexagon) at the per-module TLS block it allocates in dlopen().
 *
 * Using several variables of each kind covers:
 *   - multiple initialised ints in .tdata (all copied from the file image
 *     to their distinct offsets in the TLS block),
 *   - multiple zero-initialised ints in .tbss (all zeroed by the loader),
 *   - an initialised 64-bit value whose 8-byte alignment forces a non-zero
 *     offset, exercising the R_HEX_DTPREL_32 handler and a wide load/store.
 * Distinct offsets for every variable make sure the loader lays out .tdata
 * and .tbss correctly and that each descriptor resolves independently.
 *
 * Accessor functions are exported (rather than the TLS symbols themselves)
 * because dlsym() cannot meaningfully return the address of a
 * thread-local symbol: TLS variables have no fixed load address, only a
 * per-thread offset resolved at access time.
 */

int
testmod_add(int a, int b)
{
    return a + b;
}

int               testmod_value = 42;

static const char testmod_name_str[] = "testmod";

const char *
testmod_name(void)
{
    return testmod_name_str;
}

/* Initialised thread-local ints (.tdata): the loader must copy these values
 * from the file image into the per-module TLS block at distinct offsets. */
static __thread int       testmod_tdata1 = 111;
static __thread int       testmod_tdata2 = 222;
static __thread int       testmod_tdata3 = 333;

/* Zero-initialised thread-local ints (.tbss): the loader must zero these. */
static __thread int       testmod_tbss1;
static __thread int       testmod_tbss2;
static __thread int       testmod_tbss3;

/* Initialised 64-bit thread-local: its 8-byte alignment forces a non-zero
 * offset within the TLS block, exercising the R_HEX_DTPREL_32 handler and
 * a wide load/store. */
static __thread long long testmod_wide = 0x1122334455667788LL;

int
testmod_tdata1_get(void)
{
    return testmod_tdata1;
}
void
testmod_tdata1_set(int v)
{
    testmod_tdata1 = v;
}
int *
testmod_tdata1_addr(void)
{
    return &testmod_tdata1;
}

int
testmod_tdata2_get(void)
{
    return testmod_tdata2;
}
void
testmod_tdata2_set(int v)
{
    testmod_tdata2 = v;
}

int
testmod_tdata3_get(void)
{
    return testmod_tdata3;
}
void
testmod_tdata3_set(int v)
{
    testmod_tdata3 = v;
}

int
testmod_tbss1_get(void)
{
    return testmod_tbss1;
}
void
testmod_tbss1_set(int v)
{
    testmod_tbss1 = v;
}

int
testmod_tbss2_get(void)
{
    return testmod_tbss2;
}
void
testmod_tbss2_set(int v)
{
    testmod_tbss2 = v;
}

int
testmod_tbss3_get(void)
{
    return testmod_tbss3;
}
void
testmod_tbss3_set(int v)
{
    testmod_tbss3 = v;
}

long long
testmod_wide_get(void)
{
    return testmod_wide;
}
void
testmod_wide_set(long long v)
{
    testmod_wide = v;
}
