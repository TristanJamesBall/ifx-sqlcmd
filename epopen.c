/*
@(#)File:           $RCSfile: epopen.c,v $
@(#)Version:        $Revision: 2015.1 $
@(#)Last changed:   $Date: 2015/07/10 16:05:12 $
@(#)Purpose:        Open a pipe to/from a command
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 2002-03,2005,2007-08,2015
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

/*TABSTOP=4*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include "jlsstools.h"
#include "sqlerr.h"

#ifdef _MSC_VER     /* Used by MS Visual Studio 2008 Express */
#ifndef HAVE_POPEN
#define popen(cmd, mode)    _popen(cmd, mode)
#endif /* HAVE_POPEN */
#endif /* _MSC_VER */

#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
extern const char jlss_id_epopen_c[];
const char jlss_id_epopen_c[] = "@(#)$Id: epopen.c,v 2015.1 2015/07/10 16:05:12 jleffler Exp $";
#endif /* lint */

FILE *sql_epopen(const char *cmd, const char *mode)
{
    FILE    *fp;

    if ((fp = popen(cmd, mode)) == NULL)
        cmd_syswarning(E_CANTOPEN, cmd);
    return(fp);
}
