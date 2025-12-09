/*
@(#)File:           $RCSfile: vstrcpy.c,v $
@(#)Version:        $Revision: 1.13 $
@(#)Last changed:   $Date: 2008/02/11 08:44:50 $
@(#)Purpose:        Copy/concatenate variable number of strings
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 1988-89,1991,1995,1997,2005,2008
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

/*TABSTOP=4*/

/*
**  vstrcpy(dst, n, src1, src2, ...);
**
**  Vstrcpy copies each of its arguments src1, src2, ... into dst
**  and null terminates the string.  If given the value n = 0, it
**  simply copies a null into dst[0].
**  Vstrcpy assumes that there is enough space in dst to contain
**  the strings src1, src2, ...
**  concatenated together.
**  Vstrcpy returns a pointer to the terminal null.
*/

#include "jlss.h"
#include <stdarg.h>

#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
const char jlss_id_vstrcpy_c[] = "@(#)$Id: vstrcpy.c,v 1.13 2008/02/11 08:44:50 jleffler Exp $";
#endif /* lint */

char *vstrcpy(char *dst, int n, ...)
{
    const char     *src;
    int             i;
    va_list         args;

    va_start(args, n);

    for (i = 0; i < n; i++)
    {
        src = va_arg(args, const char *);
        while ((*dst = *src++) != '\0')
            dst++;
    }
    *dst = '\0';
    va_end(args);
    return(dst);
}

#ifdef TEST

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *nested(char *s, int n)
{
    char           *t = s;

    puts("Nested");
    while (n-- > 0)
    {
        t = nested(t, n);
        puts(s);
    }
    t = vstrcpy(t, 2, "\ngoogleplex", " widget");
    return(t);
}

/* Measurement shows that 199 bytes are necessary */
int main(void)
{
    char           *buff;
    char           *cp;

    if ((buff = malloc(200)) == 0)
    {
        puts("memory allocation failed (dammit)");
        exit(1);
    }

    cp = vstrcpy(buff, 5, "elephants", " ", "get arthritis", " ", "too easily!");
    puts(buff);
    cp = vstrcpy(cp, 1, "\nelegance incarnate");
    puts(buff);
    cp = vstrcpy(cp, 0);
    puts(buff);
    cp = nested(cp, 3);
    puts(buff);
    printf("Length: %ld\n", (long)strlen(buff));
    free(buff);
    return(0);
}

#endif /* TEST */
