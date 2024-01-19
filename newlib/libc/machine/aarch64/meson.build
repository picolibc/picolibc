#
# SPDX-License-Identifier: BSD-3-Clause
#
# Copyright © 2019 Keith Packard, 
# Copyright © 2020 Anthony Anderson
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

srcs_machine = [
    'memchr-stub.c',
    'memchr.S',
    'memcmp-stub.c',
    'memcmp.S',
    'memcpy-stub.c',
    'memcpy.S',
    'memmove-stub.c',
    'memmove.S',
    'memrchr-stub.c',
    'memrchr.S',
    'memset-stub.c',
    'memset.S',
    'rawmemchr-stub.c',
    'rawmemchr.S',
    'setjmp.S',
    'stpcpy-stub.c',
    'stpcpy.S',
    'strchr-stub.c',
    'strchr.S',
    'strchrnul-stub.c',
    'strchrnul.S',
    'strcmp-stub.c',
    'strcmp.S',
    'strcpy-stub.c',
    'strcpy.S',
    'strlen-stub.c',
    'strlen.S',
    'strncmp-stub.c',
    'strncmp.S',
    'strnlen-stub.c',
    'strnlen.S',
    'strrchr-stub.c',
    'strrchr.S',
]

subdir('sys')
subdir('machine')

foreach target : targets
    value = get_variable('target_' + target)
    set_variable('lib_machine' + target,
        static_library('machine' + target,
            srcs_machine,
            pic: false,
            include_directories: inc,
            c_args: value[1] + c_args + arg_fnobuiltin))
endforeach
