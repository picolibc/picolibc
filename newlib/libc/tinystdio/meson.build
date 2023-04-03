#
# SPDX-License-Identifier: BSD-3-Clause
#
# Copyright © 2019 Keith Packard
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
srcs_tinystdio = [
    'asprintf.c',
    'bufio.c',
    'clearerr.c',
    'ecvt_r.c',
    'ecvt.c',
    'ecvtf_r.c',
    'ecvtf.c',
    'fcvt.c',
    'fcvt_r.c',
    'fcvtf.c',
    'fcvtf_r.c',
    'gcvt.c',
    'gcvtf.c',
    'fclose.c',
    'fdevopen.c',
    'feof.c',
    'ferror.c',
    'fflush.c',
    'fgetc.c',
    'fgets.c',
    'fgetwc.c',
    'fgetws.c',
    'fileno.c',
    'filestrget.c',
    'filestrput.c',
    'filestrputalloc.c',
    'filewstrget.c',
    'fmemopen.c',
    'fprintf.c',
    'fputc.c',
    'fputs.c',
    'fputwc.c',
    'fputws.c',
    'fread.c',
    'freopen.c',
    'fscanf.c',
    'fseek.c',
    'fseeko.c',
    'ftell.c',
    'ftello.c',
    'fwide.c',
    'fwrite.c',
    'fwprintf.c',
    'fwscanf.c',
    'getchar.c',
    'getdelim.c',
    'getline.c',
    'gets.c',
    'getwchar.c',
    'matchcaseprefix.c',
    'mktemp.c',
    'perror.c',
    'printf.c',
    'putchar.c',
    'puts.c',
    'putwchar.c',
    'remove.c',
    'rewind.c',
    'scanf.c',
    'setbuf.c',
    'setbuffer.c',
    'setlinebuf.c',
    'setvbuf.c',
    'snprintf.c',
    'sprintf.c',
    'snprintfd.c',
    'snprintff.c',
    'sprintff.c',
    'sprintfd.c',
    'sscanf.c',
    'strfromf.c',
    'strfromd.c',
    'strfroml.c',
    'strtof.c',
    'strtof_l.c',
    'strtod.c',
    'strtod_l.c',
    'strtoimax.c',
    'strtoumax.c',
    'strtol.c',
    'strtoll.c',
    'strtoul.c',
    'strtoull.c',
    'strtol_l.c',
    'strtoll_l.c',
    'strtoul_l.c',
    'strtoull_l.c',
    'swprintf.c',
    'swscanf.c',
    'tmpnam.c',
    'tmpfile.c',
    'ungetc.c',
    'ungetwc.c',
    'vasprintf.c',
    'vfiprintf.c',
    'vfprintf.c',
    'vfprintff.c',
    'vfscanf.c',
    'vfiscanf.c',
    'vfscanff.c',
    'vfwprintf.c',
    'vfwscanf.c',
    'vprintf.c',
    'vscanf.c',
    'vsscanf.c',
    'vsnprintf.c',
    'vsprintf.c',
    'vswprintf.c',
    'vswscanf.c',
    'vwprintf.c',
    'vwscanf.c',
    'wprintf.c',
    'wscanf.c',
]

# exact float/string conversion code
srcs_dtoa_ryu = [
  'dtoa_ryu.c',
  'ftoa_ryu.c',
  'ryu_table.c',
  'atod_ryu.c',
  'atof_ryu.c',
  'ryu_log2pow5.c',
  'ryu_log10.c',
  'ryu_pow5bits.c',
  'ryu_umul128.c',
  'ryu_divpow2.c',
  ]
		
# smaller float/string conversion code
srcs_dtoa_float = [
  'dtoa_engine.c',
  'dtoa_data.c',
  'ftoa_data.c',
  'ftoa_engine.c',
  'atod_engine.c',
  'atof_engine.c',
]

if io_float_exact
  srcs_tinystdio += srcs_dtoa_ryu
else
  srcs_tinystdio += srcs_dtoa_float
endif

srcs_tinystdio += ['strtold.c', 'strtold_l.c', 'atold_engine.c']

if atomic_ungetc
  srcs_tinystdio += ['exchange.c', 'compare_exchange.c']
endif

srcs_tinystdio_posix = [
    'fopen.c',
    'fdopen.c',
    'fclose.c',
    'sflags.c'
]

srcs_tinystdio_posix_console = [
  'posixiob.c',
]

if posix_io
  srcs_tinystdio += srcs_tinystdio_posix
endif

if posix_console
  srcs_tinystdio += srcs_tinystdio_posix_console
endif

srcs_tinystdio_flt = [
    'vfprintf.c',
    'vfscanf.c',
]

hdrs_tinystdio = [
    'dtoa_engine.h',
    'ftoa_engine.h',
    'stdio.h',
    'stdio-bufio.h',
    'stdio_private.h',
    'xtoa_fast.h',
]

install_headers(['stdio.h', 'stdio-bufio.h'],
		install_dir: include_dir
	       )

srcs_tinystdio_use = []
foreach file : srcs_tinystdio
  s_file = fs.replace_suffix(file, '.S')
  if file in srcs_machine
    message('libc/tinystdio/' + file + ': machine overrides generic')
  elif s_file in srcs_machine
    message('libc/tinystdio/' + s_file + ': machine overrides generic')
  else
    srcs_tinystdio_use += file
  endif
endforeach

src_tinystdio = files(srcs_tinystdio_use)
