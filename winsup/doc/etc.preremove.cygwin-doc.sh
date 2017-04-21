#!/bin/bash
# /etc/preremove/cygwin-doc.sh - cygwin-doc preremove script.
# removes Cygwin Start Menu shortcuts for Cygwin User Guide and API PDF and
# HTML, and links to Cygwin web site home page and FAQ
#
# CYGWINFORALL=-A if remove for All Users
# remove local shortcuts for All Users or Current User in
# {ProgramData,~/Appdata/Roaming}/Microsoft/Windows/Start Menu/Programs/Cygwin/

cd "$(/bin/cygpath $CYGWINFORALL -P -U)/Cygwin" || exit 2

/bin/rm -f -- "User Guide (PDF).lnk" "User Guide (HTML).lnk" \
	"API (PDF).lnk" "API (HTML).lnk" "Home Page.lnk" "FAQ.lnk"

