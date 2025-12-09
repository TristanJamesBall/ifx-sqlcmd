#!/bin/sh
#
# @(#)$Id: Trace.sh,v 4.3 2015/09/21 21:59:40 jleffler Exp $
#
# Copy Output from Command to Trace directory
# With no arguments, run as a filter
# -- capture stdin
# With arguments, execute arguments as comand
# -- capture both stdout and stderr

arg0=`basename $0 .sh`

aflag=''    # Report Atria configuration
eflag=''    # Report Environment
uflag=''    # Report on configurable limits
bflag=no    # Run in background
qflag=no    # No output to stdout
wflag=no    # Wordwrap output
xflag=no    # Run command with trace

usestr="Usage: $arg0 [-abehquwxV][-L log] [--] [command args]"

err_help()
{
    echo "$usestr"
    echo ""
    echo "  -a     Report Atria ClearCase cspec"
    echo "  -b     Run command in background"
    echo "  -e     Do not report on the environment"
    echo "  -h     Print this help message and exit"
    echo "  -q     Quiet mode"
    echo "  -u     Do not report resource limits"
    echo "  -w     Wrap very long lines at 512 bytes"
    echo "  -x     Run script command using '$SHELL -x'"
    echo "  -L log Use named file as log file"
    echo "  -V     Print version information and exit"
    exit 0
}

err_version()
{
    echo "$arg0: "'TRACE Version $Revision: 4.3 $ ($Date: 2015/09/21 21:59:40 $)' | rcsmunger
    exit 0
}

TRACEFILE=""
while [ $# -gt 0 ]
do
    case "$1" in
    -h) err_help;;
    -q) qflag=yes;;
    -w) wflag=yes;;
    -e) eflag=-e;;
    -x) xflag=yes;;
    -u) uflag=-u;;
    -b) bflag=yes;;
    -L) TRACEFILE=$2; shift;;
    -V) err_version;;
    --) shift; break;;
    -*) echo "$usestr" >&2
        exit 1;;
    *)  break;;
    esac
    shift
done

if [ -z "$TRACEFILE" ]
then
    case $# in
    0) TRACEFILE=${LOGFILE:-$arg0};;
    *) TRACEFILE=`basename $1 .sh`;;
    esac
    TRACEFILE=$TRACEFILE.`date +%\Y%\m%\d-%\H%\M%\S`
fi

tracedir=`dirname $TRACEFILE`
TRACEFILE=`basename $TRACEFILE`
if [ $tracedir = "." ]
then TRACEDIR=${TRACEDIR:-$HOME/Trace}
else TRACEDIR=${tracedir}
fi
if [ ! -d $TRACEDIR ]
then mkdir $TRACEDIR || exit 1
fi
TRACEFILE=${TRACEDIR}/${TRACEFILE}

# NB: the '' argument effectively searches $PATH
exec_command(){
    cmd=${1:?}
    shift
    for d in '' /u/jleffler/bin /work?/jleffler/bin /work/jleffler/bin /home/jleffler/bin /usr/local/bin
    do
        if (${d:+$d/}$cmd ${@+"$@"}) 2>/dev/null
        then return 0
        fi
    done
    #echo "Cannot locate $cmd command" 1>&2
    return 1
}

{
# Running in background - input from /dev/null!
[ $bflag = yes ] && exec </dev/null

echo "$arg0 run started: `date`"
case $# in
0)  : OK;;
*)  echo "Command:   $@";;
esac

exec_command environ $aflag $eflag $uflag

echo "******************************************"
echo

trap 'echo; echo; echo "*** $arg0 Interrupted *** `date`"; exit 1' 1 2 3 13 15

if [ $bflag = yes ]
then prefix='nice nohup'
else prefix=''
fi

if [ $xflag = no ]
then
    case $# in
    0) $prefix cat -;;
    *) $prefix "$@";;
    esac
else
    # Notice that the -x option only works on scripts.
    # Executables cannot be run with it.
    case $# in
    0)  $prefix cat -;;
    *)  cmd="$1"; shift;
        path=`pathfile -x "$cmd" 2>/dev/null`
        if [ -z "$path" ]
        then path=`pathfile -r "$cmd" 2>/dev/null`
        fi
        if [ -z "$path" ]
        then
            echo "Failed to locate command $cmd" >&2
            exit 1
        elif file $path | grep ' executable ' >/dev/null 2>&1
        then
            # Cannot run 'sh -x' on executables...
            $prefix $cmd ${@:+"$@"}
        else
            # Could run into problems on careless Perl scripts...
            $prefix ${SHELL:-sh} -x "$path" ${@:+"$@"}
        fi;;
    esac
fi

echo
echo "******************************************"
echo "$arg0 run finished: `date`"
} 2>&1 |
(
# This part of the pipeline must be a sub-shell for background mode to work.
if [ $wflag = no ]
then CAT=cat
else CAT='ww -w512'
fi
if [ $qflag = yes ]
then
    trap '' 1 2 3 13 15
    nice $CAT >>$TRACEFILE 2>&1
elif [ $bflag = no ]
then
    trap '' 1 2 3 13 15
    $CAT | tee -a $TRACEFILE
else
    nice $CAT >>$TRACEFILE 2>&1 &
fi
echo "Trace output in: $TRACEFILE"
)
