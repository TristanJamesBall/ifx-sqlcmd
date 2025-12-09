/*
@(#)File:           $RCSfile: fmkstemp.c,v $
@(#)Version:        $Revision: 1.2 $
@(#)Last changed:   $Date: 2014/12/29 00:55:14 $
@(#)Purpose:        File stream cover for mkstemp()
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 2008,2014
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

/*TABSTOP=4*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include "posixver.h"
#include "jlss.h"
#include <stdlib.h>     /* mkstemp() */
#include <unistd.h>     /* close() */

#ifndef HAVE_MKSTEMP
extern int mkstemp(char *name);
#endif /* HAVE_MKSTEMP */

#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
extern const char jlss_id_fmkstemp_c[];
const char jlss_id_fmkstemp_c[] = "@(#)$Id: fmkstemp.c,v 1.2 2014/12/29 00:55:14 jleffler Exp $";
#endif /* lint */

FILE *fmkstemp(char *name)
{
    int fd = mkstemp(name);
    FILE *fp = 0;
    if (fd >= 0)
    {
        fp = fdopen(fd, "w+");
        if (fp == 0)
            close(fd);
    }
    return(fp);
}
