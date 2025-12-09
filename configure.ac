# @(#)$Id: configure.ac,v 2013.2 2013/12/30 06:44:10 jleffler Exp $
# SQLCMD Version 90.02 (2016-07-28)
# Process this file with autoconf to produce a configure script
# Input for AUTOCONF 2.60

AC_PREREQ(2.60)dnl Could be using some new features
AC_REVISION($Revision: 2013.2 $)

AC_INIT(reload.ec)

#AC_CONFIG_AUX_DIR(config)

AC_CONFIG_HEADER(config.h)

PACKAGE="sqlcmd"
VERSION="90.02"

AC_MSG_RESULT([Configuring $PACKAGE version $VERSION])

AC_SUBST(PACKAGE)
AC_SUBST(VERSION)

# Transfer user-defined values for CFLAGS, CPPFLAGS, LDFLAGS.
# These might be needed to locate, for example, the readline library
# and headers.
AC_SUBST(CFLAGS)
AC_SUBST(CPPFLAGS)
AC_SUBST(LDFLAGS)

#AM_PROG_INSTALL
AC_PROG_AWK
AC_PROG_CC
AC_PROG_CC_C99
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_RANLIB
AC_PROG_YACC
AC_CHECK_PROG([PERL],[perl])

dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS(sys/time.h unistd.h search.h fcntl.h)

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T
AC_TYPE_MODE_T
AC_TYPE_UINTPTR_T
AC_HEADER_TIME
AC_STRUCT_TM

# memmem() is defined in <string.h> on MacOS X Lion 10.7 and some other platforms
dnl Checks for library functions.
AC_FUNC_STRFTIME
AC_FUNC_VPRINTF
AC_CHECK_FUNCS(fcntl isatty lfind memmove mkfifo sigaction siglongjmp)
AC_CHECK_FUNCS(sleep snprintf strerror strftime strtod strtol vprintf)
AC_CHECK_FUNCS(clock_nanosleep nanosleep usleep)
AC_CHECK_FUNCS(getopt getsubopt popen vsnprintf mkstemp memmem)
AC_SYS_LARGEFILE

# Copied more or less verbatim from Autoconf 2.69 manual
AC_ARG_WITH([readline],
            [AS_HELP_STRING([--with-readline],
                [support fancy command line editing @<:@default=check@:>@])],
            [],
            [with_readline=check])
LIBREADLINE=
AS_IF([test "x$with_readline" != xno],
      [AC_CHECK_LIB([readline], [main],
            [AC_SUBST([LIBREADLINE], ["-lreadline -lncurses -ltinfo"])
             AC_DEFINE([HAVE_LIBREADLINE], [1],
                       [Define if you have libreadline])
            ],
      [if test "x$with_readline" != xcheck; then
            AC_MSG_FAILURE([--with-readline was given, but test for readline failed])
       fi],
      -lncurses -ltinfo)])

dnl Check for the GNU readline function and headers
AC_CHECK_HEADERS(readline/readline.h)

dnl Check for /dev/stdin, etc
AC_DEV_STDIN

dnl Determine properties of Informix ESQL/C
IX_ENV_INFORMIXDIR
IX_PROG_ESQLC
IX_PATH_ESQLINCL
IX_HEADER_VARCHAR
IX_DEFINE_AIXLOCT

AC_PREFIX_DEFAULT(${INFORMIXDIR})

AC_JLSS_FINE_TIMING

if test -z "$ac_cv_prog_YACC"
then
    AC_MSG_RESULT([No YACC found - renaming files so it is unnecessary])
    mv connecty.y old.connecty.y
    mv connecty.c.std connecty.c
fi

dnl Build list of missing object files for which we have suitable substitutes
MISSING_OBJECTS=
for func in lfind memmove strerror snprintf vsnprintf mkstemp
do
    cv_var=ac_cv_func_$func
    if eval test "\$$cv_var" = no
    then
        ln Missing/$func.c .
        MISSING_OBJECTS="$MISSING_OBJECTS $func.o"
        AC_MSG_RESULT(Using home-brew version of $func)
    fi
done
for func in strftime
do
    cv_var=ac_cv_func_$func
    if eval test "\$$cv_var" = no &&
       test "$ac_cv_lib_intl_strftime" = no
    then
        ln Missing/$func.c .
        MISSING_OBJECTS="$MISSING_OBJECTS $func.o"
        AC_MSG_RESULT([Using home-brew version of $func])
    fi
done
AC_SUBST(MISSING_OBJECTS)

if test $ac_cv_header_search_h = no
then
    ln Missing/search.h .
    AC_MSG_RESULT([Using home-brew version of search.h])
fi

if test "$YACC" = "yacc" && test ! -f /bin/yacc && test ! -f /usr/bin/yacc
then AC_MSG_RESULT([If you do not have yacc, run 'make no-yacc' then 'make'])
fi

dnl Generate Makefile and config.h
AC_OUTPUT(Makefile sqlcmd.ins selmultiblob.mk)
#AC_OUTPUT(config/Makefile config/sqlcmd.ins, ln config/Makefile .; ln config/sqlcmd.ins .)
