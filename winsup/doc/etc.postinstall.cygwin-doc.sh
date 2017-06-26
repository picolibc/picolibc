#!/bin/sh
# /etc/postinstall/cygwin-doc.sh - cygwin-doc postinstall script.
# installs Cygwin Start Menu shortcuts for Cygwin User Guide and API PDF and
# HTML if in doc dir, and links to Cygwin web site home page and FAQ
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

# create User Guide and API PDF and HTML shortcuts
while read target name desc
do
	[ -r $t ] && $mks -n "$name" -d "$desc" $target
done <<EOF
$doc/cygwin-ug-net.pdf		User\ Guide\ \(PDF\)  Cygwin\ User\ Guide\ PDF
$html/cygwin-ug-net/index.html	User\ Guide\ \(HTML\) Cygwin\ User\ Guide\ HTML
$doc/cygwin-api.pdf		API\ \(PDF\)	Cygwin\ API\ Reference\ PDF
$html/cygwin-api/index.html	API\ \(HTML\)	Cygwin\ API\ Reference\ HTML
EOF

# create Home Page and FAQ URL link shortcuts
while read target name desc
do
	$mks -n "$name" -d "$desc" $target
done <<EOF
$site/index.html	Home\ Page	Cygwin\ Home\ Page\ Link
$site/faq.html		FAQ	Cygwin\ Frequently\ Asked\ Questions\ Link
EOF

