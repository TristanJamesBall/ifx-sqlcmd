#!/bin/sh
#
# @(#)$Id: environ.sh,v 4.10 2015/10/05 20:22:23 jleffler Exp $
#
# Report on the current environment of the user.

arg0=`basename $0 .sh`

usage(){
    echo "Usage: $arg0 [-abdeu]" >&2
    exit 1
}

aflag=yes
bflag=yes
dflag=yes
eflag=yes
uflag=yes
while getopts abdeu flag
do
    case "$flag" in
    a)  aflag=no;;
    b)  bflag=no;;
    d)  dflag=no;;
    e)  eflag=no;;
    u)  uflag=no;;
    *)  usage;;
    esac
done

if [ $OPTIND -le $# ]
then usage
fi

# NB: the '' argument effectively searches $PATH
exec_command(){
    cmd=${1:?}
    for d in '' /u/jleffler/bin /work?/jleffler/bin /work/jleffler/bin /home/jleffler/bin \
             /usr/local/bin /usr/atria/bin /atria_release/cm_bin
    do
        if (${d:+$d/}$cmd) 2>/dev/null
        then return 0
        fi
    done
    #echo "Cannot locate $cmd command" 1>&2
    return 1
}

if [ $dflag = yes ]
then
    echo "Date:      `date`"
fi

# Solaris 10 /bin/id requires -a to produce all group information
# AIX id rejects -a; Linux accepts but ignores -a.
# Solaris 10 has /usr/xpg4/bin/id that prints all group information by default
if [ $bflag = yes ]
then
    echo "Machine:   `uname -n` (`uname -s -r`)"
    echo "User:      `[ -x /usr/xpg4/bin/id ] && /usr/xpg4/bin/id || id`"
    echo "Directory: `pwd`"
    echo "Umask:     `umask`"
    echo "Terminal:  `tty 2>/dev/null`"
fi

# On Solaris 2.4, Bourne shell handles 'ulimit -aSH' but Korn shell does not.
if [ $uflag = yes ]
then
    # Linux uses 2-4 words for the measure and another for the value.
    # The measure is in the first 24 characters, followed by a space and
    # the value.  Solaris uses a single word for the measure and another
    # for the value.  The sed commands place a pipe before the measure.
    echo
    echo "Limits (soft:hard):"
    tmp=${TMPDIR:-/tmp}/lim.$$
    trap "rm -f $tmp.[HS]; exit 1" 0 1 2 3 13 15
    ulimit -aS | sed 's/.* /&|/' | sort > $tmp.S
    ulimit -aH | sed 's/.* /&|/' | sort > $tmp.H
    join -t '|' -o 1.1,1.2,2.2 $tmp.S $tmp.H |
    awk -F'|' '{printf("%-24s %9s:%s\n", $1, $2, $3);}'
    rm -f $tmp.[HS]
    trap 0
fi

if [ $aflag = yes ]
then
    echo
    exec_command ctenv
fi

if [ $eflag = yes ]
then
    echo
    echo "Environment:"
    env | sort
fi
