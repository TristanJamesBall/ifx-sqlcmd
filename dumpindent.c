/*
@(#)File:           $RCSfile: dumpindent.c,v $
@(#)Version:        $Revision: 1.4 $
@(#)Last changed:   $Date: 2008/02/29 22:52:44 $
@(#)Purpose:        Indent handling for ESQL/C Type Dumping Functions
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 2005,2008
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

/*TABSTOP=4*/

#include "dumpesql.h"

#include <stdarg.h>

static int in_lvl = 0;
static const char indent[] = "                                       ";

#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
const char jlss_id_dumpindent_c[] = "@(#)$Id: dumpindent.c,v 1.4 2008/02/29 22:52:44 jleffler Exp $";
#endif /* lint */

int dump_setindent(int level)
{
    int old = in_lvl;
    if (level < 0)
        level = 0;
    else if (level > sizeof(indent) - 1)
        level = sizeof(indent) - 1;
    in_lvl = level;
    return(old);
}

const char *dump_getindent(void)
{
    const char *rv = indent + sizeof(indent) - 1;
    if (in_lvl > 0 && in_lvl < sizeof(indent) - 1)
        rv -= in_lvl;
    return(rv);
}

/* Cover for (v)fprintf that inserts blanks at the beginning. */
/* NB: should be called to print one line at a time (not partial lines, not multiple lines). */
void dump_print(FILE *fp, const char *format, ...)
{
    va_list args;

    fputs(dump_getindent(), fp);
    va_start(args, format);
    vfprintf(fp, format, args);
    va_end(args);
}
