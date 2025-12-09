/*
@(#)File:           $RCSfile: reload.ec,v $
@(#)Version:        $Revision: 2009.3 $
@(#)Last changed:   $Date: 2009/10/23 17:00:11 $
@(#)Purpose:        Load database using INSERT and data from file
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 1992-93,1995,1997-2000,2002-09
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

#include "context.h"
#include "emalloc.h"
#include "esqlc.h"
#include "describe.h"
#include "memory.h"
#include "sqlcmd.h"
#include "sqlerr.h"
#include "jlsstools.h"
#include "esnprintf.h"
#include "stderr.h"
#include "internal.h"
#include "esqlutil.h"
#include "kludge.h"
#include "debug.h"
#include "sqltoken.h"
#include "tokencmp.h"
#include "dumpesql.h"

#define BYTEBLOB(x)     ((x) == CLOCATORTYPE || (x) == SQLBYTES)
#define MIN(a, b)       (((a) < (b)) ? (a) : (b))
#define syntaxerror()   (sqlca.sqlcode = -201, sql_error())

static int delimident = 0;

#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
const char jlss_id_reload_ec[] = "@(#)$Id: reload.ec,v 2009.3 2009/10/23 17:00:11 jleffler Exp $";
#endif /* lint */

/*
** Skip over table name of form [database[@server]:]["owner".]tablename
**
** Only used in the context of INSERT INTO table [ ( column [, column]* ) ]
**
** Return pointer to character after end of tablename
** = database and server must be plain identifiers.
** = owner may be a plain identifier, a single-quoted string, or a delimited identifier.
** = tablename may be a plain identifier or a delimited identifier.
**
** State Table: initial state = 0 with delimident set; 8 with it unset.
**
** State  X:    error
** State  F:    tablename recognized - complete.
**
** State  0:    initial state - delimident set
** State  1:    database, owner, or tablename recognized
** State  2:    owner recognized; dot required
** State  3:    owner or tablename recognized
** State  4:    database recognized; server required
** State  5:    database recognized; owner or tablename required
** State  6:    owner recognized; tablename required
** State  7:    database@server recognized; colon required
**
** Token types:
**  P   Plain identifier
**  S   Single quoted string
**  D   Double quoted string
**  x   Anything else
**  EOS End of String
**
** Transition S is to F when delimident is enabled, to X when not.
** Transition P is a transition to F but the code has read one token too far.
**
** Current              Token Type
** State    P   S   D   @   :   .   (   EOS
**   0      1   2   3   X   X   X   X   X
**   1      X   X   X   4   5   6   P   F
**   2      X   X   X   X   X   6   X   X
**   3      X   X   X   X   X   6   P   F
**   4      7   X   X   X   X   X   X   X
**   5      3   2   3   X   X   X   X   X
**   6      F   X   S   X   X   X   X   X
**   7      X   X   X   X   5   X   X   X
*/

/* Token types and state names */
/* NB: assumes delimited identifiers are OK - they are not always. */
enum { T_IDENT, T_SQS, T_DQS, T_AT, T_COLON, T_DOT, T_PAREN, T_ERROR };
enum { S0, S1, S2, S3, S4, S5, S6, S7, SX, SF, SS, SP };

static const char stt[8][7] =
{
    {   S1, S2, S3, SX, SX, SX, SX, },
    {   SX, SX, SX, S4, S5, S6, SP, },
    {   SX, SX, SX, SX, SX, S6, SX, },
    {   SX, SX, SX, SX, SX, S6, SP, },
    {   S7, SX, SX, SX, SX, SX, SX, },
    {   S3, S2, S3, SX, SX, SX, SX, },
    {   SF, SX, SS, SX, SX, SX, SX, },
    {   SX, SX, SX, SX, S5, S5, S5, },
};

/* What type of token is in [s..e)? */
static int toktype(const char *s, const char *e)
{
    char c = *s;
    int  r;
    if (c == '@')
        r = T_AT;
    else if (c == '.')
        r = T_DOT;
    else if (c == '(')
        r = T_PAREN;
    else if (c == '\'')
        r = T_SQS;
    else if (c == '"')
        r = T_DQS;
    else if (isalpha(c) || c == '_')
        r = T_IDENT;
    else if (c == ':' && e == s + 1)
        r = T_COLON;        /* Distinguish : from :: */
    else
        r = T_ERROR;
    return(r);
}

static const char *skiptabname(char *s)
{
    const char *str;
    const char *src;
    const char *end;
    const char *p_end = s;
    int         state = 0;
    FUNCTION_NAME("skiptabname");

    TRACE((3, "-->> %s: %s\n", __func__, s));

    for (str = s; *(src = iustoken(str, &end)) != '\0' && src != end; str = end)
    {
        int t = toktype(src, end);
        int ns = SX;
        TRACE((3, "Token: <<%.*s>>\n", (int)(end - src), src));
        if (t == T_ERROR || (ns = stt[state][t]) == SX || (ns == SS && delimident == 0))
        {
            TRACE((3, "<<-- %s: error: t = %d, ns = %d\n", __func__, t, ns));
            return(0);
        }
        else if (ns == SF || (ns == SS && delimident == 1))
        {
            TRACE((3, "<<-- %s: F-state - table: %.*s\n", __func__, (int)(end - s), s));
            return(end);
        }
        else if (ns == SP)
        {
            TRACE((3, "<<-- %s: P-state - table: %.*s\n", __func__, (int)(p_end - s), s));
            return(p_end);
        }
        TRACE((3, "Transition: old state = %d, token = %d, new state = %d\n", state, t, ns));
        state = ns;
        p_end = end;
    }
    if (state == S1 || state == S3)
    {
        TRACE((3, "<<-- %s: EOS - table: %.*s\n", __func__, (int)(str - s), s));
        return(str);
    }
    return(0);
}

/*
**  Count the number of columns in the column list of an INSERT statement.
**  Do it by finding the open parenthesis, then counting the number of
**  comma-separated identifiers, and find the closing parenthesis.
**
**  State table:
**  0 - Initial state
**  1 - Read l_paren; need ID
**  2 - Read ID; need r_paren or comma
**  3 - Read r_paren: need EOS
**
**  Current       Token Read
**  State     (     )     ,       ID    EOS
**    0       1    Err   Err      Err   End
**    1      Err   Err   Err       2    Err
**    2      Err    3     1       Err   Err
**    3      Err   Err   Err      Err   End
**
** Handling DELIMIDENT - use iustoken() to parse string.
** The SQL statement we're dealing with is the parenthesized
** portion of
**     INSERT INTO SomeTable(Col1, "Col2")
** counting the number of column names cited:
*/
static int cols_in_list(const char *s)
{
    int         n = 0;
    int         state = 0;
    const char *str;
    const char *src;
    const char *end;
    FUNCTION_NAME("cols_in_list");

    TRACE((3, "-->> %s: %s\n", __func__, s));

    for (str = s; *(src = iustoken(str, &end)) != '\0' && src != end; str = end)
    {
        char c = *src;
        TRACE((3, "Token: <<%.*s>>\n", (int)(end - src), src));
        if (c == '(')
        {
            if (state != 0)
            {
                TRACE((3, "<<-- %s: error exit (1)\n", __func__));
                return(-1);
            }
            state = 1;
        }
        else if (c == ')')
        {
            if (state != 2)
            {
                TRACE((3, "<<-- %s: error exit (2)\n", __func__));
                return(-1);
            }
            state = 3;
        }
        else if (c == ',')
        {
            if (state != 2)
            {
                TRACE((3, "<<-- %s: error exit (3)\n", __func__));
                return(-1);
            }
            state = 1;
        }
        else if (isalpha(c) || c == '_' || (delimident && c == '"'))
        {
            if (state != 1)
            {
                TRACE((3, "<<-- %s: error exit (4)\n", __func__));
                return(-1);
            }
            state = 2;
            n++;
        }
        else
        {
            TRACE((3, "<<-- %s: error exit (5)\n", __func__));
            return(-1);
        }
        TRACE((3, "State: %d\n", state));
    }

    if (state != 0 && state != 3)
    {
        TRACE((3, "<<-- %s: error exit (6)\n", __func__));
        return(-1);
    }
    TRACE((3, "<<-- %s: %d\n", __func__, n));
    return(n);
}

/*
** Return number of columns in table, or zero if table name invalid.
** Allow for 'owner'.tablename, "owner".tablename and owner.tablename
** This version does (finally) allow for temporary tables!
**
** The subterfuge with a_stmt circumvents the idiotic ESQL/C
** compiler which does not recognize that even though it cannot evaluate
** the size of an array, the C compiler is able to do so -- so it complains
** with a warning:
** Warning -33208: Runtime error is possible because size of 'stmt'
** is unknown.
*/
static int      cols_in_table(const char *tabstart, const char *tabend)
{
    char            a_stmt[2 * SQL_TABNAMELEN];
    EXEC SQL BEGIN DECLARE SECTION;
    char           *stmt = a_stmt;
    EXEC SQL END DECLARE SECTION;
    int             n_columns = 0;
    int             tablen = tabend - tabstart;
    Sqlda          *d;
    FUNCTION_NAME("cols_in_table");

    TRACE((3, "-->> %s: %.*s\n", __func__, tablen, tabstart));

    esnprintf(a_stmt, sizeof(a_stmt), "SELECT * FROM %.*s", tablen, tabstart);
    TRACE((3, "Stmt: %s\n", a_stmt));
    EXEC SQL PREPARE p_cols FROM :stmt;
    if (sqlca.sqlcode != 0)
    {
        int nb = MIN(sizeof(sqlca.sqlerrm), tablen + 1);
        sqlca.sqlcode = -206;   /* Table is not in database */
        nstrcpy(sqlca.sqlerrm, nb, tabstart);
        sql_error();
    }
    else
    {
        EXEC SQL DESCRIBE p_cols INTO d;
        if (sqlca.sqlcode != 0 || d == 0)
        {
            esnprintf(a_stmt, sizeof(a_stmt), "Unexpected error on DESCRIBE for table %.*s\n",
                    tablen, tabstart);
            error_746(stmt);
        }
        n_columns = d->sqld;
        EXEC SQL FREE p_cols;
        if (sqlca.sqlcode != 0)
        {
            esnprintf(a_stmt, sizeof(a_stmt), "Unexpected error on FREE for table %.*s\n",
                    tablen, tabstart);
            error_746(stmt);
        }
    }
    TRACE((3, "<<-- %s: %d\n", __func__, n_columns));
    return(n_columns);
}

/*
**  Convert a SQ_LDINSERT statement (an INSERT statement which may
**  or may not have a VALUES clause) into an SQ_INSERT statement with
**  an appropriate VALUES clause.  Error handling in mk_insert is a
**  bit crude.
**  NB: the statement has a semi-colon at the end, which is stripped off.
*/
static char    *mk_insert(char *old_stmt)
{
    const char *src;
    const char *e;
    char  *s;
    const char  *pad;
    char  *new_stmt;
    char  *tab;
    int    i;
    int    ncols;
    FUNCTION_NAME("mk_insert");

    i = strlen(old_stmt);
    if (old_stmt[i - 1] == ';')
        old_stmt[i - 1] = '\0';
    src = old_stmt;
    s = iustoken(src, &src);    /* Confused by hints! But who writes hints in LOAD statements? */
    if (tokencmp(s, src - s, "insert", sizeof("insert")-1) != 0)
    {
        syntaxerror();
        return(NIL(char *));
    }
    s = iustoken(src, &src);    /* Confused by hints! */
    if (tokencmp(s, src - s, "into", sizeof("into")-1) != 0)
    {
        syntaxerror();
        return(NIL(char *));
    }
    tab = iustoken(src, &src);      /* Start of table name */
    /* Confused, still, by comments embedded in table name */
    e = skiptabname(tab);       /* After table name */
    if (e == 0)
    {
        syntaxerror();
        return(NIL(char *));
    }

    TRACE((3, "%s: tabname: %.*s\n", __func__, (int)(s - tab), tab));

    if ((ncols = cols_in_list(e)) == 0)
        ncols = cols_in_table(tab, e);
    if (ncols <= 0)
    {
        if (ncols < 0)
            syntaxerror();
        return(NIL(char *));
    }

    /* Watch the constant string -- the spaces are ALL needed */
    new_stmt = (char *)MALLOC(sizeof("insert into values ()") + strlen(tab) + 2
    * ncols);   /*=C++=*/
    jb_register(new_stmt);

    s = vstrcpy(new_stmt, 3, "insert into ", tab, " values(");
    pad = "?";
    for (i = 0; i < ncols; i++)
    {
        s = vstrcpy(s, 1, pad);
        pad = ",?";
    }
    (void)vstrcpy(s, 1, ")");

    return(new_stmt);
}

/*
** Count the number of blobs in the data to be loaded.
*/
int      count_blobs(Sqlda *u)
{
    Sqlva          *c;
    Sqlva          *e;
    int             count = 0;

    for (c = u->sqlvar, e = c + u->sqld; c < e; c++)
    {
        if (c->sqltype == SQLBYTES ||
            c->sqltype == SQLTEXT ||
            c->sqltype == CLOCATORTYPE)
            count++;
    }
    return(count);
}

/*
**  Release the memory associated with the blobs.
*/
void     free_blobs(Sqlda *u)
{
    Sqlva          *c;
    Sqlva          *e;
    Blob           *blob;

    for (c = u->sqlvar, e = c + u->sqld; c < e; c++)
    {
        if (c->sqltype == SQLBYTES ||
            c->sqltype == SQLTEXT ||
            c->sqltype == CLOCATORTYPE)
        {
            blob = (Blob *)c->sqldata;
            if (blob->loc_buffer != NIL(char *))
            {
                jb_unregister(blob->loc_buffer);
                FREE(blob->loc_buffer);
                blob->loc_buffer = NIL(char *);
            }
            blob->loc_bufsize = -1;
        }
    }
}

/*
** Process the INSERT part of a LOAD statement
*/
void            reload(char *ins)
{
    Sqlda          *idesc = NIL(Sqlda *);
    char           *buffer = NIL(char *);
    EXEC SQL BEGIN DECLARE SECTION;
    char           *new_stmt;
    EXEC SQL END DECLARE SECTION;
    RecLoc          rec;
    int             nblobs = 0;
    int             stage = 0;
    Sqlca           save;
    FILE           *file = ctxt_input();
    char           *name = ctxt_loadfile();
    int             nskip = ctxt_getskipsize();
    int             nput = 0;
    int             txsize = ctxt_gettransize();

    if (getenv("DELIMIDENT") != 0)
        delimident = 1;

    rec_zero(&rec);

    /* This is a one-cycle loop that simplifies error handling */
    do
    {
        /* INSERT statement in LOAD does not have VALUES clause */
        if ((new_stmt = mk_insert(ins)) == NIL(char *))
            break;

        /* Compile INSERT statement */
        stage = 0;
        EXEC SQL PREPARE s_insert FROM :new_stmt;
        TRACE((8, "reload: PREPARE complete (%ld)\n", sqlca.sqlcode));
        jb_unregister(new_stmt);
        FREE(new_stmt);             /* Clean up */
        if (sqlca.sqlcode < 0)
            break;

        stage = 1;
        EXEC SQL DESCRIBE s_insert INTO idesc;
        TRACE((8, "reload: DESCRIBE complete (%ld)\n", sqlca.sqlcode));
        if (sqlca.sqlcode < 0)
            break;

        if (db_getdebug() >= 8)
            dump_sqlda(stdout, "DESCRIBE for LOAD", idesc);

        if (sqlca.sqlcode != SQ_INSERT)
        {
            sqlca.sqlcode = -201;
            break;
        }
        nblobs = count_blobs(idesc);

        /* Allocate space for SQLDA structure */
        buffer = (char *)sql_describe(idesc);   /*=C++=*/
        jb_register(buffer);

        /* DECLARE INSERT CURSOR */
        stage = 2;
        EXEC SQL DECLARE c_insert CURSOR FOR s_insert;
        TRACE((8, "reload: DECLARE complete (%ld)\n", sqlca.sqlcode));
        if (sqlca.sqlcode < 0)
            break;

        if (pflag == SQLRELOAD)
        {
            EXEC SQL BEGIN WORK;
            /* Do not care if it worked or not */
            sqlca.sqlcode = 0;
        }

        stage = 3;
        EXEC SQL OPEN c_insert;
        TRACE((8, "reload: OPEN complete (%ld)\n", sqlca.sqlcode));
        if (sqlca.sqlcode < 0)
            break;

        /* Fetch and print data */
        stage = 4;
        sqlca.sqlcode = 0;
        {
        Memory          line;
        mem_new(&line);
        while (sqlca.sqlcode == 0)
        {
            if (scanrecord(file, idesc, &rec, name, &line) == EOF ||
                sqlca.sqlcode < 0)
                break;
            if (rec.rownum > nskip)
            {
                nput++;
                EXEC SQL PUT c_insert USING DESCRIPTOR idesc;
                TRACE((8, "reload: PUT complete (%ld)\n", sqlca.sqlcode));
#ifdef DEBUG
                if (db_getdebug() >= 9)
                    dump_sqlda(db_getfileptr(), "LOAD: PUT", idesc);
#endif /* DEBUG */
                if (sqlca.sqlcode < 0)
                    break;
            }
            else
                TRACE((8, "reload: SKIP record (%ld of %ld)\n", rec.rownum, ctxt_getskipsize()));
            if (nblobs > 0)
                free_blobs(idesc);
            if (pflag == SQLRELOAD && txsize > 0 && nput > 0 && nput % txsize == 0)
            {
                /* Intermediate transaction */
                EXEC SQL CLOSE c_insert;
                if (sqlca.sqlcode < 0)
                    break;
                EXEC SQL COMMIT WORK;
                /* Do not care if it worked or not */
                sqlca.sqlcode = 0;
                if (ctxt_getsilence() == OP_OFF)
                    fprintf(ctxt_output(), "%d rows committed\n", nput);
                EXEC SQL BEGIN WORK;
                /* Do not care if it worked or not */
                sqlca.sqlcode = 0;
                EXEC SQL OPEN c_insert;
                if (sqlca.sqlcode < 0)
                    break;
            }
        }
        mem_del(&line);
        }
        if (sqlca.sqlcode < 0)
            break;

    } while (0);

    /* Clean up: preserve first error if there was one */
    save = sqlca;
    if (stage >= 4)
    {
        EXEC SQL CLOSE c_insert;
        TRACE((8, "reload: CLOSE complete (%ld)\n", sqlca.sqlcode));
        if (save.sqlcode == 0 && sqlca.sqlcode < 0)
            save = sqlca;
    }

    if (stage >= 3)
    {
        if (pflag == SQLRELOAD)
        {
            EXEC SQL COMMIT WORK;
            /* Do not care if it worked or not */
            sqlca.sqlcode = 0;
            if (ctxt_getsilence() == OP_OFF)
            {
                if (nput == 0 || (txsize == 0 || nput % txsize != 0))
                    fprintf(ctxt_output(), "%s%d rows committed%s\n",
                            (save.sqlcode == 0 || nput <= 1) ? "" : "Up to ",
                            (save.sqlcode == 0 || nput == 0) ? nput : nput - 1,
                            (save.sqlcode == 0) ? "" : " successfully");
            }
        }
        EXEC SQL FREE c_insert;
        TRACE((8, "reload: FREE crsr complete (%ld)\n", sqlca.sqlcode));
        if (save.sqlcode == 0 && sqlca.sqlcode < 0)
            save = sqlca;
    }

    if (stage >= 2)
    {
        jb_unregister(buffer);
        FREE(buffer);
    }

    if (stage >= 1)
    {
        EXEC SQL FREE s_insert;
        TRACE((8, "reload: FREE stmt complete (%ld)\n", sqlca.sqlcode));
        if (save.sqlcode == 0 && sqlca.sqlcode < 0)
            save = sqlca;
        if (idesc != (Sqlda *)0)
            free(idesc);
    }

    /**
    ** NB: cannot release sqlda space -- the next time DESCRIBE is used, it
    ** also frees the sqlda space, which corrupts malloc very effectively!
    ** FREE(sqlda);
    ** Note that this is an ESQL/C version-specific problem.  The code
    ** which is actually in use at the moment *does* free the sqlda memory
    ** with no adverse effects on 7.2x ESQL/C on Solaris, nor on 9.1x, nor,
    ** it is thought, on 5.08.
    */

    sqlca = save;
    if (sqlca.sqlcode < 0)
        sql_error();
}
