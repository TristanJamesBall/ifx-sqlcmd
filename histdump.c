/*
@(#)File:           $RCSfile: histdump.c,v $
@(#)Version:        $Revision: 2015.2 $
@(#)Last changed:   $Date: 2015/11/06 08:04:58 $
@(#)Purpose:        Print the contents of an SQLCMD history log file
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 1992-93,1995,1997-98,2000-01,2003,2005,2015
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

/*TABSTOP=4*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#define MAIN_PROGRAM
#define USE_JLSS_GETOPT

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "getopt.h"
#include "history.h"
#include "stderr.h"
#include "jlss.h"

#define NIL(x)  ((x)0)
static const char usestr[] = "[-Vhns] [-l lower] [-u upper] [histfile ...]";
static const char optstr[] = "Vhnsl:u:";
static const char hlpstr[] =
    "  -V        Print version information and exit\n"
    "  -h        Print this help information and exit\n"
    "  -l lower  Lower history record number\n"
    "  -n        Exclude numbers in the output\n"
    "  -s        Generate summary information\n"
    "  -u upper  Upper history record number\n"
    ;

#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
extern const char jlss_id_histdump_c[];
const char jlss_id_histdump_c[] = "@(#)$Id: histdump.c,v 2015.2 2015/11/06 08:04:58 jleffler Exp $";
#endif /* lint */

static Sint4 hist_erropen(void)
{
    Sint4            cmdnum;

    if ((cmdnum = hist_open(H_READONLY)) < 0)
    {
        switch (cmdnum)
        {
        case H_CANTOPEN:
            err_error("cannot open history file %s\n", hist_file());
            break;
        case H_BADMAGIC:
            err_error("bad magic number in history file %s\n", hist_file());
            break;
        case H_BADFORMAT:
            err_error("bad format in history file %s\n", hist_file());
            break;
        case H_OLDMAGIC:
            err_error("old format of history file - please remove %s\n", hist_file());
            break;
        }
    }
    return cmdnum;
}

static void print_histrange(void)
{
    Sint4 lower;
    Sint4 upper;

    if (hist_erropen() >= 0)
    {
        hist_range(&lower, &upper);
        printf("%s: %ld .. %ld\n", hist_file(), (long)lower, (long)upper);
        hist_close();
    }
}

static void hist_print(Sint4 lower, Sint4 upper, int nflag)
{
    if (hist_erropen() >= 0)
    {
        hist_output(stdout, lower, upper, nflag);
        hist_close();
    }
}

int main(int argc, char **argv)
{
    int             opt;
    int             nflag = H_NUMBERS;
    Sint4 lower = 1L;
    Sint4 upper = 0L;
    int sflag = 0;

    err_setarg0(argv[0]);

    while ((opt = GETOPT(argc, argv, optstr)) != EOF)
    {
        switch (opt)
        {
        case 'h':
            err_help(usestr, hlpstr);
            /*NOTREACHED*/
        case 'V':
            err_version("HISTDUMP", "$Revision: 2015.2 $ ($Date: 2015/11/06 08:04:58 $)");
            /*NOTREACHED*/
        case 's':
            sflag = 1;
            break;
        case 'l':
            lower = strtol(optarg, 0, 0);
            break;
        case 'u':
            upper = strtol(optarg, 0, 0);
            break;
        case 'n':
            nflag = H_COMMAND;
            break;
        default:
            err_usage(usestr);
            /*NOTREACHED*/
        }
    }

    if (optind == argc)
    {
        if (sflag)
            print_histrange();
        else
            hist_print(lower, upper, nflag);
    }
    else
    {
        while (optind < argc)
        {
            hist_setfile(argv[optind++]);
            if (sflag)
                print_histrange();
            else
                hist_print(lower, upper, nflag);
        }
    }

    return(0);
}
