:	"@(#)$Id: sqlserver.sh,v 1.3 1998/10/20 06:20:13 jleffler Exp $"
#
#	@(#)SQLCMD Version 90.02 (2016-07-28)
#
#	SQL server
#	Runs SQLCMD monitoring a FIFO
#	Default FIFO is ${TMPDIR}/sqlcmd.in

(
fifo=$SQLCMDFIFO
while [ $# -gt 0 ]
do
	case $1 in
	-M?*)
		fifo=`expr $1 : '-F\(.*\)'`
		shift;;
	-M)	fifo=$2
		shift 2;;
	-[fed]*)
		args="$args '$1'"
		shift;;
	-[fed])
		args="$args $1 '$2'"
		shift 2;;
	*)	args="$args '$1'"
		shift;;
	esac
done

: ${TMPDIR:=/tmp}
: ${fifo:=${TMPDIR}/sqlcmd.in}

if [ ! -p $fifo ]
then
	trap "rm -f $fifo ; exit 1" 1 2 3 13 15
	mkfifo $fifo
	eval sqlcmd -x -M $fifo "$args" </dev/null >/dev/null 2>&1
	rm -f $fifo
	exit 0
fi
) &
