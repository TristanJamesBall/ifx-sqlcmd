:	"@(#)$Id: acsetup.sh,v 1.7 2008/02/10 22:43:11 jleffler Exp $"
#
# Steps required to AutoConfigure SQLCMD
#
# Assumes MAKE is cognizant of RCS files.

CONFIG_FILES1="configure.ac Makefile.in"
CONFIG_FILES2="actimer.m4 acdevstd.m4 acinformix.m4"
CONFIG_FILES3="install-sh acsetup.sh"

CONFIG_FILES="$CONFIG_FILES1 $CONFIG_FILES2 $CONFIG_FILES3"
${MAKE:-make} -s -f /dev/null $CONFIG_FILES

rm -f aclocal.m4
aclocal -I .
autoheader
autoconf
# Clean up unnecessary debris
rm -rf aclocal.m4 autom4te.cache
