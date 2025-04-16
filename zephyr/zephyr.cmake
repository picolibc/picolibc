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

  function (picolibc_option_any_true name option_list)
    foreach(option ${option_list})
      if (${option})
        set(${name} 1 PARENT_SCOPE)
        return()
      endif()
    endforeach()
    set(${name} 0 PARENT_SCOPE)
  endfunction()

  function (picolibc_option_false name option)
    if(${option})
      set(${name} 0 PARENT_SCOPE)
    else()
      set(${name} 1 PARENT_SCOPE)
    endif()
  endfunction()

  function (picolibc_option_val name val)
    set(${name} ${val} PARENT_SCOPE)
  endfunction()

  # Map Zephyr options to Picolibc options

  picolibc_option_true(__FAST_STRCMP CONFIG_PICOLIBC___FAST_STRCMP)
  picolibc_option_true(__IO_C99_FORMATS CONFIG_PICOLIBC_IO_C99_FORMATS)
  picolibc_option_true(__IO_MINIMAL_LONG_LONG CONFIG_PICOLIBC_IO_MINIMAL_LONG_LONG)
  picolibc_option_false(__IO_PERCENT_B CONFIG_PICOLIBC_IO_PERCENT_B)
  picolibc_option_true(__IO_POS_ARGS CONFIG_PICOLIBC_IO_POS_ARGS)
  picolibc_option_true(__IO_LONG_DOUBLE CONFIG_PICOLIBC_IO_LONG_DOUBLE)
  picolibc_option_true(__IO_SMALL_ULTOA CONFIG_PICOLIBC_IO_SMALL_ULTOA)
  if (CONFIG_PICOLIBC_IO_LONG_LONG)
    set(io_default l)
  elseif (CONFIG_PICOLIBC_IO_INTEGER)
    set(io_default i)
  elseif (CONFIG_PICOLIBC_IO_MINIMAL)
    set(io_default m)
  else()
    set(io_default d)
  endif()
  picolibc_option_val(__IO_DEFAULT ${io_default})
  picolibc_option_true(__IO_FLOAT_EXACT CONFIG_PICOLIBC_IO_FLOAT_EXACT)
  picolibc_option_true(__MB_CAPABLE CONFIG_PICOLIBC_MULTIBYTE)
  picolibc_option_false(__SINGLE_THREAD CONFIG_PICOLIBC_MULTITHREAD)
  picolibc_option_true(__THREAD_LOCAL_STORAGE CONFIG_THREAD_LOCAL_STORAGE)
  picolibc_option_true(__GLOBAL_ERRNO CONFIG_PICOLIBC_GLOBAL_ERRNO)
  picolibc_option_any_true(__PREFER_SIZE_OVER_SPEED "CONFIG_SIZE_OPTIMIZATIONS_AGGRESSIVE;CONFIG_SIZE_OPTIMIZATIONS")
  picolibc_option_true(__ASSERT_VERBOSE CONFIG_PICOLIBC_ASSERT_VERBOSE)

  if(NOT DEFINED CONFIG_THREAD_LOCAL_STORAGE)
    set(__PICOLIBC_ERRNO_FUNCTION z_errno_wrap)
  endif()

  # Fetch zephyr compile flags from interface
  get_property(PICOLIBC_EXTRA_COMPILE_OPTIONS TARGET zephyr_interface PROPERTY INTERFACE_COMPILE_OPTIONS)

  # Disable LTO
  list(APPEND PICOLIBC_EXTRA_COMPILE_OPTIONS $<TARGET_PROPERTY:compiler,prohibit_lto>)

  # Tell Zephyr about the module built picolibc library to link against.
  # We need to construct the absolute path to picolibc in same fashion as CMake
  # defines the library file name and location, because a generator expression
  # cannot be used as the CMake linker rule definition doesn't supports them.
  set_property(TARGET linker PROPERTY c_library
      ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}c${CMAKE_STATIC_LIBRARY_SUFFIX}
  )
  zephyr_system_include_directories($<TARGET_PROPERTY:c,INTERFACE_SYSTEM_INCLUDE_DIRECTORIES>)

  # Provide an alias target for zephyr to use
  add_custom_target(PicolibcBuild)
  add_dependencies(PicolibcBuild c)
endif()
