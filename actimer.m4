# @(#)$Id: actimer.m4,v 1.8 2009/07/27 02:43:34 jleffler Exp $
# @(#)SQLCMD Version 90.02 (2016-07-28)
# @(#)Copyright JLSS 1999-2001,2003,2007,2009
# Autoconf macro determining fine timing for JLSS timer.c code

dnl The code in timer.c (not timer.h - please note) will pick up
dnl the correct values from the HAVE_CLOCK_GETTIME etc definitions
dnl that are picked up in sequence.  There is usually no further
dnl action required.
dnl It has been reported that the gettimeofday() structure, struct
dnl timeval, is not declared automatically on some platform (Ravi
dnl at Farelogix.com 2003-04) but the details are scant.
dnl If you want to use this to configure some other code than
dnl timer.c, then you will need to include config.h if it is
dnl available, define HAVE_GETTIMEOFDAY as a default if it is not,
dnl and then code using tests on: HAVE_CLOCK_GETTIME,
dnl HAVE_GETTIMEOFDAY, HAVE_TIMES, HAVE_CLOCK, HAVE_FTIME,
dnl HAVE_TIME.

AC_DEFUN([AC_JLSS_FINE_TIMING],
[
AC_CHECK_LIB([posix4], [clock_gettime])     dnl Solaris 8 and later (maybe earlier too)
AC_REQUIRE([AC_HEADER_TIME])
AC_MSG_RESULT([checking for fine-resolution timing])
using="Fine-resolution timing will use"

AC_CHECK_FUNCS([clock_gettime])			dnl POSIX.4
if test $ac_cv_func_clock_gettime = yes
then AC_MSG_RESULT([$using POSIX.4 clock_gettime()])
else
	AC_CHECK_FUNCS([gettimeofday])		dnl SUS (Unix-98)
	if test $ac_cv_func_gettimeofday = yes
	then
		dnl Add tests for struct timeval
		AC_MSG_RESULT([$using Unix-98 gettimeofday()])
	else
		AC_CHECK_FUNCS([times])		dnl POSIX.1
		if test $ac_cv_func_times =  yes
		then AC_MSG_RESULT([$using POSIX.1 times()])
		else
			AC_CHECK_FUNCS([clock])	dnl ISO C 1990
			if test $ac_cv_func_clock = yes
			then AC_MSG_RESULT([$using ISO C 1990 clock()])
			else
				AC_CHECK_FUNCS([ftime])	dnl V7 Unix
				if test $ac_cv_func_ftime = yes
				then AC_MSG_RESULT([$using Version 7 Unix ftime()])
				else
					# Do not regard this as usable
					# NB: should not happen; ISO C defines clock().
					AC_MSG_RESULT([$using ISO C (POSIX.1) time() -- 1 second resolution (apalling!)])
				fi
			fi
		fi
	fi
fi 
])
