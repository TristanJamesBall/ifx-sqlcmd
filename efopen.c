/*
@(#)File:           $RCSfile: efopen.c,v $
@(#)Version:        $Revision: 2015.1 $
@(#)Last changed:   $Date: 2015/07/10 16:05:12 $
@(#)Purpose:        Open a file, simulating /dev/stdin etc.
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 1992,1997,1999-2000,2002-03,2005,2007,2015
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

/*TABSTOP=4*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <string.h>
#include "sqlerr.h"
#include "jlsstools.h"

#define strequal(x, y)  (strcmp(x, y) == 0)

#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
extern const char jlss_id_efopen_c[];
const char jlss_id_efopen_c[] = "@(#)$Id: efopen.c,v 2015.1 2015/07/10 16:05:12 jleffler Exp $";
#endif /* lint */

FILE *sql_efopen(const char *file, const char *mode)
{
    FILE    *fp;

    fp = (FILE *)0;
    if (strequal(file, "-") && strequal(mode, "r"))
        fp = stdin;
#ifndef HAVE_DEV_STDIN
    else if (strequal(file, "/dev/stdin") && strequal(mode, "r"))
        fp = stdin;
    else if (strequal(file, "/dev/stdout") && strequal(mode, "w"))
        fp = stdout;
    else if (strequal(file, "/dev/stderr") && strequal(mode, "w"))
        fp = stderr;
#endif /* HAVE_DEV_STDIN */
    else if ((fp = fopen(file, mode)) == NULL)
        cmd_syswarning(E_CANTOPEN, file);
    return(fp);
}
