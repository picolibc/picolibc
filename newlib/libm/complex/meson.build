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
dsrc_complex = [
  'cabs.c',
  'cacos.c',
  'cacosh.c',
  'carg.c',
  'casin.c',
  'casinh.c',
  'catan.c',
  'catanh.c',
  'ccos.c',
  'ccosh.c',
  'cephes_subr.c',
  'cexp.c',
  'cimag.c',
  'clog.c',
  'clog10.c',
  'conj.c',
  'cpow.c',
  'cproj.c',
  'creal.c',
  'csin.c',
  'csinh.c',
  'csqrt.c',
  'ctan.c',
  'ctanh.c',
]

fsrc_complex = [
  'cabsf.c',
  'casinf.c',
  'ccosf.c',
  'cimagf.c',
  'cprojf.c',
  'csqrtf.c',
  'cacosf.c',
  'casinhf.c',
  'ccoshf.c',
  'clogf.c',
  'clog10f.c',
  'crealf.c',
  'ctanf.c',
  'cacoshf.c',
  'catanf.c',
  'cephes_subrf.c',
  'conjf.c',
  'csinf.c',
  'ctanhf.c',
  'cargf.c',
  'catanhf.c',
  'cexpf.c',
  'cpowf.c',
  'csinhf.c',
]

lsrc_complex = [
  'cabsl.c',
  'creall.c',
  'cimagl.c',
  'ccoshl.c',
  'cacoshl.c',
  'clogl.c',
  'csqrtl.c',
  'cargl.c',
  'cprojl.c',
  'cexpl.c',
  'cephes_subrl.c',
  'cacosl.c',
  'ccosl.c',
  'casinl.c',
  'catanhl.c',
  'conjl.c',
  'cpowl.c',
  'ctanhl.c',
  'ctanl.c',
  'casinhl.c',
  'csinhl.c',
  'csinl.c',
  'catanl.c',
]

srcs_complex = dsrc_complex + fsrc_complex

hdrs_complex = [
    'cephes_subrf.h',
    'cephes_subr.h',
    'cephes_subrl.h',
]
foreach target : targets
	value = get_variable('target_' + target)
	set_variable('lib_complex' + target,
		static_library('complex' + target,
			srcs_complex,
			pic: false,
			include_directories: inc,
			c_args: value[1]))
endforeach
