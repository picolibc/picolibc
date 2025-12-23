#
# SPDX-License-Identifier: BSD-3-Clause
#
# Copyright © 2024 Keith Packard
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
Translate Unicode data files into ctype data
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
    ret = ()
    if is_alnum(code):
        ret += ('alnum',)
    if is_alpha(code):
        ret += ('alpha',)
    if is_blank(code):
        ret += ('blank',)
    if is_cntrl(code):
        ret += ('cntrl',)
    if is_digit(code):
        ret += ('digit',)
    if is_graph(code):
        ret += ('graph',)
    if is_print(code):
        ret += ('print',)
    if is_punct(code):
        ret += ('punct',)
    if is_space(code):
        ret += ('space',)
    if is_lower(code):
        if to_upper(code) != code:
            ret += ('case',)
        else:
            ret += ('lower',)
    if is_upper(code):
        if to_lower(code) != code:
            ret += ('case',)
        else:
            ret += ('upper',)
    if is_xdigit(code):
        ret += ('xdigit',)
    return ret

def classes_name(classes):
    ret = ""
    add = ""
    for c in classes:
        ret += add + 'CLASS_' + c
        add = "|"
    if not ret:
        ret = "CLASS_none"
    return ret

prev_above = 0

def dump_result(code, above, classes):
    global prev_above
    if classes:
        if prev_above < code:
            print('{ 0x%04x, CLASS_none },' % prev_above)
        name = classes_name(classes)
        print('{ 0x%04x, %s },' % (code, name))
        prev_above = above

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

/* This file is auto-generated from mkctype.py */
/* clang-format off */
''')   
    prev_above = 1

    prev_c = None
    for code in range(0, 0x10000):
        c = classes(code)
        if c != prev_c:
            if prev_c:
                dump_result(first, code, prev_c)
            first = code
            prev_c = c

    code += 1
    print('#if __SIZEOF_WCHAR_T__ == 2')
    if prev_above < code:
        print('{ 0x%04x, CLASS_none },' % prev_above)
    print('#else')

    for code in range(0x10000, 0xe01f0):
        c = classes(code)
        if c != prev_c:
            if prev_c:
                dump_result(first, code, prev_c)
            first = code
            prev_c = c

    code += 1
    dump_result(first, code, c)
    print('{ 0x%04x, CLASS_none },' % code)
    print('#endif')

DerivedProperties = load_derived_properties('/usr/share/unicode/DerivedCoreProperties.txt')
CodePoints = load_unicode_data('/usr/share/unicode/UnicodeData.txt')

dump_table()
