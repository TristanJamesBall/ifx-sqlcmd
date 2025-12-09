#!/bin/sh
#
# @(#)$Id: ow.sh,v 1.8 2015/11/15 08:06:03 jleffler Exp $
#
# Overwrite file
# From: The UNIX Programming Environment by Kernighan and Pike
# Amended: remove PATH setting; handle file names with blanks.

case $# in
0|1)    echo "Usage: $0 file command [arguments]" 1>&2
        exit 1;;
esac

file="$1"
shift
new=${TMPDIR:-/tmp}/ovrwr.$$.1
old=${TMPDIR:-/tmp}/ovrwr.$$.2

trap "rm -f '$new' '$old' ; exit 1" 0 1 2 15

if "$@" >"$new"
then
    cp "$file" "$old"
    trap "" 1 2 15
    cp "$new" "$file"
    rm -f "$new" "$old"
    trap 0
    exit 0
else
    echo "$0: $1 failed - $file unchanged" 1>&2
    rm -f "$new" "$old"
    trap 0
    exit 1
fi
