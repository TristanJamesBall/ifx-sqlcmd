/*
@(#)File:           $RCSfile: strcstrlit.c,v $
@(#)Version:        $Revision: 1.6 $
@(#)Last changed:   $Date: 2016/07/08 01:17:53 $
@(#)Purpose:        Convert character string to corresponding C Literal String
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 2001,2005,2007,2015-16
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

/*TABSTOP=4*/

#include "cchrstrlit.h"
#include <string.h>

#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
extern const char jlss_id_strcstrlit_c[];
const char jlss_id_strcstrlit_c[] = "@(#)$Id: strcstrlit.c,v 1.6 2016/07/08 01:17:53 jleffler Exp $";
#endif /* lint */

void str_cstrlit(const char *str, char *buffer, size_t buflen)
{
    unsigned char u;
    size_t len;

    while ((u = (unsigned char)*str++) != '\0')
    {
        chr_cstrlit(u, buffer, buflen);
        if ((len = strlen(buffer)) == 0)
            return;
        buffer += len;
        buflen -= len;
    }
}

#if defined(TEST)

#include <stdio.h>
#include <stdlib.h>

typedef struct Test
{
    const char *str;
    const char *val;
} Test;

static Test test[] =
{
    {   "a b c", "a b c" },
    {   "\f\v\t\r\"\'\n\a\377\200\001\1\x14\xF?",
        "\\f\\v\\t\\r\\\"\\'\\n\\a\\377\\200\\001\\001\\024\\017\\?"    }
};

#define DIM(x) (sizeof(x)/sizeof(*(x)))

int main(void)
{
    int rc = EXIT_SUCCESS;
    size_t i;
    char buffer[128];

    for (i = 0; i < DIM(test); i++)
    {
        str_cstrlit(test[i].str, buffer, sizeof(buffer));
        if (*buffer !=  '\0' && strcmp(test[i].val, buffer) == 0)
            printf("== PASS == output = <<%s>>\n", buffer);
        else
        {
            printf("** FAIL ** input = <<%s>> got output = <<%s>> expecting <<%s>>\n",
                    test[i].str, buffer, test[i].val);
            rc = EXIT_FAILURE;
        }
    }
    puts((rc == EXIT_SUCCESS) ? "== PASS ==" : "** FAIL **");
    return(rc);
}

#endif /* TEST */
