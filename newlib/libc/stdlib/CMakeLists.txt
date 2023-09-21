#
# SPDX-License-Identifier: BSD-3-Clause
#
# Copyright © 2022 Keith Packard
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above
#    copyright notice, this list of conditions and the following
#    disclaimer in the documentation and/or other materials provided
#    with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its
#    contributors may be used to endorse or promote products derived
#    from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
# COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
# INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
# STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
# OF THE POSSIBILITY OF SUCH DAMAGE.
#
picolibc_sources(
  a64l.c
  abort.c
  abs.c
  aligned_alloc.c
  arc4random.c
  arc4random_uniform.c
  assert.c
  assert_func.c
  assert_no_arg.c
  atof.c
  atoff.c
  atoi.c
  atol.c
  atoll.c
  btowc.c
  div.c
  drand48.c
  environ.c
  eprintf.c
  erand48.c
  _Exit.c
  getenv.c
  getenv_r.c
  getopt.c
  getsubopt.c
  imaxabs.c
  imaxdiv.c
  itoa.c
  jrand48.c
  l64a.c
  labs.c
  lcong48.c
  ldiv.c
  llabs.c
  lldiv.c
  lrand48.c
  mblen.c
  mbrlen.c
  mbrtowc.c
  mbsinit.c
  mbsnrtowcs.c
  mbsrtowcs.c
  mbstowcs.c
  mbtowc.c
  mbtowc_r.c
  mrand48.c
  mstats.c
  nrand48.c
  putenv.c
  rand48.c
  rand.c
  random.c
  rand_r.c
  reallocarray.c
  reallocf.c
  rpmatch.c
  sb_charsets.c
  seed48.c
  setenv.c
  srand.c
  srand48.c
  srandom.c
  system.c
  utoa.c
  wcrtomb.c
  wcsnrtombs.c
  wcsrtombs.c
  wcstod.c
  wcstoimax.c
  wcstol.c
  wcstoll.c
  wcstombs.c
  wcstoul.c
  wcstoull.c
  wcstoumax.c
  wctob.c
  wctomb.c
  wctomb_r.c
  pico-atexit.c
  pico-exit.c
  pico-onexit.c
  pico-cxa-atexit.c
  )

picolibc_sources_flags("-fno-builtin-malloc;-fno-builtin-free"
  nano-malloc-calloc.c
  nano-malloc-cfree.c
  nano-malloc-free.c
  nano-malloc-getpagesize.c
  nano-malloc-mallinfo.c
  nano-malloc-malloc.c
  nano-malloc-malloc_stats.c
  nano-malloc-malloc_usable_size.c
  nano-malloc-mallopt.c
  nano-malloc-memalign.c
  nano-malloc-posix_memalign.c
  nano-malloc-pvalloc.c
  nano-malloc-realloc.c
  nano-malloc-valloc.c
  )
