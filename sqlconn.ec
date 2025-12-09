/*
@(#)File:           $RCSfile: sqlconn.ec,v $
@(#)Version:        $Revision: 2011.1 $
@(#)Last changed:   $Date: 2011/09/22 19:20:14 $
@(#)Purpose:        Handle SQL connection management operations
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 2005-06,2008,2010-11
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

/*TABSTOP=4*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "stderr.h"     /* err_remark(), err_error() */
#include "sqlconn.h"
#include "debug.h"      /* TRACE */
#include "tokencmp.h"   /* tokencmp() */
#include "strdotfill.h" /* strdotfile() */
#include "jlss.h"       /* jlss_getline() */
#include "esnprintf.h"

$include esqlinfo.h;

#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
const char jlss_id_sqlconn_ec[] = "@(#)$Id: sqlconn.ec,v 2011.1 2011/09/22 19:20:14 jleffler Exp $";
#endif /* lint */

$ifndef ESQLC_CONNECT;

void sql_connect(ConnInfo *conn)
{
    err_error("CONNECT is not part of this version of ESQL/C\n");
}

void sql_disconn(ConnInfo *conn)
{
    err_error("DISCONNECT is not part of this version of ESQL/C\n");
}

void sql_setconn(ConnInfo *conn)
{
    err_error("SET CONNECTION is not part of this version of ESQL/C\n");
}

$else;

static const char env_sqlcmdpasswords[] = "SQLCMDPASSWORDS";

/* Based on the INFOTPASS mechanism in infotable by Ravikrishna <rkusenet@sympatico.ca> */
/*
** SQLCMDPASSWORDS environment variable contains a file name.
** That file contains entries in the format:  dbname|username|password
** Neither dbname nor username may contain pipe symbols - the password field can.
** If neither username nor password is specified, then the first line starting with
** the database name is used to define username and password used to connect to database.
** If username is specified (but password is not), then only an entry with the correct
** database name and username is used.
** If a username is specified and no matching entry is found, a warning is generated,
** and the connection is made without any username.
*/
enum { MAX_PASSWORD_LINELENGTH = 1024 };
enum { SQLCMDPASSWORDDELIMITER = '|' };
/* Debugging level (-Z n) needed to trace password file handling */
enum { DEBUG_LEVEL_PWDFILE = 4 };

static void read_password_file(const char *dbname, char **uname, char **upass, char *buffer, size_t buflen)
{
    char *filename;
    FILE *fp;
    size_t db_len = strlen(dbname);
    size_t us_len = (*uname == 0) ? 0 : strlen(*uname);
    int ln_len;
    int dbmatch = 0;

    TRACE((DEBUG_LEVEL_PWDFILE, "-->> read_password_file(): (dbname=%s, uname=%s)\n", dbname, (*uname?*uname:"<nothing>")));
    if ((filename = getenv(env_sqlcmdpasswords)) == 0)
    {
        /* Password file unset - not an error at all */
        TRACE((DEBUG_LEVEL_PWDFILE, "<<-- read_password_file():\n"));
        return;
    }
    TRACE((DEBUG_LEVEL_PWDFILE, "---- read_password_file(): open file %s\n", filename));
    if ((fp = fopen(filename, "r")) == 0)
    {
        err_remark("cannot open password file %s\n", filename);
        TRACE((DEBUG_LEVEL_PWDFILE, "<<-- read_password_file():\n"));
        return;
    }

    /* Validate security of opened file? */

    /* Read file, looking for matches */
    /* Note that lines beginning '#' are comments, and blank lines are ignored explicitly */
    /* They would also be ignored implicitly for not containing a suitable match. */
    while ((ln_len = jlss_getline(fp, buffer, buflen)) > 0)
    {
        char *delim1;
        TRACE((DEBUG_LEVEL_PWDFILE, "---- read line %s", buffer));
        if (buffer[0] == '#' || buffer[0] == '\n')
            continue;
        if ((delim1 = strchr(buffer, SQLCMDPASSWORDDELIMITER)) == 0)
            TRACE((DEBUG_LEVEL_PWDFILE, "---- read_password_file(): malformed input line (no pipe after dbname): %s", buffer));
        else
        {
            char *delim2;
            TRACE((DEBUG_LEVEL_PWDFILE, "---- dbname %.*s\n", delim1 - buffer, buffer));
            if ((delim2 = strchr(delim1+1, SQLCMDPASSWORDDELIMITER)) == 0)
                TRACE((DEBUG_LEVEL_PWDFILE, "---- read_password_file(): malformed input line (no pipe after uname): %s", buffer));
            else
            {
                /* dbname is in buffer..delim1 and username is in delim1+1..delim2 */
                TRACE((DEBUG_LEVEL_PWDFILE, "---- uname %.*s\n", delim2 - (delim1+1), delim1+1));
                if (tokencmp(dbname, db_len, buffer, delim1 - buffer) == 0)
                {
                    /* found match on dbname */
                    TRACE((DEBUG_LEVEL_PWDFILE, "---- match on dbname %.*s\n", delim1 - buffer, buffer));
                    dbmatch = 1;
                    if (*uname == 0 || **uname == '\0')
                    {
                        /* No user name given - first match is used */
                        *uname = delim1+1;
                        *delim2 = '\0';
                        *upass = delim2+1;
                        buffer[ln_len-1] = '\0';
                        TRACE((DEBUG_LEVEL_PWDFILE, "<<-- read_password_file(): (username = %s, password = %s)\n", *uname, *upass));
                        fclose(fp);
                        return;
                    }
                    else if (tokencmp(*uname, us_len, delim1+1, delim2 - (delim1+1)) == 0)
                    {
                        /* case-insensitive username match - haven't (yet) got a convenient case-sensitive comparator for this job */
                        *upass = delim2+1;
                        buffer[ln_len-1] = '\0';
                        TRACE((DEBUG_LEVEL_PWDFILE, "<<-- read_password_file(): (password = %s)\n", *upass));
                        fclose(fp);
                        return;
                    }
                }
            }
        }
    }
    fclose(fp);

    if (*uname != 0 && dbmatch != 0)
    {
        /* Warn if username not found with database - avoiding buffer overflow */
        char notes[512];    /* Space to spare... */
        char db[128];   /* Theoretical max is 128+128+2 */
        char us[64];
        char fn[256];
        esnprintf(notes, sizeof(notes), "(database=%s, username=%s, file=%s)",
                  strdotfill(db, sizeof(db), dbname, db_len),
                  strdotfill(us, sizeof(us), *uname, us_len),
                  strdotfill(fn, sizeof(fn), filename, strlen(filename)));
        err_remark("user name not found with database name in password file %s\n", notes);
        return;
    }

    TRACE((DEBUG_LEVEL_PWDFILE, "<<-- read_password_file():\n"));
}

static void set_conninfo(ConnInfo *conn)
{
    TRACE((6, "-->> set_conninfo\n"));
    conn->current   = 1;
    conn->logged    = (sqlca.sqlwarn.sqlwarn1 == 'W');
    conn->online    = (sqlca.sqlwarn.sqlwarn3 == 'W');
    conn->mode_ansi = (sqlca.sqlwarn.sqlwarn2 == 'W');
    TRACE((6, "<<-- set_conninfo\n"));
}

/* Execute CONNECT described by ConnInfo structure */
/*
** JL 2005-02-24: ESQL/C syntactically permits CONNECT with user name
** and no password, but connection fails with -952
** JL 2010-12-18: It is a nuisance having to create a new statement for
** each permutation of options!
** JL 2010-12-18: Policy decision - trusted context requires user name
** and password and explicit database.  Connecting to DEFAULT or without
** user name and password does not use TRUSTED.
*/
void sql_connect(ConnInfo *conn)
{
    EXEC SQL BEGIN DECLARE SECTION;
    char    *dbase = conn->database;
    char    *uconn = conn->connname;
    char    *uname = conn->username;
    char    *upass = conn->password;
    EXEC SQL END DECLARE SECTION;
    char buffer[MAX_PASSWORD_LINELENGTH];

    if (dbase != 0 && (uname == 0 || upass == 0))
        read_password_file(dbase, &uname, &upass, buffer, sizeof(buffer));

$ifndef ESQLC_TRUSTED_CONTEXT;
    conn->trusted = 0;
$endif;

    TRACE((DEBUG_LEVEL_PWDFILE, "---- sql_connect(): "
            "database = %s; connname = %s; username = %s; password = %s; wct = %d, trusted = %d, ctype = %d\n",
            dbase == 0 ? "<<>>" : dbase, uconn == 0 ? "<<>>" : uconn,
            uname == 0 ? "<<>>" : uname, upass == 0 ? "<<>>" : upass,
            conn->wct, conn->trusted, conn->ctype));

    if (conn->wct == 0)
    {
        if (conn->ctype == CONN_DEFAULT)
        {
            EXEC SQL CONNECT TO DEFAULT;
        }
        else if (conn->ctype == CONN_NONE)
        {
            assert(dbase != 0 && *dbase != '\0');
            assert(uconn == 0);
            if (uname == 0 || upass == 0)
            {
                assert(upass == 0);
                EXEC SQL CONNECT TO :dbase;
            }
            else
            {
                assert(upass != 0);
                /* Trusted! */
$ifdef ESQLC_TRUSTED_CONTEXT;
                if (conn->trusted)
                {
                    EXEC SQL CONNECT TO :dbase USER :uname USING :upass TRUSTED;
                }
                else
$endif;
                {
                    EXEC SQL CONNECT TO :dbase USER :uname USING :upass;
                }
            }
        }
        else
        {
            assert(conn->ctype == CONN_STRING);
            assert(uconn != 0);
            if (uname == 0 || upass == 0)
            {
                assert(upass == 0);
                EXEC SQL CONNECT TO :dbase AS :uconn;
            }
            else
            {
                assert(upass != 0);
                /* Trusted! */
$ifdef ESQLC_TRUSTED_CONTEXT;
                if (conn->trusted)
                {
                    EXEC SQL CONNECT TO :dbase AS :uconn
                        USER :uname USING :upass TRUSTED;
                }
                else
$endif;
                {
                    EXEC SQL CONNECT TO :dbase AS :uconn
                        USER :uname USING :upass;
                }
            }
        }
    }
    else
    {
        if (conn->ctype == CONN_DEFAULT)
        {
            EXEC SQL CONNECT TO DEFAULT
                    WITH CONCURRENT TRANSACTION;
        }
        else if (conn->ctype == CONN_NONE)
        {
            assert(dbase != 0 && *dbase != '\0');
            assert(uconn == 0);
            if (uname == 0 || upass == 0)
            {
                assert(upass == 0);
                EXEC SQL CONNECT TO :dbase
                            WITH CONCURRENT TRANSACTION;
            }
            else
            {
                assert(upass != 0);
                /* Trusted! */
$ifdef ESQLC_TRUSTED_CONTEXT;
                if (conn->trusted)
                {
                    EXEC SQL CONNECT TO :dbase USER :uname USING :upass
                                WITH CONCURRENT TRANSACTION TRUSTED;
                }
                else
$endif;
                {
                    EXEC SQL CONNECT TO :dbase USER :uname USING :upass
                                WITH CONCURRENT TRANSACTION;
                }
            }
        }
        else
        {
            assert(conn->ctype == CONN_STRING);
            if (uname == 0 || upass == 0)
            {
                assert(upass == 0);
                EXEC SQL CONNECT TO :dbase AS :uconn
                            WITH CONCURRENT TRANSACTION;
            }
            else
            {
                assert(upass != 0);
                /* Trusted! */
$ifdef ESQLC_TRUSTED_CONTEXT;
                if (conn->trusted)
                {
                    EXEC SQL CONNECT TO :dbase AS :uconn
                                USER :uname USING :upass
                                WITH CONCURRENT TRANSACTION;
                }
                else
$endif;
                {
                    EXEC SQL CONNECT TO :dbase AS :uconn
                                USER :uname USING :upass
                                WITH CONCURRENT TRANSACTION;
                }
            }
        }
    }
    if (sqlca.sqlcode == 0)
        set_conninfo(conn);
}

/* Execute DISCONNECT described by ConnInfo structure */
void     sql_disconn(ConnInfo *conn)
{
    EXEC SQL BEGIN DECLARE SECTION;
    char    *uconn = conn->connname;
    EXEC SQL END DECLARE SECTION;

    switch (conn->ctype)
    {
    case CONN_ALL:
        EXEC SQL DISCONNECT ALL;
        break;
    case CONN_CURRENT:
        EXEC SQL DISCONNECT CURRENT;
        break;
    case CONN_DEFAULT:
        EXEC SQL DISCONNECT DEFAULT;
        break;
    case CONN_STRING:
        EXEC SQL DISCONNECT :uconn;
        break;
    default:
        err_error("sql_disconn(): Internal error - DISCONNECT type out of control\n");
        break;
    }
}

/* Execute SET CONNECTION described by ConnInfo structure */
void sql_setconn(ConnInfo *conn)
{
    EXEC SQL BEGIN DECLARE SECTION;
    char    *uconn = conn->connname;
    EXEC SQL END DECLARE SECTION;

    if (conn->ctype == CONN_DEFAULT)
    {
        EXEC SQL SET CONNECTION DEFAULT;
    }
    else
    {
        assert(conn->ctype == CONN_STRING);
        EXEC SQL SET CONNECTION :uconn;
    }
}

$endif; /* ESQLC_CONNECT */
