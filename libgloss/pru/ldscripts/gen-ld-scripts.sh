#!/bin/bash
# SPDX-License-Identifier: BSD-2-Clause-FreeBSD
#
# Copyright 2018-2019 Dimitar Dimitrov <dimitar@dinux.eu>
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
# OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
# NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
# THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


# Generate additional MCU-specific linker scripts by using the
# default PRU-LD linker script.
#
# We do it automatically so that:
#  1. We only change the default linker script in binutils.
#  2. All the default script complexity stays in binutils.
#  3. Here in libgloss we only bump the memory sizes to
#     allow large test programs to be executed.

dump_modified()
{
  IMEM_SIZE=$1
  DMEM_SIZE=$2
  HEAP_SIZE=$3
  STACK_SIZE=$4

  echo "/* WARNING: automatically generated from the default pru-ld script! */"
  echo -e "\n\n"
  pru-ld --verbose | awk "
BEGIN { LDV_MARKER = 0; }
{
  if (\$0 == \"==================================================\" )
    {
      LDV_MARKER++;
    }
  else if (LDV_MARKER != 1)
    {
    }
  else if (\$0 ~ /^  imem.*ORIGIN =.*LENGTH =/)
    {
      print \"  imem   (x)   : ORIGIN = 0x20000000, LENGTH = $IMEM_SIZE\"
    }
  else if (\$0 ~ /^  dmem.*ORIGIN =.*LENGTH =/)
    {
      print \"  dmem   (rw!x) : ORIGIN = 0x0, LENGTH = $DMEM_SIZE\"
    }
  else if (\$0 ~ /^__HEAP_SIZE = DEFINED/)
    {
      print \"__HEAP_SIZE = DEFINED(__HEAP_SIZE) ? __HEAP_SIZE : $HEAP_SIZE ;\";
    }
  else if (\$0 ~ /^__STACK_SIZE = DEFINED/)
    {
      print \"__STACK_SIZE = DEFINED(__STACK_SIZE) ? __STACK_SIZE : $STACK_SIZE ;\";
    }
  else
    {
      print \$0;
    }
}"
}

#             IMEM DMEM   HEAP_SIZE    STACK_SIZE
dump_modified 256K 65536K "32 * 1024 * 1024" "1024 * 1024" | tee pruelf-sim.x
