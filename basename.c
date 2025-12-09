/*
@(#)File:           $RCSfile: basename.c,v $
@(#)Version:        $Revision: 2.4 $
@(#)Last changed:   $Date: 2008/02/11 08:44:50 $
@(#)Purpose:        Return basename of filename
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 1989,1991,1997,2001,2005,2008
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

/*TABSTOP=4*/

#include "jlss.h"
#include <string.h>

#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
const char jlss_id_basename_c[] = "@(#)$Id: basename.c,v 2.4 2008/02/11 08:44:50 jleffler Exp $";
#endif /* lint */

const char *jlss_basename(const char *s)
{
    const char  *cp;

    /* Strip trailing slashes, worrying about "/" */
    if ((cp = strrchr(s, '/')) != 0 && cp > s && cp[1] == '\0')
    {
        /* Skip backwards over trailing slashes */
        while (cp > s && *cp == '/')
            cp--;
        /* Skip backwards over non-slashes */
        while (cp > s && *cp != '/')
            cp--;
    }

    return((cp == 0 || (cp == s && cp[1] == '\0')) ? s : cp + 1);
}

#ifdef TEST
#include <stdio.h>

static char *list[] =
{
    "/usr/fred/bloggs",
    "/usr/fred/bloggs/",
    "/usr/fred/bloggs////",
    "bloggs",
    "/.",
    ".",
    "/",
    "//",
    "///",
    "////",
    "",
    0
};

int main(void)
{
    char    **name;

    for (name = list; *name != 0; name++)
    {
        printf("%-20s ->", *name);
        printf(" '%s'\n", jlss_basename(*name));
    }
    return(0);
}
#endif /* TEST */
