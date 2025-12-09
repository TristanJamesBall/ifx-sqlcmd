/*
@(#)File:           $RCSfile: getopt.h,v $
@(#)Version:        $Revision: 2015.3 $
@(#)Last changed:   $Date: 2015/09/29 07:13:26 $
@(#)Purpose:        Declarations for GETOPT(3) and GETSUBOPT(3)
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 1992-93,1996-2000,2003,2006,2008,2010-11,2013,2015
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

#ifndef GETOPT_H
#define GETOPT_H

#ifdef MAIN_PROGRAM
#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
extern const char jlss_id_getopt_h[];
const char jlss_id_getopt_h[] = "@(#)$Id: getopt.h,v 2015.3 2015/09/29 07:13:26 jleffler Exp $";
#endif /* lint */
#endif /* MAIN_PROGRAM */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

/* Note that POSIX says getopt() returns -1 rather than EOF */
#include <stdio.h>  /* Declares getopt() and getsubopt() on Solaris 7, 8 */
#ifdef HAVE_UNISTD_H
#include <unistd.h> /* Declares getopt() under SUS v3 aka POSIX 1003.1 2001 */
#endif /* HAVE_UNISTD_H */
#include <stdlib.h> /* Declares getsubopt() under SUS v3 aka POSIX 1003.1 2001 */

/* MS Visual Studio 2008 Express does not include getopt() or getsubopt() */
#ifdef _MSC_VER     /* Used by MS VS 2008 Express in <sys/stat.h> */
#if !defined(HAVE_GETOPT) || !defined(HAVE_GETSUBOPT)
#undef USE_JLSS_GETOPT
#define USE_JLSS_GETOPT
#undef USE_JLSS_GETSUBOPT
#define USE_JLSS_GETSUBOPT
#endif /* !HAVE_GETOPT || !HAVE_GETSUBOPT */
#endif /* _MSC_VER */

/*
** GNU getopt (and JLSS getopt, which is built on an ancient GNU getopt)
** provides facilities not available in standard getopt.
** Specifically, it will reorder all option arguments before all non-option
** arguments unless the environment variable POSIXLY_CORRECT is
** defined.  It can also handle optional arguments, which must be attached
** to option letter on the command line, indicated by two colons after the
** option letter.  It can be told to return all arguments in order, with a
** value of '\0' indicating a file option by starting the options string
** with a '-'.  It also has a different interface from standard getopt
** because the second (argv) argument is not const.
*/

/*
** Go with POSIX-compliant declarations for getopt() and getsubopt().
** Note, however, that at various times past, declaring either getopt() or
** getsubopt() has proven incredibly problematic, with varying alternative
** headers declaring them with varying feature test macros and with varying
** degrees of constness in the arguments.
**
** Note that on Linux, <unistd.h> includes <getopt.h>, which may pick up
** this header.  For our immediate purposes, where no code uses GNU's
** getopt_long(), it is sufficient for this code to always define getopt()
** and its global variables.  Of course, GNU's <getopt.h> does not declare
** getsubopt().
*/

#ifdef USE_JLSS_GETOPT
#define GETOPT(argc, argv, opts)    jlss_getopt(argc, argv, opts)
#define opterr  jlss_opterr
#define optind  jlss_optind
#define optarg  jlss_optarg
#define optopt  jlss_optopt
extern int jlss_getopt(int argc, char **argv, const char *opts);
#else
#define GETOPT(argc, argv, opts)    getopt(argc, argv, opts)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wredundant-decls"
extern int getopt(int argc, char *const*argv, const char *opts);
#pragma GCC diagnostic pop
#endif /* USE_JLSS_GETOPT */

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wredundant-decls"
extern int   optopt;
extern int   opterr;
extern int   optind;
extern char *optarg;
#pragma GCC diagnostic pop

#ifdef USE_JLSS_GETSUBOPT
#define GETSUBOPT(argc, argv, opts) jlss_getsubopt(argc, argv, opts)
extern int jlss_getsubopt(char **opt, char * const *names, char **value);
#else
#define GETSUBOPT(argc, argv, opts) getsubopt(argc, argv, opts)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wredundant-decls"
extern int getsubopt(char **opt, char *const *names, char **value);
#pragma GCC diagnostic pop
#endif /* USE_JLSS_GETSUBOPT */

#endif /* GETOPT_H */
