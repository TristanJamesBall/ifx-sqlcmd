/*
@(#)File:           $RCSfile: gnureadline.h,v $
@(#)Version:        $Revision: 1.2 $
@(#)Last changed:   $Date: 2013/12/30 06:44:10 $
@(#)Purpose:        Cover for GNU readline/readline.h
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 2013
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

/*TABSTOP=4*/

#ifndef JLSS_ID_GNUREADLINE_H
#define JLSS_ID_GNUREADLINE_H

/*
** There are a couple of 'issues' with readline/readline.h and
** readline/rltypedefs.h under extremely stringent compiler options.
** 1.  Unless #if defined (_FUNCTION_DEF), readline/rltypedefs.h defines
**     the parameter-less function typedefs:
**     typedef int Function ();
**     typedef void VFunction ();
**     typedef char *CPFunction ();
**     typedef char **CPPFunction ();
**
** 2.  Unless #if defined (USE_VARARGS) && defined (PREFER_STDARG), the
**     rl_message() function is declared without any prototype at all.
*/

#ifdef MAIN_PROGRAM
#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
extern const char jlss_id_gnureadline_h[];
const char jlss_id_gnureadline_h[] = "@(#)$Id: gnureadline.h,v 1.2 2013/12/30 06:44:10 jleffler Exp $";
#endif /* lint */
#endif /* MAIN_PROGRAM */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef HAVE_LIBREADLINE
#undef HAVE_READLINE_READLINE_H
#endif /* HAVE_LIBREADLINE */

#ifdef HAVE_READLINE_READLINE_H
/* Suppress non-prototype declarations of unused typedef function types in readline/rltypedefs.h */
#define _FUNCTION_DEF
#define USE_VARARGS
#define PREFER_STDARG
#include <readline/readline.h>
#include <readline/history.h>
#undef _FUNCTION_DEF
#undef USE_VARARGS
#undef PREFER_STDARG
#endif /* HAVE_READLINE_READLINE_H */

#endif /* JLSS_ID_GNUREADLINE_H */
