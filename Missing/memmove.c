/*
@(#)File:           $RCSfile: memmove.c,v $
@(#)Version:        $Revision: 1.8 $
@(#)Last changed:   $Date: 2011/11/28 04:39:39 $
@(#)Purpose:        Simulate MEMMOVE(3)
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 1991,1997,2005,2008,2011
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

/*TABSTOP=4*/

#include <string.h>

#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
const char jlss_id_memmove_c[] = "@(#)$Id: memmove.c,v 1.8 2011/11/28 04:39:39 jleffler Exp $";
#endif /* lint */

void *(memmove)(void *s1, const void *s2, size_t n)
{
    const char     *t;
    char           *t1 = (char *)s1;
    const char     *t2 = (const char *)s2;
    void           *s = s1;

    if (t1 < t2)
    {                           /* Copy forwards */
        t = t1 + n;
        while (t1 < t)
            *t1++ = *t2++;
    }
    else
    {                           /* Copy backwards */
        t = t2;
        t1 += n;
        t2 += n;
        while (t2 > t)
            *--t1 = *--t2;
    }
    return(s);
}

#ifdef TEST

#include <stdio.h>
#undef memmove

int main(void)
{
    char            buffer[80];
    int             ok = 0;

    strcpy(&buffer[0], "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    printf("buffer = %s\n", buffer);
    printf("memmove(&buffer[0], &buffer[9], 10);\n");
    memmove(&buffer[0], &buffer[9], 10);
    printf("buffer = %s\n", buffer);
    if (strcmp(buffer, "JKLMNOPQRSKLMNOPQRSTUVWXYZ") != 0)
        ok++, printf("** FAILED **\n");
    printf("memmove(&buffer[5], &buffer[0], 10);\n");
    memmove(&buffer[5], &buffer[0], 10);
    printf("buffer = %s\n", buffer);
    if (strcmp(buffer, "JKLMNJKLMNOPQRSPQRSTUVWXYZ") != 0)
        ok++, printf("** FAILED **\n");

    if (ok == 0)
        printf("OK\n");
    return(0);
}

#endif /* TEST */
