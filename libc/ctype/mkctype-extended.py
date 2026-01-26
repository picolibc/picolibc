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
Translate Unicode data files into ctype data for ISO encodings
'''

import copy
import re

class DerivedCoreProperty:

    def __init__(self, line):
        fields = line.split()
        codepoints = fields[0].split('..')
        self.first = int(codepoints[0], 16)
        if len(codepoints) > 1:
            self.last = int(codepoints[1], 16)
        else:
            self.last = self.first
        self.value = fields[2]

class CodePoint:

    def case_value(self, field):
        if field:
            return int(field, 16)
        return self.code

    def __init__(self, line):

        fields = line.split(';')

        self.code = int(fields[0], 16)
        self.name = fields[1]
        self.category = fields[2]
        self.canonical_combining = fields[3]
        self.bidirectional = fields[4]
        self.decomposition = fields[5]
        self.decimal = fields[6]
        self.digit = fields[7]
        self.numeric = fields[8]
        self.mirrored = fields[9]
        self.name_1_0 = fields[10]
        self.comment = fields[11]
        self.uppercase = self.case_value(fields[12])
        self.lowercase = self.case_value(fields[13])
        self.titlecase = self.case_value(fields[14])

    def new_code(self, code):
        new = copy.copy(self)
        new.code = code
        new.uppercase = code
        new.lowercase = code
        new.titlecase = code
        return new

def load_derived_properties(name):
    ret = {}
    with open(name) as f:
        for line in f:
            line = line.strip()
            if re.search('^[0-9A-F]+.*;', line):
                d = DerivedCoreProperty(line)
                for c in range(d.first, d.last+1):
                    if c not in ret:
                        ret[c] = []
                    ret[c] += [d.value]
    return ret

def derived_properties(code):
    global DerivedProperties
    if not code in DerivedProperties:
        return ()
    return DerivedProperties[code]

def load_unicode_data(name):
    ret = {}
    first = None
    with open(name) as file:
        for line in file:
            ud = CodePoint(line.strip())
            if ud.category == 'Cs':
                continue
            match = re.fullmatch('<([^,]*), ([^>]*)>', ud.name)
            if match:
                if match.group(2) == 'First':
                    first = ud
                    first.name = match.group(1)
                elif match.group(2) == 'Last':
                    if first:
                        for code in range(first.code, ud.code+1):
                            ret[code] = first.new_code(code)
            else:
                ret[ud.code] = ud
    return ret

def to_upper(code):
    global CodePoints
    if code in CodePoints:
        return CodePoints[code].uppercase
    return code

def to_lower(code):
    global CodePoints
    if code in CodePoints:
        return CodePoints[code].lowercase
    return code

def is_alnum(code):
    return is_alpha(code) or is_digit(code)

def is_alpha(code):
    global CodePoints
    if 'Alphabetic' in derived_properties(code):
        return True
    if code in CodePoints:
        cp = CodePoints[code]
        if cp.category == 'Nd' and not is_digit(code):
            return True
    return False

def is_blank(code):
    global CodePoints
    # Tab
    if code == 0x0009:
        return True
    # Category is space separator except for no-break space
    if code in CodePoints:
        cp = CodePoints[code]
        if cp.category == 'Zs' and '<noBreak>' not in cp.decomposition:
            return True
    return False

def is_cntrl(code):
    global CodePoints
    if code in CodePoints:
        cp = CodePoints[code]
        if not cp.name:
            return False
        # char name is <control>
        if cp.name == '<control>':
            return True
        # Line separator, paragraph separator
        if cp.category in ['Zl', 'Zp']:
            return True
    return False

def is_digit(code):
    # C says only the ASCII digits need apply here
    return 0x0030 <= code and code <= 0x0039

def is_graph(code):
    global CodePoints
    if code in CodePoints:
        cp = CodePoints[code]
        if not cp.name:
            return False
        if cp.name == '<control>':
            return False
        return not is_space(code)
    return False

def is_lower(code):
    if to_upper(code) != code:
        return True
    if 'Lowercase' in derived_properties(code):
        return True
    return False

def is_print(code):
    global CodePoints
    if code in CodePoints:
        cp = CodePoints[code]
        if not cp.name:
            return False
        if cp.name == '<control>':
            return False
        if cp.category not in ['Zl', 'Zp']:
            return True
    return False

def is_punct(code):
    return is_graph(code) and not is_alpha(code) and not is_digit(code)

def is_space(code):
    global CodePoints
    # ASCII blanks
    if code in [0x0020, 0x000c, 0x000a, 0x000d, 0x0009, 0x000b]:
        return True
    # Line separator, paragraph separator
    if code in CodePoints:
        cp = CodePoints[code]
        if cp.name and cp.category in ['Zl', 'Zp']:
            return True
        # Space separator, but not no-break space
        if cp.name and cp.category == 'Zs' and '<noBreak>' not in cp.decomposition:
            return True
    return False

def is_upper(code):
    if to_lower(code) != code:
        return True
    if 'Uppercase' in derived_properties(code):
        return True
    return False

def is_xdigit(code):
    if is_digit(code):
        return True
    # Upper case A-F
    if 0x0041 <= code and code <= 0x0046:
        return True
    # Lower case a-f
    if 0x0061 <= code and code <= 0x0066:
        return True
    return False

def classes(code):
    ret = ""
    add = ""
    if is_upper(code):
        ret += add + '_U'
        add = '|'
    elif is_lower(code):
        ret += add + '_L'
        add = '|'
    elif is_alpha(code):
        ret += add + '_U|_L'
        add = '|'
    if is_digit(code):
        ret += add + '_N'
        add = '|'
    if is_space(code):
        ret += add + '_S'
        add = '|'
    if is_punct(code):
        ret += add + '_P'
        add = '|'
    if is_cntrl(code):
        ret += add + '_C'
        add = '|'
    if is_xdigit(code) and not is_digit(code):
        ret += add + '_X'
        add = '|'
    if is_blank(code) and code != 0x0009:
        ret += add + '_B'
        add = '|'
    if not ret:
        ret = ' 0'
    return ret

georgian_ps = {
    0x00: 0x0000,
    0x01: 0x0001,
    0x02: 0x0002,
    0x03: 0x0003,
    0x04: 0x0004,
    0x05: 0x0005,
    0x06: 0x0006,
    0x07: 0x0007,
    0x08: 0x0008,
    0x09: 0x0009,
    0x0a: 0x000A,
    0x0b: 0x000B,
    0x0c: 0x000C,
    0x0d: 0x000D,
    0x0e: 0x000E,
    0x0f: 0x000F,
    0x10: 0x0010,
    0x11: 0x0011,
    0x12: 0x0012,
    0x13: 0x0013,
    0x14: 0x0014,
    0x15: 0x0015,
    0x16: 0x0016,
    0x17: 0x0017,
    0x18: 0x0018,
    0x19: 0x0019,
    0x1a: 0x001A,
    0x1b: 0x001B,
    0x1c: 0x001C,
    0x1d: 0x001D,
    0x1e: 0x001E,
    0x1f: 0x001F,
    0x20: 0x0020,
    0x21: 0x0021,
    0x22: 0x0022,
    0x23: 0x0023,
    0x24: 0x0024,
    0x25: 0x0025,
    0x26: 0x0026,
    0x27: 0x0027,
    0x28: 0x0028,
    0x29: 0x0029,
    0x2a: 0x002A,
    0x2b: 0x002B,
    0x2c: 0x002C,
    0x2d: 0x002D,
    0x2e: 0x002E,
    0x2f: 0x002F,
    0x30: 0x0030,
    0x31: 0x0031,
    0x32: 0x0032,
    0x33: 0x0033,
    0x34: 0x0034,
    0x35: 0x0035,
    0x36: 0x0036,
    0x37: 0x0037,
    0x38: 0x0038,
    0x39: 0x0039,
    0x3a: 0x003A,
    0x3b: 0x003B,
    0x3c: 0x003C,
    0x3d: 0x003D,
    0x3e: 0x003E,
    0x3f: 0x003F,
    0x40: 0x0040,
    0x41: 0x0041,
    0x42: 0x0042,
    0x43: 0x0043,
    0x44: 0x0044,
    0x45: 0x0045,
    0x46: 0x0046,
    0x47: 0x0047,
    0x48: 0x0048,
    0x49: 0x0049,
    0x4a: 0x004A,
    0x4b: 0x004B,
    0x4c: 0x004C,
    0x4d: 0x004D,
    0x4e: 0x004E,
    0x4f: 0x004F,
    0x50: 0x0050,
    0x51: 0x0051,
    0x52: 0x0052,
    0x53: 0x0053,
    0x54: 0x0054,
    0x55: 0x0055,
    0x56: 0x0056,
    0x57: 0x0057,
    0x58: 0x0058,
    0x59: 0x0059,
    0x5a: 0x005A,
    0x5b: 0x005B,
    0x5c: 0x005C,
    0x5d: 0x005D,
    0x5e: 0x005E,
    0x5f: 0x005F,
    0x60: 0x0060,
    0x61: 0x0061,
    0x62: 0x0062,
    0x63: 0x0063,
    0x64: 0x0064,
    0x65: 0x0065,
    0x66: 0x0066,
    0x67: 0x0067,
    0x68: 0x0068,
    0x69: 0x0069,
    0x6a: 0x006A,
    0x6b: 0x006B,
    0x6c: 0x006C,
    0x6d: 0x006D,
    0x6e: 0x006E,
    0x6f: 0x006F,
    0x70: 0x0070,
    0x71: 0x0071,
    0x72: 0x0072,
    0x73: 0x0073,
    0x74: 0x0074,
    0x75: 0x0075,
    0x76: 0x0076,
    0x77: 0x0077,
    0x78: 0x0078,
    0x79: 0x0079,
    0x7a: 0x007A,
    0x7b: 0x007B,
    0x7c: 0x007C,
    0x7d: 0x007D,
    0x7e: 0x007E,
    0x7f: 0x007F,
    0x80: 0x0080,
    0x81: 0x0081,
    0x82: 0x201A,
    0x83: 0x0192,
    0x84: 0x201E,
    0x85: 0x2026,
    0x86: 0x2020,
    0x87: 0x2021,
    0x88: 0x02C6,
    0x89: 0x2030,
    0x8a: 0x0160,
    0x8b: 0x2039,
    0x8c: 0x0152,
    0x8d: 0x008D,
    0x8e: 0x008E,
    0x8f: 0x008F,
    0x90: 0x0090,
    0x91: 0x2018,
    0x92: 0x2019,
    0x93: 0x201C,
    0x94: 0x201D,
    0x95: 0x2022,
    0x96: 0x2013,
    0x97: 0x2014,
    0x98: 0x02DC,
    0x99: 0x2122,
    0x9a: 0x0161,
    0x9b: 0x203A,
    0x9c: 0x0153,
    0x9d: 0x009D,
    0x9e: 0x009E,
    0x9f: 0x0178,
    0xa0: 0x00A0,
    0xa1: 0x00A1,
    0xa2: 0x00A2,
    0xa3: 0x00A3,
    0xa4: 0x00A4,
    0xa5: 0x00A5,
    0xa6: 0x00A6,
    0xa7: 0x00A7,
    0xa8: 0x00A8,
    0xa9: 0x00A9,
    0xaa: 0x00AA,
    0xab: 0x00AB,
    0xac: 0x00AC,
    0xad: 0x00AD,
    0xae: 0x00AE,
    0xaf: 0x00AF,
    0xb0: 0x00B0,
    0xb1: 0x00B1,
    0xb2: 0x00B2,
    0xb3: 0x00B3,
    0xb4: 0x00B4,
    0xb5: 0x00B5,
    0xb6: 0x00B6,
    0xb7: 0x00B7,
    0xb8: 0x00B8,
    0xb9: 0x00B9,
    0xba: 0x00BA,
    0xbb: 0x00BB,
    0xbc: 0x00BC,
    0xbd: 0x00BD,
    0xbe: 0x00BE,
    0xbf: 0x00BF,
    0xc0: 0x10D0,
    0xc1: 0x10D1,
    0xc2: 0x10D2,
    0xc3: 0x10D3,
    0xc4: 0x10D4,
    0xc5: 0x10D5,
    0xc6: 0x10D6,
    0xc7: 0x10F1,
    0xc8: 0x10D7,
    0xc9: 0x10D8,
    0xca: 0x10D9,
    0xcb: 0x10DA,
    0xcc: 0x10DB,
    0xcd: 0x10DC,
    0xce: 0x10F2,
    0xcf: 0x10DD,
    0xd0: 0x10DE,
    0xd1: 0x10DF,
    0xd2: 0x10E0,
    0xd3: 0x10E1,
    0xd4: 0x10E2,
    0xd5: 0x10F3,
    0xd6: 0x10E3,
    0xd7: 0x10E4,
    0xd8: 0x10E5,
    0xd9: 0x10E6,
    0xda: 0x10E7,
    0xdb: 0x10E8,
    0xdc: 0x10E9,
    0xdd: 0x10EA,
    0xde: 0x10EB,
    0xdf: 0x10EC,
    0xe0: 0x10ED,
    0xe1: 0x10EE,
    0xe2: 0x10F4,
    0xe3: 0x10EF,
    0xe4: 0x10F0,
    0xe5: 0x10F5,
    0xe6: 0x00E6,
    0xe7: 0x00E7,
    0xe8: 0x00E8,
    0xe9: 0x00E9,
    0xea: 0x00EA,
    0xeb: 0x00EB,
    0xec: 0x00EC,
    0xed: 0x00ED,
    0xee: 0x00EE,
    0xef: 0x00EF,
    0xf0: 0x00F0,
    0xf1: 0x00F1,
    0xf2: 0x00F2,
    0xf3: 0x00F3,
    0xf4: 0x00F4,
    0xf5: 0x00F5,
    0xf6: 0x00F6,
    0xf7: 0x00F7,
    0xf8: 0x00F8,
    0xf9: 0x00F9,
    0xfa: 0x00FA,
    0xfb: 0x00FB,
    0xfc: 0x00FC,
    0xfd: 0x00FD,
    0xfe: 0x00FE,
    0xff: 0x00FF,
}

def print_classes(code, encoding, comma):
    c = classes(code)
    try:
        if encoding == 'GEORGIAN-PS':
            ucode = georgian_ps[code]
        elif encoding == 'EUC-JP':
            if 0x80 <= code <= 0x8d or 0x90 <= code <= 0x9f:
                ucode = code
            else:
                ucode = ord(bytes([code]).decode(encoding))
        else:
            ucode = ord(bytes([code]).decode(encoding))
        c = classes(ucode)
    except:
        ucode = 0
        c = ' 0'
    if comma == '\n':
        end=''
    elif code & 7 == 7:
        end = ' \\\n'
    else:
        end = ' ' * (8 - len(c))
    if code & 7 == 0:
        print('        ', end='')
    print('%s%s' % (c, comma), end=end)

def dump_range(title, encoding, start, end):
    print(title, end='')
    for code in range(start, end + 1):
        if code == end:
            comma = '\n'
        else:
            comma = ','
        print_classes(code, encoding, comma)

def dump_table():
    global prev_above

    print('''/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2024 Keith Packard
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

/* This file is auto-generated from mkctype-extended.py */
/* clang-format off */
''')   
#    for val in (10,):
#        encoding = 'iso8859-%d' % val
#        define = '#define _CTYPE_ISO_8859_%d_' % val
#        dump_range(define + '128_254 \\\n', encoding, 128, 254)
#        dump_range(define + '255 ', encoding, 255, 255)
#    return
    for val in (1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 14, 15, 16):
        encoding = 'iso8859-%d' % val
        define = '#define _CTYPE_ISO_8859_%d_' % val
        dump_range(define + '128_254 \\\n', encoding, 128, 254)
        dump_range(define + '255 ', encoding, 255, 255)
    dump_range('#define _CTYPE_SJIS_128_254 \\\n', 'SHIFT-JIS', 128, 254)
    dump_range('#define _CTYPE_SJIS_255 ', 'SHIFT-JIS', 255, 255)
    dump_range('#define _CTYPE_EUCJP_128_254 \\\n', 'EUC-JP', 128, 254)
    dump_range('#define _CTYPE_EUCJP_255 ', 'EUC-JP', 255, 255)
    for cp in ('CP101', 'CP102', 'CP103', 'CP437', 'CP720', 'CP737', 'CP775', 'CP850', 'CP852', 'CP855',
               'CP857', 'CP858', 'CP862', 'CP866', 'CP874', 'CP1125', 'CP1250', 'CP1251',
               'CP1252', 'CP1253', 'CP1254', 'CP1255', 'CP1256', 'CP1257',
               'CP1258', 'CP20866', 'CP21866'):

        if cp == 'CP101':
            encoding = 'GEORGIAN-PS'
        elif cp == 'CP102':
            encoding = 'PT154'
        elif cp == 'CP103':
            encoding = 'KOI8-T'
        elif cp == 'CP20866':
            encoding = 'KOI8-R'
        elif cp == 'CP21866':
            encoding = 'KOI8-U'
        else:
            encoding = cp

        if cp == 'CP101':
            name = 'GEORGIAN_PS'
        elif cp == 'CP102':
            name = 'PT154'
        elif cp == 'CP103':
            name = 'KOI8_T'
        elif cp == 'CP20866':
            name = 'KOI8_R'
        elif cp == 'CP21866':
            name = 'KOI8_U'
        else:
            name = cp
                
        define = '#define _CTYPE_%s_' % name
        dump_range(define + '128_254 \\\n', encoding, 128, 254)
        dump_range(define + '255 ', encoding, 255, 255)
        

DerivedProperties = load_derived_properties('/usr/share/unicode/DerivedCoreProperties.txt')
CodePoints = load_unicode_data('/usr/share/unicode/UnicodeData.txt')

dump_table()

