#!/bin/sh
#
# @(#)$Id: rfmt.sh,v 1.7 2015/11/15 08:03:58 jleffler Exp $
#
# Generic file reformatter

usage()
{
    echo "Usage: rfmt [-c|e|f] formatter file [file ...]" >&2
    exit 1
}

# Determine formatter
case "$1" in
-c) ev=;     fmt="$2"; filter=no;  shift 2;;    # Normal mode
-e) ev=eval; fmt="$2"; filter=no;  shift 2;;    # Evaluate formatter argument
-f) ev=;     fmt="$2"; filter=yes; shift 2;;    # Pure filter mode
-*) usage;;
*)  ev=;     fmt="$1"; filter=no;  shift 1;;    # Normal mode
esac

# Use formatter on each specified file
case $# in
0)  usage;;
*)
    for file in "$@"
    do
        [ $# -eq 1 ] || echo "$file"
        if [ $filter = yes ]
        then ow "$file" $ev "$fmt" < "$file"
        else ow "$file" $ev "$fmt"   "$file"
        fi
    done;;
esac
