/*
@(#)File:           $RCSfile: getsubopt.c,v $
@(#)Version:        $Revision: 2015.2 $
@(#)Last changed:   $Date: 2015/09/29 07:13:58 $
@(#)Purpose:        Implement GETSUBOPT(3) from SVR4: parse sub-options
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 1992,1996-2000,2003,2005,2008,2011,2015
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

/*TABSTOP=4*/

#define USE_JLSS_GETSUBOPT

#include "getopt.h"
#include <string.h>

#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
extern const char jlss_id_getsubopt_c[];
const char jlss_id_getsubopt_c[] = "@(#)$Id: getsubopt.c,v 2015.2 2015/09/29 07:13:58 jleffler Exp $";
#endif /* lint */

int jlss_getsubopt(char **optionp, char * const *tokens, char **valuep)
{
    char *str = *optionp;
    char *cp;
    char * const *tp = tokens;

    if (*str == '\0')
    {
        *valuep = 0;
        return(-1);
    }
    if ((cp = strchr(str, ',')) == 0)
        *optionp = str + strlen(str);
    else
    {
        *cp = '\0';             /* Zap comma */
        *optionp = cp + 1;
    }
    if ((cp = strchr(str, '=')) == 0)
        *valuep = 0;
    else
    {
        *cp = '\0';
        *valuep = cp + 1;
    }
    for (tp = tokens; *tp != 0; tp++)
    {
        if (strcmp(str, *tp) == 0)
            return(tp - tokens);
    }
    /* Not a valid sub-option after all */
    if (*valuep != 0)
        *cp = '=';
    *valuep = str;
    return(-1);
}

#ifdef TEST

#include <stdio.h>

static char * const subopts[] =
{
    "macro",
    "names",
    "rules",
    "build",
    "input",
    "check",
    "clean",
    0
};

int main(int argc, char **argv)
{
    char *argp;
    char *vp;
    int   n;

    if (argc <= 1)
    {
        char * const *opt;
        fprintf(stderr, "Usage: %s [{suboption}=value[,...]\n", argv[0]);
        fprintf(stderr, "\twhere {suboption} in ");
        vp = "";
        for (opt = subopts; *opt; opt++)
        {
            fprintf(stderr, "%s%s", vp, *opt);
            vp = ", ";
        }
        fputc('\n', stderr);
        exit(1);
    }

    while ((argp = *++argv) != 0)
    {
        while (*argp != '\0')
        {
            n = GETSUBOPT(&argp, subopts, &vp);
            if (n < 0)
                printf("getsubopt failed %d on %s\n", n, vp);
            else if (vp == 0)
                printf("getsubopt OK %d (%s) with no value\n", n, subopts[n]);
            else
                printf("getsubopt OK %d (%s) value %s\n", n, subopts[n], vp);
            fflush(stdout);
        }
    }
    return(0);
}

#endif /* TEST */
