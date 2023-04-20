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
  bufio.c
  clearerr.c
  compare_exchange.c
  ecvt_r.c
  ecvt.c
  ecvtf_r.c
  ecvtf.c
  exchange.c
  fcvt.c
  fcvt_r.c
  fcvtf.c
  fcvtf_r.c
  gcvt.c
  gcvtf.c
  fclose.c
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
  filestrput.c
  filestrputalloc.c
  filewstrget.c
  fmemopen.c
  fprintf.c
  fputc.c
  fputs.c
  fputwc.c
  fputws.c
  fread.c
  freopen.c
  fscanf.c
  fseek.c
  fseeko.c
  ftell.c
  ftello.c
  fwide.c
  fwrite.c
  fwprintf.c
  fwscanf.c
  getchar.c
  getdelim.c
  getline.c
  gets.c
  getwchar.c
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
  snprintf.c
  sprintf.c
  snprintfd.c
  snprintff.c
  sprintff.c
  sprintfd.c
  sscanf.c
  strfromf.c
  strfromd.c
  strtof.c
  strtof_l.c
  strtod.c
  strtod_l.c
  strtoimax.c
  strtol.c
  strtoll.c
  strtoul.c
  strtoull.c
  strtoumax.c
  swprintf.c
  swscanf.c
  tmpnam.c
  tmpfile.c
  ungetc.c
  ungetwc.c
  vasprintf.c
  vfiprintf.c
  vfprintf.c
  vfprintff.c
  vfscanf.c
  vfiscanf.c
  vfscanff.c
  vfwprintf.c
  vfwscanf.c
  vprintf.c
  vscanf.c
  vsscanf.c
  vsnprintf.c
  vsprintf.c
  vswprintf.c
  vswscanf.c
  wprintf.c
  wscanf.c
  dtoa_ryu.c
  ftoa_ryu.c
  ryu_table.c
  atod_ryu.c
  atof_ryu.c
  ryu_log2pow5.c
  ryu_log10.c
  ryu_pow5bits.c
  ryu_umul128.c
  ryu_divpow2.c
  fopen.c
  fdopen.c
  fclose.c
  sflags.c
  )

if(POSIX_CONSOLE)
  picolibc_sources(
    posixiob.c
    )
endif()

picolibc_headers(""
  stdio.h
  stdio-bufio.h
  )
