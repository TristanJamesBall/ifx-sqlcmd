/*
@(#)File:           $RCSfile: quotify.c,v $
@(#)Version:        $Revision: 2015.1 $
@(#)Last changed:   $Date: 2015/07/10 16:05:12 $
@(#)Purpose:        Enclose command line argument into string enclosed by single quotes
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 2003-05,2008,2015
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

/*TABSTOP=4*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "jlss.h"       /* nstrcpy() */
#include "jlsstools.h"  /* declaration of quotify() */
#include "esnprintf.h"

#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
extern const char jlss_id_quotify_c[];
const char jlss_id_quotify_c[] = "@(#)$Id: quotify.c,v 2015.1 2015/07/10 16:05:12 jleffler Exp $";
#endif /* lint */

/* Convert command line argument into string enclosed by single quotes */
char *quotify(const char *str, char *buffer, size_t buflen)
{
    size_t len = strlen(str);
    if (len > 1 && (str[0] == '\'' || str[0] == '\"') && str[len-1] == str[0])
    {
        /* User appears to have done the job for us */
        /* Thorough job would ensure that enclose quotes are in pairs */
        assert(buflen > len);
        nstrcpy(buffer, buflen, str);
    }
    else if (len > 1)
    {
        /* Does not handle embedded quotes */
        /* Thorough job would escape (double-up) enclosed quotes */
        assert(buflen >= len + 3 && strpbrk(str, "\'\"") == 0);
        esnprintf(buffer, buflen, "'%s'", str);
    }
    else if (len == 1 && (str[0] == '\'' || str[0] == '\"'))
        esnprintf(buffer, buflen, "'\\%c'", str[0]);
    else
        esnprintf(buffer, buflen, "'%c'", str[0]);
    return(buffer);
}
