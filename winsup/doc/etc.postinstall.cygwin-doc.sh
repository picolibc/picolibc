#!/bin/sh
# /etc/postinstall/cygwin-doc.sh - cygwin-doc postinstall script.
# installs Cygwin Start Menu shortcuts for Cygwin User Guide and API PDF and
# HTML if in doc dir, and links to Cygwin web site home page and FAQ
#
# CYGWINFORALL=-A if install for All Users
# installs local shortcuts for All Users or Current User in
# {ProgramData,~/Appdata/Roaming}/Microsoft/Windows/Start Menu/Programs/Cygwin/
# exits quietly if directory does not exist as presumably no shortcuts desired

doc=/usr/share/doc/cygwin-doc
site=https://cygwin.com
cygp=/usr/bin/cygpath
mks=/usr/bin/mkshortcut
launch=/usr/bin/cygstart

html=$doc/html

# check source directories created
for d in $doc $html
do
	if [ ! -d "$d/" ]
	then
		echo "Can't find directory '$d'"
		exit 2
	fi
done

# check for programs
for p in $cygp $mks $launch
do
	if [ ! -x "$p" ]
	then
		echo "Can't find program '$p'"
		exit 2
	fi
done

# Cygwin Start Menu directory
smpc_dir="$($cygp $CYGWINFORALL -P -U --)/Cygwin"

# check Cygwin Start Menu directory exists
[ -d "$smpc_dir/" ] || exit 0

# check Cygwin Start Menu directory writable
if [ ! -w "$smpc_dir/" ]
then
	echo "Can't write to directory '$smpc_dir'"
	exit 1
fi

# create User Guide and API PDF and HTML shortcuts
while read target name desc
do
	[ -r "$target" ] && $mks $CYGWINFORALL -P -n "Cygwin/$name" -d "$desc" -- $target
done <<EOF
$doc/cygwin-ug-net.pdf		User\ Guide\ \(PDF\)  Cygwin\ User\ Guide\ PDF
$html/cygwin-ug-net/index.html	User\ Guide\ \(HTML\) Cygwin\ User\ Guide\ HTML
$doc/cygwin-api.pdf		API\ \(PDF\)	Cygwin\ API\ Reference\ PDF
$html/cygwin-api/index.html	API\ \(HTML\)	Cygwin\ API\ Reference\ HTML
EOF

# create Home Page and FAQ URL link shortcuts
while read target name desc
do
	$mks $CYGWINFORALL -P -n "Cygwin/$name" -d "$desc" -a $target -- $launch
done <<EOF
$site/index.html	Home\ Page	Cygwin\ Home\ Page\ Link
$site/faq.html		FAQ	Cygwin\ Frequently\ Asked\ Questions\ Link
EOF

