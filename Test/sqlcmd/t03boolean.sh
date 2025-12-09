#!/bin/ksh
#
# @(#)$Id: t03boolean.sh,v 2008.2 2008/07/14 14:50:05 jleffler Exp $
#
# Test SQLCMD handling BOOLEAN data

output=${STT_TEST}_output.$$
master=${STT_TEST}_master.$$
files="$output $master"

trap "rm -f $files; exit 1" 0 1 2 3 13 15

cat <<'EOF' >$master
8|f|
9|f|
10|t|
11|t|
12||
EOF

cat <<'EOF' > $output
5 rows committed
EOF

estat=0

tabname=t03boolean
# Drop test table
$SQLCMD -d $STT_DB_LOGGED -e "drop table $tabname" >/dev/null 2>&1

# Create test table
stt_test -t "CREATE" -o $master -- $SQLCMD -d $STT_DB_LOGGED -f - <<EOF
create table $tabname
(
    id  serial(8) not null primary key,
    tf  boolean
);
insert into $tabname values (0, 'F');   -- 8, false
insert into $tabname values (0, 'f');   -- 9, false
insert into $tabname values (0, 'T');   -- 10, true
insert into $tabname values (0, 't');   -- 11, true
insert into $tabname(id) values(0);     -- 12, null

format unload;
select id, tf from $tabname order by id;

EOF
estat=$?

if [ $estat = 0 ]
then
    stt_test -t "DELETE (1)" -- $SQLCMD -d $STT_DB_LOGGED -e "delete from $tabname"
    estat=$?
fi

if [ $estat = 0 ]
then
    # Load data into table (note specified order of columns)
    stt_test -t "LOAD" -- $SQLCMD -d $STT_DB_LOGGED -e 'BEGIN WORK' \
                -e "LOAD FROM '$master' INSERT INTO
    $tabname(id,tf)" -e 'COMMIT WORK'
    estat=$?
fi

if [ $estat = 0 ]
then
    # Check what we loaded
    stt_test -t "SELECT" -o $master -- $SQLCMD -d $STT_DB_LOGGED -F unload \
                -e "select id, tf from $tabname order by id"
    estat=$?
fi

if [ $estat = 0 ]
then
    # Clean out table again
    stt_test -t "DELETE (2)" -- $SQLCMD -d $STT_DB_LOGGED -e "delete from $tabname"
    estat=$?
fi

if [ $estat = 0 ]
then
    # Reload data into table
    stt_test -t "RELOAD" -o $output -- $SQLCMD -R -d $STT_DB_LOGGED -t $tabname -i $master
    estat=$?
fi

# Drop test table
$SQLCMD -d $STT_DB_LOGGED -e "drop table $tabname" >/dev/null 2>&1

rm -f $files
trap 0
exit $estat
