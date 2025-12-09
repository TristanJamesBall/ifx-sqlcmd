#!/bin/ksh
#
# @(#)$Id: t02varchar.sh,v 2008.2 2008/07/14 14:50:05 jleffler Exp $
#
# Test SQLCMD handling VARCHAR, especially trailing blanks

master=${STT_TEST}_master.$$

trap "rm -f $master; exit 1" 0 1 2 3 13 15

cat <<'EOF' >$master
abc|-1000|
abc |-1001|
abc  |-1002|
abc   |-1003|
abc    |-1004|
abcdefghijklmnopqrstuvwxyz1234  |-1005|
abcdefghijklmnopqrstuvwxyz12345 |-1006|
abcdefghijklmnopqrstuvwxyz123456|-1007|
abcdefghijklmnopqrstuvwxyz123456|-1008|
abcdefghijklmnopqrstuvwxyz12345 |-1009|
EOF

estat=0

tabname=t02varchar
# Drop test table
$SQLCMD -d $STT_DB_LOGGED -e "drop table $tabname" >/dev/null 2>&1

# Create test table
stt_test -t "CREATE" -o $master -- $SQLCMD -d $STT_DB_LOGGED -f - <<EOF
create table $tabname
(
    id  integer not null primary key,
    vc  varchar(32) not null
);
-- Use negative number to spot a 1-byte overflow more easily
insert into $tabname values (-1000, 'abc');
insert into $tabname values (-1001, 'abc ');
insert into $tabname values (-1002, 'abc  ');
insert into $tabname values (-1003, 'abc   ');
insert into $tabname values (-1004, 'abc    ');
insert into $tabname values (-1005, 'abcdefghijklmnopqrstuvwxyz1234  ');
insert into $tabname values (-1006, 'abcdefghijklmnopqrstuvwxyz12345 ');
insert into $tabname values (-1007, 'abcdefghijklmnopqrstuvwxyz123456');
insert into $tabname values (-1008, 'abcdefghijklmnopqrstuvwxyz1234567');
insert into $tabname values (-1009, 'abcdefghijklmnopqrstuvwxyz12345 7');

format unload;
select vc, id from $tabname order by id desc;

EOF
estat=$?

if [ $estat = 0 ]
then
    stt_test -t "DELETE" -- $SQLCMD -d $STT_DB_LOGGED -e "delete from $tabname"
    estat=$?
fi

if [ $estat = 0 ]
then
    # Load data into table (note specified order of columns)
    stt_test -t "LOAD" -- $SQLCMD -d $STT_DB_LOGGED -e 'BEGIN WORK' \
                -e "LOAD FROM '$master' INSERT INTO $tabname(vc,id)" -e 'COMMIT WORK'
    estat=$?
fi

if [ $estat = 0 ]
then
    # Check what we loaded
    stt_test -t "SELECT" -o $master -- $SQLCMD -d $STT_DB_LOGGED -F unload \
                -e "select vc, id from $tabname order by id desc"
    estat=$?
fi

# Drop test table
$SQLCMD -d $STT_DB_LOGGED -e "drop table $tabname" >/dev/null 2>&1

rm -f $master
trap 0
exit $estat
