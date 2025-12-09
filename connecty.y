%{
/*
@(#)File:           $RCSfile: connecty.y,v $
@(#)Version:        $Revision: 2012.1 $
@(#)Last changed:   $Date: 2012/08/05 18:33:50 $
@(#)Purpose:        Grammar for SQLCMD statements
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 1998-2010,2012
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

/*TABSTOP=4*/

/*
** This grammar recognizes CONNECT, SET CONNECTION, DISCONNECT,
** INFO, LOAD, UNLOAD and RELOAD statements.
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "context.h"
#include "esqlutil.h"
#include "internal.h"
#include "jlss.h"
#include "connect.h"
#include "stderr.h"
#include "sqltoken.h"
#include "strdotfill.h"
#include "emalloc.h"

#ifndef DO_NOT_USE_JLSSYACC_DEFINES
/*
** If you are using JLSSYACC, these defines hide all the YACC symbols which
** are normally visible (see connecty.c.std).  These defines do not normally
** cause problems with a standard version of YACC.  If they do cause you
** problems, add -DDO_NOT_USE_JLSSYACC_DEFINES to the C compiler command
** line to remove them, and let Jonathan Leffler know which version of YACC
** you are using, and which platform you are using it on.
*/
#define YYERROR_UNUSED
#define YYBACKUP_UNUSED
#define YY_SC_PARSE     static
#define YY_SC_LEX       static
#define YY_SC_SYNTAX    static
#define YYGLOBAL        static
#define YYSTATIC        static
#else
#define YYSTATIC        extern
#endif /* DO_NOT_USE_JLSSYACC_DEFINES */

/*
** BISON only defines yydebug if YYDEBUG is set.
** BYACC defines yydebug unconditionally.
** JLSSYACC defines yydebug unless explicitly suppressed with YYDEBUG_UNUSED.
** Unix YACC defines yydebug unconditionally.
*/
#ifndef YYDEBUGVAR
#define YYDEBUGVAR yydebug
#endif
#if YYBISON && !YYDEBUG
int YYDEBUGVAR = 0;
#endif

struct Token
{
    int      toktype;
    char    *start;
    char    *end;
};
typedef struct Token Token;

#define YYSTYPE Token

static StmtType stmt_type;
static ConnInfo *conn;
static InfoInfo *info;
static LoadInfo *load;
static const char *c_token;     /* Where to start next token search */

static void set_connection(ConnInfo *c, const char *s, const char *e);
static void set_database(ConnInfo *c, const char *s, const char *e);
static void set_password(ConnInfo *c, const char *s, const char *e);
static void set_username(ConnInfo *c, const char *s, const char *e);
static void set_string(const char *type, char *output, size_t outlen, const char *s, const char *e);
static void set_symbol(const char *type, char *output, size_t outlen, const char *s, const char *e);

/* Determine whether a bare filename is OK - for LOAD FROM /something/or/other.unl ... etc */
static int  bare_filename_ok = 0;

/* Primary entry point - yyparse() */
#ifndef YYJLSSYACC
YYSTATIC int  yyparse(void);
static   int  yylex(void);
static   void yyerror(const char *s);
#endif /* YYJLSSYACC */

#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
const char jlss_id_connecty_y[] = "@(#)$Id: connecty.y,v 2012.1 2012/08/05 18:33:50 jleffler Exp $";
#endif /* lint */
%}

%token K_ACCESS
%token K_ALL
%token K_APPEND
%token K_AS
%token K_BASETABLES
%token K_BY
%token K_CHECK
%token K_COLUMNS
%token K_CONCURRENT
%token K_CONNECT
%token K_CONNECTION
%token K_CONNECTIONS
%token K_CONSTRAINTS
%token K_CREATE
%token K_CSV
%token K_CURRENT
%token K_DATABASES
%token K_DEFAULT
%token K_DELIMITER
%token K_DISCONNECT
%token K_EOR
%token K_ESCAPE
%token K_EXECUTE
%token K_FILE
%token K_FIXED
%token K_FIXSEP
%token K_FIXDEL
%token K_DB2
%token K_FOR
%token K_FORMAT
%token K_FRAGMENTS
%token K_FROM
%token K_FUNCTION
%token K_HELP
%token K_INDEXES
%token K_INFO
%token K_INSERT
%token K_LOAD
%token K_PASSWORD
%token K_PIPE
%token K_PRIVILEGES
%token K_PROCBODY
%token K_PROCEDURE
%token K_PROCEDURES
%token K_QUOTE
%token K_RECORDTAG
%token K_REFERENCES
%token K_RELOAD
%token K_ROLES
%token K_SELECT
%token K_SET
%token K_SKIP
%token K_STATUS
%token K_SYNONYMS
%token K_SYSTABLES
%token K_TABLES
%token K_TO
%token K_TRANSACTIONS
%token K_TRIGBODY
%token K_TRIGGERS
%token K_TRUSTED
%token K_UNLOAD
%token K_USER
%token K_USERS
%token K_USING
%token K_VIEWBODY
%token K_VIEWS
%token K_WCT
%token K_WHERE
%token K_WITH

%token C_AT
%token C_COLON
%token C_LPAREN
%token C_PERIOD
%token C_RPAREN
%token C_SEMICOLON

%token S_IDENTIFIER
%token S_DQSTRING
%token S_SQSTRING
%token S_ERROR
%token S_NUMBER

%start statement

%%

statement
    :   /* Nothing */ opt_semicolon
        { stmt_type = STMT_NONE; }
    |   connect opt_semicolon
        { stmt_type = STMT_CONNECT; }
    |   disconnect opt_semicolon
        { stmt_type = STMT_DISCONN; }
    |   setconnect opt_semicolon
        { stmt_type = STMT_SETCONN; }
    |   setother opt_semicolon
        { stmt_type = STMT_SETOTHER; }
    |   info opt_semicolon
        { stmt_type = (StmtType) $1.toktype; /*=C++=*/ }
    |   load
        { stmt_type = STMT_LOAD; YYACCEPT; }
    |   unload
        { stmt_type = STMT_UNLOAD; YYACCEPT; }
    |   reload
        { stmt_type = STMT_RELOAD; YYACCEPT; }
    |   K_UNLOAD error token_list
        { stmt_type = STMT_UNLOADERR; }
    |   K_RELOAD error token_list
        { stmt_type = STMT_RELOADERR; }
    |   K_LOAD error token_list
        { stmt_type = STMT_LOADERR; }
    |   K_DISCONNECT error token_list
        { stmt_type = STMT_DISCONNERR; }
    |   K_CONNECT error token_list
        { stmt_type = STMT_CONNECTERR; }
    |   K_SET K_CONNECTION error token_list
        { stmt_type = STMT_SETCONNERR; }
    |   K_INFO error token_list
        { stmt_type = INFO_ERROR; }
    |   error
        {
            if (stmt_type >= INFO_BASE)
                stmt_type = INFO_ERROR;
            else if (stmt_type > STMT_NONE)
                stmt_type = (StmtType)(-stmt_type); /*=C++=*/
            else
                stmt_type = STMT_ERROR;
        }
    ;

opt_semicolon
    :   /* Nothing */
    |   C_SEMICOLON
    ;

connect
    :   K_CONNECT K_TO K_DEFAULT opt_wct
        { conn->ctype = CONN_DEFAULT; }
    |   K_CONNECT dbenv opt_conn opt_user opt_wct opt_trusted
    ;

dbenv
    :   K_TO string
        { set_database(conn, $2.start, $2.end); }
    ;

opt_wct
    :   /* Nothing */
        { conn->wct = 0; }
    |   K_WCT   /* Abbreviation */
        { conn->wct = 1; }
    |   K_WITH K_CONCURRENT K_TRANSACTIONS
        { conn->wct = 1; }
    ;

opt_trusted
    :   /* Nothing */
        { conn->trusted = 0; }
    |   K_TRUSTED
        { conn->trusted = 1; }
    ;

opt_conn
    :   /* Nothing */
        { conn->ctype = CONN_NONE; }
    |   K_AS string
        { conn->ctype = CONN_STRING;
          set_connection(conn, $2.start, $2.end);
        }
    ;

opt_user
    :   /* Nothing */
        { conn->username = 0; conn->password= 0; }
    |   username password
    ;

username
    :   K_USER string
        { set_username(conn, $2.start, $2.end); }
    ;

password
    :   using_or_password string
        { set_password(conn, $2.start, $2.end); }
    ;

using_or_password
    :   K_USING
    |   K_PASSWORD
    ;

disconnect
    :   K_DISCONNECT K_CURRENT
        { conn->ctype = CONN_CURRENT; }
    |   K_DISCONNECT K_ALL
        { conn->ctype = CONN_ALL; }
    |   K_DISCONNECT K_DEFAULT
        { conn->ctype = CONN_DEFAULT; }
    |   K_DISCONNECT string
        { conn->ctype = CONN_STRING;
          set_connection(conn, $2.start, $2.end);
        }
    ;

setconnect
    :   K_SET conntype
    ;

setother
    :   K_SET error token_list
        { yyerrok; }
    ;

token_list
    :   /* Nothing */
    |   token_list token
        { $$ = $2; }
    ;

token
    :   keyword
    |   non_keyword
    ;

keyword
    :   K_ACCESS
    |   K_ALL
    |   K_APPEND
    |   K_AS
    |   K_BASETABLES
    |   K_BY
    |   K_CHECK
    |   K_COLUMNS
    |   K_CONCURRENT
    |   K_CONNECT
    |   K_CONNECTION
    |   K_CONNECTIONS
    |   K_CONSTRAINTS
    |   K_CREATE
    |   K_CSV
    |   K_CURRENT
    |   K_DATABASES
    |   K_DB2
    |   K_DEFAULT
    |   K_DELIMITER
    |   K_DISCONNECT
    |   K_EOR
    |   K_ESCAPE
    |   K_EXECUTE
    |   K_FILE
    |   K_FIXDEL
    |   K_FIXED
    |   K_FIXSEP
    |   K_FOR
    |   K_FORMAT
    |   K_FRAGMENTS
    |   K_FROM
    |   K_FUNCTION
    |   K_HELP
    |   K_INDEXES
    |   K_INFO
    |   K_INSERT
    |   K_LOAD
    |   K_PASSWORD
    |   K_PIPE
    |   K_PRIVILEGES
    |   K_PROCBODY
    |   K_PROCEDURE
    |   K_PROCEDURES
    |   K_QUOTE
    |   K_RECORDTAG
    |   K_REFERENCES
    |   K_RELOAD
    |   K_ROLES
    |   K_SELECT
    |   K_SET
    |   K_SKIP
    |   K_STATUS
    |   K_SYNONYMS
    |   K_SYSTABLES
    |   K_TABLES
    |   K_TO
    |   K_TRANSACTIONS
    |   K_TRIGBODY
    |   K_TRIGGERS
    |   K_TRUSTED
    |   K_UNLOAD
    |   K_USER
    |   K_USERS
    |   K_USING
    |   K_VIEWBODY
    |   K_VIEWS
    |   K_WCT
    |   K_WHERE
    |   K_WITH
    ;

non_keyword
    :   C_AT
    |   C_COLON
    |   C_PERIOD
    |   C_LPAREN
    |   C_RPAREN
    |   S_ERROR
    |   string
    |   S_IDENTIFIER
    |   S_NUMBER
    ;

string
    :   S_SQSTRING
    |   S_DQSTRING
    ;

conntype
    :   K_CONNECTION string
        { conn->ctype = CONN_STRING;
          set_connection(conn, $2.start, $2.end);
        }
    |   K_CONNECTION K_DEFAULT
        { conn->ctype = CONN_DEFAULT; }
    ;

load
    :   K_LOAD K_FROM opt_file_pipe string load_opt_list K_INSERT
        {
            set_string("load file", load->file, sizeof(load->file), $4.start, $4.end);
            load->stmt = $6.start;
        }
    ;

opt_file_pipe
    :   /* Nothing */
        { load->pipe_or_file = LD_FILE; }
    |   K_FILE
        { load->pipe_or_file = LD_FILE; }
    |   K_PIPE
        { load->pipe_or_file = LD_PIPE; }
    ;

load_opt_list
    :   /* Nothing */
    |   load_opt_list load_opt
    ;

load_opt
    :   common_opt
    |   skip
    ;

common_opt
    :   delimiter
    |   quote
    |   escape
    |   eor
    |   format
    ;

format
    :   K_FORMAT string
        { set_string("format name", load->format, sizeof(load->format), $2.start, $2.end); }
    |   K_FORMAT fmt_kwd
        { set_symbol("format name", load->format, sizeof(load->format), $2.start, $2.end); }
    ;

fmt_kwd
    :   K_CSV
    |   K_DB2
    |   K_FIXDEL
    |   K_FIXED
    |   K_FIXSEP
    |   K_QUOTE
    |   K_SELECT
    |   K_UNLOAD
/*  |   K_XML */  /* Needs more complex support - and differentiation between load and unload formats */
    ;

skip
    :   K_SKIP number
        { set_symbol("skip", load->skip, sizeof(load->skip), $2.start, $2.end); }
    ;

number
    :   S_NUMBER
    ;

reload
    :   K_RELOAD K_FROM opt_file_pipe string reload_opt_list K_INSERT
        {
            set_string("reload file", load->file, sizeof(load->file), $4.start, $4.end);
            load->stmt = $6.start;
        }
    ;

reload_opt_list
    :   /* Nothing */
    |   reload_opt_list reload_opt
    ;

/* RELOAD options are the same as LOAD options, at least at the moment */
reload_opt
    :   load_opt
    ;

unload
    :   K_UNLOAD unload_dst unload_opt_list unload_src
    ;

/*
** UNLOAD [CREATE|APPEND] [TO] [FILE] filename
** -- UNLOAD TO filename
** -- UNLOAD CREATE FILE filename
** -- UNLOAD TO FILE filename
** -- UNLOAD FILE filename
** -- UNLOAD APPEND TO filename
** -- UNLOAD APPEND filename
** -- UNLOAD APPEND TO FILE filename
** -- UNLOAD APPEND TO filename
** -- UNLOAD filename???
** UNLOAD [TO] PIPE pipecommand
*/

unload_dst
    :   unload_file
        { load->pipe_or_file = LD_FILE; }
    |   unload_pipe
        { load->pipe_or_file = LD_PIPE; }
    ;

unload_file
    :   to_file
    |   create_file
    |   append_file
    ;

unload_pipe
    :   to_pipe
    ;

to_file
    :   K_TO opt_file filename
        { load->mode = FM_CREATE; }
    ;

opt_file
    :   /* Nothing */
    |   K_FILE
    ;

filename
    :   string
        { set_string("unload file", load->file, sizeof(load->file), $1.start, $1.end); }
    ;

create_file
    :   K_CREATE opt_file filename
        { load->mode = FM_CREATE; }
    ;

append_file
    :   K_APPEND opt_to opt_file filename
        { load->mode = FM_APPEND; }
    ;

to_pipe
    :   opt_to K_PIPE shell_command
        { load->pipe_or_file = LD_PIPE; }
    ;

shell_command
    :   filename
    ;

unload_opt_list
    :   /* Nothing */
    |   unload_opt_list unload_opt
    ;

/* Eventually, there will be more options */
unload_opt
    :   common_opt
    |   xmlrectag
    ;

unload_src
    :   K_SELECT
        { load->stmt = $1.start; }
    |   K_EXECUTE proc_or_func
        { load->stmt = $1.start; }
    ;

opt_to
    :   /* Nothing */
    |   K_TO
    ;

proc_or_func
    :   K_PROCEDURE
    |   K_FUNCTION
    ;

xmlrectag
    :   K_RECORDTAG string
        { set_string("XML record tag", load->xmltag, sizeof(load->xmltag), $2.start, $2.end); }
    ;

delimiter
    :   K_DELIMITER string
        { set_string("delimiter", load->delim, sizeof(load->delim), $2.start, $2.end); }
    ;

quote
    :   K_QUOTE string
        { set_string("quote", load->quote, sizeof(load->quote), $2.start, $2.end); }
    ;

eor
    :   K_EOR string
        { set_string("eor", load->eor, sizeof(load->eor), $2.start, $2.end); }
    ;

escape
    :   K_ESCAPE string
        { set_string("escape", load->escape, sizeof(load->escape), $2.start, $2.end); }
    ;

info
    :   info_unqualified
    |   info_qualified
    ;

info_unqualified
    :   K_INFO K_TABLES opt_where
        { $$.toktype = INFO_TABLES; }
    |   K_INFO K_SYSTABLES opt_where
        { $$.toktype = INFO_SYSTABLES; }
    |   K_INFO K_BASETABLES opt_where
        { $$.toktype = INFO_BASETABLES; }
    |   K_INFO K_VIEWS opt_where
        { $$.toktype = INFO_VIEWS; }
    |   K_INFO K_SYNONYMS opt_where
        { $$.toktype = INFO_SYNONYMS; }
    |   K_INFO K_PROCEDURES opt_where
        { $$.toktype = INFO_PROCEDURES; }
    |   K_INFO K_DATABASES
        { $$.toktype = INFO_DATABASES; }
    |   K_INFO K_CONNECTIONS
        { $$.toktype = INFO_CONNECTIONS; }
    |   K_INFO K_USERS opt_where
        { $$.toktype = INFO_USERS; }
    |   K_INFO K_ROLES opt_where
        { $$.toktype = INFO_ROLES; }
    |   K_INFO K_HELP
        { $$.toktype = INFO_HELP; }
    ;

opt_where
    :   /* Nothing */
    |   K_WHERE token_list
        { info->wh_start = $1.end; info->wh_end = $2.end; }
    ;

info_qualified
    :   K_INFO K_COLUMNS for_objname
        { $$.toktype = INFO_COLUMNS; }
    |   K_INFO K_INDEXES for_objname
        { $$.toktype = INFO_INDEXES; }
    |   K_INFO K_ACCESS for_objname
        { $$.toktype = INFO_ACCESS; }
    |   K_INFO K_PRIVILEGES for_objname
        { $$.toktype = INFO_ACCESS; }
    |   K_INFO K_STATUS for_objname
        { $$.toktype = INFO_STATUS; }
    |   K_INFO K_PROCBODY for_objname
        { $$.toktype = INFO_PROCBODY; }
    |   K_INFO K_FRAGMENTS for_objname
        { $$.toktype = INFO_FRAGMENTS; }
    |   K_INFO K_REFERENCES to_table
        { $$.toktype = INFO_REFS_TO; }
    |   K_INFO K_REFERENCES by_table
        { $$.toktype = INFO_REFS_BY; }
    |   K_INFO K_VIEWBODY for_objname
        { $$.toktype = INFO_VIEWBODY; }
    |   K_INFO K_TRIGGERS for_objname
        { $$.toktype = INFO_TRIGGERS; }
    |   K_INFO K_TRIGBODY for_objname
        { $$.toktype = INFO_TRIGBODY; }
    |   K_INFO K_CHECK K_CONSTRAINTS for_objname
        { $$.toktype = INFO_CONSTR_CHECK; }
    ;

for_objname
    :   K_FOR object_name
    ;

to_table
    :   K_TO object_name
    ;

by_table
    :   K_BY object_name
    ;

identifier_or_keyword
    :   S_IDENTIFIER
    |   keyword
    ;

object_name
    :   object
    |   obj_dbase object
    ;

obj_dbase
    :   obj_dbname opt_server C_COLON
    ;

opt_server
    :   /* Nothing */
    |   C_AT identifier_or_keyword
        { set_symbol("dbserver", info->server, sizeof(info->server), $2.start, $2.end); }
    ;

obj_dbname
    :   identifier_or_keyword
        { set_symbol("database", info->dbase, sizeof(info->dbase), $1.start, $1.end); }
    ;

object
    :   obj_name
        { set_symbol("table", info->table, sizeof(info->table), $1.start, $1.end); }
    |   obj_owner C_PERIOD obj_name
        {
            /* Must preserve quotes or no-quotes around */
            /* owner to work with MODE ANSI databases!! */
            set_symbol("owner", info->owner, sizeof(info->owner), $1.start, $1.end);
            set_symbol("table", info->table, sizeof(info->table), $3.start, $3.end);
        }
    ;

obj_name
    :   identifier_or_keyword
    |   S_DQSTRING
    ;

obj_owner
    :   identifier_or_keyword
    |   string
    ;

%%

struct Keyword
{
    const char *keyword;
    int         token;
};

typedef struct Keyword Keyword;

/* keylist to be maintained in alphabetic order of keyword */

/**INDENT-OFF**/
static Keyword keylist[] =
{
    {   "ACCESS",           K_ACCESS        },
    {   "ALL",              K_ALL           },
    {   "APPEND",           K_APPEND        },
    {   "AS",               K_AS            },
    {   "BASETABLES",       K_BASETABLES    },
    {   "BY",               K_BY            },
    {   "CHECK",            K_CHECK         },
    {   "COLUMNS",          K_COLUMNS       },
    {   "CONCURRENT",       K_CONCURRENT    },
    {   "CONNECT",          K_CONNECT       },
    {   "CONNECTION",       K_CONNECTION    },
    {   "CONNECTIONS",      K_CONNECTIONS   },
    {   "CONSTRAINTS",      K_CONSTRAINTS   },
    {   "CREATE",           K_CREATE        },
    {   "CSV",              K_CSV           },
    {   "CURRENT",          K_CURRENT       },
    {   "DATABASES",        K_DATABASES     },
    {   "DB2",              K_DB2           },
    {   "DEFAULT",          K_DEFAULT       },
    {   "DELIMITER",        K_DELIMITER     },
    {   "DISCONNECT",       K_DISCONNECT    },
    {   "EOR",              K_EOR           },
    {   "ESCAPE",           K_ESCAPE        },
    {   "EXECUTE",          K_EXECUTE       },
    {   "FILE",             K_FILE          },
    {   "FIXDEL",           K_FIXDEL        },
    {   "FIXED",            K_FIXED         },
    {   "FIXSEP",           K_FIXSEP        },
    {   "FOR",              K_FOR           },
    {   "FORMAT",           K_FORMAT        },
    {   "FRAGMENTS",        K_FRAGMENTS     },
    {   "FROM",             K_FROM          },
    {   "FUNCTION",         K_FUNCTION      },
    {   "HELP",             K_HELP          },
    {   "INDEXES",          K_INDEXES       },
    {   "INFO",             K_INFO          },
    {   "INSERT",           K_INSERT        },
    {   "LOAD",             K_LOAD          },
    {   "PASSWORD",         K_PASSWORD      },
    {   "PIPE",             K_PIPE          },
    {   "PRIVILEGES",       K_PRIVILEGES    },
    {   "PROCBODY",         K_PROCBODY      },
    {   "PROCEDURE",        K_PROCEDURE     },
    {   "PROCEDURES",       K_PROCEDURES    },
    {   "QUOTE",            K_QUOTE         },
    {   "RECORDTAG",        K_RECORDTAG     },
    {   "REFERENCES",       K_REFERENCES    },
    {   "RELOAD",           K_RELOAD        },
    {   "ROLES",            K_ROLES         },
    {   "SELECT",           K_SELECT        },
    {   "SET",              K_SET           },
    {   "SKIP",             K_SKIP          },
    {   "STATUS",           K_STATUS        },
    {   "SYNONYMS",         K_SYNONYMS      },
    {   "SYSTABLES",        K_SYSTABLES     },
    {   "TABLES",           K_TABLES        },
    {   "TO",               K_TO            },
    {   "TRANSACTIONS",     K_TRANSACTIONS  },
    {   "TRIGBODY",         K_TRIGBODY      },
    {   "TRIGGERS",         K_TRIGGERS      },
    {   "UNLOAD",           K_UNLOAD        },
    {   "USER",             K_USER          },
    {   "USERS",            K_USERS         },
    {   "USING",            K_USING         },
    {   "VIEWBODY",         K_VIEWBODY      },
    {   "VIEWS",            K_VIEWS         },
    {   "WCT",              K_WCT           },
    {   "WHERE",            K_WHERE         },
    {   "WITH",             K_WITH          },
/**INDENT-ON**/
};

#define DIM(x)  (sizeof(x)/sizeof(*(x)))
#define CONST_CAST(type, value) ((type)(value))

#define MAX_LEXTOKENLENGTH  (2*SQL_NAMELEN+3)

static int kw_compare(const void *p1, const void *p2)
{
    const Keyword *k1 = (Keyword *)p1;  /*=C++=*/
    const Keyword *k2 = (Keyword *)p2;  /*=C++=*/
    return(cistrcmp(k1->keyword, k2->keyword));
}

static void set_token(char *output, size_t outlen, const char *s, const char *e)
{
    size_t space = e - s + 1;
    if (outlen < space)
        space = outlen;
    nstrcpy(output, space, s);
}

static void set_symbol(const char *type, char *output, size_t outlen, const char *s, const char *e)
{
    size_t space = e - s + 1;
    if (*output != '\0')
    {
        err_remark("repeat specification of %s (old = %s, new = %.*s); using new value.\n",
                   type, output,  (int)(e - s), s);
    }
    if (outlen < space)
    {
        char ebuff[256];
        err_remark("overflow detected: %s is far too long (requested %d, %d available: %s)\n",
                    type, (int)space, (int)outlen, strdotfill(ebuff, sizeof(ebuff), s, space));
        space = outlen;
    }
    nstrcpy(output, space, s);
}

/* Duplicate a quote-enclosed string */
/* s points to opening quote; e to closing quote */
static char *strcdup(const char *s, const char *e)
{
    char *base = (char *)MALLOC(--e - ++s + 1);     /*=C++=*/
    nstrcpy(base, e - s + 1, s);
    return(base);
}

static void set_database(ConnInfo *c, const char *s, const char *e)
{
    c->database = strcdup(s, e);
}

static void set_connection(ConnInfo *c, const char *s, const char *e)
{
    c->connname = strcdup(s, e);
}

static void set_username(ConnInfo *c, const char *s, const char *e)
{
    c->username = strcdup(s, e);
}

static void set_password(ConnInfo *c, const char *s, const char *e)
{
    c->password = strcdup(s, e);
}

static void set_string(const char *type, char *output, size_t outlen, const char *s, const char *e)
{
    set_symbol(type, output, outlen, s + 1, e - 1);
}

static void set_statement(char *stmt_str)
{
#ifndef NDEBUG
    /* If assert is enabled, verify that keylist is in sorted order */
    static int done_once = 0;
    size_t i;
    if (done_once == 0)
    {
        for (i = 0; i < DIM(keylist) - 1; i++)
            assert(kw_compare(&keylist[i], &keylist[i+1]) < 0);
        done_once = 1;
    }
#endif /* NDEBUG */
    c_token = stmt_str;
}

static const char *const_skipblanks(const char *str)
{
    while (*str != '\0' && isspace(*str))
        str++;
    return(str);
}

static const char *const_skipnonblanks(const char *str)
{
    while (*str != '\0' && !isspace(*str))
        str++;
    return(str);
}

static const char *scan_for_filename(const char *str, const char **end)
{
    const char *start = const_skipblanks(str);
    if (*start == '\'' || *start == '"')
        start = sqltoken(start, end);
    else
        *end = const_skipnonblanks(start);
    return(start);
}

static int yylex(void)
{
    char        buffer[MAX_LEXTOKENLENGTH];
    const char *start;

    if (c_token == 0)
        abort();

    if (bare_filename_ok)
        start = scan_for_filename(c_token, &c_token);
    else
        start = sqltoken(c_token, &c_token);

    yylval.start = CONST_CAST(char *, start);
    yylval.end = CONST_CAST(char *, c_token);
    if (*start == '\0')
    {
        yylval.toktype = 0;
        return yylval.toktype;
    }
    set_token(buffer, sizeof(buffer), start, c_token);
#ifdef YYDEBUG
    if (YYDEBUGVAR > 1)
        printf("yylex(): token = %s\n", buffer);
#endif /* YYDEBUG */

    /* printf("yylex(): token = %s\n", buffer); */
    if (isalpha((unsigned char)buffer[0]) || buffer[0] == '_')
    {
        Keyword  kw;
        Keyword *p;
        kw.keyword = buffer;
        p = (Keyword *)bsearch(&kw, keylist, DIM(keylist), sizeof(Keyword),
                                kw_compare);    /*=C++=*/
        if (p == 0)
            yylval.toktype = S_IDENTIFIER;
        else
            yylval.toktype = p->token;
    }
    else if (buffer[0] == '\'')
    {
        yylval.toktype = S_SQSTRING;
    }
    else if (buffer[0] == '"')
    {
        yylval.toktype = S_DQSTRING;
    }
    else if (isdigit((unsigned char)buffer[0]))
    {
        yylval.toktype = S_NUMBER;
    }
    else if (buffer[0] == '.' && isdigit((unsigned char)buffer[1]))
    {
        yylval.toktype = S_NUMBER;
    }
    else if (buffer[0] == ';')
    {
        assert(buffer[1] == '\0');
        yylval.toktype = C_SEMICOLON;
    }
    else if (buffer[0] == '.')
    {
        assert(buffer[1] == '\0');
        yylval.toktype = C_PERIOD;
    }
    else if (buffer[0] == '@')
    {
        assert(buffer[1] == '\0');
        yylval.toktype = C_AT;
    }
    else if (buffer[0] == '(')
    {
        assert(buffer[1] == '\0');
        yylval.toktype = C_LPAREN;
    }
    else if (buffer[0] == ')')
    {
        assert(buffer[1] == '\0');
        yylval.toktype = C_RPAREN;
    }
    else if (buffer[0] == ':')
    {
        assert(buffer[1] == '\0');
        yylval.toktype = C_COLON;
    }
    else
    {
        yylval.toktype = S_ERROR;
    }
    return yylval.toktype;
}

static void yyerror(const char *s)
{
    /* Do nothing here -- it will be reported elsewhere */
}

/* conninfo_setempty - initialize a ConnInfo structure to all zeroes */
/* NB: must be here for standalone testing, but must also be */
/*     accessible to code in conninfo.c */
void conninfo_setempty(ConnInfo *c)
{
    static const ConnInfo zero_conninfo = { 0 };
    *c = zero_conninfo;
}

static void infoinfo_setempty(InfoInfo *i)
{
    static const InfoInfo zero_infoinfo = { "" };
    *i = zero_infoinfo;
}

static void loadinfo_setempty(LoadInfo *l)
{
    static const LoadInfo zero_loadinfo = { "" };
    *l = zero_loadinfo;
}

#ifndef YYDEBUG
#define ENTER_EXIT(dir, fun)    ((void)0)
#else
#define ENTER_EXIT(dir, fun)    enter_exit(dir, fun)
static void enter_exit(const char *dir, const char *fun)
{
    if (YYDEBUGVAR > 0)
        printf("%s %s()\n", dir, fun);
}
#endif /* YYDEBUG */

static StmtType parse_sqlstmt(char *str, InfoInfo *p_info, ConnInfo *p_conn, LoadInfo *p_load)
{
    info = p_info;
    conn = p_conn;
    load = p_load;
    stmt_type = STMT_NONE;
    loadinfo_setempty(load);
    infoinfo_setempty(info);
    conninfo_setempty(conn);
    set_statement(str);
    ENTER_EXIT("-->>", "yyparse");
    (void)yyparse();
    ENTER_EXIT("<<--", "yyparse");
    set_statement(0);
    return stmt_type;
}

StmtType parse_connstmt(char *str, ConnInfo *p_conn)
{
    InfoInfo l_info;
    LoadInfo l_load;
    return(parse_sqlstmt(str, &l_info, p_conn, &l_load));
}

StmtType parse_infostmt(char *str, InfoInfo *p_info)
{
    ConnInfo l_conn;
    LoadInfo l_load;
    return(parse_sqlstmt(str, p_info, &l_conn, &l_load));
}

StmtType parse_loadstmt(char *str, LoadInfo *p_load)
{
    ConnInfo l_conn;
    InfoInfo l_info;
    return(parse_sqlstmt(str, &l_info, &l_conn, p_load));
}

int set_yydebug(int newval)
{
#if !YYJLSSYACC && !YYBYACC && !YYBISON
    extern int YYDEBUGVAR;
#endif /* YYJLSSYACC */
    int oldval = YYDEBUGVAR;
    YYDEBUGVAR = newval;
    return(oldval);
}

#ifdef TEST

static void pr_conninfo(ConnInfo *p_conn)
{
    printf("Database = %s, Connection = %s, Username = %s, "
        "Password = %s WCT = %d, ConnType = %d, MODE ANSI = %d\n",
        p_conn->database, p_conn->connname, p_conn->username, p_conn->password,
        p_conn->wct, p_conn->ctype, p_conn->mode_ansi);
}

static void pr_loadinfo(const char *what, LoadInfo *c_load)
{
    printf("%s statement: file = %s, delim = %s, quote = %s, escape = %s, eor = <%s> stmt = %s\n",
            what, c_load->file, c_load->delim, c_load->quote, c_load->escape, c_load->eor, c_load->stmt);
}

static void pr_where(InfoInfo *p_info)
{
    if (p_info->wh_start)
    {
        int len = p_info->wh_end - p_info->wh_start;
        printf(" WHERE (%.*s)", len, p_info->wh_start);
    }
    putchar('\n');
}

static void pr_otherinfo(const char *what, InfoInfo *p_info)
{
    printf("INFO %s", what);
    pr_where(p_info);
}

static void pr_tableinfo(InfoInfo *p_info)
{
    printf("-- for table <<");
    if (p_info->dbase[0])
        printf("%s", p_info->dbase);
    if (p_info->server[0])
        printf("@%s", p_info->server);
    if (p_info->dbase[0])
        putchar(':');
    if (p_info->owner[0])
        printf("%s.", p_info->owner);
    printf("%s", p_info->table);
    puts(">>");
    pr_where(p_info);
}

int main(int argc, char **argv)
{
    char buffer[2048];
    StmtType n;
    InfoInfo c_info;
    ConnInfo c_conn;
    LoadInfo c_load;
    int debug_level = 1;

    err_setarg0(argv[0]);
    if (argc == 2)
        debug_level = atoi(argv[1]);
    else if (argc > 2)
        err_usage("[debug-level]\n  0 => no Yacc debugging\n  1 => Yacc debugging\n  2 => Lexical analyzer too");

    set_yydebug(1);
    fputs("SQL? ", stdout);
    fflush(stdout);
    while (fgets(buffer, sizeof(buffer), stdin) != 0)
    {
        buffer[strlen(buffer) - 1] = '\0';
        printf("Input: << %s >>\n", buffer);
        n = parse_sqlstmt(buffer, &c_info, &c_conn, &c_load);
        printf("Output: %d ", (int)n);
        switch (n)
        {
        case STMT_SETCONN:
            printf("SET CONNECTION\n");
            pr_conninfo(&c_conn);
            break;
        case STMT_CONNECT:
            printf("CONNECT\n");
            pr_conninfo(&c_conn);
            break;
        case STMT_DISCONN:
            printf("DISCONNECT\n");
            pr_conninfo(&c_conn);
            break;
        case STMT_SETOTHER:
            printf("<<%s>>\nA SET statement, but not a SET CONNECTION statement!\n", buffer);
            break;
        case STMT_DISCONNERR:
            printf("<<%s>>\nSyntax error in DISCONNECT!\n", buffer);
            break;
        case STMT_CONNECTERR:
            printf("<<%s>>\nSyntax error in CONNECT!\n", buffer);
            break;
        case STMT_SETCONNERR:
            printf("<<%s>>\nStarts SET CONNECTION but invalid syntax\n", buffer);
            break;

        case STMT_ERROR:
            printf("<<%s>>\nGeneric error -- unrecognized statement!\n", buffer);
            break;

        case STMT_LOAD:
            pr_loadinfo("LOAD", &c_load);
            break;
        case STMT_RELOAD:
            pr_loadinfo("RELOAD", &c_load);
            break;
        case STMT_UNLOAD:
            pr_loadinfo("UNLOAD", &c_load);
            break;

        case INFO_USERS:
            pr_otherinfo("USERS", &c_info);
            break;
        case INFO_ROLES:
            pr_otherinfo("ROLES", &c_info);
            break;
        case INFO_SYSTABLES:
            pr_otherinfo("SYSTABLES", &c_info);
            break;
        case INFO_VIEWS:
            pr_otherinfo("VIEWS", &c_info);
            break;
        case INFO_SYNONYMS:
            pr_otherinfo("SYNONYMS", &c_info);
            break;
        case INFO_BASETABLES:
            pr_otherinfo("BASETABLES", &c_info);
            break;
        case INFO_PROCEDURES:
            pr_otherinfo("PROCEDURES", &c_info);
            break;
        case INFO_TABLES:
            pr_otherinfo("TABLES", &c_info);
            break;

        case INFO_CONSTR_CHECK:
            printf("INFO CHECK CONSTRAINTS\n");
            pr_tableinfo(&c_info);
            break;
        case INFO_COLUMNS:
            printf("INFO COLUMNS\n");
            pr_tableinfo(&c_info);
            break;
        case INFO_INDEXES:
            printf("INFO INDEXES\n");
            pr_tableinfo(&c_info);
            break;
        case INFO_ACCESS:       /* aka PRIVILEGES */
            printf("INFO ACCESS\n");
            pr_tableinfo(&c_info);
            break;
        case INFO_REFS_BY:
            printf("INFO REFERENCES BY\n");
            pr_tableinfo(&c_info);
            break;
        case INFO_REFS_TO:
            printf("INFO REFERENCES TO\n");
            pr_tableinfo(&c_info);
            break;
        case INFO_TRIGGERS:
            printf("INFO TRIGGERS\n");
            pr_tableinfo(&c_info);
            break;
        case INFO_STATUS:
            printf("INFO STATUS\n");
            pr_tableinfo(&c_info);
            break;
        case INFO_PROCBODY:
            printf("INFO PROCBODY:\n");
            pr_tableinfo(&c_info);
            break;
        case INFO_FRAGMENTS:
            printf("INFO FRAGMENTS\n");
            pr_tableinfo(&c_info);
            break;
        case INFO_TRIGBODY:
            printf("INFO TRIGBODY:\n");
            pr_tableinfo(&c_info);
            break;
        case INFO_VIEWBODY:
            printf("INFO VIEWBODY:\n");
            pr_tableinfo(&c_info);
            break;

        case INFO_HELP:
            printf("INFO HELP\n");
            printf("** Implemented options:\n"
                    "-- INFO ACCESS (aka INFO PRIVILEGES)\n"
                    "-- INFO BASETABLES\n"
                    "-- INFO CHECK CONSTRAINTS FOR table\n"
                    "-- INFO COLUMNS FOR table\n"
                    "-- INFO CONNECTIONS\n"
                    "-- INFO DATABASES\n"
                    "-- INFO FRAGMENTS FOR table\n"
                    "-- INFO HELP\n"
                    "-- INFO PROCBODY FOR procname\n"
                    "-- INFO PROCEDURES\n"
                    "-- INFO REFERENCES BY table\n"
                    "-- INFO REFERENCES TO table\n"
                    "-- INFO ROLES\n"
                    "-- INFO STATUS FOR table\n"
                    "-- INFO SYNONYMS\n"
                    "-- INFO SYSTABLES\n"
                    "-- INFO TABLES\n"
                    "-- INFO TRIGBODY FOR trigger\n"
                    "-- INFO TRIGGERS FOR table\n"
                    "-- INFO USERS\n"
                    "-- INFO VIEWBODY FOR view\n"
                    "-- INFO VIEWS\n"
                    );
            break;

        case INFO_CONNECTIONS:
            printf("INFO CONNECTIONS\n");
            break;

        case INFO_DATABASES:
            printf("INFO DATABASES\n");
            break;

        case STMT_RELOADERR:
            printf("<<%s>>\nSyntax error in UNLOAD statement\n", buffer);
            break;

        case STMT_UNLOADERR:
            printf("<<%s>>\nSyntax error in UNLOAD statement\n", buffer);
            break;

        case STMT_LOADERR:
            printf("<<%s>>\nSyntax error in LOAD statement\n", buffer);
            break;

        case INFO_ERROR:
            printf("<<%s>>\nSyntax error in INFO statement\n", buffer);
            break;

        case STMT_NONE:
            printf("Empty String - ignored\n");
            break;

        default:
            printf("Unknown statement type %d in switch statement.\n", n);
            assert(0);
            break;
        }

        fputs("SQL? ", stdout);
        fflush(stdout);
    }
    puts("");
    return 0;
}

#endif /* TEST */
