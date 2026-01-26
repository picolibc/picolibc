#
# SPDX-License-Identifier: BSD-3-Clause
#
# Copyright © 2026 Keith Packard
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

# Build a set of empty libraries for compatibility with
# existing build scripts 

empty_lib_names = ['g', 'm', 'nosys']

foreach params : targets
  target = params['name']
  target_dir = params['dir']
  target_c_args = params['c_args']

  instdir = join_paths(lib_dir, target_dir)

  foreach lib_name : empty_lib_names
    if meson.version().version_compare('>=1.10')
      static_library(lib_name,
		     ['empty.c'],
                     build_subdir : target_dir,
		     install : really_install,
		     install_dir : instdir,
		     pic: false,
                     c_args: target_c_args + c_args,
		     objects : [])
    else
      target_lib_prefix = params['lib_prefix']
      static_library(join_paths(target_dir, target_lib_prefix + lib_name),
		     ['empty.c'],
		     install : really_install,
		     install_dir : instdir,
		     pic: false,
                     c_args: target_c_args + c_args,
		     objects : [])
    endif
  endforeach
endforeach
