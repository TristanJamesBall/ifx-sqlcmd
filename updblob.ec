/*
@(#)File:           $RCSfile: updblob.ec,v $
@(#)Version:        $Revision: 2015.1 $
@(#)Last changed:   $Date: 2015/07/10 15:54:49 $
@(#)Purpose:        Update a blob from file
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 1992,1997-98,2000-01,2003-05,2007-08,2015
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

/*TABSTOP=4*/

#define USE_JLSS_GETOPT

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include "posixver.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "esqlc.h"
#include "stderr.h"
#include "getopt.h"
#include "connblob.h"
#include "debug.h"
#include "esqltype.h"
#include "esnprintf.h"

#define BUFFSIZE    65536
#define NIL(x)      ((x)0)
#define DIM(x)      (sizeof(x)/sizeof(*(x)))

static const char usestr[] =
    "[-Vhnx] [-Z debug] -d dbase [-u username] [-p password] -t table -c blobcolumn \\\n"
    "               -k column=value [-k col=val ...] [-f] blobfile\n"
    "NB: -h gives more help!";

static const char fullhelp[] =
    "\nSummary of options:\n"
    "   -c blobcolumn   - name of blob column to be updated\n"
    "   -d dbase        - name of database\n"
    "  [-f] blobfile    - name of file containing blob value\n"
    "   -h              - print this help information and exit\n"
    "   -k column=value - key column and value (to identify row(s) to update)\n"
    "   -n              - no transaction\n"
    "   -p password     - specify password (beware security implications!)\n"
    "   -t table        - name of table to update (may include owner)\n"
    "   -u username     - specify username\n"
    "   -x              - trace SQL actions\n"
    "   -V              - print version information and exit\n"
    "   -Z debug        - specify debug level (if compiled with debug enabled)\n"
    ;

static const char stmt_too_long[] = "Your statement is too long for IDS!\n";

#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
extern const char jlss_id_updblob_ec[];
const char jlss_id_updblob_ec[] = "@(#)$Id: updblob.ec,v 2015.1 2015/07/10 15:54:49 jleffler Exp $";
#endif /* lint */

static void locate_file(loc_t *blob, int type, char *file)
{
    blob->loc_indicator = 0;
    blob->loc_type = type;
    blob->loc_loctype = LOCFNAME;
    blob->loc_fname = file;
    blob->loc_oflags = LOC_RONLY;
    blob->loc_size = -1;
}

/*
** Return value ready to be enclosed in single quotes by doubling
** any embedded single quotes.
*/
static char *process_quote(char *val)
{
    char    *src = val;
    char    *dst;
    char    *rv;
    char     c;
    int      qcount = 0;

    /* Count how many single quotes there are in the string */
    while ((src = strchr(src, '\'')) != 0)
    {
        qcount++;
        src++;
    }
    if (qcount == 0)
        return(val);

    /**
    ** Allocate enough space for the string with each embedded
    ** single quote replaced by two single quotes.
    */
    rv = (char *)malloc(strlen(val) + qcount + 1);
    if (rv == 0)
        err_error("out of memory\n");

    /* Copy string */
    dst = rv;
    src = val;
    while ((c = *dst++ = *src++) != '\0')
    {
        if (c == '\'')
            *dst++ = c;
    }

    return(rv);
}

/* Parse the value of a single column=value option argument */
static void parse_column(char *option, char **keycol, char **keyval)
{
    char    *s;

    if ((s = strchr(option, '=')) == NIL(char *))
        err_error("key column format error (no '=') in %s\n", option);
    *keycol = option;
    *s = '\0';
    option = s + 1;
    if (!*option)
        err_error("key column %s with no key value\n", *keycol);
    *keyval = process_quote(option);
}

int main(int argc, char **argv)
{
    EXEC SQL BEGIN DECLARE SECTION;
    char    *dbase = (char *)0;
    char    *uname = (char *)0;
    char    *upass = (char *)0;
    loc_t   blob;
    char     stmt[BUFFSIZE];
    EXEC SQL END DECLARE SECTION;
    char    *table = (char *)0;
    char    *blobcol = (char *)0;
    size_t   nkeycols = 0;
    char    *keycol[16];
    char    *keyval[16];
    char    *blobfile = (char *)0;
    int      opt;
    const char  *pad;
    char    *s;
    char    *e;
    size_t   i;
    int      nflag = 0;
    int      xflag = 0;
    size_t   len;
    int      loggeddb;
    size_t   errors = 0;

    err_setarg0(argv[0]);
    while ((opt = GETOPT(argc, argv, "Vhnxd:u:p:t:c:k:f:Z:")) != EOF)
    {
        switch (opt)
        {
        case 'h':
            err_help(usestr, fullhelp);
            break;
        case 'n':
            nflag = 1;
            break;
        case 'x':
            xflag = 1;
            break;
        case 'V':
            err_version("UPDBLOB", "$Revision: 2015.1 $ ($Date: 2015/07/10 15:54:49 $)");
            break;
        case 't':
            table = optarg;
            break;
        case 'c':
            blobcol = optarg;
            break;
        case 'd':
            dbase = optarg;
            break;
        case 'u':
            uname = optarg;
            break;
        case 'p':
            upass = optarg;
            break;
        case 'f':
            blobfile = optarg;
            break;
        case 'k':
            if (nkeycols >= DIM(keyval))
                err_error("too many key columns specified in %s\n", optarg);
            parse_column(optarg, &keycol[nkeycols], &keyval[nkeycols]);
            nkeycols++;
            break;
        case 'Z':
            db_setdebug(atoi(optarg));
            break;
        default:
            err_usage(usestr);
            break;
        }
    }

    if (blobfile == (char *)0 && optind == argc - 1)
        blobfile = argv[optind++];

    if (blobfile == NIL(char *))
    {
        err_remark("No blob file specified\n");
        errors++;
    }
    if (dbase == NIL(char *))
    {
        err_remark("No database specified\n");
        errors++;
    }
    if (table == NIL(char *))
    {
        err_remark("No table specified\n");
        errors++;
    }
    if (nkeycols == 0)
    {
        err_remark("No key columns specified\n");
        errors++;
    }
    if (blobcol == NIL(char *))
    {
        err_remark("No blob column specified\n");
        errors++;
    }
    if (optind != argc)
    {
        err_remark("Extra file arguments specified\n");
        errors++;
    }
    if (errors > 0)
        err_usage(usestr);

    locate_file(&blob, SQLBYTES, blobfile);
    if (xflag)
        printf("+ DATABASE %s\n", dbase);

    loggeddb = blob_connect(xflag, dbase, uname, upass);

    if (loggeddb && nflag == 0)
    {
        if (xflag)
            printf("+ BEGIN WORK\n");
        EXEC SQL BEGIN WORK;
        if (sqlca.sqlcode != 0)
            sql_error("begin", "work");
    }

    esnprintf(stmt, sizeof(stmt), "UPDATE %s SET %s = ? WHERE", table, blobcol);
    pad = "";
    e = stmt + sizeof(stmt);
    s = stmt + strlen(stmt);
    for (i = 0; i < nkeycols; i++)
    {
        size_t l = e - s;
        esnprintf(s, l, "%s %s = '%s'", pad, keycol[i], keyval[i]);
        pad = " AND";
        s += strlen(s);
    }

    if (xflag)
    {
        len = strlen(stmt);
        for (i = 0; i < len; i += 75)
            printf("+ %.75s\n", &stmt[i]);
    }

    EXEC SQL PREPARE p_update FROM :stmt;
    if (sqlca.sqlcode != 0)
        sql_error("prepare update", table);
    else if (nflag == 0)
    {
        EXEC SQL EXECUTE p_update USING :blob;
        if (sqlca.sqlcode != 0)
            sql_error("execute update", table);
        if (sqlca.sqlerrd[2] != 1)
            err_remark("warning: %" PRId_ixInt4 " rows updated\n", sqlca.sqlerrd[2]);
        if (xflag)
            printf("+ %" PRId_ixInt4 " rows updated\n", sqlca.sqlerrd[2]);
    }

    if (loggeddb && nflag == 0)
    {
        if (xflag)
            printf("+ COMMIT WORK\n");
        EXEC SQL COMMIT WORK;
        if (sqlca.sqlcode != 0)
            sql_error("commit work", table);
    }

    return(0);
}
