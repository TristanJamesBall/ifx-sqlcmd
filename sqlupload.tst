#!/bin/ksh

run(){
	rm -f core
	echo "+ $@"
	"$@"
	x=$?
	[ $x != 0 ] && echo "## Exit status $x"
	if [ -f core ]
	then
		pid=$(sh -c 'echo $$')
		mv core core.$pid
		echo "## Core dumped - saved in core.$pid"
	fi
}

echo "Testing started at $(date)"
echo

# This should work!
run sqlupload -V

echo "- Invalid database name"
run sqlupload -d non-existent -t table -k col1
echo "- Non-existent database"
run sqlupload -d non_existent -t table -k col1

if sqlcmd -e dbnames | egrep '^ *[0-9]+ tst_upload(@|$)' >/dev/null
then : OK
else sqlcmd - <<-!
		create database tst_upload with log
		in '/tmp/tst_upload.log' mode ansi;
		!
fi

echo "- No -k or -K option"
run sqlupload -d tst_upload -t col1

echo "- No -t option; no -k or -K option"
run sqlupload -d tst_upload -c col1

echo "- No -t option"
run sqlupload -d tst_upload -k col1

echo "- No -d option"
run sqlupload -t systables -k col1

echo "- Non-existent table (no owner)"
run sqlupload -d tst_upload -t systables -k tabid,tabname,owner

echo "- Non-existent table (invalid owner)"
run sqlupload -d tst_upload -o INFORMIX -t systables -k tabid,tabname,owner

echo "- Empty column name"
run sqlupload -d tst_upload -o informix -t systables \
	-k tabid,tabname,owner,,nrows

echo "- Non-existent column name"
run sqlupload -d tst_upload -o informix -t systables \
	-k tabid,tabname,owner,somethinglonger,nrows

echo "+ OK"
run sqlupload -d tst_upload -o informix -t systables \
	-k tabid,tabname,owner,created,nrows /dev/null

echo "- Columns in key not in column list"
run sqlupload -d tst_upload -t systables -o informix \
	-k created,tabid -c tabname,owner,tabtype
echo "- Non-existent column name"
run sqlupload -d tst_upload -t systables -o informix \
	-k tabid -c tabid,tabname,owner,tabtype,gadget

echo "- No INSERT permission"
run sqlupload -d tst_upload -t systables -o informix \
	-k tabid -c tabid,tabname,owner,tabtype,ncols /dev/null

echo "- Key column not in column list"
run sqlupload -d tst_upload -t systables -o informix \
	-k tabid -c tabname,owner,tabtype
echo "- Key list is same as column list"
run sqlupload -d tst_upload -t systables -o informix \
	-k tabname,owner,tabtype -c tabname,owner,tabtype
echo "- Key column not in column list"
run sqlupload -d tst_upload -t systables -o informix \
	-k tabname,owner,tabtype -c owner,tabtype

sqlcmd -d tst_upload - <<!
CONTINUE ON;
DROP TABLE test01;
CONTINUE OFF;

create table test01
(
	col01	INTEGER NOT NULL PRIMARY KEY CONSTRAINT pk_test01,
	col02	CHAR(20) NOT NULL
);
INSERT INTO test01 VALUES(1, "Row Number 1");
INSERT INTO test01 VALUES(2, "Row Number 2");
COMMIT WORK;
!

echo "+ OK"
run sqlupload -d tst_upload -t test01 -k col01 -c col01,col02 <<!
1|Updated Row|
3|Inserted Row|
!
sqlcmd -d tst_upload -e "SELECT * FROM test01"

echo "+ OK"
run sqlupload -I -d tst_upload -t test01 -k col01 -c col01,col02 <<!
1|Update 2 to Row 1|
4|Inserted Row|
!
sqlcmd -d tst_upload -e "SELECT * FROM test01"

echo "+ OK"
run sqlupload -U -d tst_upload -t test01 -k col01 -c col01,col02 <<!
1|Update 3 to Row 1|
5|Inserted Row|
!
sqlcmd -d tst_upload -e "SELECT * FROM test01"

echo "+ OK"
run sqlupload -S -d tst_upload -t test01 -k col01 -c col01,col02 <<!
1|Update 4 to Row 1|
6|Inserted Row|
!
sqlcmd -d tst_upload -e "SELECT * FROM test01"

echo
echo "Testing complete at $(date)"
