/*
@(#)File:           $RCSfile: sqlcmd.h,v $
@(#)Version:        $Revision: 2009.1 $
@(#)Last changed:   $Date: 2009/10/23 16:33:34 $
@(#)Purpose:        Main Include File for SQLCMD
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 1992-93,1997-98,2000-03,2005,2007-09
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

#ifndef SQLCMD_H
#define SQLCMD_H

#ifdef MAIN_PROGRAM
#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
const char jlss_id_sqlcmd_h[] = "@(#)$Id: sqlcmd.h,v 2009.1 2009/10/23 16:33:34 jleffler Exp $";
#endif /* lint */
#endif /* MAIN_PROGRAM */

/* -- Constant Definitions */

/* Canonical base names of commands */
#define SQL_CMD         "sqlcmd"
#define SQL_READ        "sqlread"
#define SQL_UNLOAD      "sqlunload"
#define SQL_RELOAD      "sqlreload"
#define SQL_UPLOAD      "sqlupload"
#define SQL_DBACCESS    "dbaccess"
#define SQL_ISQL        "isql"

/* Internal number for commands */
enum { SQLCMD    = 1 };
enum { SQLUNLOAD = 2 };
enum { SQLRELOAD = 3 };
enum { SQLFIFO   = 4 };
enum { SQLEARG   = 5 };
enum { SQLFARG   = 6 };
enum { SQLUPLOAD = 7 };
enum { DBACCESS  = 8 };

/* -- SQLCMD defaults */

/*
** The default escape is a backslash; but ctxt_setescape() expects a
** string with escapes expanded, so it must be given two backslashes
** in the literal, hence the code has 4 backslashes in the source.
*/
#define DEF_DELIM       "|"
#define DEF_ESCAPE      "\\\\"
#define DEF_QUERYLIMIT  0
#define DEF_QUOTE       "\""
#define DEF_TMPDIR      "/tmp"
#define DEF_TRANSIZE    1024
#define DEF_IBASE       0

/* -- Macro Definitions */

#define NIL(x)      ((x)0)
#define DIM(x)      (sizeof(x)/sizeof(*(x)))

/* NB: FUNCTION_NAME() must appear as last declaration for C89 compilers */
/* (because the macro expands to nothing, so the following semi-colon is an empty statement) */
#if __STDC_VERSION__ >= 199901L || defined(__GNUC__)
#define FUNCTION_NAME(x)    /* nothing - __func__ is available built in */
#else
#define FUNCTION_NAME(x)    static const char __func__[] = x
#endif /* __STDC_VERSION__ */

/* -- Declarations */

extern int      pflag;      /* Program mode (see SQLCMD et al above) */

#endif /* SQLCMD_H */
