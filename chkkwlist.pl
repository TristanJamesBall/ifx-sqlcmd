#!/usr/bin/env perl
#
# @(#)$Id: chkkwlist.pl,v 1.2 2012/08/05 18:32:03 jleffler Exp $
#
# Validate grammar to ensure that all keywords defined as tokens appear
# in keyword definition (rule), and all keywords mentioned in keyword
# definition appear as tokens.  Also, verify no duplicates.

use strict;
use warnings;

my $inkeywords = 0;
my %keyword_listed;
my %keyword_tokens;

while (<>)
{
    chomp;
    last if ($inkeywords && m/^\s*;\s*$/);
    if (m/^keyword\s*$/)
    {
        $inkeywords = 1;
    }
    elsif (m/^%token\s+K_/)
    {
        s/^%token\s+(K_\w+).*/$1/;
        #print "Token $_\n";
        $keyword_tokens{$_}++;
    }
    elsif ($inkeywords)
    {
        s/^\s+[:|]\s+(K_\w+).*/$1/;
        #print "Keyword $_\n";
        $keyword_listed{$_}++;
    }
}

foreach my $key (sort keys %keyword_tokens)
{
    print "Duplicate %token $key\n"
        if ($keyword_tokens{$key} > 1);
    print "Missing $key in definition of keyword\n"
        if !defined $keyword_listed{$key};
}

foreach my $key (sort keys %keyword_listed)
{
    print "Duplicate keyword entry for $key\n"
        if ($keyword_listed{$key} > 1);
    print "Missing %token $key in tokens\n"
        if !defined $keyword_tokens{$key};
}
