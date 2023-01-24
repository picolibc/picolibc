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

# Add sources to libc, eliding any duplicate basenames
function(picolibc_sources_flags flags)

  # Get current sources
  get_property(current_sources_real TARGET c PROPERTY SOURCES)
  get_property(current_sources_fake TARGET c PROPERTY SOURCES_FAKE)
  set(current_sources ${current_sources_real} ${current_sources_fake})
  set(sources ${ARGN})

  foreach(source IN LISTS ARGN)
    # Compare desired addition to existing sources using basenames
    get_filename_component(source_base "${source}" NAME_WLE)
    foreach(current IN LISTS current_sources)
      get_filename_component(current_base "${current}" NAME_WLE)
      if("${source_base}" STREQUAL "${current_base}")
	list(REMOVE_ITEM sources "${source}")
      endif()
    endforeach()
  endforeach()

  # Add all files that aren't duplicated
  target_sources(c PRIVATE ${sources})

  # Set flags if specified
  if(flags)
    foreach(flag ${flags})
      foreach(source ${sources})
	set_property(SOURCE ${source}
	  TARGET_DIRECTORY c
	  APPEND PROPERTY COMPILE_OPTIONS ${flag})
      endforeach()
    endforeach()
  endif()
endfunction()

function(picolibc_sources)
  picolibc_sources_flags(0 ${ARGN})
endfunction()

function(picolibc_sources_fake)
  get_property(current_sources_fake TARGET c PROPERTY SOURCES_FAKE)
  list(APPEND current_sources_fake ${ARGN})
  set_property(TARGET c PROPERTY SOURCES_FAKE ${current_sources_fake})
endfunction()

function(picolibc_headers subdir)

  # Get current headers
  get_property(current_headers GLOBAL PROPERTY PICOLIBC_HEADERS)

  set(orig_headers ${ARGN})

  foreach(header IN LISTS ARGN)
    set(rel_header "${subdir}/${header}")
    set(include 1)
    foreach(current IN LISTS current_headers)
      if("${rel_header}" STREQUAL "${current}")
	list(REMOVE_ITEM orig_headers "${header}")
	set(include 0)
      endif()
    endforeach()
    if(include)
      list(APPEND current_headers "${rel_header}")
      configure_file("${header}" "${PROJECT_BINARY_DIR}/picolibc/include/${rel_header}")
    endif()
  endforeach()
  set_property(GLOBAL PROPERTY PICOLIBC_HEADERS ${current_headers})
  install(FILES ${orig_headers} DESTINATION "include/${subdir}")
endfunction()

function(_picolibc_supported_compile_options var)
  set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
  foreach(flag ${ARGN})
    string(MAKE_C_IDENTIFIER ${flag} flag_under)
    try_compile(${flag_under} "${PROJECT_BINARY_DIR}"
      "${CMAKE_CURRENT_SOURCE_DIR}/cmake/simple-main.c"
      COMPILE_DEFINITIONS "${PICOLIBC_TEST_COMPILE_OPTIONS};${PICOLIBC_FLAG_OPTIONS};${flag}"
      )
    if(${${flag_under}})
      list(APPEND options ${flag})
    endif()
  endforeach()
  set(${var} ${options} PARENT_SCOPE)
endfunction()

# These flags are needed to generate errors for invalid command line flags
# instead of just warnings
_picolibc_supported_compile_options(PICOLIBC_FLAG_OPTIONS
  -Werror=missing-declarations
  -Werror=unknown-attributes
  -Werror=attributes
  -Werror=unsupported-floating-point-opt
  -Werror=ignored-optimization-argument
  -Wno-unused-command-line-argument
  )

function(picolibc_supported_compile_options)
  _picolibc_supported_compile_options(options ${ARGN})
  set(PICOLIBC_COMPILE_OPTIONS ${PICOLIBC_COMPILE_OPTIONS} ${options} PARENT_SCOPE)
endfunction()

function(picolibc_flag flag)

  string(REPLACE "_" "-" flag_dash ${flag})
  string(REGEX REPLACE "^-" "" flag_trim ${flag_dash})
  string(TOLOWER ${flag_trim} flag_lower)
  set(flag_file "${CMAKE_CURRENT_SOURCE_DIR}/cmake/${flag_lower}.c")

  if (NOT EXISTS "${flag_file}")
    message(ERROR " Missing compiler test file ${flag_file}")
  endif()

  set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

  try_compile(${flag} "${PROJECT_BINARY_DIR}" "${flag_file}"
    COMPILE_DEFINITIONS "${PICOLIBC_FLAG_OPTIONS}"
    )

  if (${flag})
    set(${flag} ${flag} PARENT_SCOPE)
  endif()
endfunction()

