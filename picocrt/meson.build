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

src_picocrt = []

machine_dir = 'machine' / host_cpu_family
picocrt_march_add=''
if fs.is_dir(machine_dir)
  subdir(machine_dir)
else
  src_picocrt = files('shared/crt0.c')
endif

foreach target : targets
  value = get_variable('target_' + target)

  instdir = join_paths(lib_dir, value[0])

  if picocrt_march_add != ''
    new_cflags = []
    foreach cflag : value[1]
      if cflag.startswith('-march') and not cflag.contains(picocrt_march_add)
	cflag = cflag + picocrt_march_add
      endif
      new_cflags += cflag
    endforeach
    value = [value[0], new_cflags]
  endif

  if target == ''
    crt_name = 'crt0.o'
    crt_hosted_name = 'crt0-hosted.o'
    crt_minimal_name = 'crt0-minimal.o'
    crt_semihost_name = 'crt0-semihost.o'
    libcrt_name = 'crt0'
    libcrt_hosted_name = 'crt0-hosted'
    libcrt_minimal_name = 'crt0-minimal'
    libcrt_semihost_name = 'crt0-semihost'
  else
    crt_name = join_paths(target, 'crt0.o')
    crt_hosted_name = join_paths(target, 'crt0-hosted.o')
    crt_minimal_name = join_paths(target, 'crt0-minimal.o')
    crt_semihost_name = join_paths(target, 'crt0-semihost.o')
    libcrt_name = join_paths(target, 'libcrt0')
    libcrt_hosted_name = join_paths(target, 'libcrt0-hosted')
    libcrt_minimal_name = join_paths(target, 'libcrt0-minimal')
    libcrt_semihost_name = join_paths(target, 'libcrt0-semihost')
  endif

  crt0_name = 'crt0' + target
  crt0_hosted_name = 'crt0_hosted' + target
  crt0_minimal_name = 'crt0_minimal' + target
  crt0_semihost_name = 'crt0_semihost' + target

  # The normal variant does not call 'exit' after return from main (c lingo: freestanding execution environment)
  _crt = executable(crt_name,
		    src_picocrt,
		    include_directories : inc,
		    install : true,
		    install_dir : instdir,
		    c_args : value[1] + arg_fnobuiltin + ['-ffreestanding'],
		    link_args : value[1] + ['-r', '-ffreestanding'])

  set_variable(crt0_name,
	       _crt.extract_objects(src_picocrt)
	      )

  if enable_picocrt_lib
    static_library(libcrt_name,
                   [],
                   install : true,
                   install_dir : instdir,
                   pic: false,
                   objects : [_crt.extract_objects(src_picocrt)])
  endif

  # The 'hosted' variant calls 'exit' after return from main (c lingo: hosted execution environment)
  _crt = executable(crt_hosted_name,
		    src_picocrt,
		    include_directories : inc,
		    install : true,
		    install_dir : instdir,
		    c_args : value[1] + arg_fnobuiltin + ['-ffreestanding', '-DCRT0_EXIT'],
		    link_args : value[1] + ['-r', '-ffreestanding'])

  set_variable(crt0_hosted_name,
	       _crt.extract_objects(src_picocrt)
	      )

  if enable_picocrt_lib
    static_library(libcrt_hosted_name,
                   [],
                   install : true,
                   install_dir : instdir,
                   pic: false,
                   objects : [_crt.extract_objects(src_picocrt)])
  endif

  # The 'minimal' variant doesn't call exit, nor does it invoke any constructors
  _crt = executable(crt_minimal_name,
		    src_picocrt,
		    include_directories : inc,
		    install : true,
		    install_dir : instdir,
		    c_args : value[1] + arg_fnobuiltin + ['-ffreestanding', '-DCONSTRUCTORS=0'],
		    link_args : value[1] + ['-r', '-ffreestanding'])

  set_variable(crt0_minimal_name,
	       _crt.extract_objects(src_picocrt)
	      )

  if enable_picocrt_lib
    static_library(libcrt_minimal_name,
                   [],
                   install : true,
                   install_dir : instdir,
                   pic: false,
                   objects : [_crt.extract_objects(src_picocrt)])
  endif

  if has_arm_semihost
    # The 'semihost' variant calls sys_semihost_get_cmdline to build argv
    # and calls exit when main returns
    _crt = executable(crt_semihost_name,
		      src_picocrt,
		      include_directories : inc,
		      install : true,
		      install_dir : instdir,
		      c_args : value[1] + arg_fnobuiltin + ['-ffreestanding', '-DCRT0_EXIT', '-DCRT0_SEMIHOST'],
		      link_args : value[1] + ['-r', '-ffreestanding'])

    set_variable(crt0_semihost_name,
		 _crt.extract_objects(src_picocrt)
		)

    if enable_picocrt_lib
      static_library(libcrt_semihost_name,
                     [],
                     install : true,
                     install_dir : instdir,
                     pic: false,
                     objects : [_crt.extract_objects(src_picocrt)])
    endif
  endif
endforeach
