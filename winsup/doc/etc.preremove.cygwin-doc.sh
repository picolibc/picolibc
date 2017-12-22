#!/bin/bash
# /etc/preremove/cygwin-doc.sh - cygwin-doc preremove script.
# removes Cygwin Start Menu shortcuts for Cygwin User Guide and API PDF and
# HTML, and links to Cygwin web site home page and FAQ
#
# CYGWINFORALL=-A if remove for All Users
# remove local shortcuts for All Users or Current User in
# {ProgramData,~/Appdata/Roaming}/Microsoft/Windows/Start Menu/Programs/Cygwin/
# exits quietly if directory does not exist as presumably no shortcuts desired

doc=/usr/share/doc/cygwin-doc
site=https://cygwin.com
cygp=/usr/bin/cygpath
rm=/bin/rm

html=$doc/html

# check for programs
for p in $cygp $rm
do
	if [ ! -x "$p" ]
	then
		echo "Can't find program '$p'"
		exit 2
	fi
done

# Cygwin Start Menu directory
smpc_dir="$($cygp $CYGWINFORALL -P -U --)/Cygwin"

# check Cygwin Start Menu directory still exists
[ -d "$smpc_dir/" ] || exit 0

# check Cygwin Start Menu directory writable
if [ ! -w "$smpc_dir/" ]
then
	echo "Can't write to directory '$smpc_dir'"
	exit 1
fi

# remove User Guide and API PDF and HTML, Home Page and FAQ URL link shortcuts
while read target name desc
do
	lnk="$smpc_dir/$name.lnk"
	[ -f "$lnk" ] && $rm -f -- "$lnk"
done <<EOF
$doc/cygwin-ug-net.pdf		User\ Guide\ \(PDF\)  Cygwin\ User\ Guide\ PDF
$html/cygwin-ug-net/index.html	User\ Guide\ \(HTML\) Cygwin\ User\ Guide\ HTML
$doc/cygwin-api.pdf		API\ \(PDF\)	Cygwin\ API\ Reference\ PDF
$html/cygwin-api/index.html	API\ \(HTML\)	Cygwin\ API\ Reference\ HTML
$site/index.html	Home\ Page	Cygwin\ Home\ Page\ Link
$site/faq.html		FAQ	Cygwin\ Frequently\ Asked\ Questions\ Link
EOF

