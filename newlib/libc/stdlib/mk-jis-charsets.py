#
# SPDX-License-Identifier: BSD-3-Clause
#
# Copyright © 2025 Keith Packard
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

'''
Generate multi-byte translation table for EUC-JP
'''

import copy
import re
import sys

invalid_unicode = 0xffff
invalid_jis = 0xffff
split_codes = 0x4000
struct_name = 'jis_charmap'
head = '__euc_jp_'

def to_code(byte0, byte1, byte2):
    if byte0 is not None:
        code = (byte0, byte1, byte2)
    elif byte1 is not None:
        code = (byte1, byte2)
    else:
        code = (byte2, )
    return code

def to_ucode(byte0, byte1, byte2):
    try:
        # Missing from python map
        if (byte0, byte1, byte2) == (0x8f, 0xa2, 0xb7):
            return 0xff5e
        ucode = ord(bytes(to_code(byte0, byte1, byte2)).decode('EUC_JP'))
        # ignore JIS x803 encodings for ASCII range
        if ucode < 0x80 and (byte0 is not none or byte1 is not none):
            ucode = invalid_ucode
    except:
        ucode = invalid_unicode
    return ucode

def from_ucode(ucode):
    # leave identity ascii map out of this table
    if ucode <= 0x7f:
        return invalid_jis
    # Missing from python map
    if ucode == 0xff5e:
        return bytes((0x8f, 0xa2, 0xb7))
    try:
        return chr(ucode).encode('EUC_JP')
    except:
        return invalid_jis

class Row:

    def __init__(self, byte0 = None, byte1 = None, map = None):
        self.row=[invalid_unicode]*256
        self.byte0 = byte0
        self.byte1 = byte1
        self.first = 256
        self.last = 0
        for byte2 in range(0x0, 0x100):
            ucode = to_ucode(byte0, byte1, byte2)
            self.row[byte2] = ucode
            if ucode != invalid_unicode:
                if byte2 < self.first:
                    self.first = byte2
                if byte2 > self.last:
                    self.last = byte2
        if self.first > self.last:
            self.first = 1
            self.last = 0
        if map is not None:
            self.add_to_map(map)

    def dump(self):
        if self.byte0 is not None:
            print(f'    /* {self.byte0:#04x} {self.byte1:#04x}: {self.first:#04x} - {self.last:#04x} */')
        else:
            print(f'    /* {self.byte1:#04x}: {self.first:#04x} - {self.last:#04x}*/')
        start='    '
        for byte2 in range(self.first//16*16, self.last+1):
            if byte2 % 16 == 15:
                end='\n'
            else:
                end=''
            if byte2 < self.first:
                print(f'{start:s}       ', end=end)
            else:
                print(f'{start:s}{self.row[byte2]:#06x},', end='')
            if byte2 % 16 == 15:
                start='\n    '
            else:
                start=' '
        print('')

    def add_to_map(self, map):
        for byte2 in range(self.first, self.last+1):
            if self.row[byte2] != invalid_unicode and self.row[byte2] not in map:
                map[self.row[byte2]] = to_code(self.byte0, self.byte1, byte2)
        
class URow:

    def __init__(self, byte1, map):
        self.byte1 = byte1
        self.first = 256
        self.last = 0
        self.row = [invalid_jis]*256
        for byte2 in range(0x0, 0x100):
            ucode = self.byte1 * 256 + byte2
            jis = from_ucode(ucode)
            bytes=b''
            if jis != invalid_jis:
                bytes = jis
                if byte2 < self.first:
                    self.first = byte2
                if byte2 > self.last:
                    self.last = byte2
            self.row[byte2] = bytes
        if self.first > self.last:
            self.first = 1
            self.last = 0

    def value(self, byte2):
        v = self.row[byte2]
        if len(v) == 0:
            ret = invalid_jis
        elif len(v) == 1:
            ret = v[0]
        elif len(v) == 2:
            ret = v[0] * 256 + v[1]
            if ret and ret < 0x8000:
                print(f'weird return {ret:#06x}', file=sys.stderr)
                sys.exit(1)
        else:
            ret = v[1] * 256 + v[2]
            if ret and ret < 0x8000:
                print(f'weird return {ret:#06x}', file=sys.stderr)
                sys.exit(1)
            ret -= 0x8000
        return ret

    def dump(self):
        print(f'    /* {self.byte1:#04x}: {self.first:#04x} - {self.last:#04x} */')
        start='    '
        for byte2 in range(self.first//16*16, self.last+1):
            if byte2 % 16 == 15:
                end='\n'
            else:
                end=''
            if byte2 < self.first:
                print(f'{start:s}       ', end=end)
            else:
                value = self.row[byte2]
                
                print(f'{start:s}{self.value(byte2):#06x},', end='')
            if byte2 % 16 == 15:
                start='\n    '
            else:
                start=' '
        print('')

def rows(byte0 = None, map = None):
    rows = [None]*256
    for byte1 in range(0x80, 0x100):
        rows[byte1] = Row(byte0 = byte0, byte1 = byte1, map=map)
    return rows

def u_rows(map):
    rows = [None]*256
    for byte1 in range(0x0, 0x100):
        rows[byte1] = URow(byte1, map)
    return rows
unicode_to_jis = {}


# These are easy to compute, so don't place them in the map
#row_1byte = Row()
#row_8e = Row(byte1 = 0x8e)

# Add 
rows_2byte = rows(map = unicode_to_jis)

rows_3byte = rows(byte0=0x8f, map=unicode_to_jis)

rows_unicode = u_rows(unicode_to_jis)

min_unicode = min(unicode_to_jis.keys())
max_unicode = max(unicode_to_jis.keys())

# Dump out 2 byte table

def dump_2byte(name, rows, first = None, last = None, do_map=True):
    print('')
    print(f'/* {name} tables */')

    if first is None:
        for byte1 in range(0, 0x100):
            row = rows[byte1]
            if row and row.last >= row.first:
                first = byte1
                break

    if last is None:
        for byte1 in range(0xff, -1, -1):
            row = rows[byte1]
            if row and row.last >= row.first:
                last = byte1
                break

    print(f'#ifdef define_{name}')
    print('')
    if do_map:
        print(f'#define {head}{name}_first_row {first:#04x}')
        print(f'#define {head}{name}_last_row {last:#04x}')
    else:
        row = rows[first]
        print(f'#define {head}{name}_first_col {row.first:#04x}')
        print(f'#define {head}{name}_last_col {row.last:#04x}')
    print('')
    print(f'#ifndef split_{name}')
    print(f'#define split_{name}_offset 0')
    print(f'#endif')
    print('')
    print(f'static const uint16_t {head}{name}_codes[] = {{')
    offset = 0
    first_col = 1
    last_col = 0
    dumped = False
    split = False
    for byte1 in range(first, last + 1):
        row = rows[byte1]
        if row:
            row.offset = offset
            row.split = split
            if row.last >= row.first:
                if not dumped:
                    if byte1 != first:
                        print(f'Table for {name} starts at {byte1:#04x} not {first:#04x}', file=sys.stderr)
                        sys.exit(1)
                    dumped = True
                else:
                    print('')
                if not split and offset + (row.last + 1 - row.first) > split_codes:
                    print(f'#ifdef split_{name}')
                    print(f'}};')
                    print('')
                    print(f'#define split_{name}_offset {split_codes - offset} /* {split_codes:#06x} - {offset} */')
                    print('')
                    print(f'static const uint16_t {head}{name}_codes_second[] = {{')
                    print(f'#endif')
                    print('')
                    row.split = True
                    split = True
                row.dump()
                offset += (row.last + 1 - row.first)
    print(f'}};')
    if do_map:
        print(f'static const struct {struct_name} {head}{name}_map[] = {{')
        for byte1 in range(first, last + 1):
            row = rows[byte1]
            if row:
                offset = row.offset
                if row.first > row.last:
                    offset=f'0xffff'
                elif row.split:
                    offset=f'{row.offset:5d} + split_{name}_offset'
                else:
                    offset=f'{row.offset:5d}'
                print(f'    {{ .first = {row.first:#04x}, .last = {row.last:#04x}, .offset = {offset} }}, /* {byte1:#02x} */')
            else:
                print(f'    {{ .first = 1, .last = 0, .offset = 0xffff }}')
        print(f'}};')
    print(f'#endif /* {name} */')


print(f'''\
/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2025 Keith Packard
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/* This file is auto-generated from mk-jis-charsets.py */
/* clang-format off */

#ifndef _JIS_CHARSETS_H_
#define _JIS_CHARSETS_H_

struct {struct_name} {{
    uint8_t  first;
    uint8_t  last;
    uint16_t offset;
}};

#define {head}invalid_jis {invalid_jis:#06x}
#define {head}invalid_unicode {invalid_unicode:#06x}
#define {head}split_codes {split_codes:#06x}''')

dump_2byte('unicode', rows_unicode, last=0x9f)

dump_2byte('unicode_ff', rows_unicode, first=0xff, last=0xff, do_map=False)

dump_2byte('jis_x0208', rows_2byte)

dump_2byte('jis_x0213', rows_3byte)

print('')
print('#endif /* _JIS_CHARSETS_H_ */')
