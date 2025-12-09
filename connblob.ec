/*
@(#)File:           $RCSfile: connblob.ec,v $
@(#)Version:        $Revision: 2015.1 $
@(#)Last changed:   $Date: 2015/07/10 15:39:21 $
@(#)Purpose:        Connect to database for blob manipulation programs
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 2004-06,2008,2015
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

/*TABSTOP=4*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include "connblob.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
EXEC SQL INCLUDE esqlinfo.h;
#include "stderr.h"
#include "sqlconn.h"
#include "jlss.h"   /* nstrcpy() */
#include "esnprintf.h"

#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
extern const char jlss_id_connblob_ec[];
const char jlss_id_connblob_ec[] = "@(#)$Id: connblob.ec,v 2015.1 2015/07/10 15:39:21 jleffler Exp $";
#endif /* lint */

/* Report database generated error and exit */
void sql_error(const char *s1, const char *s2)
{
    char            buffer[512];

    fflush(stdout);
    rgetmsg(sqlca.sqlcode, buffer, sizeof(buffer));
    fprintf(stderr, "SQL %" PRId_ixInt4 ": ", sqlca.sqlcode);
    if (sqlca.sqlerrd[1] != 0)
        fprintf(stderr, "(ISAM %" PRId_ixInt4 ") ", sqlca.sqlerrd[1]);
    fprintf(stderr, buffer, sqlca.sqlerrm);
    if (s1 != (char *)0)
        fprintf(stderr, "%s %s\n", s1, (s2 ? s2 : ""));
    exit(EXIT_FAILURE);
}

/* Build WHERE clause */
/* So, how do you say "I have not intention of modifying anything in keycol or keyval? */
void build_where(size_t nkeycols, char **keycol, char **keyval, char *where, size_t size)
{
    size_t      i;
    size_t      z = size;
    char       *s = where;
    const char *pad = "";
    *s = '\0';

    for (i = 0; i < nkeycols; i++)
    {
        size_t len;
        esnprintf(s, z, "%s %s = '%s'", pad, keycol[i], keyval[i]);
        pad = " AND";
        len = strlen(s);
        s += len;
        z -= len;
    }
}

/*
** Connect to given database, and indicate whether it is a logging
** database.
*/
int blob_connect(int xflag, char *p_dbase, char *p_uname, char *p_upass)
{

    if (p_dbase == 0)
        err_error("no database name supplied\n");

$ifdef ESQLC_CONNECT;
    if (xflag)
        printf("+ CONNECT TO %s\n", p_dbase);
    {
    ConnInfo conn;
    conn.database = p_dbase;
    conn.username = p_uname;
    conn.password = p_upass;
    conn.connname = 0;
    conn.ctype = CONN_NONE;

    sql_connect(&conn);
    if (sqlca.sqlcode != 0)
        sql_error("connect to", p_dbase);
    }
$else;
    {
    EXEC SQL BEGIN DECLARE SECTION;
    char *dbase = p_dbase;
    char *uname = p_uname;
    char *upass = p_upass;
    EXEC SQL END DECLARE SECTION;

    if (xflag)
        printf("+ DATABASE %s\n", dbase);
    EXEC SQL DATABASE :dbase;
    if (sqlca.sqlcode != 0)
        sql_error("database", dbase);
    }
$endif;

    /* Indicate whether database has transaction log */
    return(sqlca.sqlwarn.sqlwarn1 == 'W');
}
