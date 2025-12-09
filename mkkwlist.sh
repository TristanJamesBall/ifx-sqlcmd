#!/bin/sh
#
#	@(#)$Id: mkkwlist.sh,v 1.1 2001/11/17 07:45:50 jleffler Exp $
#
#	Generate list of keywords from Yacc grammar header file

perl -na -e 'BEGIN { print "static Keyword keylist[] =\n{\n"; }
if ($F[0] eq "#define" && $F[1] =~ m%^K_%)
{
$F[1] =~ s%^K_%%;
print qq%\t{ "$F[1]", K_$F[1] },\n%;
}
END { print "};\n"; }' y.tab.h >kwlist.c
