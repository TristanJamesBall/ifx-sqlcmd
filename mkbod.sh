#!/bin/ksh
#
#   "@(#)$Id: mkbod.sh,v 2.7 2012/05/28 21:31:19 jleffler Exp $"
#
#   @(#)SQLCMD Version 90.02 (2016-07-28)
#
#   Make Binary-Only Distribution

arg0=`basename $0 .sh`

usage()
{
    echo "Usage: $arg0 [-q] source object bod-file" >&2
    exit 1
}

qflag=no
while getopts q opt
do
    case "$opt" in
    q)  qflag=yes;;
    *)  usage;;
    esac
done
shift $(($OPTIND - 1))

[ $# -ne 3 ] && usage

SOURCEDIR=${1}
OBJECTDIR=${2}

: ${CP:="cp -p"}    # POSIX.2

mkpath(){
    # Make all directories leading to named directory
    for file in "$@"; do echo $file; done |
    awk -F/ '{  string = "";
                # $1 is empty for absolute paths, but not relative paths
                if ($1 != "") {
                    string = $1;
                    print string;
                }
                for (i = 2; i <= NF; i++) {
                    string = string "/" $i;
                    print string;
                }
            }' |
    while read directory
    do
        [ -d $directory ] || mkdir $directory
    done
}

[ -d $SOURCEDIR ] || { echo "$arg0: $SOURCEDIR is not a directory"; exit 1; } >&2
[ -f $3 ] && [ -r $3 ] || { echo "$arg0: $3 is not a file"; exit 1; } >&2
[ -d $OBJECTDIR ] || mkpath $OBJECTDIR || { echo "$arg0: unable to create directory $OBJECTDIR"; exit 1; } >&2

sed -e 's/[ 	]*#.*//' -e '/^[ 	]*$/d' ${3} |
{
status=0
while read dist comp
do

    case $dist in   # Handle variable setting lines
    *=*)    eval $dist $comp
            continue;;
    esac

    eval target=$OBJECTDIR/$dist
    if [ ! -f $target ]
    then
        tgtdir=`dirname $target`
        [ -d $tgtdir ] || mkpath $tgtdir
        [ $qflag = yes ] || echo $target
        eval comp=$comp
        case $comp in
        /*) # Absolute
            if [ ! -f $comp ]
            then
                echo "$arg0: cannot find $comp" >&2
                status=1
                continue
            fi
            $CP $comp $target
            ;;
        *)  # Relative
            eval source=$SOURCEDIR/$comp
            if [ ! -f $source ]
            then
                echo "$arg0: cannot find $source" >&2
                status=1
                continue
            fi
            if [ $source != $target ]
            then
                $CP $source $target
            fi
            ;;
        esac
    fi

done
exit $status
}
