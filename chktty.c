/*
@(#)File:           $RCSfile: chktty.c,v $
@(#)Version:        $Revision: 1.6 $
@(#)Last changed:   $Date: 2015/07/10 16:04:04 $
@(#)Purpose:        Check whether file pointer refers to a tty.
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 2000,2004-05,2015
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

/*TABSTOP=4*/

#define _POSIX_SOURCE   1

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */

#ifdef HAVE_NT_ISATTY
/* NT does not have <unistd.h> */
#define isatty(s)   _isatty(s)
#include <io.h>
#endif /* HAVE_NT_ISATTY */

#include "chktty.h"

#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
extern const char jlss_id_chktty_c[];
const char jlss_id_chktty_c[] = "@(#)$Id: chktty.c,v 1.6 2015/07/10 16:04:04 jleffler Exp $";
#endif /* lint */

int      chk_tty(FILE *fp)
{
    return(isatty(fileno(fp)));
}
