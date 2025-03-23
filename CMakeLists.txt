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

cmake_minimum_required(VERSION 3.20.0)

project(Picolibc VERSION 1.8.9 LANGUAGES C ASM)

# Set a default build type if none was specified
set(default_build_type "MinSizeRel")

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES AND NOT DEFINED ZEPHYR_BASE)
  message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
  set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE
      STRING "Choose the type of build." FORCE)
endif()

find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
  message(STATUS "Using ccache: ${CCACHE_PROGRAM}")
  set(CMAKE_C_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
endif()

if(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "arm64")
  set(CMAKE_SYSTEM_PROCESSOR "aarch64")
endif()

set(CMAKE_SYSTEM_SUB_PROCESSOR ${CMAKE_SYSTEM_PROCESSOR})

if(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "i686" OR
    ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64") 
  set(CMAKE_SYSTEM_PROCESSOR "x86")
endif()

if(ZEPHYR_BASE)
  if(NOT CONFIG_PICOLIBC_USE_MODULE)
    return()
  endif()
  include(zephyr/zephyr.cmake)
endif()

include(cmake/picolibc.cmake)

enable_testing()

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# Set all configure values to defaults for now

# Use atomics for fgetc/ungetc for re-entrancy
set(__ATOMIC_UNGETC 1)

# Always optimize strcmp for performance
if(NOT DEFINED __FAST_STRCMP)
  option(__FAST_STRCMP "Always optimize strcmp for performance" ON)
endif()

# use global errno variable
if(NOT DEFINED __GLOBAL_ERRNO)
  option(__GLOBAL_ERRNO "use global errno variable" OFF)
endif()

# use thread local storage
if(NOT DEFINED __THREAD_LOCAL_STORAGE)
  option(__THREAD_LOCAL_STORAGE "use thread local storage for static data" ON)
endif()

# use thread local storage for stack protection canary
if(NOT DEFINED __THREAD_LOCAL_STORAGE_STACK_GUARD)
  option(__THREAD_LOCAL_STORAGE_STACK_GUARD "use thread local storage for stack protection canary" OFF)
endif()

option(POSIX_CONSOLE "Use POSIX I/O for stdin/stdout/stderr" OFF)

# Optimize for space over speed

if(NOT DEFINED __PREFER_SIZE_OVER_SPEED)
  if(CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
    set(default_prefer_size ON)
  else()
    set(default_prefer_size OFF)
  endif()
  option(__PREFER_SIZE_OVER_SPEED "Prefer smaller code size" ${default_prefer_size})
endif()

# Use tiny stdio from gcc avr
option(__TINY_STDIO "Use tiny stdio from avr libc" ON)

if(NOT TLS_MODEL)
  set(TLS_MODEL "local-exec")
endif()

set(_ATEXIT_DYNAMIC_ALLOC 0)

set(__FSEEK_OPTIMIZATION 0)

set(__FVWRITE_IN_STREAMIO 0)

# Use bitfields in packed structs
picolibc_flag(__HAVE_BITFIELDS_IN_PACKED_STRUCTS)

# Compiler has support for flag to prevent mis-optimizing memcpy/memset patterns
picolibc_flag(__HAVE_CC_INHIBIT_LOOP_TO_LIBCALL)

# Compiler supports _Complex
picolibc_flag(__HAVE_COMPLEX)

set(__HAVE_FCNTL 0)

# IEEE fp funcs available
set(__IEEEFP_FUNCS 0)

# compiler supports INIT_ARRAY sections
set(__INIT_FINI_ARRAY 1)

# Support _init() and _fini() functions
set(__INIT_FINI_FUNCS 1)

# Enable MMU in pico crt startup
set(__PICOCRT_ENABLE_MMU 1)

# _set_tls and _init_tls functions available
if(NOT DEFINED __THREAD_LOCAL_STORAGE_API OR NOT __THREAD_LOCAL_STORAGE)
  set(__THREAD_LOCAL_STORAGE_API 0)
endif()

set(__SEMIHOST 1)

# math library does not set errno (offering only ieee semantics)
set(__IEEE_LIBM 1)

if(NOT DEFINED __IO_FLOAT_EXACT)
  option(__IO_FLOAT_EXACT "Provide exact binary/decimal conversion for printf/scanf" ON)
endif()

set(__PICO_EXIT 1)

if(NOT DEFINED __ASSERT_VERBOSE)
  option(__ASSERT_VERBOSE "Assert provides verbose information" ON)
endif()

if(NOT DEFINED __MB_CAPABLE)
  option(__MB_CAPABLE "Enable multi-byte support" OFF)
endif()

if(NOT DEFINED __MB_EXTENDED_CHARSETS_ALL)
  option(__MB_EXTENDED_CHARSETS_ALL "Enable UCS, ISO, Windows and JIS multibyte encodings" OFF)
endif()

if(NOT DEFINED __MB_EXTENDED_CHARSETS_UCS)
  option(__MB_EXTENDED_CHARSETS_UCS "Enable UCS encodings" OFF)
endif()

if(NOT DEFINED __MB_EXTENDED_CHARSETS_ISO)
  option(__MB_EXTENDED_CHARSETS_ISO "Enable ISO encodings" OFF)
endif()

if(NOT DEFINED __MB_EXTENDED_CHARSETS_WINDOWS)
  option(__MB_EXTENDED_CHARSETS_WINDOWS "Enable Windows encodings" OFF)
endif()

if(NOT DEFINED __MB_EXTENDED_CHARSETS_JIS)
  option(__MB_EXTENDED_CHARSETS_JIS "Enable JIS multibyte encodings" OFF)
endif()

set(__NANO_FORMATTED_IO OFF)

option(__NANO_MALLOC "Use smaller malloc implementation" ON)

set(_GLOBAL_ATEXIT OFF)

set(__UNBUF_STREAM_OPT OFF)

if(NOT DEFINED __IO_C99_FORMATS)
  option(__IO_C99_FORMATS "Support C99 formats in printf/scanf" ON)
endif()

if(NOT DEFINED __IO_LONG_LONG)
  option(__IO_LONG_LONG "Support long long in integer printf/scanf" OFF)
endif()

if(NOT DEFINED __IO_LONG_DOUBLE)
  option(__IO_LONG_DOUBLE "Support long double in printf/scanf" OFF)
endif()

if(NOT DEFINED __IO_MINIMAL_LONG_LONG)
  option(__IO_MINIMAL_LONG_LONG "Support long long in minimal printf/scanf" OFF)
endif()

if(NOT DEFINED __IO_POS_ARGS)
  option(__IO_POS_ARGS "Support positional args in integer printf/scanf" OFF)
endif()

option(__IO_FLOAT "Support floating point in printf/scanf by default" ON)

if(NOT DEFINED __IO_SMALL_ULTOA)
  option(__IO_SMALL_ULTOA "Avoid soft divide in printf" ON)
endif()

if(NOT DEFINED __IO_PERCENT_N)
  option(__IO_PERCENT_N "Support %n formats in printf" OFF)
endif()

if(NOT DEFINED __IO_PERCENT_B)
  option(__IO_PERCENT_B "Support %b/%B formats in printf/scanf" OFF)
endif()

if(NOT DEFINED __IO_WCHAR)
  option(__IO_WCHAR "Support %ls/%lc formats in printf even without multi-byte" OFF)
endif()

if(NOT DEFINED __IO_DEFAULT)
  set(__IO_DEFAULT d)
endif()

# math library sets errno
set(__MATH_ERRNO OFF)

set(_WANT_REGISTER_FINI OFF)

set(__WIDE_ORIENT OFF)

if(NOT DEFINED __ELIX_LEVEL)
  set(__ELIX_LEVEL 4)
endif()

# Use old math code for double funcs (0 no, 1 yes)
set(__OBSOLETE_MATH_FLOAT ON)

# Use old math code for double funcs (0 no, 1 yes)
set(__OBSOLETE_MATH_DOUBLE ON)

# Compute static memory area sizes at runtime instead of link time
set(__PICOCRT_RUNTIME_SIZE OFF)

if(NOT DEFINED __SINGLE_THREAD)
  option(__SINGLE_THREAD "Disable multithreading support" OFF)
endif()

set(NEWLIB_VERSION 4.3.0)
set(NEWLIB_MAJOR 4)
set(NEWLIB_MINOR 3)
set(NEWLIB_PATCH 0)

set(PICOLIBC_INCLUDE ${PROJECT_BINARY_DIR}/picolibc/include)

set(PICOLIBC_SCRIPTS "${CMAKE_CURRENT_SOURCE_DIR}/scripts")

include(CheckIncludeFile)

CHECK_INCLUDE_FILE(xtensa/config/core-isa.h _XTENSA_HAVE_CONFIG_CORE_ISA_H)

configure_file(picolibc.h.in "${PICOLIBC_INCLUDE}/picolibc.h")

set(INCLUDEDIR include)
set(LIBDIR .)
if(__THREAD_LOCAL_STORAGE)
  set(TLSMODEL "-ftls-model=${TLS_MODEL}")
endif()
set(LINK_SPEC "")
set(CC1_SPEC "")
set(CC1PLUS_SPEC "")
set(ADDITIONAL_LIBS "")
set(SPECS_EXTRA "")
set(SPECS_ISYSTEM "-isystem ${PROJECT_BINARY_DIR}/${include}")
set(SPECS_LIBPATH "-L${PROJECT_BINARY_DIR}")
set(SPECS_STARTFILE "${PROJECT_BINARY_DIR}/crt0.o")
string(APPEND SPECS_PRINTF "%{DPICOLIBC_FLOAT_PRINTF_SCANF:--defsym=vfprintf=__f_vfprintf}"
  " %{DPICOLIBC_FLOAT_PRINTF_SCANF:--defsym=vfscanf=__f_vfscanf}"
  " %{DPICOLIBC_DOUBLE_PRINTF_SCANF:--defsym=vfprintf=__d_vfprintf}"
  " %{DPICOLIBC_DOUBLE_PRINTF_SCANF:--defsym=vfscanf=__d_vfscanf}"
  " %{DPICOLIBC_INTEGER_PRINTF_SCANF:--defsym=vfprintf=__i_vfprintf}"
  " %{DPICOLIBC_INTEGER_PRINTF_SCANF:--defsym=vfscanf=__i_vfscanff}"
  " %{DPICOLIBC_MINIMAL_PRINTF_SCANF:--defsym=vfprintf=__m_vfprintf}"
  " %{DPICOLIBC_MINIMAL_PRINTF_SCANF:--defsym=vfscanf=__i_vfscanff}"
  )
set(PREFIX "${PROJECT_BINARY_DIR}")

configure_file(picolibc.specs.in "${PROJECT_BINARY_DIR}/picolibc.specs" @ONLY)

set(PICOLIBC_COMPILE_OPTIONS
  "-nostdlib"
  "-D_LIBC"
  ${TLSMODEL}
  ${TARGET_COMPILE_OPTIONS}
  ${PICOLIBC_EXTRA_COMPILE_OPTIONS}
  ${PICOLIBC_MATH_FLAGS}
  )

# Strip out any generator expressions as those cannot be used with
# try_compile

foreach(c_option "${PICOLIBC_COMPILE_OPTIONS}")
  if(NOT "${c_option}" MATCHES "\\$")
    list(APPEND PICOLIBC_TEST_COMPILE_OPTIONS "${c_option}")
  endif()
endforeach()

picolibc_supported_compile_options(
  "-fno-common"
  "-fno-stack-protector"
  "-ffunction-sections"
  "-fdata-sections"
  "-Wall"
  "-Wextra"
  "-Werror=implicit-function-declaration"
  "-Werror=vla"
  "-Warray-bounds"
  "-Wold-style-definition"
  "-frounding-math"
  "-fsignaling-nans"
  )

add_library(c STATIC)

target_compile_options(c PRIVATE ${PICOLIBC_COMPILE_OPTIONS})

set(PICOLIBC_INCLUDE_DIRECTORIES
  "${PICOLIBC_INCLUDE}")

target_include_directories(c SYSTEM PUBLIC ${PICOLIBC_INCLUDE_DIRECTORIES})

define_property(GLOBAL PROPERTY PICOLIBC_HEADERS
  BRIEF_DOCS "Installed header files"
  FULL_DOCS "These are names of header files which are to be installed.")

add_subdirectory(newlib)

install(TARGETS c
  LIBRARY DESTINATION lib
  PUBLIC_HEADER DESTINATION include)

option(TESTS "Enable tests" OFF)
if(TESTS)

  # This could use some generalization, but it's good enough to do
  # semihosting-based tests

  add_subdirectory(semihost)
  add_subdirectory(picocrt)

  set(PICOCRT_OBJ $<TARGET_OBJECTS:picocrt>)
  set(PICOCRT_SEMIHOST_OBJ $<TARGET_OBJECTS:picocrt-semihost>)

  # semihost and libc have mutual-dependencies, so place them in a
  # linker group

  set(PICOLIBC_TEST_LINK_LIBRARIES
    ${PICOCRT_SEMIHOST_OBJ}
    -Wl,--start-group c semihost -Wl,--end-group
    ${PICOLIBC_LINK_FLAGS}
    )

  add_subdirectory(test)

endif()

install(FILES ${CMAKE_BINARY_DIR}/picolibc.specs DESTINATION lib)
install(FILES ${CMAKE_BINARY_DIR}/picolibc/include/picolibc.h DESTINATION include)
