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
  asprintf.c
  atold_engine.c
  bufio.c
  clearerr.c
  dtox_engine.c
  ecvt.c
  ecvtf.c
  ecvtf_r.c
  ecvtl.c
  ecvtl_r.c
  ecvt_r.c
  fclose.c
  fcvt.c
  fcvtf.c
  fcvtf_r.c
  fcvtl.c
  fcvtl_r.c
  fcvt_r.c
  fdevopen.c
  feof.c
  ferror.c
  fflush.c
  fgetc.c
  fgets.c
  fgetwc.c
  fgetws.c
  fileno.c
  filestrget.c
  filestrputalloc.c
  filestrput.c
  filewstrget.c
  fmemopen.c
  fprintf.c
  fputc.c
  fputs.c
  fputwc.c
  fputws.c
  fread.c
  fscanf.c
  fseek.c
  fseeko.c
  ftell.c
  ftello.c
  ftox_engine.c
  fwide.c
  fwprintf.c
  fwrite.c
  fwscanf.c
  gcvt.c
  gcvtf.c
  gcvtl.c
  getchar.c
  getdelim.c
  getline.c
  gets.c
  getwchar.c
  ldtoa_engine.c
  ldtox_engine.c
  matchcaseprefix.c
  mktemp.c
  perror.c
  printf.c
  putchar.c
  puts.c
  putwchar.c
  remove.c
  rewind.c
  scanf.c
  setbuf.c
  setbuffer.c
  setlinebuf.c
  setvbuf.c
  sflags.c
  snprintf.c
  snprintfd.c
  snprintff.c
  sprintf.c
  sprintfd.c
  sprintff.c
  sscanf.c
  strfromd.c
  strfromf.c
  strfroml.c
  strtod.c
  strtod_l.c
  strtof.c
  strtof_l.c
  strtoimax.c
  strtol.c
  strtold.c
  strtold_l.c
  strtol_l.c
  strtoll.c
  strtoll_l.c
  strtoul.c
  strtoul_l.c
  strtoull.c
  strtoull_l.c
  strtoumax.c
  swprintf.c
  swscanf.c
  tmpfile.c
  tmpnam.c
  ungetc.c
  ungetwc.c
  vasprintf.c
  vffprintf.c
  vffscanf.c
  vfiprintf.c
  vfiscanf.c
  vflprintf.c
  vflscanf.c
  vfmprintf.c
  vfmscanf.c
  vfprintf.c
  vfscanf.c
  vfwprintf.c
  vfwscanf.c
  vprintf.c
  vscanf.c
  vsnprintf.c
  vsprintf.c
  vsscanf.c
  vswprintf.c
  vswscanf.c
  vwprintf.c
  vwscanf.c
  wprintf.c
  wscanf.c
  )

picolibc_sources(
  compare_exchange.c
  exchange.c
  )  

picolibc_sources(
  fdopen.c
  fopen.c
  freopen.c
  )

if(_IO_FLOAT_EXACT)
  picolibc_sources(
    atod_ryu.c
    atof_ryu.c
    dtoa_ryu.c
    ftoa_ryu.c
    ryu_divpow2.c
    ryu_log10.c
    ryu_log2pow5.c
    ryu_pow5bits.c
    ryu_table.c
    ryu_umul128.c
    )
else()
  picolibc_sources(
    atod_engine.c
    atof_engine.c
    dtoa_engine.c
    ftoa_engine.c
    )
endif()

if(POSIX_CONSOLE)
  picolibc_sources(
    posixiob_stdin.c
    posixiob_stdout.c
    posixiob_stderr.c
    )
endif()

picolibc_headers(""
  stdio.h
  stdio-bufio.h
  )
