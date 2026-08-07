#!/bin/ksh

QUERY=$"\
select 
  5::bigint                             as bigint, 
  'hello'                               as string, 
  'string with \" in it'                 as tricky_string
  ,'string with'||chr(10)||' newline'   as newline_String 
from 
  sysmaster:sysdual;
"


echo "Query is:"
echo
echo "$QUERY" | sed -r 's/^/\t/'
echo
echo
echo 'Output should look like:'
echo
{ cat - | sed -r 's/^/\t/' ;} <<EOF
"bigint"  "string"  "tricky_string"         "newline_string"
5         "hello"   "string with "" in it"  "string with
newline"
EOF
echo
echo
echo 'Runing test:'
echo
{ ./sqlcmd -F CSV -H -d sysmaster |column -t -s','  | sed -r 's/^/\t/' ;} <<EOF
$QUERY
EOF
