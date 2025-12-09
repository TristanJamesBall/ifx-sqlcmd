/*
@(#)File:           $RCSfile: appblob.ec,v $
@(#)Version:        $Revision: 2013.1 $
@(#)Last changed:   $Date: 2013/12/29 00:46:56 $
@(#)Purpose:        Update a blob by appending data from file
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 2001-05,2008,2013
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

/*TABSTOP=4*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#define USE_JLSS_GETOPT

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */
#include "esqlc.h"
#include "stderr.h"
#include "getopt.h"
#include "jlss.h"
#include "jlsstools.h"
#include "esnprintf.h"
#include "connblob.h"
#include "debug.h"
#include "esqltype.h"

#define BUFFSIZE    2048
#define NIL(x)      ((x)0)
#define DIM(x)      (sizeof(x)/sizeof(*(x)))

static char const optstr[] = "Vhnxd:u:p:t:c:k:f:Z:";
static char const usestr[] =
    "[-Vnx] "
#ifdef DEBUG
    "[-Z debug] "
#endif
    "-d dbase [-u username -p password] -t table -c blobcolumn -k "
    "column=value [-k col=val ...] [-f] blobfile";
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

static char tempfile[256];

#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
const char jlss_id_appblob_ec[] = "@(#)$Id: appblob.ec,v 2013.1 2013/12/29 00:46:56 jleffler Exp $";
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

/* ATEXIT handler to remove temporary file */
static void unlink_tempfile(void)
{
    if (unlink(tempfile) != 0)
    {
        if (errno != ENOENT)
            err_sysrem("failed to unlink file %s\n", tempfile);
    }
}

/* Open file successfully or exit with error */
static FILE *err_fopen(const char *file, const char *mode)
{
    FILE *fp = fopen(file, mode);
    if (fp == 0)
        err_syserr("failed to open file %s (mode %s)\n", file, mode);
    return(fp);
}

int main(int argc, char **argv)
{
    EXEC SQL BEGIN DECLARE SECTION;
    char    *dbase = (char *)0;
    char    *uname = (char *)0;
    char    *upass = (char *)0;
    loc_t    blob;
    char     updt[BUFFSIZE];
    char     slct[BUFFSIZE];
    EXEC SQL END DECLARE SECTION;
    char     where[BUFFSIZE];
    char    *table = (char *)0;
    char    *blobcol = (char *)0;
    size_t   nkeycols = 0;
    char    *keycol[16];
    char    *keyval[16];
    char    *blobfile = (char *)0;
    int      opt;
    size_t   i;
    int      nflag = 0;
    int      xflag = 0;
    size_t   len;
    int      loggeddb;

    err_setarg0(argv[0]);
    while ((opt = GETOPT(argc, argv, optstr)) != -1)
    {
        switch (opt)
        {
        case 'h':
            err_help(usestr, hlpstr);
            break;
        case 'n':
            nflag = 1;
            break;
        case 'x':
            xflag = 1;
            break;
        case 'V':
            err_version("UPDBLOB", "$Revision: 2013.1 $ ($Date: 2013/12/29 00:46:56 $)");
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
                err_error("too many key columns specified (%s)\n", optarg);
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

    if (blobfile == NIL(char *) && optind == argc - 1)
        blobfile = argv[optind++];
    if (blobfile == NIL(char *))
        err_remark("No blob file specified\n");
    if (dbase == NIL(char *))
        err_remark("No database specified\n");
    if (table == NIL(char *))
        err_remark("No table specified\n");
    if (nkeycols == 0)
        err_remark("No key columns specified\n");
    if (blobcol == NIL(char *))
        err_remark("No blob column specified\n");
    if (optind != argc)
        err_remark("Extra file arguments specified\n");
    if (optind != argc || blobfile == NIL(char *) || dbase == NIL(char *) ||
        table == NIL(char *) || nkeycols == 0 || blobcol == NIL(char *))
        err_usage(usestr);

    /* --KLUDGE-- Specify temporary file name for blob */
    esnprintf(tempfile, sizeof(tempfile), "/tmp/appblob.%d", (int)getpid());
    locate_file(&blob, SQLBYTES, tempfile);
    atexit(unlink_tempfile);

    loggeddb = blob_connect(xflag, dbase, uname, upass);

    if (loggeddb)
    {
        if (xflag)
            printf("+ BEGIN WORK\n");
        if (nflag == 0)
        {
            EXEC SQL BEGIN WORK;
            if (sqlca.sqlcode != 0)
                sql_error("begin", "work");
        }
    }

    /* Build WHERE clause */
    build_where(nkeycols, keycol, keyval, where, sizeof(where));
    esnprintf(slct, sizeof(slct), "SELECT %s FROM %s WHERE%s FOR UPDATE", blobcol, table, where);
    esnprintf(updt, sizeof(updt), "UPDATE %s SET %s = ? WHERE CURRENT OF c_select", table, blobcol);

    if (xflag)
    {
        len = strlen(slct);
        for (i = 0; i < len; i += 75)
            printf("+ %.75s\n", &slct[i]);
    }

    if (xflag)
    {
        len = strlen(updt);
        for (i = 0; i < len; i += 75)
            printf("+ %.75s\n", &updt[i]);
    }

    EXEC SQL PREPARE p_select FROM :slct;
    if (sqlca.sqlcode != 0)
        sql_error("prepare select", table);
    else if (nflag == 0)
    {
        EXEC SQL DECLARE c_select CURSOR FOR p_select;
        if (sqlca.sqlcode != 0)
            sql_error("declare cursor", table);
        else
        {
            EXEC SQL OPEN c_select;
            if (sqlca.sqlcode != 0)
                sql_error("open cursor", table);
            else
            {
                EXEC SQL FETCH c_select INTO :blob;
                if (sqlca.sqlcode == SQLNOTFOUND)
                    err_error("no row found by %s\n", slct);
                else if (sqlca.sqlcode != 0)
                    sql_error("fetch cursor", table);
                else
                {
                    /* Concatenate new file to blob file */
                    FILE *tfp = err_fopen(tempfile, "ab");
                    FILE *bfp = err_fopen(blobfile, "rb");
                    fcopy(bfp, tfp);
                    if (fclose(bfp) != 0)
                        err_error("failed to close (read) file %s\n", blobfile);
                    if (fclose(tfp) != 0)
                        err_error("failed to close (written) file %s\n", tempfile);
                    blob.loc_indicator = 0; /* Assume non-null */
                    blob.loc_size = -1; /* All of file */

                    EXEC SQL PREPARE p_update FROM :updt;
                    if (sqlca.sqlcode != 0)
                        sql_error("prepare update", table);
                    else if (nflag == 0)
                    {
                        EXEC SQL EXECUTE p_update USING :blob;
                        if (sqlca.sqlcode != 0)
                            sql_error("execute update", table);
                        if (sqlca.sqlerrd[2] != 1)
                            err_remark("warning: %" PRId_ixInt4 " rows updated\n",
                                       sqlca.sqlerrd[2]);
                        if (xflag)
                            printf("+ %" PRId_ixInt4 " rows updated\n", sqlca.sqlerrd[2]);
                    }
                }
            }
        }
    }

    if (loggeddb)
    {
        if (xflag)
            printf("+ COMMIT WORK\n");
        if (nflag == 0)
        {
            EXEC SQL COMMIT WORK;
            if (sqlca.sqlcode != 0)
                sql_error("commit work", table);
        }
    }

    return(0);
}
