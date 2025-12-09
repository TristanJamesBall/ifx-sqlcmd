/*
@(#)File:           $RCSfile: mkproc.ec,v $
@(#)Version:        $Revision: 2015.2 $
@(#)Last Changed:   $Date: 2015/07/10 16:05:12 $
@(#)Purpose:        Create stored procedures from named files
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 1992-93,1998,2000,2003,2005,2008,2015
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
@(#)Licence:        GNU General Public Licence Version 2
*/

#define USE_JLSS_GETOPT

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#include <sqlca.h>
#include "stderr.h"
#include "esqltype.h"
#include "getopt.h"

static void mkproc(char *file, int tflag);
static void sql_error(const char *stmt, const char *obj);

static const char usestr[] = "[-stV] -d dbase procfile [...]";

#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
extern const char jlss_id_mkproc_ec[];
const char jlss_id_mkproc_ec[] = "@(#)$Id: mkproc.ec,v 2015.2 2015/07/10 16:05:12 jleffler Exp $";
#endif /* lint */

int     main(int argc, char **argv)
{
    int     i;
    int     dflag = 0;          /* Database selected */
    int     sflag = 1;          /* Echo file names? */
    int     tflag = 1;          /* Do transactions? */
    EXEC SQL BEGIN DECLARE SECTION;
    char   *dbase;
    EXEC SQL END DECLARE SECTION;

    err_setarg0(argv[0]);
    while ((i = GETOPT(argc, argv, "d:stV")) != EOF)
    {
        switch (i)
        {
        case 'd':
            dbase = optarg;
            dflag = 1;
            EXEC SQL DATABASE :dbase;
            sql_error("database", dbase);
            break;
        case 's':
            sflag = 0;
            break;
        case 't':
            tflag = 0;
            break;
        case 'V':
            err_version("MKPROC", &"@(#)$Revision: 2015.2 $ ($Date: 2015/07/10 16:05:12 $)"[4]);
            break;
        default:
            err_usage(usestr);
            break;
        }
    }
    if (optind == argc || dflag == 0)
        err_usage(usestr);

    /* Don't echo file names if there is only one file */
    if (optind == argc - 1)
        sflag = 0;

    /* Transactions? */
    if (sqlca.sqlwarn.sqlwarn1 != 'W' && tflag == 1)
        tflag = 0;

    for (i = optind; i < argc; i++)
    {
        if (sflag)
            puts(argv[i]);
        mkproc(argv[i], tflag);
    }

    return(0);
}

/* Report complete error when error is reported. */
/*
** JL 2003-03-31: Do not treat SQLNOTFOUND as an error per c.d.i
** discussion "Batch updates with ESQL" started by Roger Aunebakk
** <aunebakk@hotmail.com> on 2003-03-26.
*/
static void sql_error(const char *stmt, const char *obj)
{
    char    buffer[512];

    if (sqlca.sqlcode != 0 && sqlca.sqlcode != SQLNOTFOUND)
    {
        fprintf(stderr, "%s: %s %s\n", err_getarg0(), stmt, obj);
        rgetmsg(sqlca.sqlcode, buffer, sizeof(buffer));
        fprintf(stderr, "SQL %" PRId_ixInt4 ": ", sqlca.sqlcode);
        fprintf(stderr, buffer, sqlca.sqlerrm);
        if (sqlca.sqlerrd[1] != 0)
        {
            rgetmsg(sqlca.sqlerrd[1], buffer, sizeof(buffer));
            fprintf(stderr, "ISAM %" PRId_ixInt4 ": ", sqlca.sqlerrd[1]);
            fprintf(stderr, buffer, sqlca.sqlerrm);
        }
        exit(1);
    }
}

static void mkproc(char *file, int tflag)
{
    EXEC SQL BEGIN DECLARE SECTION;
    char   *procfile = file;
    EXEC SQL END DECLARE SECTION;

    if (tflag)
    {
        EXEC SQL BEGIN WORK;
        sql_error("begin", "work");
    }

    EXEC SQL CREATE PROCEDURE FROM :procfile;
    sql_error("create procedure from", procfile);

    if (tflag)
    {
        EXEC SQL COMMIT WORK;
        sql_error("commit", "work");
    }
}
