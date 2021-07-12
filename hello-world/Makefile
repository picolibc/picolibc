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

CFLAGS=--oslib=semihost --crt0=hosted -Os -g

CPP_RISCV=riscv64-unknown-elf-g++ -specs=picolibcpp.specs 
CC_RISCV=riscv64-unknown-elf-gcc -specs=picolibc.specs 
CFLAGS_RISCV=$(CFLAGS) -march=rv32imac -mabi=ilp32 -Wl,-Thello-world-riscv.ld

CPP_ARM=arm-none-eabi-c++ -specs=picolibcpp.specs 
CC_ARM=arm-none-eabi-gcc -specs=picolibc.specs 
CFLAGS_ARM=$(CFLAGS) -mcpu=cortex-m3 -Wl,-Thello-world-arm.ld

CPP_AARCH64=aarch64-linux-gnu-g++ -specs=picolibcpp.specs 
CC_AARCH64=aarch64-linux-gnu-gcc -specs=picolibc.specs 
CFLAGS_AARCH64=$(CFLAGS) -Wl,-Thello-world-aarch64.ld

all: hello-world-riscv.elf hello-world-arm.elf hello-world-aarch64.elf printf.elf printf-int.elf printf-float.elf \
	hello-world++-riscv.elf hello-world++-arm.elf hello-world++-aarch64.elf


hello-world-riscv.elf: hello-world.c
	$(CC_RISCV) $(CFLAGS_RISCV) -o $@ $^ -Wl,-Map=hello-world-riscv.map

hello-world-arm.elf: hello-world.c
	$(CC_ARM) $(CFLAGS_ARM) -o $@ $^ -Wl,-Map=hello-world-arm.map

hello-world-aarch64.elf: hello-world.c
	$(CC_AARCH64) $(CFLAGS_AARCH64) -o $@ $^ -Wl,-Map=hello-world-aarch64.map

clean:
	rm -f *.elf *.map

hello-world++-native.elf: hello-world++.c++
	c++ -o $@ $^

hello-world++-arm.elf:  hello-world++.c++
	$(CPP_ARM) $(CFLAGS_ARM) -Wl,-Map=hello-world++-arm.map -o $@ $^

hello-world++-riscv.elf:  hello-world++.c++
	$(CPP_RISCV) $(CFLAGS_RISCV) -Wl,-Map=hello-world++-riscv.map -o $@ $^

hello-world++-aarch64.elf:  hello-world++.c++
	$(CPP_AARCH64) $(CFLAGS_AARCH64) -Wl,-Map=hello-world++-aarch64.map -o $@ $^

printf.elf: printf.c
	$(CC_ARM) $(CFLAGS_ARM) -o $@ $^ -Wl,-Map=printf.map

printf-int.elf: printf.c
	$(CC_ARM) $(CFLAGS_ARM) -DPICOLIBC_INTEGER_PRINTF_SCANF -o $@ $^ -Wl,-Map=printf-int.map

printf-float.elf: printf.c
	$(CC_ARM) $(CFLAGS_ARM) -DPICOLIBC_FLOAT_PRINTF_SCANF -o $@ $^ -Wl,-Map=printf-float.map
