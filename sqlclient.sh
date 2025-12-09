:	"@(#)$Id: sqlclient.sh,v 1.3 1998/10/20 06:20:13 jleffler Exp $"
#
#	@(#)SQLCMD Version 90.02 (2016-07-28)
#
#	SQL client

IN=${SQLCMDFIFO:-${TMPDIR:-/tmp}/sqlcmd.in}
t=${TMPDIR:-/tmp}/sql.$$
ERR=${t}.fifo

trap "rm -f $t.in $t.out $t.err $ERR; exit 1" 1 2 3 15
mkfifo $ERR

if [ ! -p $IN ]
then
	sqlserver
fi

# Create command file
{
echo "output $t.out;"
echo "error $ERR;"
cmd=`basename $0`
case $# in
0)	cat - ; echo ";";;
*)	case $cmd in
	sql|sqlclient)
		echo "$*;";;
	*) 	echo "$cmd $*;";;
	esac;;
esac
} >$t.in

# Initiate work
echo "input $t.in;" >$IN

# Wait for command to finish
cat $t.fifo >$t.err
cat $t.out $t.err

[ ! -s $t.err ]
status=$?
rm -f $t.out $t.in $t.err $ERR
exit $status
