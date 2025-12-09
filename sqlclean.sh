:	"@(#)$Id: sqlclean.sh,v 1.2 1992/05/10 18:56:32 jl Exp $"
#
#	@(#)SQLCMD Version 90.02 (2016-07-28)
#
#   Dual-purpose script for moving Standard Engine databases.
#
#   SQLCLEAN:
#   =========
#   Ensure that all entries in Systables.Dirpath of the
#   specified database are relative to the '.dbs' directory.
#   Informix arbitrarily changes the dirpath entry of tables
#   so that they contain the full path name of the file, which
#   causes problems when the database is moved wholesale.
#   One of the causes is ALTER TABLE -- there are other causes too.
#   The transaction log has a full path.
#
#   To transfer the database, the paths of ordinary tables are replaced
#   by just the basename, and the tabtype of the transaction log is
#   changed to "l" so the next time the database is invoked, it will be
#   without transactions.
#
#   SQLRESET:
#   =========
#   To reinstate transactions, invoke the command:
#       sqlreset database [ logname ]
#   If the full path logname is specified, that will be the name of
#   the new transaction log; otherwise the current name for the
#   transaction log will be unchanged.
#
#   CAVEAT:
#   =======
#   With Version 4.0 and beyond, you will need to be user informix
#   to be able to use this script.  That also means that user informix
#   will need connect permission on the database.

arg0=`basename $0 .sh`

SQLEXEC=${INFORMIXDIR:-/usr/informix}/lib/sqlexec
export SQLEXEC

case $arg0 in
sqlreset)
	case $# in
	1)	sqlcmd -x -d $1 -e 'update systables set tabtype = "L"
							where tabtype = "l"'
		;;

	2)	case "$2" in
		/*)	if [ ! -f $2 ]
			then
				>$2
				if [ $? != 0 ]
				then
					echo "$arg0: cannot create transaction log $2" >&2
					exit 1
				fi
				chmod 660 $2
				chgrp informix $2
			fi;;
		*)	echo "$arg0: transaction log must be absolute pathname" >&2
			exit 1;;
		esac
		sqlcmd -x -d $1 -e 'update systables set tabtype = "L", dirpath = "$2"
							where tabtype = "l"'
		;;

	*)	echo "Usage: $arg0 database [logfile]" >&2
		exit 1;;
	esac
	exit
	;;

sqlclean)
	if [ $# -ne 1 ]; then
	   echo Usage: $arg0 database >&2
	   exit 1
	fi
	sqlcmd -D'	' -d $1 -e 'select tabid, dirpath from systables
		where dirpath matches "*/*" and tabtype != "L" and tabtype != "l"' |
	{
	echo 'update systables set tabtype = "l" where tabtype = "L";'
	while read id dpath
	do
		base='"'`basename $dpath`'"'
		echo "update systables set dirpath = $base where tabid = $id;"
	done
	} |
	sqlcmd -x -d $1
	;;

*)
	echo "unknown link to sqlclean/sqlreset: $arg0" >&2
	exit 1
	;;
esac
