/* config.h.in.  Generated from configure.ac by autoheader.  */
/* Hacked into Windows XP SP2 configuration with MS Visual Studio 2008 Express */
/* @(#)$Id: config.h,v 1.3 2008/05/26 20:54:17 jleffler Exp $ */

/* Define if _isatty() is provided by <io.h> (Win32) */
#define HAVE_NT_ISATTY

/* Define if sleep() function is available - NT problem */
#undef HAVE_SLEEP

/* Define if _sleep() function is available instead of sleep() */
#define HAVE_NT_SLEEP

#if defined(HAVE_NT_SLEEP) && !defined(HAVE_SLEEP)
#define sleep(s) _sleep((s)*1000)
#define /**/HAVE_SLEEP
#endif

/* ESQL/C supports BIGINT data type */
#undef ESQLC_BIGINT

/* ESQL/C supports CONNECT statements */
#define ESQLC_CONNECT

/* ESQL/C supports dormant connections */
#define ESQLC_CONNECT_DORMANT

/* ESQL/C header locator.h typedefs ifx_loc_t */
#undef ESQLC_IFX_LOC_T

/* ESQL/C supports user-defined types */
#define ESQLC_IUSTYPES

/* ESQL/C supports rgetlmsg() */
#define ESQLC_RGETLMSG

/* ESQL/C supports SQLSTATE */
#define ESQLC_SQLSTATE

/* ESQL/C supports stored procedures */
#define ESQLC_STORED_PROCEDURES

/* ESQL/C supports string-named cursors */
#define ESQLC_VARIABLE_CURSORS

/* Define to 1 if you have the `clock' function. */
#define HAVE_CLOCK

/* Define to 1 if you have the `clock_gettime' function. */
#undef HAVE_CLOCK_GETTIME

/* Define if devices /dev/stdin, /dev/stdout, /dev/stderr exist */
#undef HAVE_DEV_STDIN

/* Define to 1 if you don't have `vprintf' but do have `_doprnt.' */
#undef HAVE_DOPRNT

/* Define to 1 if you have the `fcntl' function. */
#undef HAVE_FCNTL

/* Define to 1 if you have the <fcntl.h> header file. */
#define HAVE_FCNTL_H

/* Define to 1 if you have the `ftime' function. */
#undef HAVE_FTIME

/* Define to 1 if you have the `getopt' function. */
#undef HAVE_GETOPT

/* Define to 1 if you have the `getsubopt' function. */
#undef HAVE_GETSUBOPT

/* Define to 1 if you have the `gettimeofday' function. */
#undef HAVE_GETTIMEOFDAY

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H

/* Define to 1 if you have the `isatty' function. */
#undef HAVE_ISATTY

/* Define to 1 if you have the `lfind' function. */
#undef HAVE_LFIND

/* Define to 1 if you have the `posix4' library (-lposix4). */
#undef HAVE_LIBPOSIX4

/* Define to 1 if you have the `"curses"' library (-l"curses"). */
#undef HAVE_LIB_CURSES_

/* Define to 1 if you have the `"ncurses"' library (-l"ncurses"). */
#undef HAVE_LIB_NCURSES_

/* Define to 1 if you have the `"readline"' library (-l"readline"). */
#undef HAVE_LIB_READLINE_

/* Define to 1 if you have the `memmove' function. */
#define HAVE_MEMMOVE

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H

/* Define to 1 if you have the `mkfifo' function. */
#undef HAVE_MKFIFO

/* Define to 1 if you have the `popen' function. */
#undef HAVE_POPEN

/* Define to 1 if you have the <readline/readline.h> header file. */
#undef HAVE_READLINE_READLINE_H

/* Define to 1 if you have the <search.h> header file. */
#define HAVE_SEARCH_H

/* Define to 1 if you have the `sigaction' function. */
#undef HAVE_SIGACTION

/* Define to 1 if you have the `siglongjmp' function. */
#undef HAVE_SIGLONGJMP

/* Define to 1 if you have the `sleep' function. */
#undef HAVE_SLEEP

/* Define to 1 if you have the `snprintf' function. */
#define HAVE_SNPRINTF

/* Define to 1 if you have the <stdint.h> header file. */
#undef HAVE_STDINT_H

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H

/* Define to 1 if you have the `strerror' function. */
#define HAVE_STRERROR

/* Define to 1 if you have the `strftime' function. */
#define HAVE_STRFTIME

/* Define to 1 if you have the <strings.h> header file. */
#undef HAVE_STRINGS_H

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H

/* Define to 1 if you have the `strtod' function. */
#define HAVE_STRTOD

/* Define to 1 if you have the `strtol' function. */
#define HAVE_STRTOL

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H

/* Define to 1 if you have the <sys/time.h> header file. */
#undef HAVE_SYS_TIME_H

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H

/* Define to 1 if you have the `times' function. */
#undef HAVE_TIMES

/* Define to 1 if the system has the type `uintptr_t'. */
#undef HAVE_UINTPTR_T

/* Define to 1 if you have the <unistd.h> header file. */
#undef HAVE_UNISTD_H

/* Define if varchar.h exists under $INFORMIXDIR */
#define HAVE_VARCHAR_H

/* Define to 1 if you have the `vprintf' function. */
#define HAVE_VPRINTF

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT    "jleffler@us.ibm.com"

/* Define to the full name of this package. */
#define PACKAGE_NAME "sqlcmd"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "SQLCMD Version 90.02 (2016-07-28)"

/* Define to the one symbol short name of this package. */
#undef PACKAGE_TARNAME

/* Define to the version of this package. */
#define PACKAGE_VERSION "90.02"

/* Define as the return type of signal handlers (`int' or `void'). */
#define RETSIGTYPE void

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS

/* Define to 1 if you can safely include both <sys/time.h> and <time.h>. */
#undef TIME_WITH_SYS_TIME

/* Define to 1 if your <sys/time.h> declares `struct tm'. */
#undef TM_IN_SYS_TIME

/* Number of bits in a file offset, on hosts where this is settable. */
#undef _FILE_OFFSET_BITS

/* Define for large files, on AIX-style hosts. */
/* #undef _LARGE_FILES */

/* Define to empty if `const' does not conform to ANSI C. */
#undef const

/* Define to `int' if <sys/types.h> does not define. */
#define mode_t int

/* Define to `unsigned int' if <sys/types.h> does not define. */
/* #undef size_t */

/* Define to the type of an unsigned integer type wide enough to hold a
   pointer, if such a type exists, and if the system does not define it. */
#define uintptr_t long
