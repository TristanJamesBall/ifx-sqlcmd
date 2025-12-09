/*
@(#)File:           $RCSfile: snprintf.c,v $
@(#)Version:        $Revision: 1.3 $
@(#)Last changed:   $Date: 2015/11/15 07:50:25 $
@(#)Purpose:        Insecure surrogate for snprintf()
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 2007
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

/*TABSTOP=4*/

#include "stderr.h"
#include "kludge.h"
#include <assert.h>
#include <limits.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wredundant-decls"
#undef snprintf
extern int snprintf(char *buffer, size_t bufsiz, const char *format, ...);
#pragma GCC diagnostic pop

#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
extern const char jlss_id_snprintf_c[];
const char jlss_id_snprintf_c[] = "@(#)$Id: snprintf.c,v 1.3 2015/11/15 07:50:25 jleffler Exp $";
#endif /* lint */

int snprintf(char *buffer, size_t bufsiz, const char *format, ...)
{
    int rc;
    va_list args;
    assert(bufsiz <= INT_MAX);
    KLUDGE("Insecure surrogate for snprintf()\n");

    va_start(args, format);
    rc = vsprintf(buffer, format, args);
    va_end(args);
    if (rc > (int)bufsiz)
        err_report(ERR_ABORT, ERR_STAT,
                   "surrogate snprintf() detected buffer overflow (%ld > %lu)\n",
                   (long)rc, (unsigned long)bufsiz);
    return(rc);
}
