#!/usr/bin/perl
# $Id: makeheader.pl,v 1.1.1.1 1997-05-25 20:31:48 obachman Exp $
#
# makeheader.pl - generate a header file out of several header file (ver 1.2).
#

#
# searchpath - search a file in a path and return full path.
#
sub searchpath {
    local ( $file, $path ) = @_;
    for ( split ( /:/, $path ) ) {
	if ( -f "$_/$file" ) {
	    return "$_/$file";
	}
    }
    return "";
}
	
if ( ( $#ARGV == -1 ) || ( $ARGV[0] =~ /-help|-\?/ ) ) {
    die "
usage: makeheader [-?] {-I<includedir>} <templatefile> <outfile>

This is makeheader, a header file generation tool.
With makeheader it is possible to generate a single header
file out of several other header files.

To include a header file into the generated header file
\(= <outfile>) use the two lines 

/*MAKEHEADER*/
#include \"<includefile>\"

in <templatefile>.

If you use the alternate form

/*MAKEHEADER PUBLIC ONLY*/
#include \"<includefile>\"

only sections marked with /*BEGINPUBLIC*/ and /*ENDPUBLIC*/ are
pasted from <includefile> into <outfile>.

<includefile> is looked up in all <includedir>s, then in the
current directory, then in the base directory of
<templatefile>.\n";
}

# get list of include directories
$includes = "";
while ( $ARGV[0] =~ /^-/ ) {
    $_ = shift;
    if ( /^-I(.*)/ ) {
	$includes .= ( $1 ? $1 : shift ) . ":";
    }
}
$includes .= ".:";

# check for rest of arguments
if ( $#ARGV < 1 ) {
    die "usage: makeheader [-?] {-I<includedir>} <templatefile> <outfile>\n\n";
}

$infile = shift( @ARGV );
$outfile = shift( @ARGV );
if ( $infile =~ /(.*)\// ) {
    $includes .= "$1:";
}

open( INFILE, $infile ) || 
    die "makeheader: Can not open template file $infile for reading!\n";
open( OUTFILE, ">$outfile" ) ||
    die "makeheader: Can not open header file $outfile for writing!\n";

print OUTFILE "\n/* $outfile automatically generated by $0 on " .
    `date` . "*/\n";

while( <INFILE> ) {
    if ( /^\/\*MAKEHEADER(.*)\*\/$/ ) {
	# check whether we have to read public part only
	if ( $1 eq " PUBLIC ONLY" ) {
	    $public_only = 1;
	} else {
	    $public_only = 0;
	}

	# get name of file to include
	$_ = <INFILE>; chop;
	if ( /^\#include \"(.*)\"$/ ) {
	    $inclfile = $1;
	} else {
	    die "makeheader: $_ is not a valid include directive!\n";
	}

	$inclfile = &searchpath( $inclfile, $includes ) ||
	    die "makeheader: Can not open included file $inclfile for reading!\n";
	open( INCLFILE, $inclfile ) ||
	    die "makeheader: Can not open included file $inclfile for reading!\n";

	print OUTFILE "\n/* Stuff included from $inclfile */\n";
	if ( $public_only ) {
	    # read public part only
	    while ( <INCLFILE> ) {
		if ( /^\/\*BEGINPUBLIC\*\/$/ ) {
		    while ( <INCLFILE> ) {
			last if ( /^\/\*ENDPUBLIC\*\/$/ );
			print OUTFILE $_;
		    }
		}
	    }
	} else {
	    # just paste contents into output
	    while ( <INCLFILE> ) { print OUTFILE $_; }
	}
	close INCLFILE;
    }
    else {
	print OUTFILE $_;
    }
}

close OUTFILE;
close INFILE;
