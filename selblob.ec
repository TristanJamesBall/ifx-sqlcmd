/*
@(#)File:           $RCSfile: selblob.ec,v $
@(#)Version:        $Revision: 2013.3 $
@(#)Last changed:   $Date: 2013/12/29 00:47:29 $
@(#)Purpose:        Select a blob into a file
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 2001,2003-05,2008,2013
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

/*TABSTOP=4*/

#define USE_JLSS_GETOPT

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "esqlc.h"
#include "stderr.h"
#include "getopt.h"
#include "connblob.h"
#include "debug.h"
#include "esnprintf.h"

#define BUFFSIZE    65536
#define DIM(x)      (sizeof(x)/sizeof(*(x)))

/* Debug (-Z debug) is always accepted; it is not always effective */
static char const optstr[] = "hnxVZ:d:u:p:t:c:k:f:";
static char const usestr[] =
    "[-hnxV] "
#ifdef DEBUG
    "[-Z debug] "
#endif
    "-d dbase [-u username -p password] -t table -c blobcolumn \\\n"
    "               -k column=value [-k col=val ...] [-f] blobfile";
static char const hlpstr[] =
    "  -V            Print version information and exit\n"
#ifdef DEBUG
    "  -Z debug      Activate debug printing at given level\n"
#endif
    "  -c column     Select blob from named column\n"
    "  -d database   Select from named database\n"
    "  -f blobfile   Name of file to hold output blob\n"
    "  -h            Print this help message and exit\n"
    "  -k col=value  Primary key to identify row (may be repeated for multi-column keys)\n"
    "  -n            No execute mode (validate, but do not fetch)\n"
    "  -p password   Password for connection (BEWARE: security implications)\n"
    "  -t table      Select from named table\n"
    "  -u username   Username for connection (BEWARE: security implications)\n"
    "  -x            Enable debugging information\n"
    ;

#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
const char jlss_id_selblob_ec[] = "@(#)$Id: selblob.ec,v 2013.3 2013/12/29 00:47:29 jleffler Exp $";
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

    if ((s = strchr(option, '=')) == 0)
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
    char    *blobfile = (char *)0;
    int      opt;
    size_t   nkeycols = 0;
    char    *keycol[16];
    char    *keyval[16];
    size_t   i;
    int      xflag = 0;
    size_t   len;
    size_t   cnt = 0;
    size_t   errors = 0;

    err_setarg0(argv[0]);
    while ((opt = GETOPT(argc, argv, optstr)) != EOF)
    {
        switch (opt)
        {
        case 'h':
            err_help(usestr, hlpstr);
            break;
        case 'x':
            xflag = 1;
            break;
        case 'V':
            err_version("SELBLOB", "$Revision: 2013.3 $ ($Date: 2013/12/29 00:47:29 $)");
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

    if (blobfile == 0)
    {
        err_remark("No blob file specified\n");
        errors++;
    }
    if (dbase == 0)
    {
        err_remark("No database specified\n");
        errors++;
    }
    if (table == 0)
    {
        err_remark("No table specified\n");
        errors++;
    }
    if (blobcol == 0)
    {
        err_remark("No blob column specified\n");
        errors++;
    }
    if (nkeycols == 0)
    {
        err_remark("No key columns specified\n");
        errors++;
    }
    if (optind != argc)
    {
        err_remark("Extra file arguments specified\n");
        errors++;
    }
    if (errors > 0)
        err_usage(usestr);

    /* For SELECT, it doesn't matter whether it is a logged DB or not */
    blob_connect(xflag, dbase, uname, upass);

    locate_file(&blob, SQLBYTES, blobfile);

    len = esnprintf(stmt, sizeof(stmt), "SELECT %s FROM %s WHERE", blobcol, table);
    build_where(nkeycols, keycol, keyval, stmt + len, sizeof(stmt) - 1 - len);

    if (xflag)
    {
        len = strlen(stmt);
        for (i = 0; i < len; i += 75)
            printf("+ %.75s\n", &stmt[i]);
    }

    EXEC SQL PREPARE p_select FROM :stmt;
    if (sqlca.sqlcode != 0)
        sql_error("prepare update", table);
    EXEC SQL DECLARE c_select CURSOR FOR p_select;
    if (sqlca.sqlcode != 0)
        sql_error("declare cursor", table);
    EXEC SQL OPEN c_select;
    if (sqlca.sqlcode != 0)
        sql_error("open cursor", table);
    while (sqlca.sqlcode == 0)
    {
        EXEC SQL FETCH c_select INTO :blob;
        if (sqlca.sqlcode != 0)
            break;
        cnt++;
    }
    if (sqlca.sqlcode < 0)
        sql_error("fetch cursor", table);
    if (cnt != 1)
        err_remark("** Warning: %lu rows selected\n", (unsigned long)cnt);
    EXEC SQL CLOSE c_select;
    if (sqlca.sqlcode != 0)
        sql_error("close cursor", table);
    EXEC SQL FREE c_select;
    if (sqlca.sqlcode != 0)
        sql_error("free cursor", table);
    EXEC SQL FREE p_select;
    if (sqlca.sqlcode != 0)
        sql_error("free statement", table);

    return(0);
}
