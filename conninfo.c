/*
@(#)File:           $RCSfile: conninfo.c,v $
@(#)Version:        $Revision: 1.19 $
@(#)Last changed:   $Date: 2010/12/18 22:36:43 $
@(#)Purpose:        Record connection information
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 2000-01,2003,2005-08,2010
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
#include "debug.h"
#include "emalloc.h"

static ConnInfo *info = 0;
static size_t num_info = 0;
static size_t max_info = 0;
static ConnInfo *curr = 0;

#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
const char jlss_id_conninfo_c[] = "@(#)$Id: conninfo.c,v 1.19 2010/12/18 22:36:43 jleffler Exp $";
#endif /* lint */

/* conninfo_release - reset a Conninfo structure, freeing allocated space */
void conninfo_release(ConnInfo *c)
{
    FREE(c->database);
    FREE(c->username);
    FREE(c->password);
    FREE(c->connname);
    conninfo_setempty(c);
}

/*
** conninfo_match - compare two connections for the same name.
**
** The default connection really doesn't have a name so the connection
** name string can be a null pointer.
*/
static int conninfo_match(ConnInfo *c1, ConnInfo *c2)
{
    if (c1->connname == 0 || c2->connname == 0)
        return(-1); /* Not matched */
    return strcmp(c1->connname, c2->connname) == 0;
}

/*
** This is not really good enough.  Even if the output format is XML,
** the connection info is printed in UNLOAD format.
** Fixing this is not trivial.  One mechanism would use a temp table
** and load the data into it, then run a SELECT to retrieve the data.
** Obviously, that won't work if there are no connections.
** Alternatively, we fake an sqlda structure for the data (all
** character, not too hard) and call the correct output routines).
*/
static void conninfo_pr(FILE *fp, ConnInfo *c)
{
    char d = ctxt_getdelim();
    const char *u = c->username;
    const char *db = c->database;
    const char *cn = c->connname;

    if (u == 0)
        u = "";
    if (db == 0)
        db = "";
    if (cn == 0)
        cn = "";

#ifdef ESQLC_TRUSTED_CONTEXT
    fprintf(fp, "%s%c%s%c%s%c%s%c%s%c%s%c%s%c%s%c%s%c\n",
        cn, d,
        db, d,
        u, d,
        c->online    ? "OnLine" : "SE", d,
        c->logged    ? "logged" : "unlogged", d,
        c->mode_ansi ? "MODE ANSI" : "non-ANSI", d,
        c->wct       ? "with concurrent transactions" : "no concurrent transactions", d,
        c->trusted   ? "trusted context" : "regular context", d,
        c->current   ? "current" : "idle", d
        );
#else
    fprintf(fp, "%s%c%s%c%s%c%s%c%s%c%s%c%s%c%s%c\n",
        cn, d,
        db, d,
        u, d,
        c->online    ? "OnLine" : "SE", d,
        c->logged    ? "logged" : "unlogged", d,
        c->mode_ansi ? "MODE ANSI" : "non-ANSI", d,
        c->wct       ? "with concurrent transactions" : "no concurrent transactions", d,
        c->current   ? "current" : "idle", d
        );
#endif
}

static void unset_current(void)
{
    if (curr)
    {
        curr->current = 0;
        curr = 0;
    }
}

void conninfo_connect(ConnInfo *conn)
{
    TRACE((6, "-->> conninfo_connect\n"));
    if (num_info >= max_info)
    {
        /* Error checking in erealloc() */
        size_t curr_num = 0;
        if (curr != 0) 
            curr_num = curr - info;
        info = (ConnInfo *)REALLOC(info, ++max_info * sizeof(ConnInfo));    /*=C++=*/
        if (curr != 0)
            curr = info + curr_num;
    }
    unset_current();
    if (conn->connname == 0 && conn->ctype != CONN_DEFAULT)
    {
        /* There is no connection name for CONNECT TO DEFAULT */
        /* Database, or database@server, is connection name */
        assert(conn->database != 0);
        conn->connname = STRDUP(conn->database);
    }
    conn->current   = 1;
    conn->logged    = (sqlca.sqlwarn.sqlwarn1 == 'W');
    conn->online    = (sqlca.sqlwarn.sqlwarn3 == 'W');
    conn->mode_ansi = (sqlca.sqlwarn.sqlwarn2 == 'W');
    info[num_info]  = *conn;
    curr = &info[num_info];
    num_info++;
    TRACE((6, "<<-- conninfo_connect\n"));
}

int conninfo_current_modeansi(void)
{
    return(curr ? curr->mode_ansi : 0);
}

void conninfo_disconnect(ConnInfo *conn)
{
    size_t i;

    if (conn->ctype == CONN_ALL)
    {
        /* DISCONNECT ALL */
        TRACE((6, "-->> conninfo_disconnect (DISCONNECT ALL)\n"));
        for (i = 0; i < num_info; i++)
            conninfo_release(&info[i]);
        num_info = 0;
        curr = 0;
    }
    else if (conn->ctype == CONN_CURRENT)
    {
        TRACE((6, "-->> conninfo_disconnect (DISCONNECT CURRENT)\n"));
        for (i = 0; i < num_info; i++)
        {
            if (info[i].current == 1)
            {
                conninfo_release(&info[i]);
                info[i] = info[--num_info];
                break;
            }
        }
        curr = 0;
    }
    else
    {
        TRACE((6, "-->> conninfo_disconnect (DISCONNECT)\n"));
        for (i = 0; i < num_info; i++)
        {
            if (conninfo_match(&info[i], conn))
            {
                if (&info[i] == curr)
                    curr = 0;
                conninfo_release(&info[i]);
                info[i] = info[--num_info];
                break;
            }
        }
    }
    TRACE((6, "<<-- conninfo_disconnect\n"));
}

void conninfo_setconnection(ConnInfo *conn)
{
    size_t i;
    TRACE((6, "-->> conninfo_setconnection\n"));
    unset_current();
    for (i = 0; i < num_info; i++)
    {
        if (conninfo_match(&info[i], conn))
        {
            info[i].current = 1;
            TRACE((6, "<<-- conninfo_setconnection\n"));
            return;
        }
    }
    TRACE((6, "<<-- conninfo_setconnection (no match)\n"));
}

void conninfo_print(void)
{
    size_t i;
    FILE *fp = ctxt_output();
    char d = ctxt_getdelim();

    TRACE((6, "-->> conninfo_print\n"));

    if (ctxt_getheading() == OP_ON)
    {
        fprintf(fp, "%s%c%s%c%s%c%s%c%s%c%s%c%s%c%s%c\n",
                "connection", d,
                "database", d,
                "username", d,
                "server_type", d,
                "logging", d,
                "mode_ansi", d,
                "transactions", d,
                "currency", d
                );
    }

    for (i = 0; i < num_info; i++)
    {
        conninfo_pr(fp, &info[i]);
    }
}

