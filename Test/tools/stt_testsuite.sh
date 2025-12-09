#!/bin/ksh
#
#   @(#)$Id: stt_testsuite.sh,v 1.9 2013/12/28 22:11:14 jleffler Exp $
#
#   Control Program for SQLCMD Test Tools (STT)

[ "$STT_DEBUG_STT_TESTSUITE" = yes ] && set -x

arg0=$(basename $0 .sh)

error(){
    echo "$arg0: $*" 1>&2
    exit 1
}

# Verify that current directory is the Test directory.
if [ ! -d tools ]
then error "no tools sub-directory in $PWD!"
fi

# Ensure that tools are on path, at start of path
export PATH=$PWD/tools:$PATH

# Ensure that executables under test can be located
# Export SQLCMD and STT_SQLCMD environment variables.
builddir=$(dirname $PWD)
for program in sqlcmd sqlunload sqlreload sqlupload selblob
do
    if [ ! -x $builddir/$program ]
    then error "No executable for $program in $builddir"
    fi
    uc=$(echo $program | tr '[a-z]' '[A-Z]')
    export $uc=$builddir/$program
    export STT_$uc=$builddir/$program
done

# Ensure that SQLCMDLOG is unset to prevent corruption of
# central log files, etc.
unset SQLCMDLOG

# Default values for environment variabls.
# NB: Set DBDATE explicitly to avoid troubles with server
# started with DBDATE=Y4MD- not defaulting to DBDATE=MDY4/
# as documented when user has no DBDATE in environment.
unset DBDELIMITER
export DBDATE="DMY4/"

# Determine whether default server is OnLine or SE.
# Assume SQLCMD mostly works.
if $SQLCMD -e 'connect to default' -e 'info connections' | grep '|SE|' >/dev/null
then export STT_SERVERTYPE=se
else export STT_SERVERTYPE=online
fi

# Establish how to handle echo without a newline:
# Options are "echo -n XYZ" and "echo 'XYZ\c'".
# Cribbed from autoconf
if (echo "testing\c"; echo 1,2,3) | grep c >/dev/null; then
  # Stardent Vistra SVR4 grep lacks -e, says ghazi@caip.rutgers.edu.
  if (echo -n testing; echo 1,2,3) | sed s/-n/xn/ | grep xn >/dev/null; then
    STT_ECHO_N= STT_ECHO_C='
' STT_ECHO_T='  '
  else
    STT_ECHO_N=-n STT_ECHO_C= STT_ECHO_T=
  fi
else
  STT_ECHO_N= STT_ECHO_C='\c' STT_ECHO_T=
fi
export STT_ECHO_N STT_ECHO_C STT_ECHO_T

export STT_DB_LOGGED=logged
export STT_DB_UNLOGGED=unlogged
export STT_DB_MODEANSI=mode_ansi

# Create a log directory for this run (TRACEDIR used by Trace command)
# Beware SCCS
time=$(date +%\Y%\m%\d-%\H%\M%\S)
export TRACEDIR=$PWD/log/$time
export STT_TRACEDIR=$TRACEDIR

mkdir -p $TRACEDIR

### Do NOT set environment variables after this line
environ > $STT_TRACEDIR/environ

echo "Log directory: $TRACEDIR"

Trace -e -u "$@"

echo "Log directory: $TRACEDIR"
