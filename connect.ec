/*
@(#)File:           $RCSfile: connect.ec,v $
@(#)Version:        $Revision: 2015.1 $
@(#)Last changed:   $Date: 2015/07/10 16:05:12 $
@(#)Purpose:        Handle CONNECT, DISCONNECT and SET CONNECTION statements
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 1997-2001,2003-07,2015
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

/*TABSTOP=4*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "connect.h"
#include "context.h"
#include "esqlutil.h"
#include "internal.h"
#include "jlss.h"
#include "jlsstools.h"
#include "sqlerr.h"
#include "emalloc.h"

$include esqlinfo.h;

#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
extern const char jlss_id_connect_ec[];
const char jlss_id_connect_ec[] = "@(#)$Id: connect.ec,v 2015.1 2015/07/10 16:05:12 jleffler Exp $";
#endif /* lint */

$ifndef ESQLC_CONNECT;

void     do_connect(char *str)
{
    error_746("CONNECT is not part of this version of ESQL/C\n");
}

void     do_disconn(char *str)
{
    error_746("DISCONNECT is not part of this version of ESQL/C\n");
}

/* Assume it is some other statement than SET CONNECTION */
void     do_setconn(char *str)
{
    error_746("SET CONNECTION is not part of this version of ESQL/C\n");
}

$else;

/*
** Process CONNECT statements (keyword CONNECT already processed)
** Valid syntaxes:
**      <connect>   ::= CONNECT TO <conntype> <opt_wct>
**      <conntype>  ::= DEFAULT
**      <conntype>  ::= <dbenv> <opt_conn> <opt_user>
**      <dbenv>     ::= <string>
**      <opt_conn>  ::= AS <string>
**      <opt_conn>  ::= -- nothing
**      <opt_user>  ::= USER <string> USING <string>
**      <opt_user>  ::= -- nothing
**      <opt_wct>   ::= WITH CONCURRENT TRANSACTION
**      <opt_wct>   ::= -- nothing
** The <string> tokens can be quoted or simply literals.  If no connection
** name is given, then a connection name is manufactured by SQLCMD.  The
** (case-insensitive) connection names DEFAULT and ALL and CURRENT are
** reserved by SQLCMD, as they are needed by the DISCONNECT statement.
*/
void     do_connect(char *str)
{
    ConnInfo conn = { 0 };

    if (parse_connstmt(str, &conn) == STMT_CONNECT)
    {
        sql_connect(&conn);
        if (sqlca.sqlcode != 0)
            sql_error();
        else
        {
            if (ctxt_getverbose() == OP_ON)
                sql_dbinfo();
            conninfo_connect(&conn);
        }
    }
    else
    {
        conninfo_release(&conn);
        error_746("Syntax error in CONNECT statement");
    }
}

/*
** Process DISCONNECT statements (keyword DISCONNECT already processed)
** Valid syntaxes:
**      <disconnect>    ::= DISCONNECT CURRENT
**      <disconnect>    ::= DISCONNECT DEFAULT
**      <disconnect>    ::= DISCONNECT ALL
**      <disconnect>    ::= DISCONNECT <string>
*/
void     do_disconn(char *str)
{
    ConnInfo conn = { 0 };

    if (parse_connstmt(str, &conn) == STMT_DISCONN)
    {
        sql_disconn(&conn);
        if (sqlca.sqlcode != 0)
            sql_error();
        else
            conninfo_disconnect(&conn);
    }
    else
    {
        conninfo_release(&conn);
        error_746("Syntax error in DISCONNECT statement");
    }
}

/*
** Process SET CONNECTION statements (first word known to be SET, but
** also have to worry about other SET statements (eg SET EXPLAIN) which
** have to be processed by sql_command().  DORMANT connections are a
** 7.x (multi-threaded ESQL/C) feature, but can't be used in SQLCMD.
** Valid syntaxes:
**      <set-connection>    ::= SET CONNECTION <string>
**      <set-connection>    ::= SET CONNECTION DEFAULT
*/
void     do_setconn(char *str)
{
    ConnInfo  conn = { 0 };
    StmtType  stype;

    if ((stype = parse_connstmt(str, &conn)) == STMT_SETCONN)
    {
        /* It is SET CONNECTION */
        sql_setconn(&conn);
        if (sqlca.sqlcode != 0)
            sql_error();
        else
            conninfo_setconnection(&conn);
    }
    else if (stype == STMT_SETOTHER)
    {
        /* Not SET CONNECTION -- SET ISOLATION, or SET EXPLAIN, or ...?  */
        sql_command(str);
    }
    else
    {
        conninfo_release(&conn);
        error_746("Syntax error in SET CONNECTION statement");
    }
}

$endif; /* ESQLC_CONNECT */
