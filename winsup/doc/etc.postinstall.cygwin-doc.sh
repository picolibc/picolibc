#!/bin/bash
# /etc/postinstall/cygwin-doc.sh - cygwin-doc postinstall script.
# installs Cygwin Start Menu shortcuts for Cygwin User Guide and API PDF and
# HTML if in doc dir, and links to Cygwin web site home page and FAQ
#
# Assumes you are running setup.exe 2.510.2.2 or newer, executed by /bin/bash
# and not /bin/[da]sh (if you are running an older setup.exe, this postinstall
# script can't do anything).
#
# CYGWINFORALL=-A if install for All Users
# installs local shortcuts for All Users or Current User in
# {ProgramData,~/Appdata/Roaming}/Microsoft/Windows/Start Menu/Programs/Cygwin/

cygp=/bin/cygpath
mks=/bin/mkshortcut
un=/bin/uname
site=https://cygwin.com

# check for programs
for p in $un $cygp $mks
do
	if [ ! -x $p ]
	then
		echo "Can't find program '$p'"
		exit 2
	fi
done

doc=/usr/share/doc/cygwin-doc
html=$doc/html
smpc_dir="$($cygp $CYGWINFORALL -P -U)/Cygwin"

for d in $doc $html "$smpc_dir"
do
	if [ ! -d "$d/" ]
	then
		echo "Can't find directory '$d'"
		exit 2
	fi
done

if [ ! -w "$smpc_dir/" ]
then
	echo "Can't write to directory '$smpc_dir'"
	exit 1
fi

# mkshortcut works only in current directory - change to Cygwin Start Menu
cd "$smpc_dir" || exit 2	# quit if not found

# User Guide PDF & HTML
p=$doc/cygwin-ug-net.pdf
n="User Guide (PDF)"
d="PDF Cygwin User Guide"

[ -r $p ] && $mks -n "$n" -d "$d" $p

i=$html/cygwin-ug-net/index.html
n="User Guide (HTML)"
d="HTML Cygwin User Guide"

[ -r $i ] && $mks -n "$n" -d "$d" $i

# API PDF & HTML
p=$doc/cygwin-api.pdf
n="API (PDF)"
d="PDF Cygwin API Reference"

[ -r $p ] && $mks -n "$n" -d "$d" $p

i=$html/cygwin-api/index.html
n="API (HTML)"
d="HTML Cygwin API Reference"

[ -r $i ] && $mks -n "$n" -d "$d" $i

# Home Page URL
h=$site/index.html
n="Home Page"
d="Cygwin $n"

$mks -n "$n" -d "$d" $h

# FAQ URL
h=$site/faq.html
n="FAQ"
d="Cygwin Frequently Asked Questions (with answers)"

$mks -n "$n" -d "$d" $h

