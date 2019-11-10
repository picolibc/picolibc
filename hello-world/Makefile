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

CFLAGS=--specs=picolibc.specs --semihost -O0 -g

CC_RISCV=riscv64-unknown-elf-gcc
CFLAGS_RISCV=$(CFLAGS) -march=rv32imac -mabi=ilp32 -Wl,-Map=hello-world-riscv.map  -Wl,-Thello-world-riscv.ld

CC_ARM=arm-none-eabi-gcc
CFLAGS_ARM=$(CFLAGS) -mlittle-endian -mcpu=cortex-m3 -mthumb  -Wl,-Map=hello-world-arm.map  -Wl,-Thello-world-arm.ld

all: hello-world-riscv.elf hello-world-arm.elf

hello-world-riscv.elf: hello-world.c
	$(CC_RISCV) $(CFLAGS_RISCV) -o $@ $^

hello-world-arm.elf: hello-world.c
	$(CC_ARM) $(CFLAGS_ARM) -o $@ $^

clean:
	rm -f *.elf *.map
