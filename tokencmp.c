/*
@(#)File:           $RCSfile: tokencmp.c,v $
@(#)Version:        $Revision: 1.5 $
@(#)Last changed:   $Date: 2008/02/11 08:44:50 $
@(#)Purpose:        Compare two tokens (case insensitive)
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 2000,2004-05,2008
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

/*TABSTOP=4*/

#include "tokencmp.h"
#include <assert.h>
#include <stddef.h>
#include <ctype.h>

typedef unsigned char Uchar;

#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
const char jlss_id_tokencmp_c[] = "@(#)$Id: tokencmp.c,v 1.5 2008/02/11 08:44:50 jleffler Exp $";
#endif /* lint */

/*
**  Token comparison -- case-insensitive
*/
int tokencmp(const char *str, size_t len, const char *token, size_t toklen)
{
    char     cs;
    char     ct;
    size_t   ncmp = len;

    if (toklen < len)
        ncmp = toklen;

    assert(ncmp > 0);
    while (ncmp > 0)
    {
        cs = tolower((Uchar)*str++);
        ct = tolower((Uchar)*token++);
        if (cs != ct)
            return(cs - ct);
        ncmp--;
    }
    assert(ncmp == 0);
    if (len == toklen)
    {
        return 0;
    }
    else
    {
        /* One string is a prefix of the other */
        /* Conceptually, append NUL to end of shorter string */
        if (toklen < len)
            return(1);
        else
            return(-1);
    }
}

#ifdef TEST

#include <stdio.h>
#include <string.h>

#define DIM(x)  (sizeof(x)/sizeof(*(x)))

static char *array[] = {
    "aaa", "AAA", "aaa", "Bbb", "BbB", "aAa", "aa", "aac", "bbbb", "bbb"
};

int main(void)
{
    int i;

    /*
    ** Using strlen() in calling sequence is not particularly efficient,
    ** but this is just a functional test and not a performance test.
    */
    for (i = 0; i < DIM(array) - 1; i++)
    {
        printf("comparing '%s' and '%s' yields %d\n", array[i], array[i+1],
                tokencmp(array[i], strlen(array[i]), array[i+1],
                strlen(array[i+1])));
    }
    return(0);
}
#endif /* TEST */
