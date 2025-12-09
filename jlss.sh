#!/bin/sh
#
#   @(#)$Id: jlss.sh,v 5.12 2008/08/22 17:53:54 jleffler Exp $
#
#   @(#)Generic JLSS Installation and Uninstallation Tool
#   @(#)(C) Copyright JLSS 1992-93,1997-99,2002-03,2007

################################################################
# This script may be run by Bourne Shell or Korn Shell         #
################################################################
# This is a big script to enable all the products installed    #
# using this system to be handled on both installation and     #
# uninstallation with only two auxilliary files -- the `.lst'  #
# file and the `.ins' file.  And too make life about as easy   #
# as possible for the users, they only have to type 3 words    #
# once they have set the environment correctly:                #
#     Either:  jlss install prodcode                           #
#     Or:      jlss uninstall prodcode                         #
# where "prodcode" is the code name used by JLSS to identify   #
# the set of software which is to be installed or uninstalled. #
# The corresponding configuration files are prodcode.lst and   #
# prodcode.ins, and these are either in the current directory  #
# "." or in "./etc".  The software can be installed in either  #
# the current directory or some other directory, and the other #
# directory need not exist prior to installing the software.   #
################################################################
# This script checks whether it is run by root to ensure that  #
# all the mkdir, chmod, chgrp and chown commands succeed.  If  #
# the user will have sufficient privileges without being root, #
# then the test in the "Check Authorisation" section can be    #
# removed and installation will proceed without problems.      #
# Note that you can set CHOWN, CHGRP and CHMOD in the environ- #
# ment if desired.  This should be able to do any damage.      #
################################################################
# The script is fussy about the environment variables set in   #
# the configuration file so that any "exec" scripts can tell   #
# where the software has been placed.                          #
################################################################

################################################################
# Sample installation configuration file                       #
################################################################
# NB: Some variable names in the configuration file are fixed. #
#     These are: envbin, envdir, envusr, envgrp, envtmp,       #
#     defbin, defdir, defusr, defgrp, deftmp, IN_MSG, IN_TRAP, #
#     RM_MSG, RM_TRAP, PRODUCT, FILELIST                       #
# Of these, envbin and defbin are optional; they are used when #
# a few external files are put in a bin directory and a lot of #
# files are put in a private directory.  If all files are put  #
# in a specific directory (and people are expected to add the  #
# corresponding bin directory to their PATH, etc), then use    #
# just envdir and defdir.                                      #
# The environment variables in IN_MSG etc could be spelt out   #
# literally (eg SCCSDIR instead of $envdir), but envdir etc    #
# must be set.                                                 #
################################################################
#
#envdir=SCCSDIR
#defdir=/usr/local
#envusr=SCCSUSR
#defusr=bin
#envgrp=SCCSGRP
#defgrp=bin
#envtmp=SCCSTMP
#deftmp=${TMPDIR:-/tmp}/jlss
#
#PRODUCT='SCCS ToolKit Version 1 (1992-12-29)'
#
#IN_MSG='
#echo "    Base Directory:  $envdir=$DIR"
#echo "    Owner:           $envusr=$USR"
#echo "    Group:           $envgrp=$GRP"
#echo ""
#echo "The executables   will go in $envdir/bin"
#echo "The documentation will go in $envdir/doc"
#echo "The other files   will go in $envdir/etc"
#'
#
#IN_TRAP="
#echo 'Set: $envdir to define the location of the programs'
#echo '     $envusr to define the owner'
#echo '     $envgrp to define the group'
#trap 0
#exit 1"
#
#RM_MSG='
#echo "    Base Directory:  $envdir=$DIR"
#echo "    Owner:           $envusr=$USR"
#echo "    Group:           $envgrp=$GRP"
#echo ""
#echo "The executables   were placed in $envdir/bin"
#echo "The documentation were placed in $envdir/doc"
#echo "The other files   were placed in $envdir/etc"
#'
#
#RM_TRAP="
#echo 'Set: $envdir to define the location of the programs'
#echo '     $envusr to define the owner'
#echo '     $envgrp to define the group'
#echo '     $envtmp to define where the files are to be moved to'
#trap 0
#exit 1"
#
#FILELIST=etc/sccs.lst
################################################################

arg0=`basename $0 .sh`

#############
# Functions #
#############

version(){
    echo $arg0: 'JLSS Version $Revision: 5.12 $ ($Date: 2008/08/22 17:53:54 $)' |
    sed -e 's/\$[A-Z][a-z]*: \([^$][^$]*\) \$/\1/g'
    exit 0
}

echousage(){
    echo "Usage: $arg0 [-cfhpsxV][-d directory][-o owner][-g group] [install|uninstall] product"
    echo "Auxilliary options: [-b bindir][-t tmpdir][-m suffix]"
}

usage(){
    echousage 1>&2
    exit 1
}

help(){
    echousage
    echo ""
    echo "-b bindir    Separate bin directory"
    echo "-c           Check whether software already installed"
    echo "-d directory Main installation directory"
    echo "-f           Force"
    echo "-g group     Name of group to own software"
    echo "-h           Print this help message"
    echo "-m suffix    Append suffix to mappable file names"
    echo "-o owner     Name of owner (user) to own software"
    echo "-p           Do not set permissions"
    echo "-s           Silent operation"
    echo "-t tmpdir    Temporary directory for uninstalled software"
    echo "-u user      Name of user (owner) to own software"
    echo "-x           Trace operations"
    echo "-V           Print version information and exit"
    exit 0
}

config(){
    internal_error "fault in configuration file $config ($*)"
}

mkpath(){
    # Make all directories leading to named directory
    echo $file |
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
        if [ ! -d $directory ]
        then
            mkdir $directory
            ${CHMOD} $perms $directory
            ${CHGRP} $group $directory
            ${CHOWN} $owner $directory
        fi
    done
}

warn(){
    echo "$arg0: warning - $*" 1>&2
}

internal_error(){
    {
    echo "$arg0: $*"
    echo "$arg0: please report this to your supplier"
    echo
    [ -z "$PRODUCT" ] && echo "*** $MODE failed ***"
    [ -n "$PRODUCT" ] && echo "*** $MODE of $PRODUCT failed ***"
    echo
    } >&2
    exit 1
}

error(){
    echo "$arg0: $*" >&2
    exit 1
}

chk_usrgrp(){
    tmp=./jlss.$$
    trap "rm -f $tmp; exit 1" 1 2 3 13 15
    cp /dev/null $tmp
    if [ $root = no ]
    then
        # Check whether user can set user to specified user
        cmd=:
        if ${CHOWN} $USR $tmp >/dev/null 2>&1
        then : OK
        else
            warn "Cannot use '$CHOWN' to set owner to $USR" >&2
            CHOWN=:
            cmd="echo"
        fi
        # Check whether user can set group to specified group
        if ${CHGRP} $GRP $tmp >/dev/null 2>&1
        then : OK
        else
            warn "Cannot use '$CHGRP' to set group to $GRP" >&2
            CHGRP=:
            cmd="echo"
        fi
        eval $cmd >&2
    else
        # Check that user and group exist
        if ${CHOWN} $USR $tmp >/dev/null 2>&1
        then : OK
        else error "User $USR is not valid" >&2
        fi
        if ${CHGRP} $GRP $tmp >/dev/null 2>&1
        then : OK
        else error "Group $GRP is not valid" >&2
        fi
    fi
    rm -f $tmp
    trap 1 2 3 13 15
}

#######################
# Check authorisation #
#######################

uid=`id | sed 's/uid=\([0-9][0-9]*\)[^0-9].*/\1/'`
case "$uid" in
0)  root=yes;;
*)  root=no;;
esac

: ${CHGRP:=chgrp}
: ${CHOWN:=chown}
: ${CHMOD:=chmod}

###################
# Check arguments #
###################

check=no
force=no
setbin=
setbin=
setdir=
setgrp=
setprm=yes
settmp=
setusr=
suffix=
silent=no
trace=no

while getopts b:cd:fg:hm:o:pst:u:xV opt
do
    case $opt in
    b)  setbin=$OPTARG;;
    c)  check=yes;;
    d)  setdir=$OPTARG;;
    f)  force=yes;;
    g)  setgrp=$OPTARG;;
    h)  help;;
    m)  suffix=$OPTARG;;
    o)  setusr=$OPTARG;;
    p)  setprm=no;;
    s)  silent=yes;;
    t)  settmp=$OPTARG;;
    u)  setusr=$OPTARG;;
    x)  trace=yes;;
    V)  version;;
    *)  usage;;
    esac
done
shift `expr $OPTIND - 1`

if [ $silent = yes ]
then echo=:
else echo=echo
fi

# Do not set user and group
if [ $setprm = no ]
then
    CHGRP=:
    CHOWN=:
fi

case $# in
2)  case $1 in
    install)    MODE=Installation;;
    uninstall)  MODE=Uninstallation;;
    *)          usage;;
    esac;;
*)  usage;;
esac

PATH=$PATH:.:etc
export PATH

#############################
# Locate configuration file #
#############################

config=$2.ins
for d in . ./etc $setdir $setdir/etc
do
    if [ -f $d/$config ] && [ -r $d/$config ]
    then
        config=$d/$config
        break
    fi
done

if [ -f $config ] && [ -r $config ]
then : OK
else internal_error "cannot locate configuration file $config for ${1}ing $2"
fi

###########################
# Read configuration file #
###########################

# Ensure that we don't get affected by stray environment variables
PRODUCT=
FILELIST=
BIN=
DIR=
GRP=
TMP=
USR=
SFX=
IN_MSG=
IN_TRAP=
RM_MSG=
RM_TRAP=
defbin=
defdir=
defgrp=
defsfx=
deftmp=
defusr=
envbin=
envdir=
envgrp=
envsfx=
envtmp=
envusr=

. $config

#######################
# Check configuration #
#######################

# Remove RCS barbarisms
PRODUCT=`echo "$PRODUCT" | sed 's/[$]Product: \(.*\) [$]/\1/'`

[ -z "$PRODUCT"  ] && config "PRODUCT not set"
[ -z "$FILELIST" ] && config "FILELIST not set"
[ -z "$envdir"  ]  && config "envdir not set"
[ -z "$envgrp"  ]  && config "envgrp not set"
[ -z "$envusr"  ]  && config "envusr not set"
[ -z "$envtmp"  ]  && config "envtmp not set"
[ -z "$defdir"  ]  && config "defdir not set"
[ -z "$defgrp"  ]  && config "defgrp not set"
[ -z "$defusr"  ]  && config "defusr not set"
[ -z "$deftmp"  ]  && config "deftmp not set"
[ -z "$IN_MSG"  ]  && config "IN_MSG not set"
[ -z "$IN_TRAP" ]  && config "IN_TRAP not set"
[ -z "$RM_MSG"  ]  && config "RM_MSG not set"
[ -z "$RM_TRAP" ]  && config "RM_TRAP not set"
[ -z "$envbin"  ]  && [ -n "$defbin" ] && config "envbin set but defbin not set"
[ -n "$envbin"  ]  && [ -z "$defbin" ] && config "defbin set but envbin not set"

[ "$defusr" = "installer" ] && defusr=`id | sed 's/uid=[0-9]*(\([^)]*\)).*/\1/'`
[ "$defgrp" = "installer" ] && defgrp=`id | sed 's/.*gid=[0-9]*(\([^)]*\)).*/\1/'`

# Set the environment variables correctly.
# The values set on the command line (setdir etc) override those
# in the environment (variable names denoted by $envdir etc).
# NB: envbin and defbin do not have to be set!
tmpfile=${TMPDIR:-/tmp}/$arg0.$$
trap "rm -f $tmpfile; exit 1" 0 1 2 3 13 15

{
if [ -n "$setdir" ]
then echo "DIR=$setdir"
else echo "DIR=\${$envdir:=$defdir}"
fi
if [ -n "$setusr" ]
then echo "USR=$setusr"
else echo "USR=\${$envusr:=$defusr}"
fi
if [ -n "$setgrp" ]
then echo "GRP=$setgrp"
else echo "GRP=\${$envgrp:=$defgrp}"
fi
if [ -n "$settmp" ]
then echo "TMP=$settmp"
else echo "TMP=\${$envtmp:=$deftmp}"
fi
if [ -n "$envbin" ]
then
    if [ -n "$setbin" ]
    then echo "BIN=$setbin"
    else echo "BIN=\${$envbin:=$defbin}"
    fi
else
    # No envbin defined - manufacture from $envdir.
    envbin=`echo "$envdir" | sed 's/DIR$//'`BIN
    echo "BIN=\$DIR/bin"
fi
if [ -n "$envsfx" ]
then echo "SFX=$suffix"
fi
echo "$envbin=\$BIN"
echo "$envdir=\$DIR"
echo "$envusr=\$USR"
echo "$envgrp=\$GRP"
echo "$envtmp=\$TMP"
if [ -n "$envsfx" ]
then echo "$envsfx=\$SFX"
fi
echo export $envdir $envusr $envgrp $envtmp $envbin $envsfx
} > $tmpfile
. $tmpfile
rm -f $tmpfile

instfile(){
    src=$1
    tgt=$2
    owner=$3
    proup=$4
    perms=$5
    dname=`dirname $tgt`
    [ -d $dname ] || internal_error "$dname is not a directory"
    [ $trace = yes ] && echo "+ install $src as $tgt ($owner:$group:$perms)"
    mv $src tmp.$$          # Save file (in case it is already in situ)
    rm -f $tgt              # Remove existing destination file (if any)
    mv tmp.$$ $tgt          # Store new file in correct place
    ${CHMOD} $perms $tgt
    ${CHGRP} $group $tgt
    ${CHOWN} $owner $tgt
}

#################################
# Installation & Uninstallation #
#################################
if [ $MODE = Installation ]
then

    ######################################
    # Check whether previously installed #
    ######################################
    # Repeated code - not good!
    if [ $check = yes ]
    then
        if [ -f $DIR/$config ] && [ $DIR != . ]
        then
            newprod="$PRODUCT"
            . $DIR/$config
            PRODUCT=`echo "$PRODUCT" | sed 's/[$]Product: \(.*\) [$]/\1/'`
            if [ "$PRODUCT" = "$newprod" ]
            then $echo "$PRODUCT has already been installed."
            else $echo "There is another version, $PRODUCT, already installed."
            fi
            exit 1
        else
            $echo "$PRODUCT has not been installed yet."
            exit 0
        fi
    fi

    $echo ""
    $echo ""
    $echo "$MODE of $PRODUCT"
    $echo ""
    if [ -f $DIR/$config ] && [ $DIR != . ]
    then
        (
        newprod="$PRODUCT"
        . $DIR/$config
        PRODUCT=`echo "$PRODUCT" | sed 's/[$]Product: \(.*\) [$]/\1/'`
        $echo ""
        if [ $force = no ]
        then
            if [ "$PRODUCT" = "$newprod" ]
            then $echo "This version of the product has already been installed."
            else $echo "There is another version, $PRODUCT, already installed."
            fi
            $echo ""
            if [ $silent = no ]
            then
                echo "Do you want to remove the previous installation first?"
                echo "If you do not remove the other version, you may have"
                echo "unused files left lying around"
                echo ""
                echo "If you do     want to remove it, hit the INTERRUPT key."
                echo "If you do not want to remove it, hit the RETURN    key."
                echo ""

                IN_TRAP="echo ''
                echo 'To uninstall the old version:'
                echo '    Change directory to $DIR'
                echo '    Run: $0 uninstall $2'
                echo '    You may need to set the environment correctly again.'
                echo ''
                trap 0
                exit 1"

                trap "$IN_TRAP" 0 2 3
                read x || exit
                echo
                trap 0 2 3
                echo "OK - continuing with installation"
            fi
        else
            p1="The existing installation of"
            p3="will be overwritten."
            if [ "$PRODUCT" = "$newprod" ]
            then p2="this version of the product"
            else p2="the other version, $PRODUCT,"
            fi
            $echo "$p1 $p2 $p3"
            $echo ""
        fi
        )
    fi

    if [ $silent = no ]
    then
        echo "The software will be installed as follows:"
        echo ""
        eval "$IN_MSG"
        if [ $force = no ]
        then
            echo ""
            echo "If this is incorrect, hit the INTERRUPT key."
            echo "If this is correct,   hit the RETURN    key."
            echo ""
            echo ""
            trap "$IN_TRAP" 0 2 3
            read x || exit
            echo
            trap 0 2 3
        fi
    fi

    chk_usrgrp

    if [ $silent = no ] && [ $force = no ]
    then
        echo "$MODE of $PRODUCT started..."
        sleep 5
        echo ""
    fi

    # Install programs and files
    sed -e 's/[     ]*#.*//' -e '/^[    ]*$/d' $FILELIST |
    {
    while read file type link owner group perms
    do
        if [ $owner = - ] ; then owner=$USR ; fi
        if [ $group = - ] ; then group=$GRP ; fi
        case "$type" in
        directory)
            dirlist="$file $dirlist"
            file=$DIR/$file
            $echo "Installing directory $file"
            [ -d $file ] || mkpath $file
            ;;
        deleted)
            [ -f $DIR/$file ] && [ $trace = yes ] && echo "+ obsolete $DIR/$file"
            rm -f $DIR/$file
            ;;
        link)
            file=$DIR/$file
            link=$DIR/$link
            [ -f $link ] || internal_error "missing link file $link"
            rm -f $file
            [ $trace = yes ] && echo "+ link $link to $file"
            ln $link $file
            ;;
        maplink)
            file=$DIR/$file$suffix
            link=$DIR/$link$suffix
            [ -f $link ] || internal_error "missing link file $link"
            rm -f $file
            [ $trace = yes ] && echo "+ link $link to $file"
            ln $link $file
            ;;
        symlink)
            file=$DIR/$file
            link=$DIR/$link
            [ -f $link ] || internal_error "missing link file $link"
            [ $trace = yes ] && echo "+ symlink $link to $file"
            rm -f $file
            ln -s $link $file
            ;;
        mapsym)
            file=$DIR/$file$suffix
            link=$DIR/$link$suffix
            [ -f $link ] || internal_error "missing link file $link"
            [ $trace = yes ] && echo "+ symlink $link to $file"
            rm -f $file
            ln -s $link $file
            ;;
        file)
            [ ! -f $file ] && internal_error "$file not found"
            [ "x$link" != "x-" ] && internal_error "$file has invalid link field"
            instfile $file $DIR/$file $owner $group $perms
            ;;
        condfile)
            [ ! -f $file ] && continue
            [ "x$link" != "x-" ] && internal_error "$file has invalid link field"
            instfile $file $DIR/$file $owner $group $perms
            ;;
        mapfile)
            [ ! -f $file ] && internal_error "$file not found"
            [ "x$link" != "x-" ] && internal_error "$file has invalid link field"
            instfile $file $DIR/$file$suffix $owner $group $perms
            ;;
        configure)
            # Files are configured by $MKSCRIPT into 'link' file.
            [ -n "$MKSCRIPT" ] || internal_error "MKSCRIPT not set in configuration file"
            [ -x "$MKSCRIPT" ] || chmod +x $MKSCRIPT || internal_error "cannot make $MKSCRIPT executable"
            [ $file != $link ] || internal_error "$file is configured onto itself"
            [ ! -f $link ] || internal_error "$file is configured onto existing file $link"
            $MKSCRIPT $file >$link
            instfile $link $BIN/$link $owner $group $perms
            ;;
        *)  internal_error "unknown file type $type"
            ;;
        esac
    done
    for d in $dirlist; do echo $d; done | sort -r | xargs rmdir 2>/dev/null
    }

    $echo ""
    $echo "$MODE of $PRODUCT complete"
    $echo ""

else

    if [ $check = yes ]
    then error "Cannot check (-c) for uninstall"
    fi

    if [ $silent = no ]
    then
        echo ""
        echo ""
        echo "$MODE of $PRODUCT"
        echo ""
        echo "The software was installed as follows:"
        echo ""
        eval "$RM_MSG"
        echo ""
        echo "The uninstalled software will be moved to:"
        echo ""
        echo "    Junk Directory:   $envtmp=$TMP"
        echo ""
        echo "If this is incorrect, hit the INTERRUPT key."
        echo "If this is correct,   hit the RETURN    key."
        echo ""
        echo ""
        trap "$RM_TRAP" 0 2 3
        read x || exit
        echo
        trap 0 2 3
    fi

    chk_usrgrp

    if [ ! -f $DIR/$FILELIST ]
    then internal_error "unable to locate file $DIR/$FILELIST"
    fi

    if [ $silent = no ]
    then
        echo "$MODE of $PRODUCT started...\c"
        sleep 5
        echo ""
        echo ""
    fi

    # Uninstall programs and files
    sed -e 's/[     ]*#.*//' -e '/^[    ]*$/d' $DIR/$FILELIST |
    {
    while read file type link owner group perms
    do
        if [ $owner = - ] ; then owner=$USR ; fi
        if [ $group = - ] ; then group=$GRP ; fi
        case "$type" in
        directory)
            dirlist="$DIR/$file $dirlist"
            file=$TMP/$file
            $echo "Creating directory $file"
            [ -d $file ] || mkpath $file
            ;;
        deleted|exec)
            : OK
            ;;
        file|link|symlink|condfile|configure)
            ifile=$DIR/$file
            jfile=$TMP/$file
            [ $trace = yes ] && echo "+ remove $ifile"
            [ -f $ifile ] || echo "* missing $ifile"
            [ -f $ifile ] && mv $ifile $jfile
            ;;
        mapfile|maplink|mapsym)
            ifile=$DIR/$file$suffix
            jfile=$TMP/$file$suffix
            [ $trace = yes ] && echo "+ remove $ifile"
            [ -f $ifile ] || echo "* missing $ifile"
            [ -f $ifile ] && mv $ifile $jfile
            ;;
        *)
            internal_error "unknown file type $type"
            ;;
        esac
    done
    for d in $dirlist; do echo $d; done | sort -r | xargs rmdir 2>/dev/null
    }

    $echo ""
    $echo "$MODE of $PRODUCT complete"
    $echo ""
    $echo "Remember to remove the software from $TMP as soon as convenient"
    $echo ""

fi
