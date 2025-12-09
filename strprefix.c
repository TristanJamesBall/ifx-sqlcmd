/*
@(#)File:           $RCSfile: strprefix.c,v $
@(#)Version:        $Revision: 1.11 $
@(#)Last changed:   $Date: 2015/05/25 06:38:01 $
@(#)Purpose:        Case insensitive test for t is prefix of s
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 1987,1989,1991,1997-98,2005,2008,2015
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

/*TABSTOP=4*/

#include "jlss.h"
#include <ctype.h>

#undef tolower

#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
extern const char jlss_id_strprefix_c[];
const char jlss_id_strprefix_c[] = "@(#)$Id: strprefix.c,v 1.11 2015/05/25 06:38:01 jleffler Exp $";
#endif /* lint */

/* Case insensitive test for t is prefix of s */
/* s - In: Control string    */
/* t - In: Tested string     */
int strprefix(const char *s, const char *t)
{
    char c1;
    char c2 = '\0';

    while ((c1 = (char)tolower(*s++)) && (c2 = (char)tolower(*t++)))
    {
        if (c1 != c2)
            return(0);
    }
    if ((c1 == '\0' && *t == '\0') || (c2 == '\0'))
        return(1);
    return(0);
}

#if defined(TEST)

#include <stdio.h>

static const char *arr[] =
{
    "A string of which the other is a prefix",
    "A string",

    "Any old string",
    "ANY OLD STRING",

    "A string",
    "A different string",

    (char *)0
};

int main(void)
{
    for (const char **s = arr; *s; s += 2)
    {
        printf("\nString 1 = <<%s>>\n", *s);
        printf("String 2 = <<%s>>\n", *(s + 1));
        if (strprefix(*(s + 0), *(s + 1)))
            printf("Order 1/2: strings are the same\n");
        else
            printf("Order 1/2: strings are different\n");
        if (strprefix(*(s + 1), *(s + 0)))
            printf("Order 2/1: strings are the same\n");
        else
            printf("Order 2/1: strings are different\n");
    }
    return 0;
}

#endif /* TEST */
