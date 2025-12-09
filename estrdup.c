/*
@(#)File:           $RCSfile: estrdup.c,v $
@(#)Version:        $Revision: 5.7 $
@(#)Last changed:   $Date: 2015/06/02 03:05:40 $
@(#)Purpose:        Simulate STRDUP(3) with error exit on
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 2001,2005,2008,2015
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

/*TABSTOP=4*/

#include "emalloc.h"
#include <string.h>

#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
extern const char jlss_id_estrdup_c[];
const char jlss_id_estrdup_c[] = "@(#)$Id: estrdup.c,v 5.7 2015/06/02 03:05:40 jleffler Exp $";
#endif /* lint */

char *estrdup(char const *s)
{
    const char *src = s ? s : "";
    char *dst = (char *)MALLOC(strlen(src) + 1);    /*=C++=*/
    strcpy(dst, src);
    return(dst);
}
