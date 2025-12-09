/*
@(#)File:           $RCSfile: cistrcmp.c,v $
@(#)Version:        $Revision: 1.9 $
@(#)Last changed:   $Date: 2008/02/11 07:28:06 $
@(#)Purpose:        Case-insensitive string comparison
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 1991,1997,2001,2005,2008
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

/*TABSTOP=4*/

#include "jlss.h"
#include <ctype.h>

#undef tolower      /* Function required */

#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
const char jlss_id_cistrcmp_c[] = "@(#)$Id: cistrcmp.c,v 1.9 2008/02/11 07:28:06 jleffler Exp $";
#endif /* lint */

/*
** NB: tolower macro is not used because on Xenix, tolower is:
**     #define  tolower(c)  (isupper(c) ? _tolower(c) : (c))
**     which is guaranteed to evaluate its argument at least twice, which
**     breaks the code below magnificently.
**     It is assumed that the standard library provides a real function
**     tolower -- if your tolower in <ctype.h> is like a function and
**     evaluates its argument just once, then use the macro tolower in
**     preference to the function.
*/

/*
**  String comparison -- case-insensitive
*/
int cistrcmp(const char *str1, const char *str2)
{
    char     cs;
    char     ct;

    while ((cs = tolower(*str1++)) == (ct = tolower(*str2++)))
        if (cs == '\0' || ct == '\0')
            break;
    return(cs - ct);
}

#ifdef TEST

#include <stdio.h>

#define DIM(x)  (sizeof(x)/sizeof(*(x)))

static char *array[] = {
    "aaa", "AAA", "aaa", "Bbb", "BbB", "aAa", "aa", "bbbb"
};

int main(void)
{
    int i;

    for (i = 0; i < DIM(array) - 1; i++)
    {
        printf("comparing '%s' and '%s' yields %d\n", array[i], array[i+1],
                cistrcmp(array[i], array[i+1]));
    }
    return(0);
}
#endif /* TEST */
