/*
@(#)File:           $RCSfile: sqlinfo.ec,v $
@(#)Version:        $Revision: 2015.1 $
@(#)Last changed:   $Date: 2015/07/10 16:05:12 $
@(#)Purpose:        Handle INFO statements
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 1998-2003,2005-06,2008,2015
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
#include "esnprintf.h"
#include "sqlerr.h"
#include "debug.h"
#include "emalloc.h"
#include "sqlownobj.h"

#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
extern const char jlss_id_sqlinfo_ec[];
const char jlss_id_sqlinfo_ec[] = "@(#)$Id: sqlinfo.ec,v 2015.1 2015/07/10 16:05:12 jleffler Exp $";
#endif /* lint */

/*
** The alternative command execution functions for INFO statements are:
**  -- do_command():  provides benchmark, trace and history service.
**  -- sql_command(): does not provide benchmark, trace or history.
** There are advantages to each.  To make it easier to change our mind,
** use a function pointer rather than a fixed command name throughout.
** One more peculiar side-effect of using do_command() is that the
** history number jumps by two or more when an INFO command is executed.
** On the other hand, during development of INFO commands, it is helpful
** to be able to track the SQL that is executed.
*/
static void (*sql_exec)(const char *cmd) = do_command;

/* Statement for INFO TABLES */
/* SE uses TabID 100 with TabType = ' ' to record MODE ANSI status */
static char info_tables[] =
    "SELECT Owner, TabName, TabType, TabID FROM \"informix\".SysTables T"
    " WHERE TabID >= 100"
    "   AND TabType IN ('T', 'V', 'P', 'S')"
    "   AND (%.*s)"
    " ORDER BY TabName, Owner";

/* Statement for INFO SYSTABLES */
static char info_systables[] =
    "SELECT Owner, TabName, TabType, TabID FROM \"informix\".SysTables T"
    " WHERE TabID < 100 AND TabType IN ('T', 'V', 'P', 'S')"
    "   AND TabName NOT MATCHES \" *\""
    "   AND (%.*s)"
    " ORDER BY TabName, Owner";

/* Statement for INFO VIEWS */
static char info_views[] =
    "SELECT Owner, TabName, TabID FROM \"informix\".SysTables T"
    " WHERE TabID >= 100 AND TabType = 'V'"
    "   AND (%.*s)"
    " ORDER BY TabName, Owner";

/* Statement for INFO BASETABLES */
static char info_basetables[] =
    "SELECT Owner, TabName, TabID FROM \"informix\".SysTables T"
    " WHERE TabID >= 100 AND TabType = 'T'"
    "   AND (%.*s)"
    " ORDER BY TabName, Owner";

/* Statement for INFO SYNONYMS */
/* Only report the user's private synonyms (P) but all the public ones (S) */
static char info_synonyms[] =
    "SELECT Owner, TabName, TabType, TabID FROM \"informix\".SysTables T\n"
    " WHERE TabID >= 100 AND (%.*s)\n"
    "   AND ((TabType = 'S') OR -- Public synonyms\n"
    "        (TabType = 'P' AND Owner = USER)) -- Private synonyms for this user\n"
    " ORDER BY 2, 1";

/* Statements for INFO COLUMNS */
static char sql_columns_online[] =
    "SELECT ColNo, ColName, ColType, ColLength, 0 Extended_ID"
    "  FROM %s\"informix\".SysColumns C"
    " WHERE TabID = %ld ORDER BY ColNo";

/* Statement for INFO COLUMNS */
static char sql_columns_idsudo[] =
    "SELECT ColNo, ColName, ColType, ColLength, Extended_ID"
    "  FROM %s\"informix\".SysColumns C"
    " WHERE TabID = %ld ORDER BY ColNo";

/* Statement for INFO PROCEDURES */
static char info_procedures[] =
    "SELECT ProcID, Owner, Procname, NumArgs"
    "  FROM \"informix\".SysProcedures P"
    " WHERE (%.*s)"
    " ORDER BY ProcID";

/* Statement for INFO PROCBODY */
static char sql_procbody[] =
    "SELECT Seqno, Data"
    "  FROM %s\"informix\".SysProcBody"
    " WHERE ProcID = %ld"
    "   AND DataKey = 'T'"
    " ORDER BY SeqNo";

/* Statement for INFO VIEWBODY */
static char sql_viewbody[] =
    "SELECT Seqno, ViewText"
    "  FROM %s\"informix\".SysViews"
    " WHERE TabID = %ld"
    " ORDER BY SeqNo";

/* Statement for INFO TRIGGERS */
static char sql_triggers[] =
    "SELECT R.Owner, R.TrigName, R.TrigID, T.Owner TabOwner, T.TabName, R.Event, R.Old, R.New, R.Mode"
    "  FROM %s\"informix\".SysTables T, %s\"informix\".SysTriggers R"
    " WHERE R.TabID = %ld"
    "   AND R.TabID = T.TabID"
    "   AND T.TabID = %ld"
    " ORDER BY R.TrigName, R.Owner";

/* Statement for INFO TRIGBODY */
/* Need DataKey D (trigger definition) before DataKey A (triggered actions) */
static char sql_trigbody[] =
    "SELECT DataKey, Seqno, Data"
    "  FROM %s\"informix\".SysTrigBody"
    " WHERE TrigID = %ld"
    "   AND DataKey IN ('D', 'A')"
    " ORDER BY DataKey DESC, SeqNo ASC";

/* Statement for INFO REFERENCES TO/BY */
static char sql_references[] =
    "SELECT  C1.constrname   refng_constr_name,"
    "        C1.owner        refng_constr_owner,"
    "        C1.idxname      refng_index_name,"
    "        T1.tabid        refng_table_tabid,"
    "        T1.owner        refng_table_owner,"
    "        T1.tabname      refng_table_name,"
    "        C2.constrname   refed_constr_name,"
    "        C2.owner        refed_constr_owner,"
    "        C2.idxname      refed_index_name,"
    "        T2.tabid        refed_table_tabid,"
    "        T2.owner        refed_table_owner,"
    "        T2.tabname      refed_table_name"
    "  FROM  %s\"informix\".SysReferences R,"
    "        %s\"informix\".SysConstraints C1,"
    "        %s\"informix\".SysConstraints C2,"
    "        %s\"informix\".SysTables T2,"
    "        %s\"informix\".SysTables T1"
    " WHERE C1.constrtype = 'R'"
    "   AND C1.tabid = T1.tabid"
    "   AND C1.constrid = R.constrid"
    "   AND R.ptabid = T2.tabid"
    "   AND R.primary = C2.constrid"
    "   AND %s.tabid = %ld";

/* Statements for INFO INDEXES -- 5 of them */
/* NB: SysIndexes is a view on SysIndices under IUS or IDS/UDO */
static char info_idx_drop[] = "DROP TABLE _sqlcmd_info_index";

static char info_idx_mktemp[] =
    "SELECT -- unique explicit indexes\n"
    " 'I' idxtype, I.tabid,"
    " I.owner conowner, I.idxname conname,"
    " I.owner idxowner, I.idxname idxname"
    "\nFROM %s\"informix\".SysIndexes I"
    "\nWHERE I.tabid = %d AND I.idxname NOT MATCHES \" *\" AND i.idxtype = 'U'"
    " UNION\n"
    " SELECT -- duplicates explicit indexes\n"
    " I.idxtype, I.tabid,"
    " I.owner conowner, I.idxname conname,"
    " I.owner idxowner, I.idxname idxname"
    "\nFROM %s\"informix\".SysIndexes I"
    "\nWHERE I.tabid = %d AND I.idxname NOT MATCHES \" *\" AND i.idxtype = 'D'"
    " UNION\n"
    " SELECT -- primary keys, unique constraints, foreign keys\n"
    " C.Constrtype idxtype, I.tabid,"
    " C.Owner conowner, C.Constrname conname,"
    " I.Owner idxowner, I.idxname    idxname"
    "\nFROM %s\"informix\".SysIndexes I, %s\"informix\".SysConstraints C"
    "\nWHERE C.tabid = I.tabid"
    "   AND C.Idxname = I.idxname"
    "   AND I.tabid = %d"
    " INTO TEMP _sqlcmd_info_index";

static char info_idx_online[] =
    "SELECT t0.idxtype, t0.conowner, t0.conname, t0.idxowner, t0.idxname,"
    " c01.colname col01, c02.colname col02, c03.colname col03,"
    " c04.colname col04, c05.colname col05, c06.colname col06,"
    " c07.colname col07, c08.colname col08, c09.colname col09,"
    " c10.colname col10, c11.colname col11, c12.colname col12,"
    " c13.colname col13, c14.colname col14, c15.colname col15,"
    " c16.colname col16"
    "\nFROM _sqlcmd_info_index t0, %s\"informix\".SysIndexes t2,"
    "         %s\"informix\".SysColumns c01,"
    "   OUTER %s\"informix\".SysColumns c02,"
    "   OUTER %s\"informix\".SysColumns c03,"
    "   OUTER %s\"informix\".SysColumns c04,"
    "   OUTER %s\"informix\".SysColumns c05,"
    "   OUTER %s\"informix\".SysColumns c06,"
    "   OUTER %s\"informix\".SysColumns c07,"
    "   OUTER %s\"informix\".SysColumns c08,"
    "   OUTER %s\"informix\".SysColumns c09,"
    "   OUTER %s\"informix\".SysColumns c10,"
    "   OUTER %s\"informix\".SysColumns c11,"
    "   OUTER %s\"informix\".SysColumns c12,"
    "   OUTER %s\"informix\".SysColumns c13,"
    "   OUTER %s\"informix\".SysColumns c14,"
    "   OUTER %s\"informix\".SysColumns c15,"
    "   OUTER %s\"informix\".SysColumns c16"
    "\nWHERE t0.idxname = t2.idxname AND t0.tabid = t2.tabid"
    "   AND ABS(t2.part1)   = c01.colno AND t2.tabid = c01.tabid"
    "   AND ABS(t2.part2)   = c02.colno AND t2.tabid = c02.tabid"
    "   AND ABS(t2.part3)   = c03.colno AND t2.tabid = c03.tabid"
    "   AND ABS(t2.part4)   = c04.colno AND t2.tabid = c04.tabid"
    "   AND ABS(t2.part5)   = c05.colno AND t2.tabid = c05.tabid"
    "   AND ABS(t2.part6)   = c06.colno AND t2.tabid = c06.tabid"
    "   AND ABS(t2.part7)   = c07.colno AND t2.tabid = c07.tabid"
    "   AND ABS(t2.part8)   = c08.colno AND t2.tabid = c08.tabid"
    "   AND ABS(t2.part9)   = c09.colno AND t2.tabid = c09.tabid"
    "   AND ABS(t2.part10)  = c10.colno AND t2.tabid = c10.tabid"
    "   AND ABS(t2.part11)  = c11.colno AND t2.tabid = c11.tabid"
    "   AND ABS(t2.part12)  = c12.colno AND t2.tabid = c12.tabid"
    "   AND ABS(t2.part13)  = c13.colno AND t2.tabid = c13.tabid"
    "   AND ABS(t2.part14)  = c14.colno AND t2.tabid = c14.tabid"
    "   AND ABS(t2.part15)  = c15.colno AND t2.tabid = c15.tabid"
    "   AND ABS(t2.part16)  = c16.colno AND t2.tabid = c16.tabid"
    "\nORDER BY idxtype, conowner, conname";

static char info_idx_se[] =
    "SELECT t0.idxtype, t0.conowner, t0.conname, t0.idxowner, t0.idxname,"
    " c01.colname col01, c02.colname col02, c03.colname col03,"
    " c04.colname col04, c05.colname col05, c06.colname col06,"
    " c07.colname col07, c08.colname col08, c09.colname col09,"
    " c09.colname col10, c09.colname col11, c09.colname col12,"
    " c09.colname col13, c09.colname col14, c09.colname col15,"
    " c09.colname col16"
    "\nFROM _sqlcmd_info_index t0, %s\"informix\".SysIndexes t2,"
    "         %s\"informix\".SysColumns c01,"
    "   OUTER %s\"informix\".SysColumns c02,"
    "   OUTER %s\"informix\".SysColumns c03,"
    "   OUTER %s\"informix\".SysColumns c04,"
    "   OUTER %s\"informix\".SysColumns c05,"
    "   OUTER %s\"informix\".SysColumns c06,"
    "   OUTER %s\"informix\".SysColumns c07,"
    "   OUTER %s\"informix\".SysColumns c08,"
    "   OUTER %s\"informix\".SysColumns c09"
    "\nWHERE  t0.idxname = t2.idxname AND t0.tabid = t2.tabid"
    "   AND  ABS(t2.part1) = c01.colno AND t2.tabid = c01.tabid"
    "   AND  ABS(t2.part2) = c02.colno AND t2.tabid = c02.tabid"
    "   AND  ABS(t2.part3) = c03.colno AND t2.tabid = c03.tabid"
    "   AND  ABS(t2.part4) = c04.colno AND t2.tabid = c04.tabid"
    "   AND  ABS(t2.part5) = c05.colno AND t2.tabid = c05.tabid"
    "   AND  ABS(t2.part6) = c06.colno AND t2.tabid = c06.tabid"
    "   AND  ABS(t2.part7) = c07.colno AND t2.tabid = c07.tabid"
    "   AND  ABS(t2.part8) = c08.colno AND t2.tabid = c08.tabid"
    "   AND  -1            = c09.colno AND t2.tabid = c09.tabid"
    "\nORDER BY idxtype, conowner, conname";

static char probe_online[] = "SELECT part9 FROM %s\"informix\".SysIndexes";
static char probe_idsudo[] = "SELECT extended_id FROM %s\"informix\".SysColumns";

static char info_users[] =
    "SELECT username, usertype FROM \"informix\".SysUsers U"
    " WHERE UserType != 'G'"
    "   AND (%.*s)"
    " ORDER BY username";

static char info_roles[] =
    "SELECT username, usertype FROM \"informix\".SysUsers U"
    " WHERE UserType = 'G'"
    "   AND (%.*s)"
    " ORDER BY username";

static char sql_access[] =
    "SELECT ta.grantor, ta.grantee, ta.tabauth, c.colname, ca.colauth"
    "  FROM %s\"informix\".SysTabAuth TA,"
    " OUTER (%s\"informix\".SysColauth CA, %s\"informix\".SysColumns C)"
    " WHERE ta.tabid   = ca.tabid"
    "   AND ta.grantor = ca.grantor"
    "   AND ta.grantee = ca.grantee"
    "   AND ca.colno   = c.colno"
    "   AND ca.tabid   = c.tabid"
    "   AND ta.tabid   = %ld"
    " ORDER BY grantor, grantee";

/* Statement for INFO STATUS */
static char sql_status[] =
    "SELECT TabID, Tabname, Owner, TabType, RowSize, Ncols, Nrows, Nindexes, Created, Version"
    "  FROM %s\"informix\".SysTables T"
    " WHERE TabID = %ld";

/* Statement for INFO FRAGMENTS */
static char sql_fragments[] =
    "SELECT IndexName, Dbspace, FragType, ExprText, Strategy"
    "  FROM %s\"informix\".SysFragments F"
    " WHERE TabID = %ld"
    " ORDER BY IndexName, Dbspace";

/* Statement for INFO CHECK CONSTRAINTS */
static char sql_checks[] =
    "SELECT co.constrid, constrname, owner, tabid, ck.seqno, ck.checktext"
    "  FROM %s\"informix\".SysConstraints co, %s\"informix\".syschecks ck"
    " WHERE ck.constrid = co.constrid"
    "   AND co.constrtype = 'C'"
    "   AND ck.TYPE = 'T'"
    "   AND TabID = %ld"
    " ORDER BY CO.Constrname, Co.Owner, CK.seqno";

/* Deal with simple INFO queries with single replacement for WHERE clause */
static void info_where(const char *sql_fmt, const char *wh_start, const char *wh_end)
{
    int wh_len = (wh_start) ? (wh_end - wh_start) : sizeof("1=1");
    int sql_len = strlen(sql_fmt) + wh_len;
    char *sql = (char *)MALLOC(sql_len);    /*=C++=*/
    const char *wh = wh_start ? wh_start : "1=1";
    esnprintf(sql, sql_len, sql_fmt, wh_len, wh);
    (*sql_exec)(sql);
    FREE(sql);
}

/*
** Convert connection information into full table name.
** NB: quotes or absence of quotes around owner name are preserved by
** grammar so that MODE ANSI databases work correctly.  Hence, do not
** enclose owner in quotes here; it would defeat the good work done
** by the grammar.
*/

static void info_viewbody(InfoInfo *info)
{
    char buffer[1024];
    long viewid;
    char dbase[300];

    viewid = sql_tabid(info->table, info->owner, info->dbase, info->server,
                        conninfo_current_modeansi());

    if (viewid < 0)
        sql_error();
    else
    {
        sql_mkdbasename(info->dbase, info->server, dbase, sizeof(dbase));
        esnprintf(buffer, sizeof(buffer), sql_viewbody, dbase, viewid);
        TRACE((3, "info_viewbody: %s\n", buffer));
        (*sql_exec)(buffer);
    }
}

static void info_trigbody(InfoInfo *info)
{
    char buffer[1024];
    long trigid;
    char dbase[300];

    trigid = sql_trigid(info->table, info->owner, info->dbase, info->server,
                        conninfo_current_modeansi());

    if (trigid < 0)
        sql_error();
    else
    {
        sql_mkdbasename(info->dbase, info->server, dbase, sizeof(dbase));
        esnprintf(buffer, sizeof(buffer), sql_trigbody, dbase, trigid);
        TRACE((3, "info_trigbody: %s\n", buffer));
        (*sql_exec)(buffer);
    }
}

static void info_procbody(InfoInfo *info)
{
    char buffer[1024];
    long procid;
    char dbase[300];

    procid = sql_procid(info->table, info->owner, info->dbase, info->server,
                        conninfo_current_modeansi());

    if (procid < 0)
        sql_error();
    else
    {
        sql_mkdbasename(info->dbase, info->server, dbase, sizeof(dbase));
        esnprintf(buffer, sizeof(buffer), sql_procbody, dbase, procid);
        TRACE((3, "info_procbody: %s\n", buffer));
        (*sql_exec)(buffer);
    }
}

/* Check whether given statement can be prepared */
/* Return 1 if it can; 0 otherwise. */
/* Used to probe for OnLine versus SE and IUS versus OnLine */
static int probe_dbs(const char *stmt, const char *dbase)
{
    EXEC SQL BEGIN DECLARE SECTION;
    char buffer[8196];
    EXEC SQL END DECLARE SECTION;
    int status = 0;

    esnprintf(buffer, sizeof(buffer), stmt, dbase);
    EXEC SQL PREPARE p_probe FROM :buffer;
    if (sqlca.sqlcode == 0)
    {
        /* Probe successful */
        EXEC SQL FREE p_probe;
        status = 1;
    }
    return(status);
}

/*
** Convert connection information into full table name.
** NB: quotes or absence of quotes around owner name are preserved by
** grammar so that MODE ANSI databases work correctly.  Hence, do not
** enclose owner in quotes here; it would defeat the good work done
** by the grammar.
*/

static void info_columns(InfoInfo *info)
{
    char buffer[1024];
    long tabid;
    char dbase[300];
    const char *sql_columns;

    TRACE((3, "info_columns: owner = <<%s>>\n", info->owner));
    TRACE((3, "info_columns: table = <<%s>>\n", info->table));
    tabid = sql_tabid(info->table, info->owner, info->dbase, info->server,
                        conninfo_current_modeansi());

    if (tabid < 0)
        sql_error();
    else
    {
        sql_mkdbasename(info->dbase, info->server, dbase, sizeof(dbase));
        if (probe_dbs(probe_idsudo, dbase))
            sql_columns = sql_columns_idsudo;
        else
            sql_columns = sql_columns_online;
        esnprintf(buffer, sizeof(buffer), sql_columns, dbase, tabid);
        TRACE((3, "info_columns: %s\n", buffer));
        (*sql_exec)(buffer);
    }
}

/*
** Convert connection information into full table name.
** NB: quotes or absence of quotes around owner name are preserved by
** grammar so that MODE ANSI databases work correctly.  Hence, do not
** enclose owner in quotes here; it would defeat the good work done
** by the grammar.
*/

static void info_refs_by(InfoInfo *info)
{
    char buffer[1024];
    long tabid;
    char dbase[300];

    tabid = sql_tabid(info->table, info->owner, info->dbase, info->server,
                        conninfo_current_modeansi());

    if (tabid < 0)
        sql_error();
    else
    {
        sql_mkdbasename(info->dbase, info->server, dbase, sizeof(dbase));
        esnprintf(buffer, sizeof(buffer), sql_references, dbase, dbase, dbase, dbase, dbase, "T2", tabid);
        TRACE((3, "info_refs_by: %s\n", buffer));
        (*sql_exec)(buffer);
    }
}

static void info_refs_to(InfoInfo *info)
{
    char buffer[1024];
    long tabid;
    char dbase[300];

    tabid = sql_tabid(info->table, info->owner, info->dbase, info->server,
                        conninfo_current_modeansi());

    if (tabid < 0)
        sql_error();
    else
    {
        sql_mkdbasename(info->dbase, info->server, dbase, sizeof(dbase));
        esnprintf(buffer, sizeof(buffer), sql_references, dbase, dbase, dbase, dbase, dbase, "T1", tabid);
        TRACE((3, "info_refs_to: %s\n", buffer));
        (*sql_exec)(buffer);
    }
}

static void info_access(InfoInfo *info)
{
    char buffer[1024];
    long tabid;
    char dbase[300];

    tabid = sql_tabid(info->table, info->owner, info->dbase, info->server,
                        conninfo_current_modeansi());

    if (tabid < 0)
        sql_error();
    else
    {
        sql_mkdbasename(info->dbase, info->server, dbase, sizeof(dbase));
        esnprintf(buffer, sizeof(buffer), sql_access, dbase, dbase, dbase, tabid);
        TRACE((3, "info_access: %s\n", buffer));
        (*sql_exec)(buffer);
    }
}

static void info_status(InfoInfo *info)
{
    char buffer[1024];
    long tabid;
    char dbase[300];

    tabid = sql_tabid(info->table, info->owner, info->dbase, info->server,
                        conninfo_current_modeansi());

    if (tabid < 0)
        sql_error();
    else
    {
        sql_mkdbasename(info->dbase, info->server, dbase, sizeof(dbase));
        esnprintf(buffer, sizeof(buffer), sql_status, dbase, tabid);
        TRACE((3, "info_status: %s\n", buffer));
        (*sql_exec)(buffer);
    }
}

static void info_fragments(InfoInfo *info)
{
    char buffer[1024];
    long tabid;
    char dbase[300];

    tabid = sql_tabid(info->table, info->owner, info->dbase, info->server,
                        conninfo_current_modeansi());

    if (tabid < 0)
        sql_error();
    else
    {
        sql_mkdbasename(info->dbase, info->server, dbase, sizeof(dbase));
        esnprintf(buffer, sizeof(buffer), sql_fragments, dbase, tabid);
        TRACE((3, "info_fragments: %s\n", buffer));
        (*sql_exec)(buffer);
    }
}

static void info_checks(InfoInfo *info)
{
    char buffer[1024];
    long tabid;
    char dbase[300];

    tabid = sql_tabid(info->table, info->owner, info->dbase, info->server,
                        conninfo_current_modeansi());

    if (tabid < 0)
        sql_error();
    else
    {
        sql_mkdbasename(info->dbase, info->server, dbase, sizeof(dbase));
        esnprintf(buffer, sizeof(buffer), sql_checks, dbase, dbase, tabid);
        TRACE((3, "info_checks: %s\n", buffer));
        (*sql_exec)(buffer);
    }
}

static void info_triggers(InfoInfo *info)
{
    char buffer[1024];
    long tabid;
    char dbase[300];

    tabid = sql_tabid(info->table, info->owner, info->dbase, info->server,
                        conninfo_current_modeansi());

    if (tabid < 0)
        sql_error();
    else
    {
        sql_mkdbasename(info->dbase, info->server, dbase, sizeof(dbase));
        esnprintf(buffer, sizeof(buffer), sql_triggers, dbase, dbase, tabid, tabid);
        TRACE((3, "info_triggers: %s\n", buffer));
        (*sql_exec)(buffer);
    }
}

static void info_indexes(InfoInfo * info)
{
    EXEC SQL BEGIN DECLARE SECTION;
    char buffer[8196];
    EXEC SQL END DECLARE SECTION;
    long tabid;
    char dbase[300];

    tabid = sql_tabid(info->table, info->owner, info->dbase, info->server,
                        conninfo_current_modeansi());

    if (tabid < 0)
        sql_error();
    else
    {
        ctxt_newcontext();
        ctxt_setcontinue(op_push);
        ctxt_setcontinue(op_on);
        ctxt_setsilence(op_push);
        ctxt_setsilence(op_on);
        (*sql_exec)(info_idx_drop);
        ctxt_setsilence(op_pop);
        ctxt_setcontinue(op_pop);

        sql_mkdbasename(info->dbase, info->server, dbase, sizeof(dbase));
        esnprintf(buffer, sizeof(buffer), info_idx_mktemp, dbase, tabid, dbase, tabid, dbase,
                dbase, tabid);
        TRACE((3, "info_indexes: %s\n", buffer));
        (*sql_exec)(buffer);
        if (probe_dbs(probe_online, dbase))
        {
            /* OnLine or IDS/UDO */
            esnprintf(buffer, sizeof(buffer), info_idx_online, dbase, dbase, dbase, dbase, dbase,
                    dbase, dbase, dbase, dbase, dbase, dbase, dbase, dbase,
                    dbase, dbase, dbase, dbase);
        }
        else
        {
            /* Presumably SE */
            esnprintf(buffer, sizeof(buffer), info_idx_se, dbase, dbase, dbase, dbase, dbase,
                    dbase, dbase, dbase, dbase, dbase);
        }
        TRACE((3, "info_indexes: %s\n", buffer));
        (*sql_exec)(buffer);
        ctxt_setcontinue(op_push);
        ctxt_setcontinue(op_on);
        ctxt_setsilence(op_push);
        ctxt_setsilence(op_on);
        (*sql_exec)(info_idx_drop);
        ctxt_endcontext();
    }
}

void     sql_info(char *str)
{
    InfoInfo stmt;

    switch (parse_infostmt(str, &stmt))
    {
    case INFO_TABLES:
        info_where(info_tables, stmt.wh_start, stmt.wh_end);
        break;

    case INFO_SYSTABLES:
        info_where(info_systables, stmt.wh_start, stmt.wh_end);
        break;

    case INFO_COLUMNS:
        info_columns(&stmt);
        break;

    case INFO_PROCBODY:
        info_procbody(&stmt);
        break;

    case INFO_PROCEDURES:
        info_where(info_procedures, stmt.wh_start, stmt.wh_end);
        break;

    case INFO_VIEWS:
        info_where(info_views, stmt.wh_start, stmt.wh_end);
        break;

    case INFO_VIEWBODY:
        info_viewbody(&stmt);
        break;

    case INFO_BASETABLES:
        info_where(info_basetables, stmt.wh_start, stmt.wh_end);
        break;

    case INFO_SYNONYMS:
        info_where(info_synonyms, stmt.wh_start, stmt.wh_end);
        break;

    case INFO_INDEXES:
        info_indexes(&stmt);
        break;

    case INFO_ACCESS: /* aka PRIVILEGES */
        info_access(&stmt);
        break;

    case INFO_DATABASES:
        sql_dbnames();
        break;

    case INFO_CONNECTIONS:
        conninfo_print();
        break;

    case INFO_REFS_BY:
        info_refs_by(&stmt);
        break;

    case INFO_REFS_TO:
        info_refs_to(&stmt);
        break;

    case INFO_STATUS:
        info_status(&stmt);
        break;

    case INFO_FRAGMENTS:
        info_fragments(&stmt);
        break;

    case INFO_CONSTR_CHECK:
        info_checks(&stmt);
        break;

    case INFO_TRIGGERS:
        info_triggers(&stmt);
        break;

    case INFO_TRIGBODY:
        info_trigbody(&stmt);
        break;

    case INFO_USERS:
        info_where(info_users, stmt.wh_start, stmt.wh_end);
        break;

    case INFO_ROLES:
        info_where(info_roles, stmt.wh_start, stmt.wh_end);
        break;

    case INFO_HELP:
        fprintf(ctxt_output(),
                "-- Help for the INFO command:\n"
                "-- INFO ACCESS FOR table (aka INFO PRIVILEGES)\n"
                "-- INFO BASETABLES [WHERE ...]\n"
                "-- INFO CHECK CONSTRAINTS FOR table\n"
                "-- INFO COLUMNS FOR table\n"
                "-- INFO CONNECTIONS\n"
                "-- INFO DATABASES\n"
                "-- INFO FRAGMENTS FOR table\n"
                "-- INFO HELP\n"
                "-- INFO INDEXES FOR table\n"
                "-- INFO PROCBODY FOR procname\n"
                "-- INFO PROCEDURES [WHERE ...]\n"
                "-- INFO REFERENCES BY table\n"
                "-- INFO REFERENCES TO table\n"
                "-- INFO ROLES [WHERE ...]\n"
                "-- INFO STATUS FOR table\n"
                "-- INFO SYNONYMS [WHERE ...]\n"
                "-- INFO SYSTABLES [WHERE ...]\n"
                "-- INFO TABLES [WHERE ...]\n"
                "-- INFO TRIGBODY FOR trigger\n"
                "-- INFO TRIGGERS FOR table\n"
                "-- INFO USERS [WHERE ...]\n"
                "-- INFO VIEWBODY for view\n"
                "-- INFO VIEWS [WHERE ...]\n"
                );
        break;

    case INFO_ERROR:
        error_746("Syntax error in INFO statement");
        break;

    default:
        error_746("Internal error handling INFO statement");
        break;
    }
}
