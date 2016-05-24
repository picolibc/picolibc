#!/usr/bin/perl -w
#
# This file is part of Cygwin.
# 
# This software is a copyrighted work licensed under the terms of the
# Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
# details.

use strict;

if (@ARGV) {
	my $infile = $ARGV[0];
	my $outfile = $infile;
	$outfile =~ s/\.html$/.body/;
	if ($infile ne $outfile) {
		open my $input, '<', $infile or die "Failed to open $infile: $!\n";
		my $html = do { local $/; <$input> };	# slurp!
		my ($body) = $html =~ m|<body[^>]*>(.*)</body>|is;
		if ($body) {
			open my $output, '>', $outfile
					or die "Failed to write $outfile: $!\n";
			print $output $body;
		}
		else {
			print STDERR "Could not find <body> element in $infile!\n\n";
			exit 3;
		}
	}
	else {
		print STDERR "Input file name must end in .html!\n\n";
		exit 2;
	}
}
else {
	print STDERR <<USAGE;
usage: $0 <input.html>

    Transforms input.html to input.body by extracting whatever is
    between <body> and </body> in input.html.

USAGE
	exit 1;
}
