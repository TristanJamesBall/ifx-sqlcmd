/*
@(#)File:           $RCSfile: connect.h,v $
@(#)Version:        $Revision: 2015.1 $
@(#)Last changed:   $Date: 2015/07/10 16:05:12 $
@(#)Purpose:        Interface to CONNECT, DISCONNECT, SET CONNECTION parser
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 1998-2007,2015
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

/*TABSTOP=4*/

#ifndef CONNECT_H
#define CONNECT_H

#include "esqlc.h"
#include "context.h"
#include "sqlconn.h"

#ifdef MAIN_PROGRAM
#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
extern const char jlss_id_connect_h[];
const char jlss_id_connect_h[] = "@(#)$Id: connect.h,v 2015.1 2015/07/10 16:05:12 jleffler Exp $";
#endif /* lint */
#endif  /* MAIN_PROGRAM */

#ifndef MAX_EOR
#define MAX_EOR 8
#endif /* MAX_EOR */

enum { MAX_FNAME_LENGTH  = 256 };
enum { MAX_OWNER_LENGTH  = (2*32+3) };
enum { MAX_OBJECT_LENGTH = (2*128+3) };
enum { MAX_FORMAT_LENGTH = 12 };
enum { MAX_XMLTAG_LENGTH = 32 };
enum { MAX_NUMBER_LENGTH = 32 };

/* Statement types -- avoid 0 */
enum StmtType
{
    INFO_ERROR = -9,    /* Must be most negative number */
    STMT_UPLOADERR = -8,
    STMT_RELOADERR = -7,
    STMT_LOADERR = -6,
    STMT_UNLOADERR = -5,
    STMT_SETCONNERR = -4,
    STMT_DISCONNERR = -3,
    STMT_CONNECTERR = -2,
    STMT_ERROR = -1,
    STMT_NONE = 0,
    STMT_SETOTHER = 1,
    STMT_CONNECT = -STMT_CONNECTERR,
    STMT_SETCONN = -STMT_SETCONNERR,
    STMT_DISCONN = -STMT_DISCONNERR,
    STMT_LOAD = -STMT_LOADERR,
    STMT_UNLOAD = -STMT_UNLOADERR,
    STMT_RELOAD = -STMT_RELOADERR,
    STMT_UPLOAD = -STMT_UPLOADERR,
    /* Only INFO_xyz statements have numbers larger than INFO_BASE */
    INFO_BASE = -INFO_ERROR,
    INFO_TABLES,        /* Tables, Views, Public Synonyms, Private Synonyms */
    INFO_SYSTABLES,     /* SQLCMD only */
    INFO_BASETABLES,    /* SQLCMD only */
    INFO_VIEWS,         /* SQLCMD only */
    INFO_SYNONYMS,      /* SQLCMD only */
    INFO_PROCEDURES,    /* SQLCMD only */
    INFO_PROCBODY,      /* SQLCMD only */
    INFO_VIEWBODY,      /* SQLCMD only */
    INFO_COLUMNS,
    INFO_INDEXES,
    INFO_ACCESS,        /* aka PRIVILEGES */
    INFO_REFS_TO,       /* SQLCMD only */
    INFO_REFS_BY,       /* SQLCMD only */
    INFO_STATUS,        /* */
    INFO_FRAGMENTS,     /* */
    INFO_CONSTR_CHECK,  /* not implemented */
    INFO_DATABASES,     /* SQLCMD only */
    INFO_CONNECTIONS,   /* SQLCMD only */
    INFO_TRIGGERS,      /* SQLCMD only */
    INFO_TRIGBODY,      /* SQLCMD only */
    INFO_USERS,         /* SQLCMD only */
    INFO_ROLES,         /* SQLCMD only */
    INFO_HELP           /* SQLCMD only */
};
typedef enum StmtType StmtType;

struct InfoInfo
{
    char     dbase[MAX_DBASE_LENGTH];   /* Database for DB-qualified object name */
    char     server[MAX_DBASE_LENGTH];  /* Server for DB-qualified object name */
    char     owner[MAX_OWNER_LENGTH];   /* Owner qualified object name */
    char     table[MAX_OBJECT_LENGTH];  /* Object name */
    char    *wh_start;      /* INFO - Start of condition for WHERE clause */
    char    *wh_end;        /* INFO - One beyond end of condition for WHERE clause */
};
typedef struct InfoInfo InfoInfo;

struct LoadInfo
{
    char     file[MAX_FNAME_LENGTH];    /* Load/Unload File Name */
    char     delim[2];                  /* LOAD/UNLOAD Delimiter */
    char     quote[2];                  /* LOAD/UNLOAD Quote */
    char     escape[2];                 /* LOAD/UNLOAD Escape */
    char     format[MAX_FORMAT_LENGTH]; /* LOAD/UNLOAD Format (CSV, etc) */
    char     xmltag[MAX_XMLTAG_LENGTH]; /* UNLOAD XML Record tag */
    char     eor[MAX_EOR];              /* LOAD/UNLOAD Escape */
    char     skip[MAX_NUMBER_LENGTH];   /* Number of rows to skip */
    LoadDest pipe_or_file;              /* LOAD/UNLOAD from/to pipe or file */
    FileMode mode;                      /* CREATE or APPEND to file */
    char    *stmt;                      /* SELECT statement for UNLOAD; INSERT statement for LOAD */
};
typedef struct LoadInfo LoadInfo;

extern StmtType parse_connstmt(char *str, ConnInfo *p_conn);
extern StmtType parse_infostmt(char *str, InfoInfo *p_info);
extern StmtType parse_loadstmt(char *str, LoadInfo *p_load);

extern int  conninfo_current_modeansi(void);
extern void conninfo_connect(ConnInfo *conn);
extern void conninfo_disconnect(ConnInfo *conn);
extern void conninfo_print(void);
extern void conninfo_setconnection(ConnInfo *conn);
extern void conninfo_setempty(ConnInfo *conn);
extern void conninfo_release(ConnInfo *c);

extern int set_yydebug(int);

extern void     do_connect(char *str);
extern void     do_disconn(char *str);
extern void     do_setconn(char *str);

#endif  /* CONNECT_H */
