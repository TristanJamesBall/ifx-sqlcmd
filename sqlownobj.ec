/*
@(#)File:           $RCSfile: sqlownobj.ec,v $
@(#)Version:        $Revision: 2016.1 $
@(#)Last changed:   $Date: 2016/01/18 03:11:29 $
@(#)Purpose:        Determine Identifier for an Owned Object
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 2000-03,2005-06,2016
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

/*TABSTOP=4*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "esqlutil.h"
#include "sqlownobj.h"
#include "sqlquote.h"
#include "jlss.h"
#include "debug.h"

struct OwnedObjCtl
{
    const char *quotes;
    const char *function;
    const char *objtype;
    const char *idcol;
    const char *namecol;
    const char *ownercol;
    const char *systable;
};

typedef struct OwnedObjCtl OwnedObjCtl;

static OwnedObjCtl ctl_table =
{
    "\"", "sql_tabid", "table", "TabID", "TabName", "Owner", "SysTables"
};

static OwnedObjCtl ctl_trig =
{
    "\"", "sql_trigid", "trigger", "TrigID", "TrigName", "Owner", "SysTriggers"
};

static OwnedObjCtl ctl_proc =
{
    "\"'", "sql_procid", "procedure", "ProcID", "ProcName", "Owner", "SysProcedures"
};

static const char sql_owned[] =
    "SELECT %s FROM %s\"informix\".%s WHERE %s = %s AND %s = %s";
static const char sql_modeansi[] =
    "SELECT %s FROM %s\"informix\".%s WHERE %s = %s AND %s = USER";
static const char sql_unowned[] =
    "SELECT %s FROM %s\"informix\".%s WHERE %s = %s";

#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
extern const char jlss_id_sqlownobj_ec[];
const char jlss_id_sqlownobj_ec[] = "@(#)$Id: sqlownobj.ec,v 2016.1 2016/01/18 03:11:29 jleffler Exp $";
#endif /* lint */

/*
** Convert separate database and server name components into a composite
** database prefix suitable for prepending to table names.  It includes,
** therefore, a terminal ':'.
*/
char *sql_mkdbasename(
    const char *dbase,      /* Name of database, possibly null or empty */
    const char *server,     /* Name of server, possibly null or empty */
    char *dbname,           /* Output full database name */
    size_t maxdblen)        /* Maximum space to use */
{
    char *dst = dbname;

    if (server == 0)
        server = "";

    if (dbase == 0 || dbase[0] == '\0')
    {
        /* Cannot specify server only */
        assert(server[0] == '\0');
        *dbname = '\0';
    }
    else
    {
        size_t len = strlen(dbase) + 2; /* 2 for colon and terminal NUL */
        if (server[0] != '\0')
            len += strlen(server) + 1;  /* 1 for @ separator */
        if (len > maxdblen)
            dbname = 0;
        else
        {
            dst = vstrcpy(dst, 1, dbase);
            if (server[0] != '\0')
                dst = vstrcpy(dst, 2, "@", server);
            dst = vstrcpy(dst, 1, ":");
        }
    }
    DB_TRACE(5, "sql_mkdbasename: returning <<%s>>\n", dbname);
    return dbname;
}

/*
** Convert connection information into full table name.
** NB: quotes or absence of quotes around owner name are preserved by
** grammar so that MODE ANSI databases work correctly.  Hence, do not
** enclose owner in quotes here; it would defeat the good work done
** by the grammar.
*/
char *sql_mktablename(
    const char *table,  /* Table name */
    const char *owner,  /* Owner name, possibly in quotes, possibly null */
    const char *dbase,  /* Database/server name, possibly null or empty */
    char *output,       /* Result string */
    size_t outlen)      /* Space in result string */
{
    char *dst = output;
    size_t totlen = 0;
    size_t len = 0;

    if (owner == 0)
        owner = "";
    if (dbase == 0)
        dbase = "";
    DB_TRACE(5, "-->> sql_mktablename(): table = %s, owner = %s, dbase = %s\n", table, owner, dbase);

    /* Table is always specified */
    assert(table != 0 && table[0] != '\0');
    totlen = strlen(table) + 1;
    if ((len = strlen(owner)) > 0)
        totlen += len + 1;
    if ((len = strlen(dbase)) > 0)
        totlen += len + 1;
    if (totlen > outlen)
        return 0;

    *output = '\0';
    dst = output;
    if (dbase[0])
    {
        /* Allow for dbase strings with or without ':' at end */
        dst = vstrcpy(dst, 1, dbase);
        if (*(dst - 1) != ':')
            dst = vstrcpy(dst, 1, ":");
    }
    if (owner[0])
        dst = vstrcpy(dst, 2, owner, ".");
    strcat(dst, table);

    DB_TRACE(5, "<<-- sql_mktablename:() <<%s>>\n", output);
    return output;
}

static long sql_owned_object_id(
    const char *object,     /* Name of table */
    const char *owner,      /* Owner, optionally quoted, or null */
    const char *dbase,      /* Database name or null */
    const char *server,     /* Server name or null */
    OwnedObjCtl *ctl,
    int         mode_ansi)  /* Flag: 0 => not MODE ANSI database; otherwise MODE ANSI */
{
    struct sqlca_s s_sqlca;
    EXEC SQL BEGIN DECLARE SECTION;
    long objid = -1;
    char buffer[4096];
    EXEC SQL END DECLARE SECTION;
    char dbname[132];   /* Allow for flex-names */
    char locobj1[2*SQL_NAMELEN+3];  /* Allow for flex-names */
    char locobj2[2*SQL_NAMELEN+3];  /* Allow for flex-names */
    char locown1[2*SQL_USERLEN+3];  /* Allow for flex-names */
    char locown2[2*SQL_USERLEN+3];  /* Allow for flex-names */

    if (sql_mkdbasename(dbase, server, dbname, sizeof(dbname)) == 0)
    {
        sql_printerror(stderr);
        return(-1);
    }

    /**
    ** Note that both the owner and the table names can be delimited
    ** identifiers.  There's a big difference between the way you treat
    ** delimited identifiers in the text of a statement and the way they
    ** need to appear for lookup in the system catalog.  The input to
    ** this function is an identifier as it would appear in the text of
    ** a statement.
    **
    ** The Informix 9.20 servers blur the distinction between EXECUTE
    ** PROCEDURE "identifier"() and EXECUTE PROCEDURE 'identifier'(),
    ** regrettably (but there is no equivalent blurring for table
    ** names).  For compatibility, this code has to do the same, and the
    ** ctl->quotes string indicates which quotes are allowed.  The owner
    ** name can be enclosed in single or double quotes.
    */

    /* Is first char of object name a valid quote for that object type? */
    if (strchr(ctl->quotes, object[0]) != 0)
    {
        /*
        ** Assume DELIMIDENT set; eliminate leading/trailing double
        ** quotes, then convert to single-quoted string
        */
        if ((sql_unquote_string(locobj1, sizeof(locobj1), object) != 0) ||
            (sql_quote_string(locobj2, sizeof(locobj2), locobj1, '\'') != 0))
        {
            DB_TRACE(0, "%s(): strlen(object) = %lu, sizeof(locobj1) = %lu\n", ctl->function, strlen(object), sizeof(locobj1));
            sqlca.sqlcode = -746;
            sprintf(sqlca.sqlerrm, "%s(): unable to parse quoted %s name (%.32s)", ctl->function, ctl->objtype, object);
            return(-1);
        }
        DB_TRACE(4, "%s(): quoted %s <<%s>> -> <<%s>>\n", ctl->function, ctl->objtype, object, locobj2);
    }
    else
    {
        strcpy(locobj1, object);
        strlower(locobj1);
        if (sql_quote_string(locobj2, sizeof(locobj2), locobj1, '\'') != 0)
        {
            sqlca.sqlcode = -746;
            sprintf(sqlca.sqlerrm, "%s(): unable to parse unquoted %s name (%s.32)", ctl->function, ctl->objtype, object);
            return(-1);
        }
        DB_TRACE(4, "%s(): unquoted %s <<%s>> -> <<%s>>\n", ctl->function, ctl->objtype, object, locobj2);
    }

    if (owner == 0)
        owner = "";
    if (owner[0] == '\0')
    {
        /* No owner */
        DB_TRACE(4, "%s(): no owner\n", ctl->function);
        if (mode_ansi)
            sprintf(buffer, sql_modeansi, ctl->idcol, dbname, ctl->systable, ctl->namecol, locobj2, ctl->ownercol);
        else
            sprintf(buffer, sql_unowned, ctl->idcol, dbname, ctl->systable, ctl->namecol, locobj2);
    }
    else if (owner[0] == '\'' || owner[0] == '"')
    {
        /* Quoted owner */
        /* Assume DELIMIDENT set and cope with embedded quotes */
        if ((sql_unquote_string(locown1, sizeof(locown1), owner) != 0) ||
            (sql_quote_string(locown2, sizeof(locown2), locown1, '\'') != 0))
        {
            sqlca.sqlcode = -746;
            sprintf(sqlca.sqlerrm, "%s(): unable to parse quoted owner of %s name (%s.32)", ctl->function, ctl->objtype, owner);
            return(-1);
        }
        DB_TRACE(4, "%s(): quoted owner <<%s>> -> <<%s>>\n", ctl->function, owner, locown2);
        sprintf(buffer, sql_owned, ctl->idcol, dbname, ctl->systable, ctl->namecol, locobj2, ctl->ownercol, locown2);
    }
    else
    {
        /* Unquoted owner */
        strcpy(locown1, owner);
        if (mode_ansi)
            strupper(locown1);
        else
            strlower(locown1);
        if (sql_quote_string(locown2, sizeof(locown2), locown1, '\'') != 0)
        {
            sqlca.sqlcode = -746;
            sprintf(sqlca.sqlerrm, "%s(): unable to parse unquoted owner of %s name (%s.32)", ctl->function, ctl->objtype, owner);
            return(-1);
        }
        DB_TRACE(4, "%s(): unquoted owner <<%s>>\n", ctl->function, locown2);
        sprintf(buffer, sql_owned, ctl->idcol, dbname, ctl->systable, ctl->namecol, locobj2, ctl->ownercol, locown2);
    }

    DB_TRACE(3, "%s: query = %s\n", ctl->function, buffer);
    EXEC SQL PREPARE p_sql_objid_q002 FROM :buffer;
    if (sqlca.sqlcode < 0)
        return(-1);
    EXEC SQL DECLARE c_sql_objid_q002 CURSOR FOR p_sql_objid_q002;
    if (sqlca.sqlcode < 0)
    {
        s_sqlca = sqlca;
        EXEC SQL FREE p_sql_objid_q002;
        sqlca = s_sqlca;
        return(-1);
    }
    EXEC SQL OPEN c_sql_objid_q002;
    if (sqlca.sqlcode < 0)
    {
        s_sqlca = sqlca;
        EXEC SQL FREE c_sql_objid_q002;
        EXEC SQL FREE p_sql_objid_q002;
        sqlca = s_sqlca;
        return(-1);
    }
    EXEC SQL FETCH c_sql_objid_q002 INTO :objid;
    if (sqlca.sqlcode < 0)
    {
        s_sqlca = sqlca;
        EXEC SQL CLOSE c_sql_objid_q002;
        EXEC SQL FREE c_sql_objid_q002;
        EXEC SQL FREE p_sql_objid_q002;
        sqlca = s_sqlca;
        return(-1);
    }
    if (sqlca.sqlcode == SQLNOTFOUND)
    {
        EXEC SQL CLOSE c_sql_objid_q002;
        EXEC SQL FREE c_sql_objid_q002;
        EXEC SQL FREE p_sql_objid_q002;
        sqlca.sqlcode = -746;
        sprintf(sqlca.sqlerrm, "%s %.32s not found in database", ctl->objtype, object);
        return(-1);
    }
    EXEC SQL CLOSE c_sql_objid_q002;
    if (sqlca.sqlcode < 0)
    {
        s_sqlca = sqlca;
        EXEC SQL FREE c_sql_objid_q002;
        EXEC SQL FREE p_sql_objid_q002;
        sqlca = s_sqlca;
        return(-1);
    }
    EXEC SQL FREE c_sql_objid_q002;
    if (sqlca.sqlcode < 0)
    {
        s_sqlca = sqlca;
        EXEC SQL FREE p_sql_objid_q002;
        sqlca = s_sqlca;
        return(-1);
    }
    EXEC SQL FREE p_sql_objid_q002;
    if (sqlca.sqlcode < 0)
        return(-1);
    DB_TRACE(3, "%s(): returning objid = %ld\n", ctl->function, objid);
    return objid;
}

long sql_tabid(
    const char *table,      /* Name of table */
    const char *owner,      /* Owner, optionally quoted, or null */
    const char *dbase,      /* Database name or null */
    const char *server,     /* Server name or null */
    int         mode_ansi)  /* Flag: 0 => not MODE ANSI database; otherwise MODE ANSI */
{
    return(sql_owned_object_id(table, owner, dbase, server, &ctl_table, mode_ansi));
}

long sql_procid(
    const char *proc,       /* Name of procedure */
    const char *owner,      /* Owner, optionally quoted, or null */
    const char *dbase,      /* Database name or null */
    const char *server,     /* Server name or null */
    int         mode_ansi)  /* Flag: 0 => not MODE ANSI database; otherwise MODE ANSI */
{
    return(sql_owned_object_id(proc, owner, dbase, server, &ctl_proc, mode_ansi));
}

long sql_trigid(
    const char *trigger,    /* Name of trigger */
    const char *owner,      /* Owner, optionally quoted, or null */
    const char *dbase,      /* Database name or null */
    const char *server,     /* Server name or null */
    int         mode_ansi)  /* Flag: 0 => not MODE ANSI database; otherwise MODE ANSI */
{
    return(sql_owned_object_id(trigger, owner, dbase, server, &ctl_trig, mode_ansi));
}

#ifdef TEST

#include <stdlib.h>
#include "getopt.h"
#include "stderr.h"

static char optstr[] = "VD:d:o:s:t:p:r:";
static char usestr[] = "[-V] [-D debug] -d dbase [-t table|-p proc|-r trigger]"
    " [-o owner] [-s server]";

int main(int argc, char **argv)
{
    EXEC SQL BEGIN DECLARE SECTION;
    char *dbase = 0;
    EXEC SQL END DECLARE SECTION;
    char *objname = 0;
    int pflag = 0;
    int rflag = 0;
    int tflag = 0;
    char *owner = 0;
    char *server = 0;
    char *dbname = 0;
    char *svname = 0;
    char *objtype = 0;
    int mode_ansi;
    int opt;
    long objid = -1;

    err_setarg0(argv[0]);

#ifdef DEBUG
    db_setdebug(3);
#endif /* DEBUG */

    while ((opt = GETOPT(argc, argv, optstr)) != EOF)
    {
        switch (opt)
        {
        case 'D':
            db_setdebug(atoi(optarg));
            break;
        case 'd':
            dbase = optarg;
            break;
        case 'o':
            owner = optarg;
            break;
        case 'p':
            objname = optarg;
            pflag = 1;
            objtype = "Procedure";
            break;
        case 'r':
            objname = optarg;
            rflag = 1;
            objtype = "Trigger";
            break;
        case 't':
            objname = optarg;
            tflag = 1;
            objtype = "Table";
            break;
        case 's':
            server = optarg;
            break;
        case 'V':
            err_version("SQLTABID", &"@(#)$Revision: 2016.1 $ ($Date: 2016/01/18 03:11:29 $)"[4]);
            break;
        default:
            err_usage(usestr);
            break;
        }
    }

    if ((tflag == 0 && pflag == 0 && rflag == 0) || dbase == 0)
        err_usage(usestr);

    EXEC SQL DATABASE :dbase;
    if (sqlca.sqlcode != 0)
    {
        sql_printerror(stderr);
        exit(1);
    }
    dbname = dbase;
    svname = server;
    if (sqlca.sqlwarn.sqlwarn3 != 'W')
    {
        /* SE database -- do not include database or server */
        dbname = 0;
        svname = 0;
    }
    mode_ansi = (sqlca.sqlwarn.sqlwarn2 == 'W');

    if (pflag)
        objid = sql_procid(objname, owner, dbname, svname, mode_ansi);
    else if (rflag)
        objid = sql_trigid(objname, owner, dbname, svname, mode_ansi);
    else if (tflag)
        objid = sql_tabid(objname, owner, dbname, svname, mode_ansi);

    if (owner == 0)
        owner = "";
    if (server == 0)
        server = "";
    printf("Database: %s\nServer:   %s\nMode:     %s\n%s:    %s\nOwner:    %s\n"
            "ObjectID:    %ld\n", dbase, server, (mode_ansi ? "MODE ANSI" : "non-ANSI"),
            objtype, objname, owner, objid);
    if (sqlca.sqlcode < 0)
        sql_printerror(stdout);
    return(0);
}

#endif /* TEST */
