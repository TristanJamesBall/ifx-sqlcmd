/*
@(#)File:           $RCSfile: insblob.ec,v $
@(#)Version:        $Revision: 2008.4 $
@(#)Last changed:   $Date: 2008/06/08 20:55:24 $
@(#)Purpose:        Insert single row of data with blobs from files
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 2002-05,2008
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

#include "esqlc.h"
#include "stderr.h"
#include "getopt.h"
#include "emalloc.h"
#include "connblob.h"
#include "debug.h"
#include "esnprintf.h"

#define MAXBLOBS    256
#define MAXCOLUMNS  1024
#define STMTSIZE    65536
#define NIL(x)      ((x)0)
#define DIM(x)      (sizeof(x)/sizeof(*(x)))

static const char usestr[] =
    "[-Vnx] [-Z debug] -d dbase [-u username -p password] -t table -b blobcol=file [-b blobcol=file ...] "
    "-c col=val [-c col=val ...] ";

#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
const char jlss_id_insblob_ec[] = "@(#)$Id: insblob.ec,v 2008.4 2008/06/08 20:55:24 jleffler Exp $";
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
** any embedded single quotes.  Not worrying about leaking memory.
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
    ** single quote replaced by two single quotes.  MALLOC does
    ** the error checking!
    */
    rv = (char *)MALLOC(strlen(val) + qcount + 1);  /*=C++=*/

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
        err_error("column format error (no '=') in %s\n", option);
    *keycol = option;
    *s = '\0';
    option = s + 1;
    if (!*option)
        err_error("column %s has no value\n", *keycol);
    *keyval = process_quote(option);
}

int main(int argc, char **argv)
{
    EXEC SQL BEGIN DECLARE SECTION;
    char    *dbase = (char *)0;
    char    *uname = (char *)0;
    char    *upass = (char *)0;
    loc_t    blob[MAXBLOBS];
    char     stmt[STMTSIZE];
    EXEC SQL END DECLARE SECTION;
    char    *table = (char *)0;
    size_t   nblobcols = 0;
    size_t   nregcols = 0;
    char    *regcol[MAXCOLUMNS];
    char    *regval[MAXCOLUMNS];
    char    *blobfile[MAXBLOBS];
    char    *blobcol[MAXBLOBS];
    int      opt;
    const char *pad;
    char    *s;
    size_t   i;
    size_t   z;
    int      nflag = 0;
    int      xflag = 0;
    size_t   len;
    int      loggeddb;
    Sqlda    sql_desc;
    Sqlda   *sql_ptr = &sql_desc;
    Sqlva   *sqlva;
    ixInt2   notnull = 0;
    size_t   errors = 0;

    err_setarg0(argv[0]);

    if (argc <= 1)
        err_usage(usestr);

    while ((opt = GETOPT(argc, argv, "Vnxd:u:p:t:c:b:Z:")) != EOF)
    {
        switch (opt)
        {
        case 'n':
            nflag = 1;
            break;
        case 'x':
            xflag = 1;
            break;
        case 'V':
            err_version("INSBLOB", "$Revision: 2008.4 $ ($Date: 2008/06/08 20:55:24 $)");
            break;
        case 't':
            table = optarg;
            break;
        case 'b':
            if (nblobcols >= DIM(blob))
                err_error("too many blob columns specified (%s)\n", optarg);
            parse_column(optarg, &blobcol[nblobcols], &blobfile[nblobcols]);
            locate_file(&blob[nblobcols], SQLBYTES, blobfile[nblobcols]);
            nblobcols++;
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
        case 'c':
            if (nregcols >= DIM(regval))
                err_error("too many columns specified (%s)\n", optarg);
            parse_column(optarg, &regcol[nregcols], &regval[nregcols]);
            nregcols++;
            break;
        case 'Z':
            db_setdebug(atoi(optarg));
            break;
        default:
            err_usage(usestr);
            break;
        }
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
    if (nblobcols == 0)
    {
        err_remark("No blob columns specified\n");
        errors++;
    }
    if (nregcols == 0)
    {
        err_remark("No regular columns specified\n");
        errors++;
    }
    if (optind != argc)
    {
        err_remark("Extra file arguments specified\n");
        errors++;
    }
    if (errors > 0)
        err_usage(usestr);

    if (xflag)
        printf("+ DATABASE %s\n", dbase);

    loggeddb = blob_connect(xflag, dbase, uname, upass);

    if (loggeddb && xflag)
        printf("+ Logged database -- transaction will be used\n");

    /* Generate SQL statement - regular columns before blob columns */
    esnprintf(stmt, sizeof(stmt), "INSERT INTO %s(", table);
    pad = "";
    s = stmt + strlen(stmt);
    z = sizeof(stmt) - 1 - (s - stmt);
    for (i = 0; i < nregcols; i++)
    {
        esnprintf(s, z, "%s%s", pad, regcol[i]);
        pad = ", ";
        len = strlen(s);
        s += len;
        z -= len;
    }
    for (i = 0; i < nblobcols; i++)
    {
        esnprintf(s, z, "%s%s", pad, blobcol[i]);
        len = strlen(s);
        s += len;
        z -= len;
    }
    pad = ") VALUES(";
    for (i = 0; i < nregcols; i++)
    {
        esnprintf(s, z, "%s'%s'", pad, regval[i]);
        pad = ", ";
        len = strlen(s);
        s += len;
        z -= len;
    }
    for (i = 0; i < nblobcols; i++)
    {
        esnprintf(s, z, "%s?", pad);
        len = strlen(s);
        s += len;
        z -= len;
    }
    esnprintf(s, z, ")");

    /* Build SQLDA descriptor for blob values */
    /* Even empty files are not null blobs */
    sql_desc.sqld = nblobcols;
    sql_desc.sqlvar = (Sqlva *)MALLOC(nblobcols * sizeof(Sqlva));
    for (i = 0; i < nblobcols; i++)
    {
        sqlva = &sql_desc.sqlvar[i];
        sqlva->sqltype = SQLBYTES;         /* variable type        */
        sqlva->sqllen = sizeof(Blob);      /* length in bytes      */
        sqlva->sqldata = (char *)&blob[i]; /* pointer to data      */
        sqlva->sqlind = &notnull;          /* pointer to indicator */
        sqlva->sqlname = blobcol[i];       /* variable name        */
        sqlva->sqlitype = 0;               /* ind variable type    */
        sqlva->sqlilen = 0;                /* ind length in bytes  */
        sqlva->sqlidata = 0;               /* ind data pointer     */
    }
    /* dump_sqlda(stderr, "Generated SQL Descriptor", sql_ptr); */

    if (loggeddb)
    {
        if (xflag)
            printf("+ BEGIN WORK\n");
        if (nflag == 0)
        {
            EXEC SQL BEGIN WORK;
            if (sqlca.sqlcode != 0)
                sql_error("begin work", table);
        }
    }

    /* Print text of generated SQL statement */
    if (xflag)
    {
        len = strlen(stmt);
        for (i = 0; i < len; i += 75)
            printf("+ %.75s\n", &stmt[i]);
    }

    EXEC SQL PREPARE p_insert FROM :stmt;
    if (sqlca.sqlcode != 0)
        sql_error("prepare update", table);
    else if (nflag == 0)
    {
        EXEC SQL EXECUTE p_insert USING DESCRIPTOR sql_ptr;
        if (sqlca.sqlcode != 0)
            sql_error("execute insert", table);
        if (sqlca.sqlerrd[2] != 1)
            err_remark("warning: %" PRId_ixInt4 " rows updated\n", sqlca.sqlerrd[2]);
        if (xflag)
            printf("+ %" PRId_ixInt4 " row(s) inserted\n", sqlca.sqlerrd[2]);
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
