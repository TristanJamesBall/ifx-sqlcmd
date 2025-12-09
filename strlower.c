/*
@(#)File:           $RCSfile: strlower.c,v $
@(#)Version:        $Revision: 2.6 $
@(#)Last changed:   $Date: 2015/05/25 06:33:48 $
@(#)Purpose:        Convert string to lower case
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 1992,1997-98,2005,2007-08,2015
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

/*TABSTOP=4*/

#include "jlss.h"
#include <ctype.h>

#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
extern const char jlss_id_strlower_c[];
const char jlss_id_strlower_c[] = "@(#)$Id: strlower.c,v 2.6 2015/05/25 06:33:48 jleffler Exp $";
#endif /* lint */

/* Case-convert string to lower case; return pointer to terminal null */
char    *strlower(char *s)
{
    char    *t = s;
    char     c;

    while ((c = *t) != '\0')
        *t++ = (char)tolower(c);
    return(t);
}

#if defined(TEST)

static const char * const list[] =
{
    "ALL UPPER CASE OR NON-ALPHA _0123456789~`!@#$%^&*()-=+[]{}|\\\"':;/?.,<>",
    "all lower case",
    "mIxEd CaSe",
    (char *)0
};

#include <stdio.h>
#include <string.h>
#include <assert.h>

int main(void)
{
    const char * const * s;
    char     *end;
    char     *src;
    char      buffer[128];

    for (s = list; *s != (char *)0; s++)
    {
        strcpy(buffer, *s);
        printf("Before: %s\n", buffer);
        end = strlower(buffer);
        assert(*end == '\0');
        for (src = buffer; src < end; src++)
        {
            char c = *src;
            assert(!isalpha(c) || islower(c));
        }
        printf("After:  %s\n", buffer);
    }
    return(0);
}

#endif /* TEST */
