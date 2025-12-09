dnl @(#)$Id: acinformix.m4,v 2010.2 2010/12/18 22:11:42 jleffler Exp $
dnl
dnl Autoconf macros for supporting Informix ESQL/C
dnl
dnl @(#)Copyright JLSS 1999,2001-05,2007-09
dnl
dnl Versions 5.00 through 9.53, and 2.90 through 4.99 supported (4.10 due 2010Q4).
dnl Don't ask why 2.90 follows 9.53 - it's quaint and obscure and boring!
dnl And a confounded nuisance!

dnl Determine correct value for INFORMIXDIR
AC_DEFUN([IX_ENV_INFORMIXDIR],
[
AC_MSG_CHECKING([for INFORMIXDIR])
ixd_specified=false
AC_ARG_WITH(informixdir,
[   --with-informixdir=dir  Where to find your Informix installation.],
[   INFORMIXDIR=${withval}
    ixd_specified=true
])

if test $ixd_specified = false
then
    INFORMIXDIXDIR=$INFORMIXDIR
    if test x$INFORMIXDIR = x
    then INFORMIXDIR=/usr/informix
    fi
fi

if test -d $INFORMIXDIR
then AC_MSG_RESULT([$INFORMIXDIR])
else AC_MSG_ERROR([Unable to determine valid value for INFORMIXDIR])
fi

AC_SUBST([INFORMIXDIR])
])

dnl Determine ESQL/C compiler and version number, and whether there
dnl is support for CONNECT and IUS data types.
dnl Set ESQLC_COMMAND, ESQLC_VERSION, ESQLC_VERTEXT
AC_DEFUN([IX_PROG_ESQLC],
[
AC_REQUIRE([IX_ENV_INFORMIXDIR])
AC_REQUIRE([AC_PROG_CC])

AC_MSG_CHECKING([for ESQL/C compiler and version])

ESQLC_COMMAND=""
ESQLC_VERSION=""
ESQLC_VERTEXT=""
ESQLC_CCFLAGS=""
for esqlc in esql $INFORMIXDIR/bin/esql
do
    dnl Sample output from 'esql -V' for ESQL/C 5.20
    dnl     IBM INFORMIX-ESQL Version 5.20.UC1
    dnl     Software Serial Number RDS#R123456
    dnl Sample output from 'esql -V' prior to CSDK 2.80:
    dnl     INFORMIX-ESQL Version 7.24.UC5
    dnl     Software Serial Number ACN#A287337
    dnl Sample output from 'esql -V' from CSDK 2.80:
    dnl     IBM Informix CSDK Version 2.80, IBM Informix-ESQL Version 9.52.UC1
    dnl     Software Serial Number RDS#N000000
    dnl Sample output from 'esql -V' from CSDK 2.90:
    dnl     IBM Informix CSDK Version 2.90, IBM Informix-ESQL Version 2.90.UC1
    dnl     Software Serial Number RDS#N000000
    dnl Using 'sed 1q' instead of 'sed -n 1p' generates error messages.
    dnl Note that you cannot easily use [0-9] notation in the sed scripts.
    dnl Note that even if esql cannot be found, the back-tick command
    dnl can succeed - hence check on non-empty $esql_output.
    if esql_output=`$esqlc -V 2>/dev/null | sed -n 1p` && test -n "$esql_output"
    then
        esqlv=`echo "$esql_output" |
               sed -e 's/.*-ESQL Version *//' \
                   -e 's/\\.//' -e 's/\\..*//' `
        if test "x$esqlv" = "x"
        then AC_MSG_ERROR([Version information from $esqlc -V not parsable -- $esql_output])
        else
            AC_MSG_RESULT([compiler $esqlc (version $esqlv)])
            ESQLC_COMMAND=$esqlc
            ESQLC_VERSION=$esqlv
            ESQLC_VERTEXT="$esql_output"
            AC_MSG_CHECKING([for 32-bit vs 64-bit ESQL/C])
            dnl Cannot easily use square brackets in setting esqlb because
            dnl M4 strips the square brackets (would like .*\.\(.\)[^.]*$ as regex)
            esqlb=`echo "$esql_output" | sed -e 's/.*\.\(.\).*$/\1/'`
            dnl Cannot easily use case ... [HTU] either...
            case "$esqlb" in
            H)  ESQLC_BITS=32;;
            T)  ESQLC_BITS=32;;
            U)  ESQLC_BITS=32;;
            F)  ESQLC_BITS=64;;
            *)  AC_MSG_ERROR([Number of bits not determinable from $esql -V ($esqlb) -- $esql_output]);;
            esac
            dnl Dunno why $ESQLC_BITS in place of 32/64 in AC_DEFINE does not work
            if test $ESQLC_BITS = 32
            then AC_DEFINE(ESQLC_BITS,32,[Is ESQL/C a 32-bit version or a 64-bit version])
            else AC_DEFINE(ESQLC_BITS,64,[Is ESQL/C a 32-bit version or a 64-bit version])
            fi
            AC_MSG_RESULT([$ESQLC_BITS bits])
            break
        fi
    fi
done
if test x$ESQLC_COMMAND = x
then AC_MSG_ERROR([Could not find a valid ESQL/C compiler])
fi

AC_SUBST(ESQLC_COMMAND)
AC_SUBST(ESQLC_VERSION)
AC_SUBST(ESQLC_VERTEXT)

dnl Ensure that ESQL/C compiler works
AC_MSG_CHECKING([ESQL/C compiler works])
esql_ec=esqlc-$$.ec
program=esqlc-$$
rm -f $esql_ec
{
echo '#include <sqlca.h>'
echo '#include <stdio.h>'
echo 'int main(void) { if (sqlca.sqlcode != 0) puts("non-zero sqlca.sqlcode"); return(0); }'
} > $esql_ec
if INFORMIXC="$CC" $ESQLC_COMMAND -o $program $esql_ec && ./$program
then AC_MSG_RESULT([OK])
else AC_MSG_ERROR([failed])
fi
rm -f $esql_ec $program $program.c $program.o

AC_MSG_CHECKING([ESQL/C Features])
esqltest=esql-$$.h
rm -f $esqltest
{
echo "/* ESQL/C Features determined by configure script */"
echo '$define ESQLC_BITS' "$ESQLC_BITS;"
echo '$define ESQLC_STORED_PROCEDURES;'
AC_DEFINE([ESQLC_STORED_PROCEDURES],1,[ESQL/C supports stored procedures])
echo '$define ESQLC_VARIABLE_CURSORS;'
AC_DEFINE([ESQLC_VARIABLE_CURSORS],1,[ESQL/C supports string-named cursors])

if test $ESQLC_VERSION -ge 500 && test $ESQLC_VERSION -lt 600
then
    echo '$undef ESQLC_CONNECT;'
    echo '$undef ESQLC_SQLSTATE;'
    echo '$undef ESQLC_RGETLMSG;'
else
    echo '$define ESQLC_CONNECT;'
    echo '$define ESQLC_SQLSTATE;'
    echo '$define ESQLC_RGETLMSG;'
    AC_DEFINE([ESQLC_CONNECT],1,[ESQL/C supports CONNECT statements])
    AC_DEFINE([ESQLC_SQLSTATE],1,[ESQL/C supports SQLSTATE])
    AC_DEFINE([ESQLC_RGETLMSG],1,[ESQL/C supports rgetlmsg()])
fi

if test $ESQLC_VERSION -ge 500 && test $ESQLC_VERSION -lt 700
then echo '$undef  ESQLC_CONNECT_DORMANT;'
else echo '$define ESQLC_CONNECT_DORMANT;'
     AC_DEFINE([ESQLC_CONNECT_DORMANT],1,[ESQL/C supports dormant connections])
fi

if test $ESQLC_VERSION -ge 290 && test $ESQLC_VERSION -lt 500
then echo '$define ESQLC_IUSTYPES;'
     AC_DEFINE([ESQLC_IUSTYPES],1,[ESQL/C supports user-defined types])
elif test $ESQLC_VERSION -ge 900
then echo '$define ESQLC_IUSTYPES;'
     AC_DEFINE([ESQLC_IUSTYPES],1,[ESQL/C supports user-defined types])
else echo '$undef  ESQLC_IUSTYPES;'
fi

if test $ESQLC_VERSION -ge 350 && test $ESQLC_VERSION -lt 500
then echo '$define ESQLC_BIGINT;'
     AC_DEFINE([ESQLC_BIGINT],1,[ESQL/C supports BIGINT data type])
else echo '$undef  ESQLC_BIGINT;'
fi

if test $ESQLC_VERSION -ge 370 && test $ESQLC_VERSION -lt 500
then echo '$define ESQLC_TRUSTED_CONTEXT;'
     AC_DEFINE([ESQLC_TRUSTED_CONTEXT],1,[ESQL/C supports trusted contexts])
else echo '$undef  ESQLC_TRUSTED_CONTEXT;'
fi
dnl Determine whether locator.h provides a typedef for ifx_loc_t.
dnl CSDK 3.00.xC2 did not do so; CSDK 3.00.xC3 and later does.
dnl If it is present, we should use it.
esqlprog=esql-test-$$
cat > $esqlprog.ec <<'!'
    #include "locator.h"
    int somefunc() { ifx_loc_t blob; return(0); }
!
if $ESQLC_COMMAND -c $esqlprog.ec 2>/dev/null && test -f $esqlprog.o
then echo '$define ESQLC_IFX_LOC_T;'
     AC_DEFINE([ESQLC_IFX_LOC_T],1,[ESQL/C header locator.h typedefs ifx_loc_t])
else echo '$undef  ESQLC_IFX_LOC_T;'
fi
rm -f $esqlprog.ec $esqlprog.c $esqlprog.o
} > $esqltest
if cmp -s esqlinfo.h $esqltest 2>/dev/null
then
    AC_MSG_RESULT([esqlinfo.h unchanged])
    rm -f $esqltest
else
    rm -f esqlinfo.h
    mv $esqltest esqlinfo.h
    AC_MSG_RESULT([esqlinfo.h written])
fi

dnl Patch 5.xx ESQL/C script to recognize INFORMIXC...
if test $ESQLC_VERSION -lt 600 && test $ESQLC_VERSION -ge 500
then
    AC_CHECK_PROG(patch, patch, patch)
    if test $patch
    then
        AC_MSG_CHECKING([should we patch esql script])
        if test ! -f ./esql
        then
            AC_MSG_RESULT(yes)
            cp $INFORMIXDIR/bin/esql .
            $patch < esql-5.x-patch
            chmod +x esql
        else
            AC_MSG_RESULT([no - there is already an esql script here])
        fi
    else
        AC_MSG_RESULT([no patch program -- cannot patch esql script])
    fi
fi

AC_MSG_CHECKING([special C compiler flags for ESQL/C])
dnl Uncover any ESQL/C specific C compiler flags
dnl Needed for 64-bit support on Solaris, for example
dnl The second substitution for $INFORMIXDIR/incl deals with ESQL/C 5.x.
dnl This check must be done after patching ESQL/C 5.x compilers.
dnl The inclusion of $CC in INFORMIXC makes this work with a system of
dnl butchered ESQL/C compiler scripts used by Jonathan Leffler
dnl without harming anyone else.
esqlf=`INFORMIXC="echo $CC XXX" esql YYY.c -c`
esqlf=`echo $esqlf | sed -e 's/.*XXX//' -e 's/YYY.*//'`
esqlf=`echo $esqlf | sed -e "s%-I *$INFORMIXDIR/incl/esql%%"`
ESQLC_CCFLAGS=`echo $esqlf | sed -e "s%-I *$INFORMIXDIR/incl%%"`
if test -z "$ESQLC_CCFLAGS"
then AC_MSG_RESULT([no])
else AC_MSG_RESULT([yes ESQLC_CCFLAGS=$ESQLC_CCFLAGS])
fi
AC_SUBST(ESQLC_CCFLAGS)

])

dnl Determine ESQL/C header directory and set ESQLC_INCLDIR
AC_DEFUN([IX_PATH_ESQLINCL],
[
AC_REQUIRE([IX_ENV_INFORMIXDIR])

AC_MSG_CHECKING([for ESQL/C header directory])
ESQLC_INCLDIR=
for esqli in $INFORMIXDIR/incl/esql $INFORMIXDIR/incl/tools $INFORMIXDIR/incl
do
    if test -d $esqli
    then
        ESQLC_INCLDIR=$esqli
        break
    fi
done
if test x$ESQLC_INCLDIR = x
then AC_MSG_ERROR([Could not find a ESQL/C header directory])
else AC_MSG_RESULT([$ESQLC_INCLDIR])
fi

AC_SUBST(ESQLC_INCLDIR)
])

dnl Determine presence or absence of ESQL/C varchar.h header
AC_DEFUN([IX_HEADER_VARCHAR],
[
AC_REQUIRE([IX_PATH_ESQLINCL])

AC_MSG_CHECKING(for ESQL/C varchar.h header)
if test -f $ESQLC_INCLDIR/varchar.h
then
    AC_DEFINE(HAVE_VARCHAR_H,1, [Define if varchar.h exists under $INFORMIXDIR])
    AC_MSG_RESULT(yes)
else
    if test -f Missing/varchar.h
    then
        if cmp -s varchar.h Missing/varchar.h 2>/dev/null
        then
            AC_MSG_RESULT(no - using version previously copied from Missing directory)
        else
            rm -f varchar.h
            cp Missing/varchar.h .
            AC_MSG_RESULT(no - copying version from Missing directory)
        fi
    else
        AC_MSG_RESULT(no - creating minimal varchar.h)
        cat - <<! | sed 's/^[   ][  ]*//' > varchar.h
        #ifndef VARCHAR_H
        #define VARCHAR_H
        #define MAXVCLEN        (255)
        #define VCLENGTH(len)   (VCMAX(len) + 1)
        #define VCMIN(size)     (((size) >> 8) & 0xFF)
        #define VCMAX(size)     ((size) & 0xFF)
        #define VCSIZ(max, min) ((((min) & 0xFF) << 8) | ((max) & 0xFF))
        #endif VARCHAR_H
!
    fi
    AC_DEFINE(HAVE_VARCHAR_H,1,[Define if varchar.h exists under $INFORMIXDIR])
fi
])

dnl determine whether we need to worry about AIX and the loc_t from
dnl /usr/include/sys/localdef31.h; the body of the file is protected
dnl by _H_LOCALEDEF, but the damage is done before config.h is read,
dnl so the define has to be on the command line.
AC_DEFUN([IX_DEFINE_AIXLOCT],
[
AC_CANONICAL_SYSTEM
AC_MSG_CHECKING([for loc_t problems on AIX])

case "$host_os" in
aix*)
    AC_MSG_RESULT([yes; use -D_H_LOCALEDEF workaround])
    AC_DEFINE([ESQLC_AIX_LOC_T],1,[Define if compiled on AIX with loc_t in <sys/localedef31.h>])
    ESQLC_AIXLOCT=-D_H_LOCALEDEF;;
*)
    AC_MSG_RESULT([no, thank goodness!])
    ESQLC_AIXLOCT="";;
esac
AC_SUBST(ESQLC_AIXLOCT)

case "$host_os" in
aix*)
    if test $ESQLC_BITS = 64
    then ESQLC_AIX_ARFLAGS="-X64"
    else ESQLC_AIX_ARFLAGS="-X32"
    fi
    AC_SUBST(ESQLC_AIX_ARFLAGS)
    ;;
esac
])
