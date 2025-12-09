#!/bin/ksh
#
# @(#)$Id: stt_test.sh,v 1.3 2008/07/14 03:23:14 jleffler Exp $
#
# SQLCMD Test Tools (STT) - Run and Analyze a single SQLCMD Test
#
# Usage:
#   stt_test -t tag [-o out] [-e err] [-s status] [-hxV] [-d dir] -- \
#            command [-options ...] [arguments ...]
#
#   -d dir      Trace output directory - debris goes here (${TRACEDIR:-.})
#   -e err      Expected contents of standard error  (/dev/null)
#   -h          Print help message and exit
#   -o out      Expected contents of standard output (/dev/null)
#   -s status   Expected exit status (0)
#   -t tag      Tag is a string used to identify this test in messages.
#   -x          Trace execution
#   -V          Print version message and exit
#
# The exit status of stt_test is 0 if the output files and exit status
# are as expected and there is no core dump.  Otherwise, it is 1.
#
# I/O redirection on the command is handled by this script.
#
# NB: The only environment variables used here explicitly are $STT_TEST
#     and $TRACEDIR.  Clearly, many others are used implicitly.
# NB: All output from this command is on stderr.

arg0=$(basename $0 .sh)

usestr="Usage: $arg0 -t tag [-o out] [-e err] [-s status] [-hxV] [-d dir] -- \\
               command [-options ...] [arguments ...]"
hlpstr="Option summary:

   -d dir      Trace output directory - debris goes here (${TRACEDIR:-.})
   -e err      Expected contents of standard error  (/dev/null)
   -h          Print help message and exit
   -o out      Expected contents of standard output (/dev/null)
   -s status   Expected exit status (0)
   -t tag      Tag is a string used to identify this test in messages.
   -x          Trace execution
   -V          Print version message and exit

The exit status of stt_test is 0 if the output files and exit status
are as expected and there is no core dump.  Otherwise, it is 1.
"

error()
{
    echo "$@" 1>&2
    exit 1
}

use_error()
{
    echo "$@" 1>&2
    error "$usestr"
}

double_echo()
{
    echo "$@"
    echo "$@" 1>&2
}

# Set defaults
xflag=no
ofile=/dev/null
efile=/dev/null
estat=0
tag=""
dirnm=${TRACEDIR:-.}

exit=0

while getopts t:o:e:s:d:xVh arg
do
    case "$arg" in
    (t) tag="$OPTARG";;
    (o) ofile="$OPTARG";;
    (e) efile="$OPTARG";;
    (s) estat="$OPTARG";;
    (d) dirnm="$OPTARG";;
    (x) xflag="yes";;
    (h) echo "$usestr"
        echo "$hlpstr"
        exit 0;;
    (V) echo "$arg0:" 'STT_TEST Version $Revision: 1.3 $ ($Date: 2008/07/14 03:23:14 $)' | rcsmunger
        exit 0;;
    (*) error "$usestr";;
    esac
done

shift $(($OPTIND - 1))

# Validate arguments.  Note /dev/null fails -f test (it's a character special device).
[ -z "$tag" ]     && use_error "No tag specified."
[ $# -eq 0 ]      && use_error "No command specified."
[ ! -d "$dirnm" ] && use_error "'Directory' $dirnm is not a directory"
[ ! -f "$ofile" ] && [ "$ofile" != "/dev/null" ] &&
                     use_error "'File' $ofile is not a file"
[ ! -f "$efile" ] && [ "$efile" != "/dev/null" ] &&
                     use_error "'File' $efile is not a file"

base=${STT_TEST:-stt_test}.$$
tmp=${TMPDIR:-/tmp}/$base
trap "rm -f $tmp.std???; exit 1" 0 1 2 3 13 15

# Execute supplied command
if [ $xflag = yes ]
then
    echo "== $tag" 1>&2
    echo "++ $@" 1>&2
fi

rm -f core

( "$@" ) >$tmp.stdout 2>$tmp.stderr

status=$?

{

# Determine status - no reporting yet!
[ "$status" -eq "$estat" ] || exit=1
[ ! -f core ]              || exit=1
cmp -s $ofile $tmp.stdout  || exit=1
cmp -s $efile $tmp.stderr  || exit=1

if [ $exit = 0 ]
then
    double_echo "== PASS == $tag"
else

    # Report what is necessary to understand the failure
    double_echo "!! FAIL !! $tag"
    double_echo "-- CMND -- $@"
    double_echo "-- DATA -- Directory: $dirnm"
    double_echo "-- DATA -- Basename:  $base"

    # Preserve the evidence
    [ -f core ] && mv core $dirnm/$base.core
    erract=$base.stderr.actual
    errexp=$base.stderr.wanted
    outact=$base.stdout.actual
    outexp=$base.stdout.wanted
    # It would be embarrassing if root ran the tests and removed /dev/null!
    cp $efile $dirnm/$errexp
    cp $ofile $dirnm/$outexp
    [ "$efile" != "/dev/null" ] && rm -f "$efile"
    [ "$ofile" != "/dev/null" ] && rm -f "$ofile"
    mv $tmp.stderr $dirnm/$erract
    mv $tmp.stdout $dirnm/$outact

    if [ "$status" -ne "$estat" ]
    then double_echo "== unexpected exit status (got $status; wanted $estat) from $tag"
    fi

    if [ -f core ]
    then double_echo "== found core dump after running $tag"
    fi

    if (cd $dirnm; cmp -s $outexp $outact)
    then : OK
    else
        double_echo "== unexpected standard output from $tag"
        (cd $dirnm; diff -u $outexp $outact 2>&1)
    fi

    if (cd $dirnm; cmp -s $errexp $erract)
    then : OK
    else
        double_echo "== unexpected standard error from $tag"
        (cd $dirnm; diff -u $errexp $erract 2>&1)
    fi

fi

rm -f $tmp.std???
trap 0
exit $exit

} >$dirnm/$base.master
