#!/bin/ksh
#
# @(#)$Id: t01mutexopt.sh,v 2016.1 2016/07/28 20:11:27 jleffler Exp $
#
# Test the SQLCMD mutually exclusive options (-C, -J, -R, -U)

stdout=${STT_TEST}_stdout.$$
stderr=${STT_TEST}_stderr.$$

trap "rm -f $stdout $stderr; exit 1" 0 1 2 3 13 15

estat=0

for cmd in $SQLCMD $SQLRELOAD $SQLUNLOAD
do
    for opts in "-C -U" "-C -R" "-C -J" "-U -J" "-U -R" "-R -J"
    do
        $cmd $opts >$stdout 2>$stderr
        if [ -s $stdout ]
        then
            echo "$STT_TEST: non-empty stdout detected for $opts" 1>&2
            cat $stdout 1>&2
            estat=1
        fi
        # Detect characteristic error messages
        if grep '(-C, -J, -R, -U)' $stderr >/dev/null
        then : OK
        else
            echo "$STT_TEST: correct error message missing in stderr for $opts" 1>&2
            cat $stdout 1>&2
            estat=1
        fi
    done
done

rm -f $stdout $stderr
trap 0
exit $estat
