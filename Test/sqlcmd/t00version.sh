#!/bin/ksh
#
# @(#)$Id: t00version.sh,v 2008.1 2008/07/14 14:50:05 jleffler Exp $"
#
# Test the SQLCMD version number option

# Non-standard test: allowed to echo to standard output.
$SQLCMD -V
