#!/bin/ksh
#
# @(#)$Id: gccpragmata.sh,v 1.5 2016/07/15 23:08:42 jleffler Exp $
#
# Replace #pragma GCC lines with comments

# Problem reported by Ray Burns <Ray.Burns@velocityglobal.co.nz> for
# SQLCMD and stderr.c on 2015-10-19.  GCC 4.6.4 (assume 4.6.x for all
# valid x) and all later versions of GCC support #pragma GCC as used in
# JLSS code.  GCC 4.5.4 and 4.4.7 (and hence assume all verrsions of GCC
# prior to GCC 4.6) do not support #pragma GCC as used in JLSS code.
# This script converts the GCC pragmata into comments.  The underlying
# source code does not depend on the pragmata being noticed.
#
# Note that this script does not attempt to deal with all the deviant
# ways of invoking #pragma GCC (like _Pragma("GCC diagnostics push") or
# /**/ #pragma GCC diagnostics push
#
# Note that the printing from grep can be fettled by specifying options
# such as: gccpragmata -p -- -n *.[ch]
# The '--' marks the end of options to this script; any options after it
# are passed verbatim to grep.  Options -c, -h, -l, -n, -r, -v could all
# be useful on occasion.

arg0=$(basename "$0" .sh)

usemsg="Usage: $arg0 [-clnpu][-hV] [file ...]"

usage()
{
    echo "$usemsg" >&2
    exit 1
}
help()
{
    echo "$usemsg"
    echo "  -c  Convert #pragma GCC to a comment"
    echo "  -h  Print this help message and exit"
    echo "  -l  List files containing #pragma GCC"
    echo "  -n  Print line numbers"
    echo "  -p  Print GCC pragmata lines only (default)"
    echo "  -u  Convert comments containing #pragma GCC to active code"
    echo "  -V  Print version information and exit"
    exit 0
}

grep_options=""
add_comments=no
prt_comments=no
sub_comments=no
while getopts chlnpuV opt
do
    case "$opt" in
    (c) add_comments=yes;;
    (h) help;;
    (l) grep_options="$grep_options -l";;
    (n) grep_options="$grep_options -n";;
    (p) prt_comments=yes;;
    (u) sub_comments=yes;;
    (V) echo "$arg0:" 'GCCPRAGMATA Version $Revision: 1.5 $ ($Date: 2016/07/15 23:08:42 $)' | rcsmunger; exit 0;;
    (*) usage;;
    esac
done

shift $(($OPTIND - 1))

case "$add_comments$sub_comments$prt_comments" in
(*yes*yes*)
    echo "$arg0: mutually exclusive options (a combination of -c, -p, -u) specified" >&2
    exit 1
    ;;
(nonono)
    prt_comments=yes;;  # Default is to print GCC pragmata
esac

if [ "$prt_comments" = "yes" ]
then
    # Reasonably general input - both comment and non-comment forms
    exec grep $grep_options -E -e '^(/\*)?[[:space:]]*#[[:space:]]*pragma[[:space:]]+GCC[[:space:]]+' "$@"
elif [ "$sub_comments" = "yes" ]
then
    # Looks rather specifically for what this script would generate.
    exec sed -e 's%^/\* #pragma GCC \(.*\) \*/$%#pragma GCC \1%' "$@"
else
    # Reasonably general input; rather specific canonicalized output.
    exec sed -e 's%^[[:space:]]*#[[:space:]]*pragma[[:space:]]\{1,\}GCC[[:space:]]\{1,\}\(.*\)%/* #pragma GCC \1 */%' "$@"
fi

