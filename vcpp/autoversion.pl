#     Revelade Revolution Autoversion Script
#
# This script increments the build number by 1 at each execution.
# It is used by default as a pre-build step.
# If you don't have perl installed you can remove the pre-build step
# from the project file/makefile.

use strict;
use warnings;
#use utf8;
use 5.016;
use Time::Piece;

my $verfile;
my $vercontent = "";
my $mjver=0;
my $mnver=0;
my $patch=0;
my $build=0;
open $verfile, "<../shared/version.h";
while (<$verfile>)
{
	if (/^(#define RR_VER_([\w]*)\s*)(\d*)\s*$/)
	{
		if ($2 eq "MAJOR") { $mjver = $3; }
		elsif ($2 eq "MINOR") { $mnver = $3; }
		elsif ($2 eq "PATCH") { $patch = $3; }
		if ($2 eq "BUILD")
		{
			$build = $3+1;
			$vercontent .= $1.($build)."\n";
		}
		else { $vercontent .= "$1$3\n"; }
	}
	elsif (/^#define RR_VERSION_(\w*)(\s*)(.*)$/)
	{
		$vercontent .= "#define RR_VERSION_$1$2";
		if ($1 eq "VAL") { $vercontent .= "$mjver.$mnver\n"; }
		elsif ($1 eq "STRING") { $vercontent .= "\"$mjver.$mnver.$patch.$build\"\n";$mnver = $3; }
		elsif ($1 eq "DATE") { my $date = localtime; $vercontent .= '"'.$date->dmy()."\"\n"; }
		else { $vercontent .= "$3\n"; }
	}
	else
	{
		$vercontent .= $_;
	}
}

close $verfile;

open $verfile, ">../shared/version.h";
print $verfile $vercontent;
close $verfile;
