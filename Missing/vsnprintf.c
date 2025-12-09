/*
@(#)File:           $RCSfile: vsnprintf.c,v $
@(#)Version:        $Revision: 1.2 $
@(#)Last changed:   $Date: 2015/11/15 07:50:25 $
@(#)Purpose:        Insecure surrogate for vsnprintf()
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 2008
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

/*TABSTOP=4*/

#include <stdarg.h>
#include "stderr.h"
#include "kludge.h"
#include <assert.h>
#include <limits.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wredundant-decls"
#undef vsnprintf
extern int vsnprintf(char *buffer, size_t bufsiz, const char *format, va_list args);
#pragma GCC diagnostic pop

#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
extern const char jlss_id_snprintf_c[];
const char jlss_id_snprintf_c[] = "@(#)$Id: vsnprintf.c,v 1.2 2015/11/15 07:50:25 jleffler Exp $";
#endif /* lint */

int vsnprintf(char *buffer, size_t bufsiz, const char *format, va_list args)
{
    int rc;
    assert(bufsiz <= INT_MAX);
    KLUDGE("Insecure surrogate for vsnprintf()\n");

    rc = vsprintf(buffer, format, args);
    if (rc > (int)bufsiz)
        err_report(ERR_ABORT, ERR_STAT,
                   "surrogate vsnprintf() detected buffer overflow (%ld > %lu)\n",
                   (long)rc, (unsigned long)bufsiz);
    return(rc);
}
