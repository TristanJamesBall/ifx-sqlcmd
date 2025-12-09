/*
@(#)File:           $RCSfile: nvstrcpy.c,v $
@(#)Version:        $Revision: 2.4 $
@(#)Last changed:   $Date: 2008/02/11 07:28:06 $
@(#)Purpose:        Safely copy/concatenate variable number of strings
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 1988-89,1991,1995,1997,2002,2005,2008
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

/*TABSTOP=4*/

/*
**  nvstrcpy(dst, sz, n, src1, src2, ...);
**
**  Nvstrcpy copies each of its arguments src1, src2, ... into dst
**  and null terminates the string - ensuring that it does not use more
**  than sz bytes.  If given the value n = 0, it simply copies a null
**  into dst[0].
**  Compare with vstrcpy which assumes that there is enough space in dst
**  to contain the strings src1, src2, ... concatenated together.
**  Nvstrcpy returns a pointer to the terminal null (unless there's an
**  overflow, in which case it returns NULL).
**  Derived in 2002 from vstrcpy(), which has a long history.
*/

#include "jlss.h"
#include <string.h>
#include <stdarg.h>
#include <assert.h>

#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
const char jlss_id_nvstrcpy_c[] = "@(#)$Id: nvstrcpy.c,v 2.4 2008/02/11 07:28:06 jleffler Exp $";
#endif /* lint */

char *nvstrcpy(char *tgt, size_t sz, int n, ...)
{
    const char     *src;
    char *dst = tgt;
    int             i;
    va_list         args;
    char *end = dst + sz - 1;
    char *rv;

    va_start(args, n);

    for (i = 0; i < n; i++)
    {
        src = va_arg(args, const char *);
        while ((*dst = *src++) != '\0' && dst < end)
            dst++;
    }
    rv = (dst >= end && *dst != '\0') ? 0 : dst;
    *dst = '\0';
    va_end(args);
    assert(dst <= end);
    assert(strlen(tgt) + 1 <= sz);
    return(rv);
}

#ifdef TEST

#include <stdio.h>

static void putstr(const char *str)
{
    static int counter = 0;
    printf("%.2d: %p - %s\n", ++counter, str, str);
    fflush(stdout);
}

static char *nested(char *s, size_t sz, int n)
{
    char           *t = s;

    puts("Nested");
    fflush(stdout);
    while (n-- > 0)
    {
        t = nested(t, sz - (t - s), n);
        putstr(s);
        if (t == 0)
            break;
    }
    if (t != 0)
        t = nvstrcpy(t, sz - (t - s), 2, "\ngoogleplex", " widget");
    putstr(s);
    return(t);
}

/* Measurement shows that 199 bytes are necessary - strlen() reports 198 */
int main(void)
{
    char           buff[200];
    char           *cp;

    cp = nvstrcpy(buff, sizeof(buff), 5, "elephants", " ", "get arthritis", " ", "too easily!");
    putstr(buff);
    cp = nvstrcpy(cp, sizeof(buff) - (cp - buff), 1, "\nelegance incarnate");
    putstr(buff);
    cp = nvstrcpy(cp, sizeof(buff) - (cp - buff), 0);
    putstr(buff);
    cp = nested(cp, sizeof(buff) - (cp - buff), 3);
    putstr(buff);
    printf("Length: %ld\n", (long)strlen(buff));
    printf("Available: %ld\n", sizeof(buff) - (cp - buff));
    cp = nvstrcpy(cp, sizeof(buff) - (cp - buff), 2, "gobbledegook\n", "heartthrob\n");
    assert(cp == 0);
    printf("Length: %ld\n", (long)strlen(buff));
    putstr(buff);

    {
    char b2[20];
    cp = nvstrcpy(b2, sizeof(b2), 1, "abcdefghijklmnopqrst");
    assert(cp == 0 && strlen(b2) == sizeof(b2) - 1);
    cp = nvstrcpy(b2, sizeof(b2), 1, "abcdefghijklmnopqrs");
    assert(cp == b2 + sizeof(b2) - 1);
    cp = nvstrcpy(b2, sizeof(b2), 2, "abcdefghij", "klmnopqrst");
    assert(cp == 0 && strlen(b2) == sizeof(b2) - 1);
    cp = nvstrcpy(b2, sizeof(b2), 2, "abcdefghij", "klmnopqrs");
    assert(cp == b2 + sizeof(b2) - 1);
    cp = nvstrcpy(b2, sizeof(b2), 3, "abcde", "fghij", "klmnopqrst");
    assert(cp == 0 && strlen(b2) == sizeof(b2) - 1);
    cp = nvstrcpy(b2, sizeof(b2), 3, "abcde", "fghij", "klmnopqrs");
    assert(cp == b2 + sizeof(b2) - 1);
    cp = nvstrcpy(b2, sizeof(b2), 4, "abcde", "fghij", "klmno", "pqrst");
    assert(cp == 0 && strlen(b2) == sizeof(b2) - 1);
    cp = nvstrcpy(b2, sizeof(b2), 4, "abcde", "fghij", "klmno", "pqrs");
    assert(cp == b2 + sizeof(b2) - 1);
    }

    return(0);
}

#endif /* TEST */
