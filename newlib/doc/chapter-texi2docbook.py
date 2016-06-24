#!/usr/bin/env python
#
# python script to convert the handwritten chapter .texi files, which include
# the generated files for each function, to DocBook XML
#
# all we care about is the content of the refentries, so all this needs to do is
# convert the @include of the makedoc generated .def files to xi:include of the
# makedocbook generated .xml files.
#

from __future__ import print_function
import sys
import re

def main():
    first_node = True

    print ('<?xml version="1.0" encoding="UTF-8"?>')
    print ('<!DOCTYPE chapter PUBLIC "-//OASIS//DTD DocBook V4.5//EN" "http://www.oasis-open.org/docbook/xml/4.5/docbookx.dtd">')

    for l in sys.stdin.readlines():
	l = l.rstrip()

	# transform @file{foo} to <filename>foo</filename>
	l = re.sub("@file{(.*?)}", "<filename>\\1</filename>", l)

	if l.startswith("@node"):
	    l = l.replace("@node", "", 1)
	    l = l.strip()
	    l = l.lower()
	    if first_node:
		print ('<chapter id="%s" xmlns:xi="http://www.w3.org/2001/XInclude">' % l.replace(' ', '_'))
		first_node = False
	elif l.startswith("@chapter "):
	    l = l.replace("@chapter ", "", 1)
	    print ('<title>%s</title>' % l)
	elif l.startswith("@include "):
	    l = l.replace("@include ", "", 1)
	    l = l.replace(".def", ".xml", 1)
	    print ('<xi:include href="%s"/>' % l.strip())

    print ('</chapter>')

if __name__ == "__main__" :
    main()
