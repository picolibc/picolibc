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
Generate single-byte translation tables
'''

import copy
import re

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

def translate(code, encoding):
    try:
        if encoding == 'GEORGIAN-PS':
            ucode = georgian_ps[code]
        else:
            ucode = ord(bytes([code]).decode(encoding))
    except:
        ucode = 0
    return ucode

def dump_range(encoding, base, start, end):
    name = encoding.replace('-', '_')
    print("  [ locale_%s - locale_%s ] = " % (name, base.replace('-', '_')))
    print("  { ", end="")
    max_ucode = 0
    for code in range(start, end + 1):
        if code == end:
            e = ' },\n'
        elif (code - start) % 8 == 7:
            e = ',\n    '
        else:
            e = ', '
        ucode = translate(code, encoding)
        if ucode == 0:
            print("%d" % ucode, end = e)
        else:
            print("%#x" % ucode, end = e)
        if ucode > max_ucode:
            max_ucode = ucode
    return max_ucode

def dump_table():
    print('''/*
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

/* This file is auto-generated from mk-sb-charsets.py */
/* clang-format off */
''')   
    max_ucode_iso = {}
    print('''#ifdef __MB_EXTENDED_CHARSETS_ISO
/* Tables for the ISO-8859-x to UTF conversion.  The first index into the
   table is a value computed from locale id - locale_ISO_8859-2 (ISO-8859-1
   is not included as it doesn't require translation).
   The second index is the value of the incoming character - 0xa0.
   Values < 0xa0 don't have to be converted anyway. */
const uint16_t __iso_8859_conv[14][0x60] = {''')
    for val in (2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 14, 15, 16):
        encoding = 'ISO-8859-%d' % val
        max = dump_range(encoding, 'ISO_8859_2', 0xa0, 0xff)
        max_ucode_iso['locale_%s - locale_%s' % (encoding.replace('-','_'), 'ISO_8859_2')] = max
    print('};')
    print('const uint16_t __iso_8859_max[14] = {')
    for max in max_ucode_iso:
        print("    [%s] = %#x," % (max, max_ucode_iso[max]))
    print('};')
    print('#endif /* __MB_EXTENDED_CHARSETS_ISO */')
    print('')
    
    max_ucode_cp = {}
    print('''#ifdef __MB_EXTENDED_CHARSETS_WINDOWS
/* Tables for the Windows default singlebyte ANSI codepage conversion.
   The first index into the table is a value computed from locale id
   minus locale_WINDOWS_BASE, the second index is the value of the
   incoming character - 0x80.
   Values < 0x80 don't have to be converted anyway. */
const uint16_t __cp_conv[27][0x80] = {''')
    for cp in ('CP437', 'CP720', 'CP737', 'CP775', 'CP850', 'CP852', 'CP855',
               'CP857', 'CP858', 'CP862', 'CP866', 'CP874', 'CP1125', 'CP1250', 'CP1251',
               'CP1252', 'CP1253', 'CP1254', 'CP1255', 'CP1256', 'CP1257',
               'CP1258', 'CP20866', 'CP21866', 'CP101', 'CP102', 'CP103'):
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
        else:
            name = cp
                
        max = dump_range(encoding, 'WINDOWS_BASE', 0x80, 0xff)
        max_ucode_cp['locale_%s - locale_%s' % (encoding.replace('-','_'), 'WINDOWS_BASE')] = max
    print('};')
    print('const uint16_t __cp_max[27] = {')
    for max in max_ucode_cp:
        print("    [%s] = %#x," % (max, max_ucode_cp[max]))
    print('};')
    print('#endif /* __MB_EXTENDED_CHARSETS_WINDOWS */')
    print('')


dump_table()

