/*
@(#)File:           $RCSfile: chkfifo.c,v $
@(#)Version:        $Revision: 2015.1 $
@(#)Last changed:   $Date: 2015/07/10 16:04:31 $
@(#)Purpose:        Check whether named file is a FIFO
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 1992,1997,2000,2003,2005,2015
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

/*TABSTOP=4*/
#define _POSIX_SOURCE 1

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
extern const char jlss_id_chkfifo_c[];
const char jlss_id_chkfifo_c[] = "@(#)$Id: chkfifo.c,v 2015.1 2015/07/10 16:04:31 jleffler Exp $";
#endif /* lint */

#ifdef HAVE_MKFIFO

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "jlsstools.h"

#ifndef S_ISFIFO
error S_ISFIFO is not defined as a macro
#endif

/* Is named file a FIFO? */
int chk_fifo(const char *fifo)
{
    struct stat s;

    if (stat(fifo, &s) < 0)
        return(1);
    else if (S_ISFIFO(s.st_mode))
        return(0);
    else
        return(1);
    /*NOTREACHED*/
}

#else

/* No FIFOs on system -- the file is not a FIFO! */
int chk_fifo(const char *fifo)
{
    return(1);
}

#endif /* HAVE_MKFIFO */
