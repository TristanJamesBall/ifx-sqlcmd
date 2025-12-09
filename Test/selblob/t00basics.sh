#!/bin/ksh
#
# @(#)$Id: t00basics.sh,v 1.1 2013/12/28 22:29:56 jleffler Exp $
#
# Test SQLCMD handling VARCHAR, especially trailing blanks

master=${STT_TEST}_master.$$

trap "rm -f $master.?; exit 1" 0 1 2 3 13 15

tb100="This is text for the TEXT blob 100"
tb999="This is text for the TEXT blob 999 and it is longer than the other one"
cat <<EOF >$master.1
100|$tb100\\
|
999|$tb999\\
|
EOF

estat=0

tabname=t02blob
# Drop test table
$SQLCMD -d $STT_DB_LOGGED -e "drop table $tabname" >/dev/null 2>&1

# Create test table
stt_test -t "CREATE" -- $SQLCMD -s -d $STT_DB_LOGGED -f - <<EOF
create table $tabname
(
    id    integer not null primary key,
    data  text in table not null
);
reload from "$master.1" insert into $tabname;
EOF
estat=$?

if [ $estat = 0 ]
then
    stt_test -t "SELECT.1" -- $SELBLOB -d $STT_DB_LOGGED -t $tabname -c data \
            -k id=100 -f $master.2
    estat=$?
fi

if [ $estat = 0 ]
then
    stt_test -t "SELECT.2" -- $SELBLOB -d $STT_DB_LOGGED -t $tabname -c data \
            -k id=999 -f $master.3
    estat=$?
fi

if [ $estat = 0 ]
then
    # Check what we loaded and unloaded
    echo "$tb100" > $master.4
    echo "$tb999" > $master.5
    diff $master.2 $master.4 && diff $master.3 $master.5
    estat=$?
fi

# Drop test table
$SQLCMD -d $STT_DB_LOGGED -e "drop table $tabname" >/dev/null 2>&1

rm -f $master.?
trap 0
exit $estat
