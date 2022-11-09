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

# Zephyr-specific hooks for picolibc

if(CONFIG_PICOLIBC_USE_MODULE)
  zephyr_include_directories(${PROJECT_BINARY_DIR}/picolibc/include)

  zephyr_compile_definitions(__LINUX_ERRNO_EXTENSIONS__)

  function (picolibc_option_true name option)
    if(${option})
      set(${name} 1 PARENT_SCOPE)
    else()
      set(${name} 0 PARENT_SCOPE)
    endif()
  endfunction()

  function (picolibc_option_false name option)
    if(${option})
      set(${name} 0 PARENT_SCOPE)
    else()
      set(${name} 1 PARENT_SCOPE)
    endif()
  endfunction()

  # Map Zephyr options to Picolibc options

  picolibc_option_true(FAST_STRCMP CONFIG_PICOLIBC_FAST_STRCMP)
  picolibc_option_true("_WANT_IO_C99_FORMATS" CONFIG_PICOLIBC_IO_C99_FORMATS)
  picolibc_option_true("_WANT_IO_LONG_LONG" CONFIG_PICOLIBC_IO_LONG_LONG)
  picolibc_option_false("_WANT_IO_PERCENT_B" CONFIG_PICOLIBC_IO_PERCENT_B)
  picolibc_option_true("FORMAT_DEFAULT_DOUBLE" CONFIG_PICOLIBC_IO_FLOAT)
  picolibc_option_false("FORMAT_DEFAULT_INTEGER" CONFIG_PICOLIBC_IO_FLOAT)
  set(FORMAT_DEFAULT_FLOAT 0)
  picolibc_option_true("_IO_FLOAT_EXACT" CONFIG_PICOLIBC_IO_FLOAT_EXACT)
  picolibc_option_true("__HAVE_LOCALE_INFO__" CONFIG_PICOLIBC_LOCALE_INFO)
  picolibc_option_true("__HAVE_LOCALE_INFO_EXTENDED__" CONFIG_PICOLIBC_LOCALE_EXTENDED_INFO)
  picolibc_option_true("_MB_CAPABLE" CONFIG_PICOLIBC_MULTIBYTE)
  picolibc_option_false("__SINGLE_THREAD__" CONFIG_PICOLIBC_MULTITHREAD)
  picolibc_option_true("PICOLIBC_TLS" CONFIG_THREAD_LOCAL_STORAGE)
  picolibc_option_true("NEWLIB_GLOBAL_ERRNO" CONFIG_PICOLIBC_GLOBAL_ERRNO)
  picolibc_option_true(PREFER_SIZE_OVER_SPEED CONFIG_SIZE_OPTIMIZATIONS)

  if(NOT DEFINED CONFIG_THREAD_LOCAL_STORAGE)
    set(__PICOLIBC_ERRNO_FUNCTION z_errno_wrap)
  endif()

  # Fetch zephyr compile flags from interface
  get_property(PICOLIBC_EXTRA_COMPILE_OPTIONS TARGET zephyr_interface PROPERTY INTERFACE_COMPILE_OPTIONS)

  # Enable POSIX APIs

  zephyr_compile_options(-D_POSIX_C_SOURCE=200809)

  # Link to C library and libgcc

  zephyr_link_libraries(c -lgcc)

  # Provide an alias target for zephyr to use

  add_custom_target(PicolibcBuild)
  add_dependencies(PicolibcBuild c)
endif()
