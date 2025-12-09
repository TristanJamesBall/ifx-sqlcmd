#!/bin/ksh
#
# @(#)$Id: t05text.sh,v 2008.2 2008/07/14 14:50:05 jleffler Exp $
#
# Test SQLCMD handling TEXT - basic

output=${STT_TEST}_output.$$
master=${STT_TEST}_master.$$

trap "rm -f $output $master; exit 1" 0 1 2 3 13 15

estat=0

cat <<'EOF' >$master
12345|abcdef|
EOF

cat $master - <<'EOF' >$output
1 rows committed
EOF

tabname=t05text
# Drop test table
$SQLCMD -d $STT_DB_LOGGED -e "drop table $tabname" >/dev/null 2>&1

# Create test table
stt_test -t "CREATE, LOAD, UNLOAD" -o $output -- $SQLCMD -d $STT_DB_LOGGED -f - <<EOF
create table $tabname
(
    id  integer not null primary key,
    tx  text in table
);
reload from "$master"     insert   into $tabname;
unload to   "/dev/stdout" select * from $tabname;
EOF
estat=$?

# Drop test table
$SQLCMD -d $STT_DB_LOGGED -e "drop table $tabname" >/dev/null 2>&1

rm -f $output $master
trap 0
exit $estat
