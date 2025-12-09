#!/bin/ksh
#
# @(#)$Id: stt_run.sh,v 1.7 2011/05/13 00:27:54 jleffler Exp $
#
# SQLCMD Test Tools (STT) - Run command, verify exit status, test for core dumps

# NB: core dump issues now detected by stt_test - when it is used

[ "$STT_DEBUG_STT_RUN" = yes ] && set -x

verbose="no"    # Do not let stdout/stderr leak
exit=0          # Expected exit status from command

while getopts e:qv opt
do
    case $opt in
    e)  exit=$OPTARG;;
    v)  verbose="yes";;
    q)  verbose="no";;
    esac
done

shift $((OPTIND - 1))

if [ $# -le 0 ]
then
    echo "$0: no arguments" 1>&2
    exit 1
fi

export STT_TEST=$(basename $PWD)-$(basename $1 .sh)

rm -f core

TRACEFILE=${TRACEDIR:?}/$STT_TEST.`date +%\Y%\m%\d-%\H%\M`

{
echo "Command: $*"
echo "Date:    $(date)"
echo
} > $TRACEFILE


if [ $verbose = yes ]
then
    echo "$STT_TEST"
    ${SHELL:-sh} "$@" >>$TRACEFILE 2>&1
    estat=$?
    sed -e '1,/^$/d' -e 's/^/    /' $TRACEFILE
else
    echo $STT_ECHO_N "$STT_TEST$STT_ECHO_C"
    ${SHELL:-sh} "$@" >>$TRACEFILE 2>&1
    estat=$?
    STT_TEST=""
fi

if [ -f core ]
then
    echo "$STT_TEST *** FAILED -- CORE DUMP ***" 1>&2
    mv core core.$STT_TEST
elif [ $estat != $exit ]
then echo "$STT_TEST *** FAILED -- INCORRECT EXIT STATUS (got $estat, wanted $exit) ***" 1>&2
else echo "$STT_TEST - OK"
fi
