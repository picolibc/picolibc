#
# SPDX-License-Identifier: BSD-3-Clause
#
# Copyright © 2021 Keith Packard
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

target_sources(semihost PRIVATE
  e9_exit.c
  e9_io.c
  ../../fake/fake_close.c
  ../../fake/fake_fstat.c
  ../../fake/fake_getentropy.c
  ../../fake/fake_getpid.c
  ../../fake/fake_gettimeofday.c
  ../../fake/fake_isatty.c
  ../../fake/fake_kill.c
  ../../fake/fake_lseek.c
  ../../fake/fake_open.c
  ../../fake/fake_read.c
  ../../fake/fake_stat.c
  ../../fake/fake_unlink.c
  ../../fake/fake_write.c
  )

add_executable(bios bios.S)

target_compile_options(bios PRIVATE
  -m32
  -nostdlib
  -T bios.ld
  -Wl,-b,elf32-i386
  -Wl,-melf_i386
  ${PICOLIBC_COMPILE_OPTIONS}
  )

target_link_options(bios PRIVATE
  -m32
  -nostdlib
  -T ${CMAKE_CURRENT_SOURCE_DIR}/bios.ld
  -Wl,-b,elf32-i386
  -Wl,-melf_i386
  ${PICOLIBC_COMPILE_OPTIONS}
  )

target_include_directories(bios SYSTEM PUBLIC ${PICOLIBC_INCLUDE_DIRECTORIES})

add_custom_target(bios.bin objcopy -O binary --only-section=.text bios bios.bin)

add_dependencies(bios.bin bios)

add_dependencies(semihost bios.bin)
