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

set(tests
  printf_scanf
  printf-tests
  rand
  regex
  ungetc
  fenv
  long_double
  math_errhandling
  malloc
  tls
  ffs
  setjmp
  atexit
  on_exit
  math-funcs
  timegm
  time-tests
  test-strtod
  test-strchr
  test-memset
  test-put
  test-efcvt
  malloc_stress
  posix-io
  )

set(tests_fail
  test-except
  )
  
function(picolibc_test test)
  add_executable(${test} ${test}.c ${ARGN})

  target_link_libraries(${test} LINK_PUBLIC ${PICOLIBC_TEST_LINK_LIBRARIES})

  target_compile_options(${test} PRIVATE "-fno-builtin" ${PICOLIBC_COMPILE_OPTIONS})

  target_link_options(${test} PRIVATE ${TARGET_COMPILE_OPTIONS})

  add_test(
    NAME ${test}
    COMMAND "${PICOLIBC_SCRIPTS}/run-thumbv7m" $<TARGET_FILE:${test}>
    )
  set_tests_properties(${test} PROPERTIES SKIP_RETURN_CODE 77)
endfunction()

foreach(test IN LISTS tests)
  picolibc_test(${test})
endforeach()

foreach(test IN LISTS tests_fail)
  picolibc_test(${test})
  set_tests_properties(${test} PROPERTIES WILL_FAIL ON)
endforeach()

picolibc_test(rounding-mode rounding-mode-sub.c)
