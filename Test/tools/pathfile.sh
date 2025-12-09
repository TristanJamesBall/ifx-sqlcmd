#!/bin/ksh
#
# @(#)$Id: pathfile.sh,v 2.4 2011/12/10 05:16:37 jleffler Exp $
#
# Which command is executed

# Loosely based on which from Kernighan & Pike "The UNIX Programming Environment"

oldpath=$PATH
PATH=/bin:/usr/bin

usage()
{
    echo "Usage: $0 [-Aafrwx] [-p path] command ..." >&2
    exit 1
}

Aflag=no    # Absolute names
tflag=-x    # Check for executability by default (-r read, -w write, -f exist)
aflag=no    # All possible occurrences
while getopts Aafrwxp: opt
do
    case $opt in
    ([frwx])
        tflag=$opt;;
    (p) oldpath=$OPTARG;;
    (a) aflag=yes;;
    (A) Aflag=yes;;
    (*) usage;;
    esac
done
shift $(($OPTIND - 1))

case $# in
0)  usage;;
esac

PATHDIRS=`echo $oldpath | sed ' s/^:/.:/
                                s/::/:.:/g
                                s/:$/:./
                                s/:/ /g'`

echoabsname()
{
    name=$1
    case "$1" in
    (/*)    : OK;;
    (*)     if [ $Aflag = yes ]
            then name=$(echo "$(pwd)/$1" | sed 's%/./%/%g')
            fi
            ;;
    esac
    echo "$name"
}

for cmd in $*
do
    fflag=no
    case "$cmd" in
    */*)
        if [ ! -d $cmd ] && [ $tflag $cmd ]
        then echoabsname $cmd
        else echo "$cmd: not found" 1>&2
        fi;;
    *)
        for directory in $PATHDIRS
        do
            if [ ! -d $directory/$cmd ] && [ $tflag $directory/$cmd ]
            then
                echoabsname $directory/$cmd
                fflag=yes
                [ $aflag = no ] && break
            fi
        done
        if [ $fflag = no ]
        then
            echo "$cmd: not found" >&2
        fi
    esac
done
