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
subdir('sys')
install_headers('stdio.h',
		install_dir: include_dir)

general_srcs_stdio = [
  'clearerr.c',
  'fclose.c',
  'fdopen.c',
  'feof.c',
  'ferror.c',
  'fflush.c',
  'fgetc.c',
  'fgetpos.c',
  'fgets.c',
  'fileno.c',
  'findfp.c',
  'flags.c',
  'fopen.c',
  'fprintf.c',
  'fputc.c',
  'fputs.c',
  'fread.c',
  'freopen.c',
  'fscanf.c',
  'fseek.c',
  'fseeko.c',
  'fsetpos.c',
  'ftell.c',
  'fvwrite.c',
  'fwalk.c',
  'fwrite.c',
  'getc.c',
  'getchar.c',
  'getc_u.c',
  'getchar_u.c',
  'getdelim.c',
  'getline.c',
  'gets.c',
  'makebuf.c',
  'perror.c',
  'printf.c',
  'putc.c',
  'putchar.c',
  'putc_u.c',
  'putchar_u.c',
  'puts.c',
  'refill.c',
  'remove.c',
  'rewind.c',
  'rget.c',
  'scanf.c',
  'sccl.c',
  'setbuf.c',
  'setbuffer.c',
  'setlinebuf.c',
  'setvbuf.c',
  'sfputs.c',
  'sfputws.c',
  'sprint.c',
  'ssprint.c',
  'ssputs.c',
  'ssputws.c',
  'sswprint.c',
  'swprint.c',
  'wbufw.c',
  'snprintf.c',
  'sprintf.c',
  'sscanf.c',
  'stdio.c',
  'tmpfile.c',
  'tmpnam.c',
  'ungetc.c',
  'vdprintf.c',
  'vprintf.c',
  'vscanf.c',
  'vsnprintf.c',
  'vsprintf.c',
  'vsscanf.c',
  'wbuf.c',
  'wsetup.c',
]

general_int_srcs_stdio = [
  'fiprintf.c',
  'fiscanf.c',
  'iprintf.c',
  'iscanf.c',
  'siprintf.c',
  'siscanf.c',
  'sniprintf.c',
  'vdiprintf.c',
  'vfprintf.c',
  'vfwprintf.c',
  'viprintf.c',
  'viscanf.c',
  'vsiprintf.c',
  'vsiscanf.c',
  'vsniprintf.c',
]

if newlib_nano_formatted_io == false
  general_srcs_stdio += general_int_srcs_stdio
endif

elix_2_int_srcs_stdio = [
  'asiprintf.c',
  'vasiprintf.c',
]

elix_2_srcs_stdio = [
  'asprintf.c',
  'fcloseall.c',
  'fseeko.c',
  'ftello.c',
  'getw.c',
  'mktemp.c',
  'putw.c',
  'vasprintf.c',
]

if newlib_nano_formatted_io == false
  elix_2_srcs_stdio += elix_2_int_srcs_stdio
endif

elix_4_int_srcs_stdio = [
  'asniprintf.c',
  'diprintf.c',
  'vasniprintf.c',
]

elix_4_srcs_stdio = [
  'asnprintf.c',
  'clearerr_u.c',
  'dprintf.c',
  'feof_u.c',
  'ferror_u.c',
  'fflush_u.c',
  'fgetc_u.c',
  'fgets_u.c',
  'fgetwc.c',
  'fgetwc_u.c',
  'fgetws.c',
  'fgetws_u.c',
  'fileno_u.c',
  'fmemopen.c',
  'fopencookie.c',
  'fpurge.c',
  'fputc_u.c',
  'fputs_u.c',
  'fputwc.c',
  'fputwc_u.c',
  'fputws.c',
  'fputws_u.c',
  'fread_u.c',
  'fsetlocking.c',
  'funopen.c',
  'fwide.c',
  'fwprintf.c',
  'fwrite_u.c',
  'fwscanf.c',
  'getwc.c',
  'getwc_u.c',
  'getwchar.c',
  'getwchar_u.c',
  'open_memstream.c',
  'putwc.c',
  'putwc_u.c',
  'putwchar.c',
  'putwchar_u.c',
  'stdio_ext.c',
  'swprintf.c',
  'swscanf.c',
  'ungetwc.c',
  'vasnprintf.c',
  'vswprintf.c',
  'vswscanf.c',
  'vwprintf.c',
  'vwscanf.c',
  'wprintf.c',
  'wscanf.c',
]

if newlib_nano_formatted_io == false
  elix_4_srcs_stdio += elix_4_int_srcs_stdio
endif

elix_srcs_stdio = []

if newlib_elix_level >= 2
  elix_srcs_stdio += elix_2_srcs_stdio
endif

if newlib_elix_level >= 4
  elix_srcs_stdio += elix_4_srcs_stdio
endif

nano_printf_srcs_stdio = [
  'nano-svfprintf.c',
  'nano-svfscanf.c',
  'nano-vfprintf.c',
  'nano-vfprintf_float.c',
  'nano-vfprintf_i.c',
  'nano-vfscanf.c',
  'nano-vfscanf_float.c',
  'nano-vfscanf_i.c',
  'svfiwprintf.c',
  'svfwprintf.c',
  'vfiwprintf.c',
  'svfiwscanf.c',
  'svfwscanf.c',
  'vfiwscanf.c',
  'vfwscanf.c',
]

std_printf_srcs_stdio = [
  'svfiprintf.c',
  'svfprintf.c',
  'svfiscanf.c',
  'svfscanf.c',
  'vfiprintf.c',
  'vfscanf.c',
  'vfiscanf.c',
  'svfiwprintf.c',
  'svfwprintf.c',
  'vfiwprintf.c',
  'svfiwscanf.c',
  'svfwscanf.c',
  'vfiwscanf.c',
  'vfwscanf.c',
]

srcs_stdio = general_srcs_stdio

if newlib_nano_formatted_io == false
  srcs_stdio += general_int_srcs_stdio
  srcs_stdio += std_printf_srcs_stdio
else
  srcs_stdio += nano_printf_srcs_stdio
endif

srcs_stdio += elix_srcs_stdio

hdrs_stdio = [
    'floatio.h',
    'fvwrite.h',
    'local.h',
    'nano-vfprintf_local.h',
    'nano-vfscanf_local.h',
    'stdio_ext.h',
    'stdio.h',
    'vfieeefp.h',
]

srcs_stdio_use = []
foreach file : srcs_stdio
  s_file = fs.replace_suffix(file, '.S')
  if file in srcs_machine
    message('libc/stdio/' + file + ': machine overrides generic')
  elif s_file in srcs_machine
    message('libc/stdio/' + s_file + ': machine overrides generic')
  else
    srcs_stdio_use += file
  endif
endforeach

src_stdio = files(srcs_stdio_use)
