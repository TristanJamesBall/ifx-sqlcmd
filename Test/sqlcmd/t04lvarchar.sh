#!/bin/ksh
#
# @(#)$Id: t04lvarchar.sh,v 2015.1 2015/01/21 07:22:53 jleffler Exp $
#
# Test SQLCMD handling LVARCHAR data

output=${STT_TEST}_output.$$
master=${STT_TEST}_master.$$
input1=${STT_TEST}_input1.$$
input2=${STT_TEST}_input2.$$
files="$output $input1 $input2 $master"

trap "rm -f $files; exit 1" 0 1 2 3 13 15

estat=0
tabname=t05lvarchar

# Drop test table
$SQLCMD -d $STT_DB_LOGGED -e "drop table $tabname" >/dev/null 2>&1

# Create test table
stt_test -t "CREATE" -- $SQLCMD -d $STT_DB_LOGGED -f - <<EOF
create table $tabname
(
    id  serial(8) not null primary key,
    lv  lvarchar
);
insert into $tabname values (0, 'This is a short lVARCHAR field');  -- 8
insert into $tabname(id) values(0);     -- 9, null
EOF
estat=$?

cat <<'EOF' >$master
8|This is a short lVARCHAR field|
9||
EOF

cat <<'EOF' >$input1
10|This is the first line of a multi-line LVARCHAR\
This is the second such line\
And this is the third\
There will continue to be more lines added\
Until boredom\
Or nausea\
Or ad infinitum\
Sets in.|
11|This is the first line of another multi-line LVARCHAR\
with only a couple of lines, this time.\
|
EOF

cat $master $input1 >$input2

cat <<'EOF' >$output
4 rows committed
EOF

if [ $estat = 0 ]
then
    # Select the data in UNLOAD format...
    stt_test -t "SELECT (1)" -o $master -- $SQLCMD -d $STT_DB_LOGGED \
                -F unload -e "select id, lv from $tabname order by id"
    estat=$?
fi

if [ $estat = 0 ]
then
    # Clean out table
    stt_test -t "DELETE (1)" -- $SQLCMD -d $STT_DB_LOGGED -e "delete from $tabname"
    estat=$?
fi

if [ $estat = 0 ]
then
    # Load data into table
    stt_test -t "LOAD" -- $SQLCMD -d $STT_DB_LOGGED -e 'BEGIN WORK' \
                -e "LOAD FROM '$input2' INSERT INTO $tabname" -e 'COMMIT'
    estat=$?
fi

if [ $estat = 0 ]
then
    # Check what we loaded
    stt_test -t "SELECT (2)" -o $input2 -- $SQLCMD -d $STT_DB_LOGGED \
                -F unload -e "select id, lv from $tabname order by id"
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
    stt_test -t "RELOAD" -o $output -- $SQLCMD -R -d $STT_DB_LOGGED -t $tabname -i $input2
    estat=$?
fi

if [ $estat = 0 ]
then
    # Check what we loaded again
    stt_test -t "UNLOAD" -o $input2 -- $SQLCMD -d $STT_DB_LOGGED \
                "unload to '/dev/stdout' select id, lv from $tabname order by id"
    estat=$?
fi

# Drop test table
$SQLCMD -d $STT_DB_LOGGED -e "drop table $tabname" >/dev/null 2>&1

rm -f $files
trap 0
exit $estat
