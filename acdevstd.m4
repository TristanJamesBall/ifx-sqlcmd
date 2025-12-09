dnl @(#)$Id: acdevstd.m4,v 1.4 2007/04/16 04:54:01 jleffler Exp $
dnl
dnl Autoconf macro for determining /dev/stdin, /dev/stdout, /dev/stderr

dnl Determine whether /dev/stdin, /dev/stdout and /dev/stderr exist
dnl Assumes they will be character devices if they do exist
AC_DEFUN([AC_DEV_STDIN],
[
AC_MSG_CHECKING([for /dev/stdin, /dev/stdout, /dev/stderr])
AC_CACHE_VAL(ac_cv_dev_stdin,
[ 
ac_cv_dev_stdin=yes
for dev in /dev/stdin /dev/stdout /dev/stderr
do
	if test -c $dev
	then : OK
	else ac_cv_dev_stdin=no
	fi
done
])

if test $ac_cv_dev_stdin = yes
then
	AC_MSG_RESULT(yes)
	AC_DEFINE(HAVE_DEV_STDIN,1,
		[Define if devices /dev/stdin, /dev/stdout, /dev/stderr exist])
else
	AC_MSG_RESULT(no)
fi
])
