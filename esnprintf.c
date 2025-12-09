/*
@(#)File:           $RCSfile: esnprintf.c,v $
@(#)Version:        $Revision: 1.9 $
@(#)Last changed:   $Date: 2016/07/13 00:00:35 $
@(#)Purpose:        Error-checking cover for snprintf()
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 2008-11,2015-16
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

/*TABSTOP=4*/

#include "posixver.h"
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */
#include "esnprintf.h"
#include "stderr.h"

#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
extern const char jlss_id_esnprintf_c[];
const char jlss_id_esnprintf_c[] = "@(#)$Id: esnprintf.c,v 1.9 2016/07/13 00:00:35 jleffler Exp $";
#endif /* lint */

static int internal_evsnprintf(const char *function, char *buffer, size_t buflen, const char *format, va_list args)
{
    int rv;
    rv = vsnprintf(buffer, buflen, format, args);
    if (rv < 0 || (size_t)rv >= buflen)
        err_abort("%s() - buffer overflow (needed %d, given %d bytes; format <<%s>>)\n",
                  function, rv, (int)buflen, format);
    return(rv);
}

int evsnprintf(char *buffer, size_t buflen, const char *format, va_list args)
{
    int rv = internal_evsnprintf("evsnprintf", buffer, buflen, format, args);
    return(rv);
}

int esnprintf(char *buffer, size_t buflen, const char *format, ...)
{
    va_list args;
    int rv;

    va_start(args, format);
    rv = internal_evsnprintf("esnprintf", buffer, buflen, format, args);
    va_end(args);
    return(rv);
}

#ifdef TEST

int main(void)
{
    puts("!! FAIL !! - no tests written!");
    return(1);
}

#endif /* TEST */
