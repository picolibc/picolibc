#!/usr/bin/env python3
#
# python script to process makedoc instructions in a source file and produce
# DocBook XML output
#

#
# This performs 3 stages of processing on it's input, in a similar fashion
# to makedoc:
#
# 1. Discard everything outside of /*  */ comments
# 2. Identify lines which contains commands (a single uppercase word)
# 3. Apply each command to the text of the following lines (up to the next
#    command or the end of the comment block), to produce some output
#
# The resulting output contains one or more DocBook XML refentry elements.
#
# To make the output a valid XML document which can be xincluded, those refentry
# elements are contained by a refcontainer element.  refcontainer is not part of
# the DocBook DTD and should be removed by a suitable XSLT.
#

from __future__ import print_function

import fcntl
import sys
import os
import re
from optparse import OptionParser
import lxml.etree
import ply.lex as lex
import ply.yacc as yacc

rootelement = None  # root element of the XML tree
refentry = None     # the current refentry
verbose = 0

def dump(s, stage, threshold=1):
    if verbose > threshold:
        print('*' * 40, file=sys.stderr)
        print(stage, file=sys.stderr)
        print('*' * 40, file=sys.stderr)
        print('%s' % s, file=sys.stderr)
        print('*' * 40, file=sys.stderr)

#
# Stage 1
#

def skip_whitespace_and_stars(i, src):

    while i < len(src) and (src[i].isspace() or (src[i] == '*' and src[i + 1] != '/')):
        i += 1

    return i

# Discard everything not inside '/*  */' style-comments which start at column 0
# Discard any leading blank space or '*'
# Discard a single leading '.'
# Discard blank lines after a blank line
def comment_contents_generator(src):
    i = 0

    while i < len(src) - 2:
        if src[i] == '\n' and src[i + 1] == '/' and src[i + 2] == '*':
            i = i + 3

            i = skip_whitespace_and_stars(i, src)

            if src[i] == '.':
                i += 1

            while i < len(src):
                if src[i] == '\n':
                    yield '\n'
                    i += 1

                    # allow a single blank line
                    if i < len(src) and src[i] == '\n':
                        yield '\n'
                        i += 1

                    i = skip_whitespace_and_stars(i, src)

                elif src[i] == '*' and src[i + 1] == '/':
                    i = i + 2
                    # If we have just output \n\n, this adds another blank line.
                    # This is the only way a double blank line can occur.
                    yield '\nEND\n'
                    break
                else:
                    yield src[i]
                    i += 1
        else:
            i += 1

def remove_noncomments(src):
    src = '\n' + src
    dst = ''.join(comment_contents_generator(src))
    dump(dst, 'extracted from comments')

    return dst

#
# Stage 2
#

# A command is a single word of at least 3 characters, all uppercase, and alone on a line
def iscommand(l):
    if re.match(r'^[A-Z_]{3,}\s*$', l):

        return True
    return False

def command_block_generator(content):
    command = 'START'
    text = ''

    for l in content.splitlines():
        if iscommand(l):
            yield (command, text)
            command = l.rstrip()
            text = ''
        else:
            text = text + l + '\n'
    yield (command, text)

# Look for commands, which give instructions how to process the following input
def process(content):
    content = content.lstrip()

    dump(content, 'about to process for commands')

    # process into a list of tuples of commands and the associated following text
    # it is important to maintain the order of the sections the commands generate
    processed = list(command_block_generator(content))

    return processed

#
# Stage 3
#

#  invoke each command on it's text
def perform(processed):
    for i in processed:
        c = i[0].rstrip()
        t = i[1].strip() + '\n'

        if verbose:
            print("performing command '%s'" % c, file=sys.stderr)

        if c in command_dispatch_dict:
            command_dispatch_dict[c](c, t)
        else:
            print("command '%s' is not recognized" % c, file=sys.stderr)
            # the text following an unrecognized command is discarded

# FUNCTION (aka TYPEDEF)
#
def function(c, l):
    global refentry
    global rootelement

    l = l.strip()
    if verbose:
        print('FUNCTION %s' % l, file=sys.stderr)

    separator = '---'

    if ';' in l:
        # fpclassify has an unusual format we also need to handle
        spliton = ';'
        l = l.splitlines()[0]
    elif len(l.splitlines()) > 1:
        # a few pages like mktemp have two '---' lines
        spliton = ';'
        o = ''
        for i in l.splitlines():
            if separator in i:
                o += i + ';'
            else:
                o += i
        l = o[:-1]
    else:
        spliton = '\n'

    namelist = []
    descrlist = []
    for a in l.split(spliton):
        (n, d) = a.split(separator, 1)
        namelist = namelist + n.split(',')
        descrlist = descrlist + [d]

    # only copysign and log1p use <[ ]> markup in descr,
    # only gets() uses << >> markup
    # but we should handle it correctly
    descr = line_markup_convert(', '.join(descrlist))

    # fpclassify includes an 'and' we need to discard
    namelist = map(lambda v: re.sub(r'^and ', r'', v.strip(), 1), namelist)
    # strip off << >> surrounding name
    namelist = map(lambda v: v.strip().lstrip('<').rstrip('>'), namelist)
    # instantiate list to make it subscriptable
    namelist = list(namelist)

    if verbose:
        print(namelist, file=sys.stderr)
    # additional alternate names may also appear in INDEX commands

    # create the root element if needed
    if rootelement is None:
        rootelement = lxml.etree.Element('refentrycontainer')

    # FUNCTION implies starting a new refentry
    if refentry is not None:
        sys.exit("multiple FUNCTIONs without NEWPAGE")

    # create the refentry
    refentry = lxml.etree.SubElement(rootelement, 'refentry')
    refentry.append(lxml.etree.Comment(' Generated by makedocbook.py '))
    refentry.set('id', namelist[0].lstrip('_'))

    refmeta = lxml.etree.SubElement(refentry, 'refmeta')
    # refentrytitle will be same as refdescriptor, the primary name
    refentrytitle = lxml.etree.SubElement(refmeta, 'refentrytitle')
    refentrytitle.text = namelist[0]
    manvolnum = lxml.etree.SubElement(refmeta, 'manvolnum')
    manvolnum.text = '3'

    refnamediv = lxml.etree.SubElement(refentry, 'refnamediv')
    # refdescriptor is the primary name, assume we should use the one which
    # appears first in the list
    refdescriptor = lxml.etree.SubElement(refnamediv, 'refdescriptor')
    refdescriptor.text = namelist[0]
    # refname elements exist for all alternate names
    for n in namelist:
        refname = lxml.etree.SubElement(refnamediv, 'refname')
        refname.text = n
    refpurpose = lxml.etree.SubElement(refnamediv, 'refpurpose')
    refnamediv.replace(refpurpose, lxml.etree.fromstring('<refpurpose>' + descr + '</refpurpose>'))

    # Only FUNCTION currently exists, which implies that the SYNOPSIS should be
    # a funcsynopsis.  If TYPEDEF was to be added, SYNOPSIS should be processed
    # in a different way, probably producing a refsynopsis.

# INDEX
# may occur more than once for each FUNCTION giving alternate names this
# function should be indexed under
#
def index(c, l):
    l = l.strip()

    if verbose:
        print('INDEX %s' % l, file=sys.stderr)

    # discard anything after the first word
    l = l.split()[0]

    # add indexterm
    # (we could just index under all the refnames, but we control the indexing
    # separately as that is what makedoc does)
    indexterm = lxml.etree.SubElement(refentry, 'indexterm')
    primary = lxml.etree.SubElement(indexterm, 'primary')
    primary.text = l

    # to validate, it seems we need to maintain refentry elements in a certain order
    refentry[:] = sorted(refentry, key=lambda x: x.tag if isinstance(x.tag, str) else '')

    # adds another alternate refname
    refnamediv = refentry.find('refnamediv')

    # as long as it doesn't already exist
    if not refnamediv.xpath(('refname[.="%s"]') % l):
        refname = lxml.etree.SubElement(refnamediv, 'refname')
        refname.text = l
        if verbose > 1:
            print('added refname %s' % l, file=sys.stderr)
    else:
        if verbose > 1:
            print('duplicate refname %s discarded' % l, file=sys.stderr)

    # to validate, it seems we need to maintain refnamediv elements in a certain order
    refnamediv[:] = sorted(refnamediv, key=lambda x: x.tag)


# SYNOPSIS aka ANSI_SYNOPSIS
# ANSI-style synopsis
#
# Note that makedoc would also process <<code>> markup here, but there are no
# such uses.
#
def synopsis(c, t):
    refsynopsisdiv = lxml.etree.SubElement(refentry, 'refsynopsisdiv')
    funcsynopsis = lxml.etree.SubElement(refsynopsisdiv, 'funcsynopsis')

    s = ''
    for l in t.splitlines():
        if re.match(r'\s*(#|\[|struct)', l):
            # preprocessor # directives, structs, comments in square brackets
            funcsynopsisinfo = lxml.etree.SubElement(funcsynopsis, 'funcsynopsisinfo')
            funcsynopsisinfo.text = l.strip() + '\n'
        elif re.match(r'[Ll]ink with', l):
            pass
        else:
            s = s + l

            # a prototype without a terminating ';' is an error
            if s.endswith(')'):
                sys.exit("'%s' missing terminating semicolon" % l)
                s = s + ';'

            if ';' in s:
                synopsis_for_prototype(funcsynopsis, s)
                s = ''

    if s.strip():
        sys.exit("surplus synopsis '%s'" % s)

def synopsis_for_prototype(funcsynopsis, s):
    s = s.strip()

    # funcsynopsis has a very detailed content model, so we need to massage the
    # bare prototype into it.  Fortunately, since the parameter names are marked
    # up, we have enough information to do this.
    for fp in s.split(';'):
        fp = fp.strip()
        if fp:

            if verbose:
                print("'%s'" % fp, file=sys.stderr)

            match = re.match(r'(.*?)([\w\d]*) ?\((.*)\)', fp)

            if verbose:
                print(match.groups(), file=sys.stderr)

            funcprototype = lxml.etree.SubElement(funcsynopsis, 'funcprototype')
            funcdef = lxml.etree.SubElement(funcprototype, 'funcdef')
            funcdef.text = match.group(1)
            function = lxml.etree.SubElement(funcdef, 'function')
            function.text = match.group(2)

            if match.group(3).strip() == 'void':
                void = lxml.etree.SubElement(funcprototype, 'void')
            else:
                # Split parameters on ',' except if it is inside ()
                for p in re.split(r',(?![^()]*\))', match.group(3)):
                    p = p.strip()

                    if verbose:
                        print(p, file=sys.stderr)

                    if p == '...':
                        varargs = lxml.etree.SubElement(funcprototype, 'varargs')
                    else:
                        paramdef = lxml.etree.SubElement(funcprototype, 'paramdef')
                        parameter = lxml.etree.SubElement(paramdef, 'parameter')

                        # <[ ]> enclose the parameter name
                        match2 = re.match(r'(.*)<\[(.*)\]>(.*)', p)

                        if verbose:
                            print(match2.groups(), file=sys.stderr)

                        paramdef.text = match2.group(1)
                        parameter.text = match2.group(2)
                        parameter.tail = match2.group(3)


# DESCRIPTION
# (RETURNS, ERRORS, PORTABILITY, BUGS, WARNINGS, SEEALSO, NOTES  are handled the same)
#
# Create a refsect with a title corresponding to the command
#
# Nearly all the the existing DESCRIPTION contents could be transformed into
# DocBook with a few regex substitutions.  Unfortunately, pages like sprintf and
# sscanf, have very complex layout using nested tables and itemized lists, which
# it is best to parse in order to transform correctly.
#
def refsect(t, s):
    refsect = lxml.etree.SubElement(refentry, 'refsect1')
    title = lxml.etree.SubElement(refsect, 'title')
    title.text = t.title()

    if verbose:
        print('%s has %d paragraphs' % (t, len(s.split('\n\n'))), file=sys.stderr)

    if verbose > 1:
        dump(s, 'before lexing')

        # dump out lexer token sequence
        lex.input(s)
        for tok in lexer:
            print(tok, file=sys.stderr)

    # parse the section text for makedoc markup and the few pieces of texinfo
    # markup we understand, and output an XML marked-up string
    xml = parser.parse(s, tracking=True, debug=(verbose > 2))

    dump(xml, 'after parsing')

    xml = '<refsect1>' + xml + '</refsect1>'

    refsect.extend(lxml.etree.fromstring(xml))

def seealso(c, t):
    refsect('SEE ALSO', t)

# NEWPAGE
#
# start a new refentry

def newpage(c, t):
    global refentry
    refentry = None

# command dispatch table

def discarded(c, t):
    return

command_dispatch_dict = {
    'FUNCTION':      function,
    'TYPEDEF':       function,     # TYPEDEF is not currently used, but described in doc.str
    'INDEX':         index,
    'TRAD_SYNOPSIS': discarded,    # K&R-style synopsis, obsolete and discarded
    'ANSI_SYNOPSIS': synopsis,
    'SYNOPSIS':      synopsis,
    'DESCRIPTION':   refsect,
    'RETURNS':       refsect,
    'ERRORS':        refsect,
    'PORTABILITY':   refsect,
    'BUGS':          refsect,
    'WARNINGS':      refsect,
    'SEEALSO':       seealso,
    'NOTES':         refsect,      # NOTES is not described in doc.str, so is currently discarded by makedoc, but that doesn't seem right
    'QUICKREF':      discarded,    # The intent of QUICKREF and MATHREF is not obvious, but they don't generate any output currently
    'MATHREF':       discarded,
    'START':         discarded,    # a START command is inserted to contain the text before the first command
    'END':           discarded,    # an END command is inserted merely to terminate the text for the last command in a comment block
    'NEWPAGE':       newpage,
}

#
# Utility functions
#

# apply transformations which are easy to do in-place
def line_markup_convert(p):
    s = p

    # escape characters not allowed in XML
    s = s.replace('&', '&amp;')
    s = s.replace('<', '&lt;')
    s = s.replace('>', '&gt;')

    # convert <<somecode>> to <code>somecode</code> and <[var]> to
    # <varname>var</varname>
    # also handle nested << <[ ]> >> correctly
    s = s.replace('&lt;&lt;', '<code>')
    s = s.replace('&lt;[', '<varname>')
    s = s.replace(']&gt;', '</varname>')
    s = s.replace('&gt;&gt;', '</code>')

    # also convert some simple texinfo markup
    # convert @emph{foo} to <emphasis>foo</emphasis>
    s = re.sub(r'@emph{(.*?)}', r'<emphasis>\1</emphasis>', s)
    # convert @strong{foo} to <emphasis role=strong>foo</emphasis>
    s = re.sub(r'@strong{(.*?)}', r'<emphasis role="strong">\1</emphasis>', s)
    # convert @minus{} to U+2212 MINUS SIGN
    s = s.replace('@minus{}', '&#x2212;')
    # convert @dots{} to U+2026 HORIZONTAL ELLIPSIS
    s = s.replace('@dots{}', '&#x2026;')

    # convert xref and pxref
    s = re.sub(r'@xref{(.*?)}', r"See <xref linkend='\1'/>", s)

    # very hacky way of dealing with @* to force a newline
    s = s.replace('@*', '</para><para>')

    # fail if there are unhandled texinfo commands
    match = re.search(r'(?<!@)@[^@\s]+', s)
    if match:
        sys.exit("texinfo command '%s' remains in output" % match.group(0))

    # process the texinfo escape for an @
    s = s.replace('@@', '@')

    if (verbose > 3) and (s != p):
        print('%s-> line_markup_convert ->\n%s' % (p, s), file=sys.stderr)

    return s

#
# lexer
#

texinfo_commands = {
    'ifnottex': 'IFNOTTEX',
    'end ifnottex': 'ENDIFNOTTEX',
    'tex': 'IFTEX',
    'end tex': 'ENDIFTEX',
    'comment': 'COMMENT',
    'c ': 'COMMENT',
    'multitable': 'MULTICOLUMNTABLE',
    'end multitable': 'ENDMULTICOLUMNTABLE',
    'headitem': 'MCT_HEADITEM',
    'tab': 'MCT_COLUMN_SEPARATOR',
    'item': 'MCT_ITEM',
}

# token names
tokens = [
    'BLANKLINE',
    'BULLETEND',
    'BULLETSTART',
    'COURIER',
    'EOF',
    'ITEM',
    'TABLEEND',
    'TABLESTART',
    'TEXINFO',
    'TEXT',
] + list(set(texinfo_commands.values()))

# regular expression rules for tokens, in priority order
# (all these expressions should match a whole line)
def t_TEXINFO(t):
    # this matches any @command. but not @command{} which just happens to be at
    # the start of a line
    r'@\w+[^{]*?\n'

    # if the line starts with a known texinfo command, change t.type to the
    # token for that command
    for k in texinfo_commands.keys():
        if t.value[1:].startswith(k):
            t.type = texinfo_commands[k]
            break

    return t

def t_COURIER(t):
    r'[.|].*\n'
    t.value = line_markup_convert(t.value[1:])
    return t

def t_BULLETSTART(t):
    r'O\+\n'
    return t

def t_BULLETEND(t):
    r'O-\n'
    return t

def t_TABLESTART(t):
    r'o\+\n'
    return t

def t_TABLEEND(t):
    r'o-\n'
    return t

def t_ITEM(t):
    r'o\s.*\n'
    t.value = re.sub(r'o\s', r'', lexer.lexmatch.group(0), 1)
    t.value = line_markup_convert(t.value)
    return t

def t_TEXT(t):
    r'.+\n'
    t.value = line_markup_convert(t.value)
    t.lexer.lineno += 1
    return t

def t_BLANKLINE(t):
    r'\n'
    t.lexer.lineno += 1
    return t

def t_eof(t):
    if hasattr(t.lexer, 'at_eof'):
        # remove eof flag ready for lexing next input
        delattr(t.lexer, 'at_eof')
        t.lexer.lineno = 0
        return None

    t.type = 'EOF'
    t.lexer.at_eof = True

    return t

# Error handling rule
def t_error(t):
    sys.exit("tokenization error, remaining text '%s'" % t.value)

lexer = lex.lex()

#
# parser
#

def parser_verbose(p):
    if verbose > 2:
        print(p[0], file=sys.stderr)

def p_input(p):
    '''input : paragraph
             | input paragraph'''
    if len(p) == 3:
        p[0] = p[1] + '\n' + p[2]
    else:
        p[0] = p[1]
    parser_verbose(p)

# Strictly, text at top level should be paragraphs (i.e terminated by a
# BLANKLINE), while text contained in rows or bullets may not be, but this
# grammar doesn't enforce that for simplicity's sake.
def p_paragraph(p):
    '''paragraph : paragraph_content maybe_eof_or_blankline'''
    p[0] = '<para>\n' + p[1] + '</para>'
    parser_verbose(p)

def p_paragraph_content(p):
    '''paragraph_content : paragraph_line
                         | paragraph_line paragraph_content'''
    if len(p) == 3:
        p[0] = p[1] + p[2]
    else:
        p[0] = p[1]
    parser_verbose(p)

def p_paragraph_line(p):
    '''paragraph_line : TEXT
                      | texinfocmd
                      | courierblock
                      | table
                      | bulletlist'''
    p[0] = p[1]

def p_empty(p):
    'empty :'
    p[0] = ''

def p_maybe_eof_or_blankline(p):
    '''maybe_eof_or_blankline : empty
                              | EOF
                              | BLANKLINE
                              | BLANKLINE EOF'''
    p[0] = ''

def p_maybe_lines(p):
    '''maybe_lines : empty
                   | paragraph maybe_lines'''
    if len(p) == 3:
        p[0] = p[1] + p[2]
    else:
        p[0] = p[1]
    parser_verbose(p)

def p_maybe_blankline(p):
    '''maybe_blankline : empty
                       | BLANKLINE'''
    p[0] = ''

def p_courierblock(p):
    '''courierblock : courier'''
    p[0] = '<literallayout class="monospaced">' + p[1] + '</literallayout>'
    parser_verbose(p)

def p_courier(p):
    '''courier : COURIER
               | COURIER courier'''
    if len(p) == 3:
        p[0] = p[1] + p[2]
    else:
        p[0] = p[1]
    parser_verbose(p)

def p_bullet(p):
    '''bullet : ITEM maybe_lines
              | ITEM BLANKLINE maybe_lines'''
    if len(p) == 3:
        # Glue any text in ITEM into the first para of maybe_lines
        # (This is an unfortunate consequence of the line-based tokenization we do)
        if p[2].startswith('<para>'):
            p[0] = '<listitem><para>' + p[1] + p[2][len('<para>'):] + '</listitem>'
        else:
            p[0] = '<listitem><para>' + p[1] + '</para>' + p[2] + '</listitem>'
    else:
        p[0] = '<listitem><para>' + p[1] + '</para>' + p[3] + '</listitem>'
    parser_verbose(p)

def p_bullets(p):
    '''bullets : bullet
               | bullet bullets'''
    if len(p) == 3:
        p[0] = p[1] + '\n' + p[2]
    else:
        p[0] = p[1]
    parser_verbose(p)

def p_bulletlist(p):
    '''bulletlist : BULLETSTART bullets BULLETEND maybe_blankline'''
    p[0] = '<itemizedlist>' + p[2] + '</itemizedlist>'
    parser_verbose(p)

def p_row(p):
    '''row : ITEM maybe_lines
           | ITEM BLANKLINE maybe_lines'''
    if len(p) == 3:
        p[0] = '<row><entry><code>' + p[1] + '</code></entry><entry>' + p[2] + '</entry></row>'
    else:
        p[0] = '<row><entry><code>' + p[1] + '</code></entry><entry>' + p[3] + '</entry></row>'
    parser_verbose(p)

def p_rows(p):
    '''rows : row
            | row rows'''
    if len(p) == 3:
        p[0] = p[1] + '\n' + p[2]
    else:
        p[0] = p[1]
    parser_verbose(p)

def p_table(p):
    '''table : TABLESTART rows TABLEEND maybe_blankline'''
    p[0] = '<informaltable><tgroup cols="2"><tbody>' + p[2] + '</tbody></tgroup></informaltable>'
    parser_verbose(p)

def p_texinfocmd(p):
    '''texinfocmd : unknown_texinfocmd
                  | comment
                  | multitable
                  | nottex
                  | tex'''
    p[0] = p[1]

def p_unknown_texinfocmd(p):
    '''unknown_texinfocmd : TEXINFO'''
    print("unknown texinfo command '%s'" % p[1].strip(), file=sys.stderr)
    p[0] = p[1]
    parser_verbose(p)

def p_nottex(p):
    '''nottex : IFNOTTEX paragraph_content ENDIFNOTTEX'''
    p[0] = p[2]

def p_tex(p):
    '''tex : IFTEX paragraph_content ENDIFTEX'''
    # text for TeX formatter inside @iftex is discarded
    p[0] = ''

def p_comment(p):
    '''comment : COMMENT'''
    # comment text is discarded
    p[0] = ''

def p_mct_columns(p):
    '''mct_columns : maybe_lines
                   | maybe_lines MCT_COLUMN_SEPARATOR mct_columns'''
    if len(p) == 4:
        p[0] = '<entry>' + p[1] + '</entry>' + p[3]
    else:
        p[0] = '<entry>' + p[1] + '</entry>'
    parser_verbose(p)

def p_mct_row(p):
    '''mct_row : MCT_ITEM mct_columns'''
    p[0] = '<row>' + p[2] + '</row>'
    parser_verbose(p)

def p_mct_rows(p):
    '''mct_rows : mct_row
                | mct_row mct_rows'''
    if len(p) == 3:
        p[0] = p[1] + '\n' + p[2]
    else:
        p[0] = p[1]
    parser_verbose(p)

def p_mct_header(p):
    '''mct_header : MCT_HEADITEM mct_columns'''
    p[0] = '<row>' + p[2] + '</row>'
    parser_verbose(p)

def p_multitable(p):
    '''multitable : MULTICOLUMNTABLE mct_header mct_rows ENDMULTICOLUMNTABLE'''
    # this doesn't handle the prototype row form of @multitable, only the @columnfractions form
    colfrac = p[1].replace('@multitable @columnfractions', '').split()
    colspec = '\n'.join(['<colspec colwidth="%s*"/>' % (c) for c in colfrac])
    header = '<thead>' + p[2] + '</thead>\n'
    body = '<tbody>' + p[3] + '</tbody>\n'
    p[0] = '<informaltable><tgroup cols="' + str(len(colfrac)) + '">' + colspec + header + body + '</tgroup></informaltable>'
    parser_verbose(p)


def p_error(t):
    sys.exit('parse error at line %d, token %s, next token %s' % (t.lineno, t, parser.token()))


# protect creating the parser with a lockfile, so that when multiple processes
# are running this script simultaneously, we don't get one of them generating a
# parsetab.py file, while another one attempts to read it...
#
# see also https://github.com/dabeaz/ply/pull/184
with open(os.path.join(os.path.dirname(__file__), 'parsetab.lock'), 'w+') as lockfile:
    fcntl.flock(lockfile.fileno(), fcntl.LOCK_EX)
    parser = yacc.yacc(start='input')
    fcntl.flock(lockfile.fileno(), fcntl.LOCK_UN)

#
#
#

def main(file):
    content = file.read()
    content = remove_noncomments(content)
    processed = process(content)
    perform(processed)

    # output the XML tree
    s = lxml.etree.tostring(rootelement, pretty_print=True, encoding='unicode')

    if not s:
        print('No output produced (perhaps the input has no makedoc markup?)', file=sys.stderr)
        exit(1)

    print(s)


#
#
#
if __name__ == '__main__':
    options = OptionParser()
    options.add_option('-v', '--verbose', action='count', dest='verbose', default=0)
    (opts, args) = options.parse_args()

    verbose = opts.verbose

    if len(args) > 0:
        main(open(args[0], 'rb'))
    else:
        main(sys.stdin)
