/*
@(#)File:           $RCSfile: upload.ec,v $
@(#)Version:        $Revision: 2013.1 $
@(#)Last changed:   $Date: 2013/09/08 23:16:54 $
@(#)Purpose:        ESQL/C code for SQLUPLOAD
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 1998-2001,2003-06,2008,2010,2012
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

/* Rampant SVR4-ism (and SVR1-ism, SVR2-ism, SVR3-ism) */
#include <search.h> /* lfind() */

#include "connect.h"
#include "context.h"
#include "debug.h"
#include "emalloc.h"
#include "esqlc.h"
#include "describe.h" /* Must be after esqlc.h */
#include "esqlutil.h"
#include "sqlownobj.h"
#include "jlss.h"
#include "jlsstools.h"
#include "esnprintf.h"
#include "kludge.h"
#include "sqlcmd.h"
#include "sqlerr.h"
#include "stderr.h"
#include "upload.h"
#include "usesqlda.h"
#include "dumpesql.h"

struct ColList
{
    char   *string;             /* Single string containing column names */
    char  **names;              /* Array of pointers to names in string */
    size_t  count;              /* Number of pointers in names array */
};
typedef struct ColList ColList;

struct SqlStmt
{
    char        *stmttext;
    size_t       stmtsize;
    const char  *stmtname;
    const char  *crsrname;
    Sqlda *idesc;
};
typedef struct SqlStmt SqlStmt;

/* UPLOAD data working data */
struct Upload
{
    KeyType keytype;        /* Type of key (unique or non-unique) */
    size_t stop_after;      /* Errors allowed before stopping */
    size_t log_counter;     /* Rows reported by log_heuristic() */
    size_t rows_read;       /* Number of data rows read */
    size_t fmt_errors;      /* Number of rows with a format error */
    size_t sql_errors;      /* Number of rows causing an SQL error */
    size_t updates_exec;    /* Number of UPDATE statements executed */
    size_t inserts_exec;    /* Number of INSERT statements executed */
    size_t selects_exec;    /* Number of SELECT statements executed */
    size_t rows_updated;    /* Number of rows actually updated */
    size_t rows_inserted;   /* Number of rows actually inserted */
    size_t rows_selected;   /* Number of rows actually selected */
    char fulltable[256];    /* Complete table name */
    FILE *lgfp;             /* Log file */
    FILE *rjfp;             /* Reject file */
    Sqlda *scn_sqlda;       /* Description of input date */
    SqlStmt record;         /* SCAN SELECT statement */
    SqlStmt insert;         /* INSERT statement */
    SqlStmt update;         /* UPDATE statement */
    SqlStmt select;         /* SELECT statement */
};
typedef struct Upload Upload;

static Volume volume = V_QUIET;

enum { HEURISTIC_SYMBOLS_PER_LINE = 60 };

#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
const char jlss_id_upload_ec[] = "@(#)$Id: upload.ec,v 2013.1 2013/09/08 23:16:54 jleffler Exp $";
#endif /* lint */

/* -- Database Connections -- */

/* Make connection to database */
int db_connect(ConnInfo *conn)
{
    EXEC SQL BEGIN DECLARE SECTION;
    char *database = conn->database;
    EXEC SQL END DECLARE SECTION;

    TRACE((3, "-->> db_connect():\n"));
$ifdef ESQLC_CONNECT;
    EXEC SQL BEGIN DECLARE SECTION;
    char *username = conn->username;
    char *password = conn->password;
    char *connname = conn->connname;
    EXEC SQL END DECLARE SECTION;

    if (username != 0 && password != 0)
    {
        EXEC SQL CONNECT TO :database AS :connname
            USER :username USING :password;
    }
    else
    {
        EXEC SQL CONNECT TO :database AS :connname;
    }
$else;
    EXEC SQL DATABASE :database;
$endif; /* ESQLC_CONNECT */

    if (sqlca.sqlcode != 0)
        sql_printerror(ctxt_error());
    else
    {
        conn->online    |= (sqlca.sqlwarn.sqlwarn3 == 'W') ? IS_ONLINE_DB: 0;
        conn->mode_ansi |= (sqlca.sqlwarn.sqlwarn2 == 'W') ? IS_MODE_ANSI: 0;
        conn->logged    |= (sqlca.sqlwarn.sqlwarn1 == 'W') ? IS_LOGGED_DB: 0;
    }
    TRACE((3, "<<-- db_connect(): %ld\n", sqlca.sqlcode));
    return sqlca.sqlcode;
}

/* Break connection to database */
int db_disconnect(ConnInfo *conn)
{
    /* In case we're dealing with a MODE ANSI database and did nothing */
    EXEC SQL ROLLBACK WORK;

$ifdef ESQLC_CONNECT;
    {
    EXEC SQL BEGIN DECLARE SECTION;
    char *connname = conn->connname;
    EXEC SQL END DECLARE SECTION;
    EXEC SQL DISCONNECT :connname;
    }
$else;
    EXEC SQL CLOSE DATABASE;
$endif; /* ESQLC_CONNECT */

    if (sqlca.sqlcode != 0)
        sql_printerror(ctxt_error());
    return sqlca.sqlcode;
}

/* -- Column Lists -- */

/* Count number of occurrences of specified character in string */
static int count_char(const char *str, char match)
{
    int count = 0;
    char c;

    while ((c = *str++) != '\0')
    {
        if (c == match)
            count++;
    }
    return count;
}

/* String comparison for bsearch() or qsort() */
static int qs_compare(const void *s1, const void *s2)
{
    const char **c1 = (const char **)s1;
    const char **c2 = (const char **)s2;
    return(cistrcmp(*c1, *c2));
}

/* Remove column names that start with a plus */
static void remove_nonpluscols(ColList *list)
{
    int i;
    int j;
    int n = list->count;

    for (j = 0, i = 0; i < n; i++)
    {
        if (list->names[i][0] == '+')
            list->names[j++] = list->names[i] + 1;
        else
            list->count--;
        /* There must be at least one column name without a plus */
        assert(list->count > 0);
    }
}

/* 'Constructor' for ColList type */
static int create_collist(char *str, ColList *list)
{
    char **colp;
    char *s;
    int i;
    int n;

    list->string = 0;
    list->names = 0;
    list->count = 0;

    list->string = STRDUP(str);

    /* Count commas separating key columns; add one for number of columns */
    n = count_char(list->string, ',') + 1;
    if ((list->names = (char **)malloc(n * sizeof(char *))) == 0) /*=C++=*/
    {
        free(list->string);
        list->string = 0;
        return -1;
    }
    colp = list->names;
    s = list->string;
    for (i = 0; i < n; i++)
    {
        assert(s != 0 && *s != '\0');
        *colp++ = s;
        s = strchr(s, ',');
        if (s)
            *s++ = '\0';
    }
    assert(s == 0);
    list->count = n;

    return 0;
}

/* Get the SQLDA structure for the columns in the table. */
static Sqlda *get_sqlda(const char *tabname, const char *p_stmt, Sqlda **ptr)
{
    Sqlda *idesc = 0;
    EXEC SQL BEGIN DECLARE SECTION;
    char buffer[512];
    const char *stmt = p_stmt;
    EXEC SQL END DECLARE SECTION;

    esnprintf(buffer, sizeof(buffer), "SELECT * FROM %s", tabname);
    EXEC SQL PREPARE :stmt FROM :buffer;
    if (sqlca.sqlcode != 0)
        sql_printerror(ctxt_error());
    else
    {
        EXEC SQL DESCRIBE :stmt INTO idesc;
        if (sqlca.sqlcode < 0)
            sql_printerror(ctxt_error());
        assert(sqlca.sqlcode == 0);
        *ptr = idesc;
    }
    return idesc;
}

/* Generate column list from the SELECT statement */
static int generate_collist(const char *tabname, ColList *list)
{
    Sqlda *idesc;
    int totlen;
    char *str;
    int i;
    EXEC SQL BEGIN DECLARE SECTION;
    const char stmt[] = "__sqlupload_selall";
    EXEC SQL END DECLARE SECTION;

    list->string = 0;
    list->names = 0;
    list->count = 0;

    if (get_sqlda(tabname, stmt, &idesc) == 0)
        return -1;

    totlen = 0;
    for (i = 0; i < idesc->sqld; i++)
        totlen += strlen(idesc->sqlvar[i].sqlname) + 1;
    list->string = (char *)malloc(totlen * sizeof(char)); /*=C++=*/
    if (list->string == 0)
    {
        EXEC SQL FREE :stmt;
        return -1;
    }
    list->names = (char **)malloc(idesc->sqld * sizeof(char *)); /*=C++=*/
    if (list->names == 0)
    {
        free(list->string);
        list->string = 0;
        EXEC SQL FREE :stmt;
        return -1;
    }
    str = list->string;
    for (i = 0; i < idesc->sqld; i++)
    {
        list->names[i] = str;
        str = vstrcpy(str, 1, idesc->sqlvar[i].sqlname) + 1;
    }
    EXEC SQL FREE :stmt;
    list->count = idesc->sqld;
    qsort(list->names, list->count, sizeof(char *), qs_compare);
    return 0;
}

static void delete_collist(ColList *list)
{
    free(list->string);
    free(list->names);
    list->string = 0;
    list->names = 0;
    list->count = 0;
}

/* Restore column list structure to unsorted order */
static void reset_collist(ColList *list)
{
    size_t i;
    char *p;

    assert(list->string != 0);
    assert(list->names != 0);
    assert(list->count != 0);
    p = list->string;
    for (i = 0; i < list->count; i++)
    {
        list->names[i] = p;
        p += strlen(p) + 1;
    }
}

static int validate_collist(ColList *list, const char *str)
{
    int i;
    int n = list->count - 1;

    qsort(list->names, list->count, sizeof(char *), qs_compare);
    for (i = 0; i < n - 1; i++)
    {
        if (list->names[i][0] == '\0')
        {
            err_remark("empty name found in %s list.\n", str);
            return -1;
        }
        if (cistrcmp(list->names[i], list->names[i+1]) == 0)
        {
            err_remark("duplicate name entry (%s) found in %s list.\n",
                        list->names[i], str);
            return -1;
        }
    }
    return 0;
}

static void print_collist(FILE *fp, const ColList *list, const char *tag1, const char *tag2)
{
    size_t i;

    fprintf(fp, "%s\n", tag1);
    for (i = 0; i < list->count; i++)
        fprintf(fp, "%s[%lu] = <<%s>>\n", tag2, (unsigned long)(i + 1), list->names[i]);
}

/* Check that columns in key list are all listed in column list */
static int compare_key_col(ColList *key, ColList *col)
{
    size_t i;
    int errs = 0;

    for (i = 0; i < key->count; i++)
    {
        if (bsearch(&key->names[i], col->names, col->count, sizeof(char *), qs_compare) == 0)
        {
            errs++;
            err_remark("key column '%s' is not listed in column list\n",
                        key->names[i]);
        }
    }
    if (errs == 0 && key->count >= col->count)
    {
        assert(key->count == col->count);
        errs++;
        err_remark("the key column list is the same as the column list\n");
    }
    return (errs > 0) ? -1 : 0;
}

/* -- Validation Routines -- */

static int num_unique_indexes(long p_tabid)
{
    EXEC SQL BEGIN DECLARE SECTION;
    long tabid = p_tabid;
    int nidx;
    EXEC SQL END DECLARE SECTION;

    EXEC SQL SELECT COUNT(*)
                INTO :nidx
                FROM "informix".SysIndexes
                WHERE Tabid = :tabid
                  AND IdxType = 'U';

    return(nidx);
}

static void get_key_columns(long tabid, char **keycols)
{
    error_746("get_key_columns() not implemented yet");
    exit(EXIT_FAILURE);
}

/* Format full table name from table name and optional owner */
/* !Unaware of DELIMIDENT and owner """utter bastard"""! */
static void set_fulltablename(char *table, char *owner, char *fulltable, size_t fulltab_size)
{
    if (owner)
        esnprintf(fulltable, fulltab_size, "\"%s\".%s", owner, table);
    else
        esnprintf(fulltable, fulltab_size, "%s", table);
}

/* Check whether given table exists by preparing 'SELECT * FROM table' */
static long table_exists(char *fulltable)
{
    EXEC SQL BEGIN DECLARE SECTION;
    char buffer[512];
    EXEC SQL END DECLARE SECTION;
    long exists = 0;

    TRACE((3, "-->> table_exists():\n"));
    esnprintf(buffer, sizeof(buffer), "SELECT * FROM %s", fulltable);
    if (ctxt_gettrace())
        fprintf(ctxt_error(), "+ %s\n", buffer);
    EXEC SQL PREPARE p_selectall FROM :buffer;
    if (sqlca.sqlcode != 0)
        sql_printerror(ctxt_error());
    else
    {
        EXEC SQL FREE p_selectall;
        exists = 1;
    }

    TRACE((3, "<<-- table_exists(): %ld\n", exists));
    return exists;
}

static int check_columns_in_table(char *fulltable, ColList *list)
{
    size_t          i;
    int             errs = 0;
    int             j;
    int             found;
    Sqlda          *idesc;
    char           *name;
    EXEC SQL BEGIN DECLARE SECTION;
    const char stmt[] = "__sqlupload_selall";
    EXEC SQL END DECLARE SECTION;

    if (get_sqlda(fulltable, stmt, &idesc) == 0)
        return -1;

    errs = 0;
    for (i = 0; i < list->count; i++)
    {
        name = list->names[i];
        if (*name == '+')
            name++;
        found = 0;
        for (j = 0; j < idesc->sqld; j++)
        {
            if (cistrcmp(name, idesc->sqlvar[j].sqlname) == 0)
            {
                found = 1;
                break;
            }
        }
        if (found == 0)
        {
            errs++;
            err_remark("table %s does not contain column %s\n",
                       fulltable, name);
        }
    }

    EXEC SQL FREE :stmt;

    return (errs > 0) ? -1 : 0;
}

/* Create the SELECT statement */
static int
make_select_stmt(ColList *key, ColList *col, char *fulltable, Sqlda *scn_sqlda, SqlStmt *stmt)
{
    Sqlda *idesc;
    char  colinfo[20];
    char *end;
    char *max = stmt->stmttext + stmt->stmtsize;
    const char *pad;
    size_t   i;
    size_t   j;

    assert(stmt != 0 && stmt->idesc == 0 && scn_sqlda != 0);

    idesc = stmt->idesc = alloc_sqlda(key->count);
    if (idesc == 0)
        return -1;

    end = vstrcpy(stmt->stmttext, 2, "SELECT 1 FROM ", fulltable);

    /* Create the WHERE clause. */
    pad = "\n    WHERE ";
    j = 0;
    for (i = 0; i < key->count; i++)
    {
        char **ent;
        ent = (char **)lfind(&key->names[i], col->names, &col->count, sizeof(char *), qs_compare); /*=C++=*/
        assert(ent != 0);
        idesc->sqlvar[j++] = scn_sqlda->sqlvar[ent - col->names];
        esnprintf(colinfo, sizeof(colinfo), "{%lu}", (unsigned long)((ent - col->names) + 1));
        end = vstrcpy(end, 4, pad, key->names[i], colinfo, " = ?");
        pad = "\n      AND ";
    }

    assert(end < max);
    assert(j <= (size_t)scn_sqlda->sqld);
    return 0;
}

/* Create the UPDATE statement */
static int
make_update_stmt(ColList *key, ColList *col, char *fulltable, Sqlda *scn_sqlda, SqlStmt *update)
{
    Sqlda *idesc;
    char  colinfo[20];
    char *end;
    char *max = update->stmttext + update->stmtsize;
    char *name;
    const char *pad;
    size_t   i;
    size_t   j;

    assert(update != 0 && update->idesc == 0 && scn_sqlda != 0);

    /* Over-estimate of data needed */
    idesc = update->idesc = alloc_sqlda(scn_sqlda->sqld);
    if (idesc == 0)
        return -1;

    end = vstrcpy(update->stmttext, 3, "UPDATE ", fulltable, "\nSET (");

    /* Create the SET clause. */
    /* Add the various column names in col and not in key */
    pad = "";
    j = 0;
    for (i = 0; i < col->count; i++)
    {
        if (bsearch(&col->names[i], key->names, key->count, sizeof(char *), qs_compare) == 0)
        {
            idesc->sqlvar[j++] = scn_sqlda->sqlvar[i];
            name = col->names[i];
            if (*name == '+')
            {
                name++;
                esnprintf(colinfo, sizeof(colinfo), " {K:%lu}", (unsigned long)(i + 1));
            }
            else
                esnprintf(colinfo, sizeof(colinfo), " {%lu}", (unsigned long)(i + 1));
            end = vstrcpy(end, 3, pad, name, colinfo);
            pad = ", ";
        }
        if (end > max)
            break;
    }

    end = vstrcpy(end, 1, ") = (");
    pad = "";
    for (i = 0; i < col->count - key->count; i++)
    {
        end = vstrcpy(end, 2, pad, "?");
        pad = ",";
        if (end > max)
            break;
    }
    end = vstrcpy(end, 1, ")\n");

    reset_collist(key);
    pad = "WHERE ";
    for (i = 0; i < key->count; i++)
    {
        char **ent = (char **)lfind(&key->names[i], col->names, &col->count, sizeof(char *), qs_compare); /*=C++=*/
        assert(ent != 0);
        idesc->sqlvar[j++] = scn_sqlda->sqlvar[ent - col->names];
        esnprintf(colinfo, sizeof(colinfo), "{%lu}", (unsigned long)((ent - col->names) + 1));
        end = vstrcpy(end, 4, pad, key->names[i], colinfo, " = ?");
        pad = " AND ";
    }
    assert(end < max);
    assert(j <= (size_t)scn_sqlda->sqld);
    return 0;
}

/* Create the INSERT statement */
static int
make_insert_stmt(ColList *col, ColList *upd, char *fulltable, Sqlda *scn_sqlda, SqlStmt *insert)
{
    Sqlda *idesc = 0;
    char  colinfo[20];
    char *end;
    char *max = insert->stmttext + insert->stmtsize;
    char *name;
    const char *pad;
    size_t   i;
    size_t   j;
    size_t   n;

    assert(insert != 0 && insert->idesc == 0 && scn_sqlda != 0);

    /* Over-estimate of data needed */
    idesc = insert->idesc = alloc_sqlda(scn_sqlda->sqld);
    if (idesc == 0)
        return -1;

    end = vstrcpy(insert->stmttext, 3, "INSERT INTO ", fulltable, "(");
    /*
    ** Add the various column names in col, omitting those in upd
    ** without a plus in front of the name.
    ** The test on upd->count is to avoid problems with Sun Solaris 2.5.1.
    ** That bsearch() makes one comparison when both the number of elements
    ** in the array is zero and the base address is zero; it makes no
    ** comparisons when the number of elements is zero and the base address
    ** is valid.  Since the ColList has a null pointer when the number of
    ** elements is zero, the comparison fails with a core dump when the -c
    ** option is not specified on the SQLUPLOAD command line.
    */
    pad = "";
    j = 0;
    for (i = 0; i < col->count; i++)
    {
        name = col->names[i];
        if (*name == '+')
            name++;
        else if (upd->count > 0)
        {
            if (bsearch(&name, upd->names, upd->count, sizeof(char *), qs_compare) != 0)
                continue;
        }
        esnprintf(colinfo, sizeof(colinfo), " {%lu}", (unsigned long)(i+1));
        idesc->sqlvar[j++] = scn_sqlda->sqlvar[i];
        end = vstrcpy(end, 3, pad, name, colinfo);
        pad = ", ";
        if (end >= max)
            break;
    }
    idesc->sqld = j;

    end = vstrcpy(end, 1, ")\nVALUES (");
    pad = "";
    n = col->count - upd->count;
    assert(n > 0);
    for (i = 0; i < n; i++)
    {
        end = vstrcpy(end, 2, pad, "?");
        pad = ",";
        if (end >= max)
            break;
    }
    end = vstrcpy(end, 1, ")\n");
    assert(end < max);
    return 0;
}

static int make_scan_sqlda(ColList *col, char *fulltable, SqlStmt *stmt)
{
    EXEC SQL BEGIN DECLARE SECTION;
    char *s_text = stmt->stmttext;
    const char *s_name = stmt->stmtname;
    EXEC SQL END DECLARE SECTION;
    Sqlda *idesc;
    size_t i;
    char *dst = s_text;
    const char *pad;
    char *name;

    dst = vstrcpy(dst, 1, "SELECT ");
    pad = "";
    for (i = 0; i < col->count; i++)
    {
        name = col->names[i];
        if (*name == '+')
            name++;
        dst = vstrcpy(dst, 2, pad, name);
        pad = ", ";
    }
    dst = vstrcpy(dst, 2, " FROM ", fulltable);

    TRACE((3, "SCAN: %s\n", s_text));
    EXEC SQL PREPARE :s_name FROM :s_text;
    if (sqlca.sqlcode != 0)
    {
        sql_printerror(ctxt_error());
        return -1;
    }
    EXEC SQL DESCRIBE :s_name INTO idesc;
    if (sqlca.sqlcode != 0)
    {
        sql_printerror(ctxt_error());
        return -1;
    }
    stmt->idesc = idesc;
    return 0;
}


/* -- UPLOAD Operation Control Routines -- */
static void log_heuristic(HMode c, Upload *upld)
{
    if (c == H_INSERT || c == H_UPDATE)
        upd_heuristic(HMode_to_char(c));
    if (volume == V_VERBOSE)
    {
        putc((char)c, ctxt_output());
        upld->log_counter++;
        if (upld->log_counter % HEURISTIC_SYMBOLS_PER_LINE == 0 && c != H_NEWLINE)
            putc('\n', ctxt_output());
    }
}

/* Log rejected data.  NB: mem_data(line) ends with newline */
static void upld_rej_line(FILE *fp, size_t recnum, Memory *line)
{
    fprintf(fp, "%lu:\n%s", (unsigned long)recnum, mem_data(line));
}

static void upld_log_error(FILE *fp, size_t recnum)
{
    err_logmsg(fp, ERR_REM, 0, "Failed to upload record number %lu\n",
               (unsigned long)recnum);
    sql_printerror(fp);
}

static int new_upld_error(Upload *upld, Memory *line)
{
    if (upld->rjfp)
        upld_rej_line(upld->rjfp, upld->rows_read, line);
    upld->sql_errors++;
    upld_log_error(ctxt_error(), upld->rows_read);
    if (upld->lgfp)
    {
        upld_log_error(upld->lgfp, upld->rows_read);
        upld_rej_line(upld->lgfp, upld->rows_read, line);
    }
    if (upld->stop_after > 0 && upld->sql_errors >= upld->stop_after)
    {
        error_746("Too many errors -- stopping");
        EXEC SQL ROLLBACK WORK;
        return(-1);
    }
    return(0);
}

static int prep_locktable(const LockMode lockmode, const char *fulltable)
{
    EXEC SQL BEGIN DECLARE SECTION;
    char buffer[512];
    EXEC SQL END DECLARE SECTION;
    int sqlerr;

    esnprintf(buffer, sizeof(buffer), "LOCK TABLE %s IN %s MODE", fulltable, lockmode);
    EXEC SQL PREPARE p_locktable FROM :buffer;
    if (sqlca.sqlcode < 0)
    {
        sql_printerror(ctxt_error());
        return(sqlca.sqlcode);
    }

    esnprintf(buffer, sizeof(buffer), "UNLOCK TABLE %s", fulltable);
    EXEC SQL PREPARE p_unlocktable FROM :buffer;
    if (sqlca.sqlcode < 0)
    {
        sql_printerror(ctxt_error());
        sqlerr = sqlca.sqlcode;
        EXEC SQL FREE p_locktable;
        return(sqlerr);
    }
    return(0);
}

static void free_locktable(void)
{
    EXEC SQL FREE p_locktable;
    EXEC SQL FREE p_unlocktable;
}

static int begin_work(DbsType dbs_type)
{
    if ((dbs_type & IS_LOGGED_DB) && !(dbs_type & IS_MODE_ANSI))
    {
        EXEC SQL BEGIN WORK;
        if (sqlca.sqlcode < 0)
        {
            sql_printerror(ctxt_error());
            return(-1);
        }
    }
    EXEC SQL EXECUTE p_locktable;
    if (sqlca.sqlcode < 0)
    {
        sql_printerror(ctxt_error());
        return(-1);
    }
    return(0);
}

static int commit_work(DbsType dbs_type)
{
    if (dbs_type & IS_LOGGED_DB)
    {
        EXEC SQL COMMIT WORK;
    }
    else
    {
        EXEC SQL EXECUTE p_unlocktable;
    }
    if (sqlca.sqlcode < 0)
    {
        sql_printerror(ctxt_error());
        return(-1);
    }
    return(0);
}

static int stmt_prepare(SqlStmt *stmt)
{
    EXEC SQL BEGIN DECLARE SECTION;
    const char *s_name = stmt->stmtname;
    char *s_text = stmt->stmttext;
    EXEC SQL END DECLARE SECTION;
    EXEC SQL PREPARE :s_name FROM :s_text;
    return(sqlca.sqlcode);
}

static int stmt_declare(SqlStmt *stmt)
{
    EXEC SQL BEGIN DECLARE SECTION;
    const char *s_stmt = stmt->stmtname;
    const char *s_crsr = stmt->crsrname;
    EXEC SQL END DECLARE SECTION;
    EXEC SQL DECLARE :s_crsr CURSOR FOR :s_stmt;
    return(sqlca.sqlcode);
}

static void stmt_execute(SqlStmt *stmt)
{
    EXEC SQL BEGIN DECLARE SECTION;
    const char *s_name = stmt->stmtname;
    EXEC SQL END DECLARE SECTION;
    Sqlda *idesc = stmt->idesc;
    TRACE((3, "-->> stmt_execute('%s')\n", stmt->stmttext));
    EXEC SQL EXECUTE :s_name USING DESCRIPTOR idesc;
    if (sqlca.sqlcode != 0)
        TRACE((1, "stmt_execute: SQL %d\n", sqlca.sqlcode));
    TRACE((3, "<<-- stmt_execute()\n"));
}

static int stmt_freestmt(SqlStmt *stmt)
{
    EXEC SQL BEGIN DECLARE SECTION;
    const char *s_stmt = stmt->stmtname;
    EXEC SQL END DECLARE SECTION;
    EXEC SQL FREE :s_stmt;
    return(sqlca.sqlcode);
}

static int stmt_freecrsr(SqlStmt *stmt)
{
    EXEC SQL BEGIN DECLARE SECTION;
    const char *s_crsr = stmt->crsrname;
    EXEC SQL END DECLARE SECTION;
    EXEC SQL FREE :s_crsr;
    return(sqlca.sqlcode);
}

static int stmt_opencrsr(SqlStmt *stmt)
{
    EXEC SQL BEGIN DECLARE SECTION;
    const char *s_crsr = stmt->crsrname;
    EXEC SQL END DECLARE SECTION;
    Sqlda *idesc = stmt->idesc;
    EXEC SQL OPEN :s_crsr USING DESCRIPTOR idesc;
    return(sqlca.sqlcode);
}

static int stmt_closecrsr(SqlStmt *stmt)
{
    EXEC SQL BEGIN DECLARE SECTION;
    const char *s_crsr = stmt->crsrname;
    EXEC SQL END DECLARE SECTION;
    EXEC SQL CLOSE :s_crsr;
    return(sqlca.sqlcode);
}

static int stmt_fetchcrsr(SqlStmt *stmt)
{
    EXEC SQL BEGIN DECLARE SECTION;
    const char *s_crsr = stmt->crsrname;
    short flag;
    EXEC SQL END DECLARE SECTION;
    KLUDGE("Should FETCH into an sqlda structure!");;
    EXEC SQL FETCH :s_crsr INTO :flag;
    return(sqlca.sqlcode);
}

/* Upload by doing SELECT then INSERT or UPDATE */
static int upld_select(Upload *upld, Memory *line)
{
    int rows_found = 0;

    if (stmt_opencrsr(&upld->select) < 0)
    {
        if (new_upld_error(upld, line) != 0)
            return(-1);
    }
    upld->selects_exec++;
    while (stmt_fetchcrsr(&upld->select) == 0)
    {
        rows_found++;
        upld->rows_selected++;
    }
    if (sqlca.sqlcode < 0)
    {
        if (new_upld_error(upld, line) != 0)
        {
            stmt_closecrsr(&upld->select);
            return(-1);
        }
    }
    if (stmt_closecrsr(&upld->select) < 0)
    {
        if (new_upld_error(upld, line) != 0)
            return(-1);
    }
    if (rows_found == 0)
    {
        stmt_execute(&upld->insert);
        upld->inserts_exec++;
        if (sqlca.sqlcode < 0)
        {
            if (new_upld_error(upld, line) != 0)
                return(-1);
        }
        upld->rows_inserted++;
        log_heuristic(H_INSERT, upld);
    }
    else
    {
        stmt_execute(&upld->update);
        upld->updates_exec++;
        if (sqlca.sqlcode < 0)
        {
            if (new_upld_error(upld, line) != 0)
                return(-1);
        }
        if (sqlca.sqlcode == 0)
            upld->rows_updated += sqlca.sqlerrd[2];
        if (sqlca.sqlerrd[2] > 1 && upld->keytype != K_DUPS)
        {
            /* Oops! */
            KLUDGE("UPLOAD/INSERT multi-row update handling not implemented");
            error_746("UPLOAD/INSERT multi-row update handling not implemented");
        }
        log_heuristic(H_UPDATE, upld);
    }
    return(0);
}

/* UPLOAD by doing INSERT then UPDATE */
static int upld_insert(Upload *upld, Memory *line)
{
    stmt_execute(&upld->insert);
    upld->inserts_exec++;
    if (sqlca.sqlcode == 0)
    {
        upld->rows_inserted++;
        log_heuristic(H_INSERT, upld);
    }
    else
    {
        stmt_execute(&upld->update);
        upld->updates_exec++;
        if (sqlca.sqlcode == 0)
            upld->rows_updated += sqlca.sqlerrd[2];
        if (sqlca.sqlcode < 0)
        {
            if (new_upld_error(upld, line) != 0)
                return(-1);
        }
        else if (sqlca.sqlerrd[2] > 1 && upld->keytype != K_DUPS)
        {
            /* Oops! */
            KLUDGE("UPLOAD/INSERT multi-row update handling not implemented");
            error_746("UPLOAD/INSERT multi-row update handling not implemented");
        }
        else
            log_heuristic(H_UPDATE, upld);
    }
    return(0);
}

/* UPLOAD by doing UPDATE then INSERT */
static int upld_update(Upload *upld, Memory *line)
{
    TRACE((5, "Doing UPDATE first\n"));
    stmt_execute(&upld->update);
    upld->updates_exec++;
    TRACE((5, "UPDATE SQL = %ld, NROWS = %ld\n", sqlca.sqlcode,
            sqlca.sqlerrd[2]));
    if (sqlca.sqlcode == 0)
        upld->rows_updated += sqlca.sqlerrd[2];
    if (sqlca.sqlcode == 0 && sqlca.sqlerrd[2] == 1)
        log_heuristic(H_UPDATE, upld);
    else if ((sqlca.sqlcode == 0 && sqlca.sqlerrd[2] == 0) ||
             sqlca.sqlcode == SQLNOTFOUND)
    {
        /* Update failed, or processed zero rows */
        TRACE((5, "UPDATE updated zero rows; do INSERT\n"));
        dump_sqlda(stderr, "Before insert - insert SQLDA", upld->insert.idesc);
        stmt_execute(&upld->insert);
        upld->inserts_exec++;
        if (sqlca.sqlcode < 0)
        {
            if (new_upld_error(upld, line) != 0)
                return(-1);
        }
        else
        {
            upld->rows_inserted++;
            log_heuristic(H_INSERT, upld);
        }
    }
    else if (sqlca.sqlcode == 0 && sqlca.sqlerrd[2] > 1)
    {
        /* Oops! */
        KLUDGE("UPLOAD/UPDATE multi-row update handling not implemented");
        error_746("UPLOAD/UPDATE multi-row update handling not implemented");
    }
    else
    {
        /* Update failed, presumably with a reason */
        TRACE((5, "UPDATE failed; assume that INSERT would too\n"));
        assert(sqlca.sqlcode < 0 || sqlca.sqlerrd[2] < 0);
        if (new_upld_error(upld, line) != 0)
            return(-1);
    }
    return(0);
}

/* Check for null values in PK columns. */
static void validate_for_null_pk_values(Upload *upld, Memory *line)
{
    KLUDGE("Null primary key validation not implemented");
}

/*
** Process an UPLOAD request
**
** Given:
** ctrl      - UPLOAD statement control block
** upld      - UPLOAD statement working data
**
** Context is set so that input comes from ctxt_input(), etc.
**
** The reject file and log file are open...
*/
static int upload(Control *ctrl, Upload *upld)
{
    FILE           *file = ctxt_input();
    HMode           hmode;
    Sqlda          *idesc = 0;
    char           *name = ctxt_loadfile();
    int             nblobs = 0;
    int             sqlerr = 0;
    int             stage = 0;
    RecLoc          rec;

    rec_zero(&rec);

    volume = ctrl->volume;

    /* This is a one-cycle loop that simplifies error handling */
    do
    {
        /* Compile INSERT statement */
        sqlerr = stmt_prepare(&upld->insert);
        if (sqlerr < 0)
        {
            sql_printerror(ctxt_error());
            break;
        }
        stage = 1;

        /* Compile UPDATE statement */
        sqlerr = stmt_prepare(&upld->update);
        if (sqlerr < 0)
        {
            sql_printerror(ctxt_error());
            break;
        }
        stage = 2;

        /* Compile SELECT statement */
        if (ctrl->hmode == H_SELECT)
        {
            sqlerr = stmt_prepare(&upld->select);
            if (sqlerr < 0)
            {
                sql_printerror(ctxt_error());
                break;
            }
            stage = 3;
            sqlerr = stmt_declare(&upld->select);
            if (sqlerr < 0)
            {
                sql_printerror(ctxt_error());
                break;
            }
            stage = 4;
        }

        nblobs = count_blobs(upld->record.idesc);

        if (pflag == SQLUPLOAD)
        {
            if (begin_work(ctrl->dbs_type) != 0)
                break;
        }

        /* Fetch and UPLOAD data */
        stage = 4;
        upld->rows_read = 0;
        {
        Memory          line;
        mem_new(&line);
        while (scanrecord(file, upld->record.idesc, &rec, name, &line) != EOF)
        {
            upld->rows_read = rec.rownum;
            TRACE((3, "record %d: data = %s\n", upld->rows_read, mem_data(&line)));

            /* Skipping the first N input records */
            /* Ignore errors in these records */
            if (upld->rows_read < ctrl->skip_first)
            {
                TRACE((3, "record %d: skipped\n", upld->rows_read));
                continue;
            }
            /**
            ** Problem: what to do if the input record contains a null as the
            ** one of the values in the current record.  This should not happen
            ** because a primary key should not accept nulls!
            ** What does it take to handle a null key, or partially null key?
            ** The standard pre-prepared UPDATE and SELECT statements are not
            ** usable.  We're supposed to change the primary key, so the insert
            ** is likely to succeed.  In fact, if we're updating a primary key
            ** at all, the INSERT heuristic should never be used.
            ** In the interim, if any of the primary key input columns are
            ** null, reject the row.  And if any of the updated primary key
            ** columns are null, reject the row.  If people want to play with
            ** non-relational data, use a non-relational tool -- meaning
            ** something other than SQLUPLOAD!
            ** NB: for output nulls, error -703 (Primary key on table (%s) has
            ** a field with a null key value) is almost suitable (finderr lists
            ** insert a null value into a column that is part of a primary key
            ** as a reason for this error).  There isn't an obvious error for
            ** the input nulls, so use -746 with our own message.
            */
            validate_for_null_pk_values(upld, &line);

            /* Handle errors in the record */
            if (sqlca.sqlcode < 0)
            {
                if (new_upld_error(upld, &line) != 0)
                    break;
                upld->fmt_errors++;
                continue;
            }
            hmode = get_heuristic();
            TRACE((3, "Heuristic = %c\n", hmode));
            if (hmode == H_SELECT)
            {
                if (upld_select(upld, &line) != 0)
                    break;
            }
            else if (hmode == H_UPDATE)
            {
                if (upld_update(upld, &line) != 0)
                    break;
            }
            else if (hmode == H_INSERT)
            {
                if (upld_insert(upld, &line) != 0)
                    break;
            }
            else
            {
                error_746("Internal error in get_heuristic()");
                break;
            }

            if (nblobs > 0)
                free_blobs(idesc);

            if (upld->rows_read % ctxt_gettransize() == 0 && pflag == SQLUPLOAD)
            {
                /* Intermediate transaction */
                if (commit_work(ctrl->dbs_type) != 0)
                    break;
                log_heuristic(H_COMMIT, upld);
                if (ctxt_getsilence() == OP_OFF && volume != V_VERBOSE)
                {
                    unsigned long nrows = upld->rows_read;
                    fprintf(ctxt_output(), "%lu rows committed\n", nrows);
                }
                if (begin_work(ctrl->dbs_type) != 0)
                    break;
            }
        }
        mem_del(&line);
        }
        if (sqlca.sqlcode < 0)
            break;

    } while (0);

    /* Clean up: preserve first error if there was one */
    sqlerr = sqlca.sqlcode;

    if (stage >= 4)
    {
        if (pflag == SQLUPLOAD)
        {
            if (commit_work(ctrl->dbs_type) != 0)
            {
                if (sqlerr == 0)
                    sqlerr = sqlca.sqlcode;
            }
            if (ctxt_getsilence() == OP_OFF)
            {
                if (upld->rows_read == 0 || upld->rows_read % ctxt_gettransize() != 0)
                    log_heuristic(H_COMMIT, upld);
                log_heuristic(H_NEWLINE, upld);
                if (upld->rows_read == 0 || upld->rows_read %
                ctxt_gettransize() != 0 || volume == V_VERBOSE)
                {
                    unsigned long nrows = upld->rows_read;
                    if (sqlerr != 0 && upld->rows_read != 0)
                        nrows--;
                    fprintf(ctxt_output(), "%s%lu rows committed%s\n",
                            (sqlerr == 0 || upld->rows_read <= 1) ? "" : "Up to ",
                            nrows, (sqlerr == 0) ? "" : " successfully");
                }
            }
        }
    }

    if (stage >=  4)
    {
        stmt_freecrsr(&upld->select);
        if (sqlerr == 0 && sqlca.sqlcode < 0)
            sqlerr = sqlca.sqlcode;
    }

    if (stage >= 3)
    {
        stmt_freestmt(&upld->select);
        if (sqlerr == 0 && sqlca.sqlcode < 0)
            sqlerr = sqlca.sqlcode;
    }

    if (stage >= 2)
    {
        stmt_freestmt(&upld->update);
        if (sqlerr == 0 && sqlca.sqlcode < 0)
            sqlerr = sqlca.sqlcode;
    }

    if (stage >= 1)
    {
        stmt_freestmt(&upld->insert);
        if (sqlerr == 0 && sqlca.sqlcode < 0)
            sqlerr = sqlca.sqlcode;
    }

    return(sqlerr);
}

static void release_upload(Upload *upld, void *data, ColList **a_list, size_t n_list)
{
    size_t i;

    for (i = 0; i < n_list; i++)
        delete_collist(a_list[i]);
    if (upld->record.idesc)
    {
        sql_release(upld->record.idesc, data, 1);
        stmt_freestmt(&upld->record);
        dealloc_sqlda(upld->insert.idesc);
        dealloc_sqlda(upld->update.idesc);
    }
    if (upld->lgfp)
        fclose(upld->lgfp);
    if (upld->rjfp)
        fclose(upld->rjfp);
}

static void fprint_size_t(FILE *fp, const char *tag, size_t val)
{
    fprintf(fp, "%s = %lu\n", tag, (unsigned long)val);
}

static void upload_stats(FILE *fp, Upload *upld, HMode hmode)
{
    fprintf(fp, "== SUMMARY of UPLOAD to %s ==\n", upld->fulltable);
    fprint_size_t(fp, "Data rows read", upld->rows_read);
    if (upld->fmt_errors != 0)
        fprint_size_t(fp, "Rows with format errors", upld->fmt_errors);
    if (upld->sql_errors != 0)
        fprint_size_t(fp, "SQL errors", upld->sql_errors);
    if (hmode == H_SELECT)
    {
        fprint_size_t(fp, "SELECT statements executed", upld->selects_exec);
        if (upld->rows_selected != upld->selects_exec)
            fprint_size_t(fp, "Rows actually selected", upld->rows_selected);
    }
    fprint_size_t(fp, "UPDATE statements executed", upld->updates_exec);
    if (upld->updates_exec != upld->rows_updated)
        fprint_size_t(fp, "Rows actually updated", upld->rows_updated);
    fprint_size_t(fp, "INSERT statements executed", upld->inserts_exec);
    if (upld->inserts_exec != upld->rows_inserted)
        fprint_size_t(fp, "Rows actually inserted", upld->rows_inserted);
}

/* Process UPLOAD statement using information given in argument */
static int process_upload(Control *ctrl)
{
    Upload   upld_data = { K_UNIQUE };      /* ANSI C compilers */
    Upload  *upld = &upld_data;
    ColList  key = { 0, 0, 0 };     /* ANSI C compilers */
    ColList  col = { 0, 0, 0 };     /* ANSI C compilers */
    ColList  upd = { 0, 0, 0 };     /* ANSI C compilers */
    ColList *a_list[3];             /* GCC allows initializer; others don't */
    size_t   n_list = 0;
    void    *data = 0;
    char     scn_stmt[32768];
    char     sel_stmt[32768];
    char     ins_stmt[32768];
    char     upd_stmt[32768];
    int      nidx;
    long     tabid;
    int      status = 0;

    TRACE((3, "-->> process_upload():\n"));
    a_list[0] = &key;
    a_list[1] = &col;
    a_list[2] = &upd;

    upld->keytype = ctrl->keytype;
    upld->stop_after = ctrl->stop_after;

    upld->record.stmttext = scn_stmt;
    upld->update.stmttext = upd_stmt;
    upld->select.stmttext = sel_stmt;
    upld->insert.stmttext = ins_stmt;
    upld->record.stmtsize = sizeof(scn_stmt);
    upld->update.stmtsize = sizeof(upd_stmt);
    upld->select.stmtsize = sizeof(sel_stmt);
    upld->insert.stmtsize = sizeof(ins_stmt);
    upld->record.stmtname = "upload_record";
    upld->update.stmtname = "upload_update";
    upld->select.stmtname = "upload_select";
    upld->insert.stmtname = "upload_insert";
    upld->select.crsrname = "cursor_select";

    set_fulltablename(ctrl->table, ctrl->owner, upld->fulltable, sizeof(upld->fulltable));
    if (table_exists(upld->fulltable) <= 0)
    {
        TRACE((3, "<<-- process_upload() - error in table_exists()\n"));
        return(EXIT_FAILURE);
    }
    if ((tabid = sql_tabid(ctrl->table, ctrl->owner, 0, 0, ctrl->dbs_type & IS_MODE_ANSI)) < 0)
    {
        TRACE((3, "<<-- process_upload() - error in sql_tabid()\n"));
        return(EXIT_FAILURE);
    }

    /* Check that there is at least one unique index on the table */
    if ((nidx = num_unique_indexes(tabid)) == 0)
    {
        error_746("There are no unique indexes on the table.");
        return(EXIT_FAILURE);
    }

    if (ctrl->keycols == 0)
    {
        /* Exactly one unique index: use it as the key */
        if (nidx == 1)
            get_key_columns(tabid, &ctrl->keycols);
        else
        {
            error_746("There must be exactly one unique index on the table"
                        " unless you specify -k or -K");
            return(EXIT_FAILURE);
        }
    }

    if (create_collist(ctrl->keycols, &key) != 0)
        return(EXIT_FAILURE);

    n_list++;

    if (validate_collist(&key, "key columns") != 0)
    {
        release_upload(upld, data, a_list, n_list);
        return(EXIT_FAILURE);
    }

    if (db_getdebug() >= 9)
        print_collist(stdout, &key, "Sorted Key Column List", "KeyCol");

    if (ctrl->columns == 0)
    {
        if (generate_collist(upld->fulltable, &col) != 0)
        {
            release_upload(upld, data, a_list, n_list);
            return(EXIT_FAILURE);
        }
        n_list++;
        /* Leave 'upd' as an empty column list */
    }
    else
    {
        if (create_collist(ctrl->columns, &col) != 0)
        {
            release_upload(upld, data, a_list, n_list);
            return(EXIT_FAILURE);
        }
        n_list++;
        if (validate_collist(&col, "column list") != 0)
        {
            release_upload(upld, data, a_list, n_list);
            return(EXIT_FAILURE);
        }
        if (create_collist(ctrl->columns, &upd) != 0)
        {
            release_upload(upld, data, a_list, n_list);
            return(EXIT_FAILURE);
        }
        n_list++;
        remove_nonpluscols(&upd);
        if (validate_collist(&upd, "updated key columns") != 0)
        {
            release_upload(upld, data, a_list, n_list);
            return(EXIT_FAILURE);
        }
    }

    if (db_getdebug() >= 9)
    {
        print_collist(stdout, &col, "Sorted Column List", "Column");
        print_collist(stdout, &upd, "Updated Key Column List", "Column");
    }

    /* Check that all the columns in the key list are in the column list */
    if (compare_key_col(&key, &col) != 0)
    {
        release_upload(upld, data, a_list, n_list);
        return(EXIT_FAILURE);
    }

    /* OK - don't need the sorted list any more */
    reset_collist(&col);
    if (db_getdebug() >= 9)
        print_collist(stdout, &col, "Unsorted Column List", "Column");

    /* Check that all the columns in the list are in the table */
    if (check_columns_in_table(upld->fulltable, &col) != 0)
    {
        release_upload(upld, data, a_list, n_list);
        return(EXIT_FAILURE);
    }

    if (make_scan_sqlda(&col, upld->fulltable, &upld->record) != 0)
    {
        release_upload(upld, data, a_list, n_list);
        return(EXIT_FAILURE);
    }

    if ((data = sql_describe(upld->record.idesc)) == 0)
    {
        release_upload(upld, data, a_list, n_list);
        return(EXIT_FAILURE);
    }

    if (db_getdebug() >= 9)
        dump_sqlda(stdout, "described scan data", upld->record.idesc);

    if (make_update_stmt(&key, &col, upld->fulltable, upld->record.idesc, &upld->update) != 0)
    {
        release_upload(upld, data, a_list, n_list);
        return(EXIT_FAILURE);
    }

    if (db_getdebug() >= 8)
    {
        printf("UPDATE stmt: %s\n", upld->update.stmttext);
        dump_sqlda(stdout, "described update idesc", upld->update.idesc);
    }

    if (make_insert_stmt(&col, &upd, upld->fulltable, upld->record.idesc, &upld->insert) != 0)
    {
        release_upload(upld, data, a_list, n_list);
        return(EXIT_FAILURE);
    }

    if (db_getdebug() >= 8)
    {
        printf("INSERT stmt: %s\n", upld->insert.stmttext);
        dump_sqlda(stdout, "described insert sqlda", upld->insert.idesc);
    }

    if (ctrl->hmode == H_SELECT)
    {
        if (make_select_stmt(&key, &col, upld->fulltable, upld->record.idesc, &upld->select) != 0)
        {
            release_upload(upld, data, a_list, n_list);
            return(EXIT_FAILURE);
        }
        if (db_getdebug() >= 8)
        {
            printf("SELECT stmt: %s\n", upld->select.stmttext);
            dump_sqlda(stdout, "described select sqlda", upld->select.idesc);
        }
    }

    if (ctrl->rejfile)
    {
        if ((upld->rjfp = fopen(ctrl->rejfile, "w")) == 0)
        {
            err_sysrem("failed to open file %s in '%s' mode\n",
                        ctrl->rejfile, "w");
            release_upload(upld, data, a_list, n_list);
            return(EXIT_FAILURE);
        }
    }

    if (ctrl->logfile)
    {
        if ((upld->lgfp = fopen(ctrl->logfile, "w")) == 0)
        {
            err_sysrem("failed to open file %s in '%s' mode\n",
                        ctrl->logfile, "w");
            release_upload(upld, data, a_list, n_list);
            return(EXIT_FAILURE);
        }
    }

    if (prep_locktable(ctrl->lockmode, upld->fulltable) < 0)
    {
        release_upload(upld, data, a_list, n_list);
        return(EXIT_FAILURE);
    }

    /* OK - go ahead and UPLOAD! */
    status = upload(ctrl, upld);

    if (ctrl->volume != V_QUIET)
        upload_stats(ctxt_output(), upld, ctrl->hmode);

    /* Release resources */
    free_locktable();
    release_upload(upld, data, a_list, n_list);

    return(status);
}

/* Process UPLOAD statement using information given in argument */
int upload_stmt(Control *ctrl)
{
    int status = 0;
    char buffer[10];

    TRACE((3,"-->> upload_stmt():\n"));
    /* Create and initialize new SQLCMD context */
    ctxt_newcontext();

    if (ctrl->inpfile == 0)
        ctxt_newinput(stdin, "standard input");
    else
        status = ctxt_setinput(ctrl->inpfile);
    if (status != 0)
        return(status);
    ctxt_setdelim(quotify(ctrl->delim, buffer, sizeof(buffer)));
    ctxt_setescape(quotify(ctrl->escape, buffer, sizeof(buffer)));
    ctxt_settransize(ctrl->txsize);

    status = process_upload(ctrl);

    /* Terminate SQLCMD context */
    ctxt_endcontext();

    TRACE((3,"<<-- upload_stmt(): %d\n", status));
    return(status ? 1 : 0);
}
