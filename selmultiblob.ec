/*
@(#)File:           $RCSfile: selmultiblob.ec,v $
@(#)Version:        $Revision: 2013.2 $
@(#)Last changed:   $Date: 2013/12/29 01:34:20 $
@(#)Purpose:        Select multiple blobs into files
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 2001-05,2008,2013
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

/*TABSTOP=4*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#define USE_JLSS_GETOPT

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "esqlc.h"
#include "stderr.h"
#include "getopt.h"
#include "ixblob.h"
#include "esqlutil.h"
#include "jlss.h"
#include "connblob.h"
#include "debug.h"
#include "esnprintf.h"

#define BUFFSIZE    65536
#define DIM(x)      (sizeof(x)/sizeof(*(x)))

/* Debug (-Z debug) is always accepted; it is not always effective */
static char const optstr[] = "Vhxd:u:p:t:c:D:w:Z:";
static char const usestr[] =
    "[-hxV] -d dbase [-u username -p password] -t table -c blobcolumn [-c blobcol ...] "
    "[-w whereclause] [-D dir]"
#ifdef DEBUG
    " [-Z debug]"
#endif
    ;
static char const hlpstr[] =
    "  -V           Print the version information and exit\n"
#ifdef DEBUG
    "  -Z debug     Activate debug printing at given level\n"
#endif
    "  -c blobcol   Extract data from the named (BYTE or TEXT) column\n"
    "  -d database  Work with the named database\n"
    "  -h           Print this help and exit\n"
    "  -p password  Password for connection (BEWARE: security implications)\n"
    "  -t table     Work with the named table\n"
    "  -u username  Username for connection (BEWARE: security implications)\n"
    "  -w where     Add specified condition to WHERE clause in select statement\n"
    "  -x           Enable debugging information\n"
    ;

#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
const char jlss_id_selmultiblob_ec[] = "@(#)$Id: selmultiblob.ec,v 2013.2 2013/12/29 01:34:20 jleffler Exp $";
#endif /* lint */

int main(int argc, char **argv)
{
    EXEC SQL BEGIN DECLARE SECTION;
    char    *dbase = (char *)0;
    char    *uname = (char *)0;
    char    *upass = (char *)0;
    loc_t    blobs[16];
    char     stmt[BUFFSIZE];
    EXEC SQL END DECLARE SECTION;
    char    *table = (char *)0;
    size_t   nblobcols = 0;
    char    *blobcol[16];
    char    *where = 0;
    char    *blobdir = 0;
    int      opt;
    const char  *pad;
    char    *s;
    size_t   i;
    size_t   z;
    int      xflag = 0;
    size_t   len;
    unsigned long cnt = 0;
    size_t   errors = 0;
    Sqlda   *blobdesc;

    err_setarg0(argv[0]);
    while ((opt = GETOPT(argc, argv, optstr)) != -1)
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
            err_version("SELMULTIBLOB", "$Revision: 2013.2 $ ($Date: 2013/12/29 01:34:20 $)");
            break;
        case 't':
            table = optarg;
            break;
        case 'c':
            if (nblobcols >= DIM(blobcol))
                err_error("too many blob columns specified (%s)\n", optarg);
            blobcol[nblobcols++] = optarg;
            break;
        case 'd':
            if (dbase != 0)
                err_error("multiple databases specified (%s)\n", optarg);
            dbase = optarg;
            break;
        case 'u':
            uname = optarg;
            break;
        case 'p':
            upass = optarg;
            break;
        case 'D':
            if (blobdir != 0)
                err_error("multiple blob directories specified (%s)\n", optarg);
            blobdir = optarg;
            break;
        case 'w':
            if (where != 0)
                err_error("multiple where clauses specified (%s)\n", optarg);
            where = optarg;
            break;
        case 'Z':
            db_setdebug(atoi(optarg));
            break;
        default:
            err_usage(usestr);
            break;
        }
    }

    if (blobdir == (char *)0)
    {
        static char def_blobdir[] = ".";    /*=C++=*/
        blobdir = def_blobdir;
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
    if (nblobcols == 0)
    {
        err_remark("No blob columns specified\n");
        errors++;
    }
    if (optind != argc)
    {
        err_remark("Extra file arguments specified\n");
        errors++;
    }
    if (errors > 0)
        err_usage(usestr);

    blob_setlocmode(BLOB_IN_NAMEFILE);
    blob_setdirectory(blobdir);
    for (i = 0; i < nblobcols; i++)
        blob_locate(&blobs[i], BLOB_DEFAULT);

    /* For SELECT, it doesn't matter whether it is a logged DB or not */
    blob_connect(xflag, dbase, uname, upass);

    stmt[0] = '\0';
    pad = "SELECT";
    s = stmt;
    z = sizeof(stmt);
    for (i = 0; i < nblobcols; i++)
    {
        esnprintf(s, z, "%s %s", pad, blobcol[i]);
        pad = ",";
        len = strlen(s);
        s += len;
        z -= len;
    }
    esnprintf(s, z, " FROM %s", table);
    len = strlen(s);
    s += len;
    z -= len;
    if (where != 0)
        esnprintf(s, z, " WHERE %s", where);

    if (xflag)
    {
        len = strlen(stmt);
        for (i = 0; i < len; i += 75)
            printf("+ %.75s\n", &stmt[i]);
    }

    EXEC SQL PREPARE p_select FROM :stmt;
    if (sqlca.sqlcode != 0)
        sql_error("prepare select", table);
    EXEC SQL DESCRIBE p_select INTO blobdesc;
    if (sqlca.sqlcode != 0)
        sql_error("describe select", "p_select");
    assert(nblobcols == (size_t)blobdesc->sqld);
    for (i = 0; i < nblobcols; i++)
    {
        switch (blobdesc->sqlvar[i].sqltype)
        {
        case SQLBYTES:
        case SQLTEXT:
            /* OK! */
            break;
        default:
            {
            int t = blobdesc->sqlvar[i].sqltype;
            int l = blobdesc->sqlvar[i].sqllen;
            err_error("non-blob type %s for column %s\n", sqltype(t, l), blobdesc->sqlvar[i].sqlname);
            }
            break;
        }
        blobdesc->sqlvar[i].sqldata = (char *)&blobs[i];    /*=C++=*/
    }
    EXEC SQL DECLARE c_select CURSOR FOR p_select;
    if (sqlca.sqlcode != 0)
        sql_error("declare cursor", table);
    EXEC SQL OPEN c_select;
    if (sqlca.sqlcode != 0)
        sql_error("open cursor", table);
    if (mkpath(blobdir, 0755) != 0)
        err_syserr("failed to create directory %s\n", blobdir);
    while (sqlca.sqlcode == 0)
    {
        EXEC SQL FETCH c_select USING DESCRIPTOR blobdesc;
        if (sqlca.sqlcode != 0)
            break;
        cnt++;
        for (i = 0; i < nblobcols; i++)
        {
            char newname[1024];
            esnprintf(newname, sizeof(newname), "%s/%s.%04lu", blobdir, blobcol[i], cnt);
            if (xflag)
                err_remark("renaming %s to %s\n", jlss_basename(blobs[i].loc_fname), jlss_basename(newname));
            if (rename(blobs[i].loc_fname, newname) != 0)
                err_syserr("failed to rename file %s to %s\n",
                blobs[i].loc_fname, newname);
        }
    }
    if (sqlca.sqlcode < 0)
        sql_error("fetch cursor", table);
    if (xflag)
        err_remark("%lu rows selected\n", cnt);
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
