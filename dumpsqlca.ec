/*
@(#)File:           $RCSfile: dumpsqlca.ec,v $
@(#)Version:        $Revision: 2008.2 $
@(#)Last changed:   $Date: 2008/01/19 20:45:52 $
@(#)Purpose:        Dump/intepret contents of SQLCA structure
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 1987,1991,1995,1997-99,2001,2003,2005,2007-08
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

/*TABSTOP=4*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

/* This is an ESQL/C file to simplify the compiling.  */
#include "dumpesql.h"
#include "esqlutil.h"

static const char *errd[] =
{
    "Estimated number of rows",
    "ISAM error or serial number",
    "Number of rows processed",
    "Estimated CPU time",
    "Offset of error into RDSQL statement",
    "ROWID of last row"
};

static const char *warn[] =
{
    "Any warning set",
    "Data item truncated (database has TX log)",
    "Aggregate encountered NULL (MODE ANSI database)",
    "Mismatch between select-list and INTO (OnLine Engine)",
    "UPDATE/DELETE without where (FLOAT<->DECIMAL conversion)",
    "Non-ANSI SQL",
    "Data Fragment skipped (OnLine running in secondary mode)",
    "Not used (DB_LOCALE does not match database locale)"
};

#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
const char jlss_id_dumpsqlca_ec[] = "@(#)$Id: dumpsqlca.ec,v 2008.2 2008/01/19 20:45:52 jleffler Exp $";
#endif /* lint */

void dumpsqlca(FILE *fp, const char *tag)
{
    dump_sqlca(fp, tag, &sqlca);
}

void dump_sqlca(FILE *fp, const char *tag, const Sqlca *psqlca)
{
    int    i;
    const char   *cptr;
    const ixInt4 *lptr;

    dump_print(fp, "-----SQLCA----- %s\n", tag);
    dump_print(fp, "sqlcode = %" PRId_ixInt4 "\n", psqlca->sqlcode);
    dump_print(fp, "sqlerrm = '%s'\n", psqlca->sqlerrm);
    dump_print(fp, "sqlerrp = '%s'\n", psqlca->sqlerrp);
    lptr = &psqlca->sqlerrd[0];
    for (i = 0; i < 6; i++)
        dump_print(fp, "sqlerrd[%d] = %" PRId_ixInt4 ": (%s)\n", i, *lptr++, errd[i]);
    cptr = &psqlca->sqlwarn.sqlwarn0;
    for (i = 0; i < 8; i++)
        dump_print(fp, "sqlwarn%d = `%c': (%s)\n", i, *cptr++, warn[i]);
    dump_print(fp, "---SQLCA END---\n");
    fflush(fp);
}

$ifdef TEST;

#include <stdlib.h>

static void sql_error(void)
{
    sql_printerror(stderr);
    exit(1);
}

int main(int argc, char **argv)
{
    EXEC SQL BEGIN DECLARE SECTION;
    char *dbname = "stores7";
    char *stmt = "SELECT * FROM Orders";
    EXEC SQL END DECLARE SECTION;
    Sqlda *ptr;

    if (argc > 1)
        dbname = argv[1];
    if (argc > 2)
        stmt = argv[2];

    EXEC SQL WHENEVER ERROR CALL sql_error;
    EXEC SQL DATABASE :dbname;
    dump_sqlca(stdout, "Post DATABASE statement", &sqlca);
    EXEC SQL PREPARE p_stmt FROM :stmt;
    dump_sqlca(stdout, "Post PREPARE statement", &sqlca);
    EXEC SQL DESCRIBE p_stmt INTO ptr;
    dump_sqlca(stdout, "Post DESCRIBE statement", &sqlca);
    return 0;
}

$endif; /* TEST */
