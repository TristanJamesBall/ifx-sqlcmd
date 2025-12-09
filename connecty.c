/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 1 "connecty.y"

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

#line 180 "y.tab.c"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif


/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    K_ACCESS = 258,                /* K_ACCESS  */
    K_ALL = 259,                   /* K_ALL  */
    K_APPEND = 260,                /* K_APPEND  */
    K_AS = 261,                    /* K_AS  */
    K_BASETABLES = 262,            /* K_BASETABLES  */
    K_BY = 263,                    /* K_BY  */
    K_CHECK = 264,                 /* K_CHECK  */
    K_COLUMNS = 265,               /* K_COLUMNS  */
    K_CONCURRENT = 266,            /* K_CONCURRENT  */
    K_CONNECT = 267,               /* K_CONNECT  */
    K_CONNECTION = 268,            /* K_CONNECTION  */
    K_CONNECTIONS = 269,           /* K_CONNECTIONS  */
    K_CONSTRAINTS = 270,           /* K_CONSTRAINTS  */
    K_CREATE = 271,                /* K_CREATE  */
    K_CSV = 272,                   /* K_CSV  */
    K_CURRENT = 273,               /* K_CURRENT  */
    K_DATABASES = 274,             /* K_DATABASES  */
    K_DEFAULT = 275,               /* K_DEFAULT  */
    K_DELIMITER = 276,             /* K_DELIMITER  */
    K_DISCONNECT = 277,            /* K_DISCONNECT  */
    K_EOR = 278,                   /* K_EOR  */
    K_ESCAPE = 279,                /* K_ESCAPE  */
    K_EXECUTE = 280,               /* K_EXECUTE  */
    K_FILE = 281,                  /* K_FILE  */
    K_FIXED = 282,                 /* K_FIXED  */
    K_FIXSEP = 283,                /* K_FIXSEP  */
    K_FIXDEL = 284,                /* K_FIXDEL  */
    K_DB2 = 285,                   /* K_DB2  */
    K_FOR = 286,                   /* K_FOR  */
    K_FORMAT = 287,                /* K_FORMAT  */
    K_FRAGMENTS = 288,             /* K_FRAGMENTS  */
    K_FROM = 289,                  /* K_FROM  */
    K_FUNCTION = 290,              /* K_FUNCTION  */
    K_HELP = 291,                  /* K_HELP  */
    K_INDEXES = 292,               /* K_INDEXES  */
    K_INFO = 293,                  /* K_INFO  */
    K_INSERT = 294,                /* K_INSERT  */
    K_LOAD = 295,                  /* K_LOAD  */
    K_PASSWORD = 296,              /* K_PASSWORD  */
    K_PIPE = 297,                  /* K_PIPE  */
    K_PRIVILEGES = 298,            /* K_PRIVILEGES  */
    K_PROCBODY = 299,              /* K_PROCBODY  */
    K_PROCEDURE = 300,             /* K_PROCEDURE  */
    K_PROCEDURES = 301,            /* K_PROCEDURES  */
    K_QUOTE = 302,                 /* K_QUOTE  */
    K_RECORDTAG = 303,             /* K_RECORDTAG  */
    K_REFERENCES = 304,            /* K_REFERENCES  */
    K_RELOAD = 305,                /* K_RELOAD  */
    K_ROLES = 306,                 /* K_ROLES  */
    K_SELECT = 307,                /* K_SELECT  */
    K_SET = 308,                   /* K_SET  */
    K_SKIP = 309,                  /* K_SKIP  */
    K_STATUS = 310,                /* K_STATUS  */
    K_SYNONYMS = 311,              /* K_SYNONYMS  */
    K_SYSTABLES = 312,             /* K_SYSTABLES  */
    K_TABLES = 313,                /* K_TABLES  */
    K_TO = 314,                    /* K_TO  */
    K_TRANSACTIONS = 315,          /* K_TRANSACTIONS  */
    K_TRIGBODY = 316,              /* K_TRIGBODY  */
    K_TRIGGERS = 317,              /* K_TRIGGERS  */
    K_TRUSTED = 318,               /* K_TRUSTED  */
    K_UNLOAD = 319,                /* K_UNLOAD  */
    K_USER = 320,                  /* K_USER  */
    K_USERS = 321,                 /* K_USERS  */
    K_USING = 322,                 /* K_USING  */
    K_VIEWBODY = 323,              /* K_VIEWBODY  */
    K_VIEWS = 324,                 /* K_VIEWS  */
    K_WCT = 325,                   /* K_WCT  */
    K_WHERE = 326,                 /* K_WHERE  */
    K_WITH = 327,                  /* K_WITH  */
    C_AT = 328,                    /* C_AT  */
    C_COLON = 329,                 /* C_COLON  */
    C_LPAREN = 330,                /* C_LPAREN  */
    C_PERIOD = 331,                /* C_PERIOD  */
    C_RPAREN = 332,                /* C_RPAREN  */
    C_SEMICOLON = 333,             /* C_SEMICOLON  */
    S_IDENTIFIER = 334,            /* S_IDENTIFIER  */
    S_DQSTRING = 335,              /* S_DQSTRING  */
    S_SQSTRING = 336,              /* S_SQSTRING  */
    S_ERROR = 337,                 /* S_ERROR  */
    S_NUMBER = 338                 /* S_NUMBER  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif
/* Token kinds.  */
#define YYEMPTY -2
#define YYEOF 0
#define YYerror 256
#define YYUNDEF 257
#define K_ACCESS 258
#define K_ALL 259
#define K_APPEND 260
#define K_AS 261
#define K_BASETABLES 262
#define K_BY 263
#define K_CHECK 264
#define K_COLUMNS 265
#define K_CONCURRENT 266
#define K_CONNECT 267
#define K_CONNECTION 268
#define K_CONNECTIONS 269
#define K_CONSTRAINTS 270
#define K_CREATE 271
#define K_CSV 272
#define K_CURRENT 273
#define K_DATABASES 274
#define K_DEFAULT 275
#define K_DELIMITER 276
#define K_DISCONNECT 277
#define K_EOR 278
#define K_ESCAPE 279
#define K_EXECUTE 280
#define K_FILE 281
#define K_FIXED 282
#define K_FIXSEP 283
#define K_FIXDEL 284
#define K_DB2 285
#define K_FOR 286
#define K_FORMAT 287
#define K_FRAGMENTS 288
#define K_FROM 289
#define K_FUNCTION 290
#define K_HELP 291
#define K_INDEXES 292
#define K_INFO 293
#define K_INSERT 294
#define K_LOAD 295
#define K_PASSWORD 296
#define K_PIPE 297
#define K_PRIVILEGES 298
#define K_PROCBODY 299
#define K_PROCEDURE 300
#define K_PROCEDURES 301
#define K_QUOTE 302
#define K_RECORDTAG 303
#define K_REFERENCES 304
#define K_RELOAD 305
#define K_ROLES 306
#define K_SELECT 307
#define K_SET 308
#define K_SKIP 309
#define K_STATUS 310
#define K_SYNONYMS 311
#define K_SYSTABLES 312
#define K_TABLES 313
#define K_TO 314
#define K_TRANSACTIONS 315
#define K_TRIGBODY 316
#define K_TRIGGERS 317
#define K_TRUSTED 318
#define K_UNLOAD 319
#define K_USER 320
#define K_USERS 321
#define K_USING 322
#define K_VIEWBODY 323
#define K_VIEWS 324
#define K_WCT 325
#define K_WHERE 326
#define K_WITH 327
#define C_AT 328
#define C_COLON 329
#define C_LPAREN 330
#define C_PERIOD 331
#define C_RPAREN 332
#define C_SEMICOLON 333
#define S_IDENTIFIER 334
#define S_DQSTRING 335
#define S_SQSTRING 336
#define S_ERROR 337
#define S_NUMBER 338

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;


int yyparse (void);



/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_K_ACCESS = 3,                   /* K_ACCESS  */
  YYSYMBOL_K_ALL = 4,                      /* K_ALL  */
  YYSYMBOL_K_APPEND = 5,                   /* K_APPEND  */
  YYSYMBOL_K_AS = 6,                       /* K_AS  */
  YYSYMBOL_K_BASETABLES = 7,               /* K_BASETABLES  */
  YYSYMBOL_K_BY = 8,                       /* K_BY  */
  YYSYMBOL_K_CHECK = 9,                    /* K_CHECK  */
  YYSYMBOL_K_COLUMNS = 10,                 /* K_COLUMNS  */
  YYSYMBOL_K_CONCURRENT = 11,              /* K_CONCURRENT  */
  YYSYMBOL_K_CONNECT = 12,                 /* K_CONNECT  */
  YYSYMBOL_K_CONNECTION = 13,              /* K_CONNECTION  */
  YYSYMBOL_K_CONNECTIONS = 14,             /* K_CONNECTIONS  */
  YYSYMBOL_K_CONSTRAINTS = 15,             /* K_CONSTRAINTS  */
  YYSYMBOL_K_CREATE = 16,                  /* K_CREATE  */
  YYSYMBOL_K_CSV = 17,                     /* K_CSV  */
  YYSYMBOL_K_CURRENT = 18,                 /* K_CURRENT  */
  YYSYMBOL_K_DATABASES = 19,               /* K_DATABASES  */
  YYSYMBOL_K_DEFAULT = 20,                 /* K_DEFAULT  */
  YYSYMBOL_K_DELIMITER = 21,               /* K_DELIMITER  */
  YYSYMBOL_K_DISCONNECT = 22,              /* K_DISCONNECT  */
  YYSYMBOL_K_EOR = 23,                     /* K_EOR  */
  YYSYMBOL_K_ESCAPE = 24,                  /* K_ESCAPE  */
  YYSYMBOL_K_EXECUTE = 25,                 /* K_EXECUTE  */
  YYSYMBOL_K_FILE = 26,                    /* K_FILE  */
  YYSYMBOL_K_FIXED = 27,                   /* K_FIXED  */
  YYSYMBOL_K_FIXSEP = 28,                  /* K_FIXSEP  */
  YYSYMBOL_K_FIXDEL = 29,                  /* K_FIXDEL  */
  YYSYMBOL_K_DB2 = 30,                     /* K_DB2  */
  YYSYMBOL_K_FOR = 31,                     /* K_FOR  */
  YYSYMBOL_K_FORMAT = 32,                  /* K_FORMAT  */
  YYSYMBOL_K_FRAGMENTS = 33,               /* K_FRAGMENTS  */
  YYSYMBOL_K_FROM = 34,                    /* K_FROM  */
  YYSYMBOL_K_FUNCTION = 35,                /* K_FUNCTION  */
  YYSYMBOL_K_HELP = 36,                    /* K_HELP  */
  YYSYMBOL_K_INDEXES = 37,                 /* K_INDEXES  */
  YYSYMBOL_K_INFO = 38,                    /* K_INFO  */
  YYSYMBOL_K_INSERT = 39,                  /* K_INSERT  */
  YYSYMBOL_K_LOAD = 40,                    /* K_LOAD  */
  YYSYMBOL_K_PASSWORD = 41,                /* K_PASSWORD  */
  YYSYMBOL_K_PIPE = 42,                    /* K_PIPE  */
  YYSYMBOL_K_PRIVILEGES = 43,              /* K_PRIVILEGES  */
  YYSYMBOL_K_PROCBODY = 44,                /* K_PROCBODY  */
  YYSYMBOL_K_PROCEDURE = 45,               /* K_PROCEDURE  */
  YYSYMBOL_K_PROCEDURES = 46,              /* K_PROCEDURES  */
  YYSYMBOL_K_QUOTE = 47,                   /* K_QUOTE  */
  YYSYMBOL_K_RECORDTAG = 48,               /* K_RECORDTAG  */
  YYSYMBOL_K_REFERENCES = 49,              /* K_REFERENCES  */
  YYSYMBOL_K_RELOAD = 50,                  /* K_RELOAD  */
  YYSYMBOL_K_ROLES = 51,                   /* K_ROLES  */
  YYSYMBOL_K_SELECT = 52,                  /* K_SELECT  */
  YYSYMBOL_K_SET = 53,                     /* K_SET  */
  YYSYMBOL_K_SKIP = 54,                    /* K_SKIP  */
  YYSYMBOL_K_STATUS = 55,                  /* K_STATUS  */
  YYSYMBOL_K_SYNONYMS = 56,                /* K_SYNONYMS  */
  YYSYMBOL_K_SYSTABLES = 57,               /* K_SYSTABLES  */
  YYSYMBOL_K_TABLES = 58,                  /* K_TABLES  */
  YYSYMBOL_K_TO = 59,                      /* K_TO  */
  YYSYMBOL_K_TRANSACTIONS = 60,            /* K_TRANSACTIONS  */
  YYSYMBOL_K_TRIGBODY = 61,                /* K_TRIGBODY  */
  YYSYMBOL_K_TRIGGERS = 62,                /* K_TRIGGERS  */
  YYSYMBOL_K_TRUSTED = 63,                 /* K_TRUSTED  */
  YYSYMBOL_K_UNLOAD = 64,                  /* K_UNLOAD  */
  YYSYMBOL_K_USER = 65,                    /* K_USER  */
  YYSYMBOL_K_USERS = 66,                   /* K_USERS  */
  YYSYMBOL_K_USING = 67,                   /* K_USING  */
  YYSYMBOL_K_VIEWBODY = 68,                /* K_VIEWBODY  */
  YYSYMBOL_K_VIEWS = 69,                   /* K_VIEWS  */
  YYSYMBOL_K_WCT = 70,                     /* K_WCT  */
  YYSYMBOL_K_WHERE = 71,                   /* K_WHERE  */
  YYSYMBOL_K_WITH = 72,                    /* K_WITH  */
  YYSYMBOL_C_AT = 73,                      /* C_AT  */
  YYSYMBOL_C_COLON = 74,                   /* C_COLON  */
  YYSYMBOL_C_LPAREN = 75,                  /* C_LPAREN  */
  YYSYMBOL_C_PERIOD = 76,                  /* C_PERIOD  */
  YYSYMBOL_C_RPAREN = 77,                  /* C_RPAREN  */
  YYSYMBOL_C_SEMICOLON = 78,               /* C_SEMICOLON  */
  YYSYMBOL_S_IDENTIFIER = 79,              /* S_IDENTIFIER  */
  YYSYMBOL_S_DQSTRING = 80,                /* S_DQSTRING  */
  YYSYMBOL_S_SQSTRING = 81,                /* S_SQSTRING  */
  YYSYMBOL_S_ERROR = 82,                   /* S_ERROR  */
  YYSYMBOL_S_NUMBER = 83,                  /* S_NUMBER  */
  YYSYMBOL_YYACCEPT = 84,                  /* $accept  */
  YYSYMBOL_statement = 85,                 /* statement  */
  YYSYMBOL_opt_semicolon = 86,             /* opt_semicolon  */
  YYSYMBOL_connect = 87,                   /* connect  */
  YYSYMBOL_dbenv = 88,                     /* dbenv  */
  YYSYMBOL_opt_wct = 89,                   /* opt_wct  */
  YYSYMBOL_opt_trusted = 90,               /* opt_trusted  */
  YYSYMBOL_opt_conn = 91,                  /* opt_conn  */
  YYSYMBOL_opt_user = 92,                  /* opt_user  */
  YYSYMBOL_username = 93,                  /* username  */
  YYSYMBOL_password = 94,                  /* password  */
  YYSYMBOL_using_or_password = 95,         /* using_or_password  */
  YYSYMBOL_disconnect = 96,                /* disconnect  */
  YYSYMBOL_setconnect = 97,                /* setconnect  */
  YYSYMBOL_setother = 98,                  /* setother  */
  YYSYMBOL_token_list = 99,                /* token_list  */
  YYSYMBOL_token = 100,                    /* token  */
  YYSYMBOL_keyword = 101,                  /* keyword  */
  YYSYMBOL_non_keyword = 102,              /* non_keyword  */
  YYSYMBOL_string = 103,                   /* string  */
  YYSYMBOL_conntype = 104,                 /* conntype  */
  YYSYMBOL_load = 105,                     /* load  */
  YYSYMBOL_opt_file_pipe = 106,            /* opt_file_pipe  */
  YYSYMBOL_load_opt_list = 107,            /* load_opt_list  */
  YYSYMBOL_load_opt = 108,                 /* load_opt  */
  YYSYMBOL_common_opt = 109,               /* common_opt  */
  YYSYMBOL_format = 110,                   /* format  */
  YYSYMBOL_fmt_kwd = 111,                  /* fmt_kwd  */
  YYSYMBOL_skip = 112,                     /* skip  */
  YYSYMBOL_number = 113,                   /* number  */
  YYSYMBOL_reload = 114,                   /* reload  */
  YYSYMBOL_reload_opt_list = 115,          /* reload_opt_list  */
  YYSYMBOL_reload_opt = 116,               /* reload_opt  */
  YYSYMBOL_unload = 117,                   /* unload  */
  YYSYMBOL_unload_dst = 118,               /* unload_dst  */
  YYSYMBOL_unload_file = 119,              /* unload_file  */
  YYSYMBOL_unload_pipe = 120,              /* unload_pipe  */
  YYSYMBOL_to_file = 121,                  /* to_file  */
  YYSYMBOL_opt_file = 122,                 /* opt_file  */
  YYSYMBOL_filename = 123,                 /* filename  */
  YYSYMBOL_create_file = 124,              /* create_file  */
  YYSYMBOL_append_file = 125,              /* append_file  */
  YYSYMBOL_to_pipe = 126,                  /* to_pipe  */
  YYSYMBOL_shell_command = 127,            /* shell_command  */
  YYSYMBOL_unload_opt_list = 128,          /* unload_opt_list  */
  YYSYMBOL_unload_opt = 129,               /* unload_opt  */
  YYSYMBOL_unload_src = 130,               /* unload_src  */
  YYSYMBOL_opt_to = 131,                   /* opt_to  */
  YYSYMBOL_proc_or_func = 132,             /* proc_or_func  */
  YYSYMBOL_xmlrectag = 133,                /* xmlrectag  */
  YYSYMBOL_delimiter = 134,                /* delimiter  */
  YYSYMBOL_quote = 135,                    /* quote  */
  YYSYMBOL_eor = 136,                      /* eor  */
  YYSYMBOL_escape = 137,                   /* escape  */
  YYSYMBOL_info = 138,                     /* info  */
  YYSYMBOL_info_unqualified = 139,         /* info_unqualified  */
  YYSYMBOL_opt_where = 140,                /* opt_where  */
  YYSYMBOL_info_qualified = 141,           /* info_qualified  */
  YYSYMBOL_for_objname = 142,              /* for_objname  */
  YYSYMBOL_to_table = 143,                 /* to_table  */
  YYSYMBOL_by_table = 144,                 /* by_table  */
  YYSYMBOL_identifier_or_keyword = 145,    /* identifier_or_keyword  */
  YYSYMBOL_object_name = 146,              /* object_name  */
  YYSYMBOL_obj_dbase = 147,                /* obj_dbase  */
  YYSYMBOL_opt_server = 148,               /* opt_server  */
  YYSYMBOL_obj_dbname = 149,               /* obj_dbname  */
  YYSYMBOL_object = 150,                   /* object  */
  YYSYMBOL_obj_name = 151,                 /* obj_name  */
  YYSYMBOL_obj_owner = 152                 /* obj_owner  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_int16 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if !defined yyoverflow

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* !defined yyoverflow */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  75
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   579

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  84
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  69
/* YYNRULES -- Number of rules.  */
#define YYNRULES  233
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  313

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   338


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   199,   199,   201,   203,   205,   207,   209,   211,   213,
     215,   217,   219,   221,   223,   225,   227,   229,   231,   243,
     244,   248,   250,   254,   260,   261,   263,   269,   270,   276,
     277,   285,   286,   290,   295,   300,   301,   305,   307,   309,
     311,   318,   322,   327,   328,   333,   334,   338,   339,   340,
     341,   342,   343,   344,   345,   346,   347,   348,   349,   350,
     351,   352,   353,   354,   355,   356,   357,   358,   359,   360,
     361,   362,   363,   364,   365,   366,   367,   368,   369,   370,
     371,   372,   373,   374,   375,   376,   377,   378,   379,   380,
     381,   382,   383,   384,   385,   386,   387,   388,   389,   390,
     391,   392,   393,   394,   395,   396,   397,   398,   399,   400,
     401,   402,   403,   404,   405,   406,   407,   411,   412,   413,
     414,   415,   416,   417,   418,   419,   423,   424,   428,   432,
     437,   446,   447,   449,   454,   455,   459,   460,   464,   465,
     466,   467,   468,   472,   474,   479,   480,   481,   482,   483,
     484,   485,   486,   491,   496,   500,   508,   509,   514,   518,
     536,   538,   543,   544,   545,   549,   553,   558,   559,   563,
     568,   573,   578,   583,   587,   588,   593,   594,   598,   600,
     605,   606,   610,   611,   615,   620,   625,   630,   635,   640,
     641,   645,   647,   649,   651,   653,   655,   657,   659,   661,
     663,   665,   670,   671,   676,   678,   680,   682,   684,   686,
     688,   690,   692,   694,   696,   698,   700,   705,   709,   713,
     717,   718,   722,   723,   727,   731,   732,   737,   742,   744,
     754,   755,   759,   760
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if YYDEBUG || 0
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "K_ACCESS", "K_ALL",
  "K_APPEND", "K_AS", "K_BASETABLES", "K_BY", "K_CHECK", "K_COLUMNS",
  "K_CONCURRENT", "K_CONNECT", "K_CONNECTION", "K_CONNECTIONS",
  "K_CONSTRAINTS", "K_CREATE", "K_CSV", "K_CURRENT", "K_DATABASES",
  "K_DEFAULT", "K_DELIMITER", "K_DISCONNECT", "K_EOR", "K_ESCAPE",
  "K_EXECUTE", "K_FILE", "K_FIXED", "K_FIXSEP", "K_FIXDEL", "K_DB2",
  "K_FOR", "K_FORMAT", "K_FRAGMENTS", "K_FROM", "K_FUNCTION", "K_HELP",
  "K_INDEXES", "K_INFO", "K_INSERT", "K_LOAD", "K_PASSWORD", "K_PIPE",
  "K_PRIVILEGES", "K_PROCBODY", "K_PROCEDURE", "K_PROCEDURES", "K_QUOTE",
  "K_RECORDTAG", "K_REFERENCES", "K_RELOAD", "K_ROLES", "K_SELECT",
  "K_SET", "K_SKIP", "K_STATUS", "K_SYNONYMS", "K_SYSTABLES", "K_TABLES",
  "K_TO", "K_TRANSACTIONS", "K_TRIGBODY", "K_TRIGGERS", "K_TRUSTED",
  "K_UNLOAD", "K_USER", "K_USERS", "K_USING", "K_VIEWBODY", "K_VIEWS",
  "K_WCT", "K_WHERE", "K_WITH", "C_AT", "C_COLON", "C_LPAREN", "C_PERIOD",
  "C_RPAREN", "C_SEMICOLON", "S_IDENTIFIER", "S_DQSTRING", "S_SQSTRING",
  "S_ERROR", "S_NUMBER", "$accept", "statement", "opt_semicolon",
  "connect", "dbenv", "opt_wct", "opt_trusted", "opt_conn", "opt_user",
  "username", "password", "using_or_password", "disconnect", "setconnect",
  "setother", "token_list", "token", "keyword", "non_keyword", "string",
  "conntype", "load", "opt_file_pipe", "load_opt_list", "load_opt",
  "common_opt", "format", "fmt_kwd", "skip", "number", "reload",
  "reload_opt_list", "reload_opt", "unload", "unload_dst", "unload_file",
  "unload_pipe", "to_file", "opt_file", "filename", "create_file",
  "append_file", "to_pipe", "shell_command", "unload_opt_list",
  "unload_opt", "unload_src", "opt_to", "proc_or_func", "xmlrectag",
  "delimiter", "quote", "eor", "escape", "info", "info_unqualified",
  "opt_where", "info_qualified", "for_objname", "to_table", "by_table",
  "identifier_or_keyword", "object_name", "obj_dbase", "opt_server",
  "obj_dbname", "object", "obj_name", "obj_owner", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-211)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-233)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
       6,  -211,     8,     7,   107,    16,    38,    42,    21,  -211,
      48,  -211,   -36,   -36,   -36,   -36,  -211,  -211,  -211,   -36,
    -211,  -211,  -211,    -5,    46,  -211,  -211,  -211,  -211,  -211,
    -211,  -211,  -211,    26,    -6,    62,    26,  -211,  -211,    26,
    -211,    26,    26,    26,    -6,    27,    -6,    26,    -6,    -6,
      -6,    26,    26,    -6,    26,    -6,  -211,    -2,  -211,    -2,
    -211,    13,  -211,  -211,    12,    52,    19,  -211,  -211,  -211,
    -211,  -211,  -211,  -211,    49,  -211,  -211,  -211,  -211,  -211,
    -211,   262,   -19,  -211,    -7,    30,   262,   262,   343,  -211,
    -211,  -211,    26,  -211,  -211,  -211,  -211,  -211,  -211,   343,
     343,  -211,  -211,  -211,  -211,  -211,  -211,  -211,  -211,  -211,
    -211,  -211,  -211,   262,  -211,  -211,    -7,   262,    -7,   262,
    -211,  -211,  -211,   262,  -211,    52,  -211,    -7,    -7,   201,
      -7,  -211,  -211,  -211,  -211,  -211,  -211,  -211,  -211,  -211,
    -211,  -211,  -211,  -211,  -211,  -211,  -211,  -211,  -211,  -211,
    -211,  -211,  -211,  -211,  -211,  -211,  -211,  -211,  -211,  -211,
    -211,  -211,  -211,  -211,  -211,  -211,  -211,  -211,  -211,  -211,
    -211,  -211,  -211,  -211,  -211,  -211,  -211,  -211,  -211,  -211,
    -211,  -211,  -211,  -211,  -211,  -211,  -211,  -211,  -211,  -211,
    -211,  -211,  -211,  -211,  -211,  -211,  -211,  -211,  -211,  -211,
    -211,  -211,  -211,  -211,  -211,  -211,  -211,  -211,  -211,  -211,
    -211,  -211,  -211,  -211,    81,  -211,  -211,    -7,   -19,   -20,
    -211,    31,  -211,  -211,    23,  -211,   343,    45,  -211,  -211,
      43,   262,  -211,  -211,  -211,  -211,  -211,   262,    -7,  -211,
    -211,  -211,    -7,    -7,    -7,    34,     2,    -7,    -7,  -211,
    -211,  -211,  -211,  -211,  -211,  -211,  -211,  -211,  -211,  -211,
    -211,    60,  -211,    40,  -211,  -211,  -211,    -7,    47,  -211,
     500,    54,   422,   156,   166,  -211,  -211,  -211,  -211,  -211,
    -211,  -211,  -211,  -211,  -211,  -211,  -211,  -211,  -211,  -211,
    -211,  -211,  -211,  -211,  -211,  -211,  -211,  -211,  -211,  -211,
    -211,  -211,  -211,  -211,    39,  -211,  -211,  -211,  -211,  -211,
    -211,  -211,  -211
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,    18,     0,     0,     0,     0,     0,     0,     0,    20,
       0,     2,    19,    19,    19,    19,     8,    10,     9,    19,
     189,   190,    43,     0,    29,    43,    38,    37,    39,   127,
     126,    40,    43,     0,   202,     0,     0,   198,   197,     0,
     201,     0,     0,     0,   202,     0,   202,     0,   202,   202,
     202,     0,     0,   202,     0,   202,    43,   131,    43,   131,
      43,     0,    41,    43,   180,   167,   167,   174,   160,   161,
     162,   163,   164,   165,     0,     1,     3,     4,     5,     6,
       7,    15,    24,    23,     0,    31,    14,    17,     0,   206,
      43,   193,     0,   204,   210,   205,   207,   209,   196,     0,
       0,   211,   212,   200,   208,   195,   192,   191,   215,   214,
     199,   213,   194,    13,   132,   133,     0,    12,     0,    42,
      43,   129,   128,    11,   181,   167,   168,     0,     0,     0,
       0,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    65,    66,
      67,    68,    69,    70,    71,    73,    74,    72,    64,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   120,   119,   121,   124,   122,   125,    44,
      45,    46,   123,    25,     0,    21,    30,     0,    24,     0,
     220,   231,   221,   233,   227,   217,     0,   225,   222,   228,
       0,   203,   216,   219,   218,   134,   156,    16,     0,   169,
     170,   166,     0,     0,     0,     0,     0,     0,     0,   178,
     176,   142,   175,   159,   177,   138,   139,   141,   140,   173,
     172,     0,    33,    27,    36,    35,    32,     0,   230,   223,
       0,     0,     0,     0,     0,   171,   185,   187,   188,   183,
     182,   179,   145,   148,   149,   147,   146,   150,   151,   152,
     143,   144,   186,   184,    26,    28,    22,    34,   226,   224,
     231,   230,   229,   130,     0,   135,   136,   137,   155,   158,
     157,   154,   153
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -211,  -211,   187,  -211,  -211,   -89,  -211,  -211,  -211,  -211,
    -211,  -211,  -211,  -211,  -211,   -22,  -211,   -87,  -211,    -3,
    -211,  -211,    72,  -211,  -142,     5,  -211,  -211,  -211,  -211,
    -211,  -211,  -211,  -211,  -211,  -211,  -211,  -211,   -61,  -126,
    -211,  -211,  -211,  -211,  -211,  -211,  -211,    74,  -211,  -211,
    -211,  -211,  -211,  -211,  -211,  -211,    56,  -211,    94,  -211,
    -211,  -210,   -10,  -211,  -211,  -211,   -84,  -125,  -211
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,    10,    11,    12,    24,   215,   296,    85,   218,   219,
     266,   267,    13,    14,    15,    81,   209,   210,   211,   212,
      62,    16,   116,   273,   305,   306,   251,   291,   307,   312,
      17,   274,   310,    18,    67,    68,    69,    70,   127,   240,
      71,    72,    73,   260,   129,   252,   253,    74,   281,   254,
     255,   256,   257,   258,    19,    20,    91,    21,    89,   101,
     102,   224,   225,   226,   271,   227,   228,   229,   230
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      31,   222,   241,    86,   259,   128,   -19,     1,    25,    22,
      87,    26,   222,   222,   120,    82,   268,    56,     2,   282,
      83,   264,    63,  -230,   114,    27,    64,    28,     3,   283,
     284,   285,   286,   121,   113,    99,   117,    65,   119,    58,
     115,   123,     9,    60,     4,   126,     5,   265,    75,   287,
      57,   213,    84,   214,   288,    61,     6,    88,   122,     7,
     298,  -181,   301,  -180,   238,    90,   289,    23,   231,   279,
       8,   124,    59,    29,    30,    29,    30,    92,   126,   280,
      66,   216,    29,    30,     9,   223,   100,    29,    30,   233,
     234,   130,   261,    29,    30,   217,   223,   223,   237,  -232,
      98,  -230,   103,   295,   105,   106,   107,  -127,    32,   110,
      33,   112,   275,   235,    34,   236,    35,    36,   270,   272,
     294,    37,   311,  -232,   239,   239,    38,   239,   299,   263,
      93,   118,   309,    94,   250,    95,    96,    97,   125,   222,
      39,   104,   269,    40,    41,   108,   109,   302,   111,     0,
      42,    43,     0,    44,     0,     0,    45,     0,    46,     0,
       0,     0,    47,    48,    49,    50,     0,     0,    51,    52,
       0,     0,     0,    53,     0,    54,    55,   242,     0,   243,
     244,     0,     0,   222,     0,   222,   232,   242,   246,   243,
     244,     0,     0,     0,     0,   303,     0,     0,   246,    76,
      77,    78,    79,   247,     0,   308,    80,     0,     0,     0,
     304,     0,     0,   247,   262,     0,     0,     0,     0,     0,
     304,     0,   242,   223,   243,   244,   245,     0,     0,     0,
       0,     0,     0,   246,     0,   239,     0,     0,     0,   276,
     277,   278,     0,   290,   292,   293,     0,     0,   247,   248,
       0,     0,     0,   249,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   297,   131,   132,   133,   134,   135,
     136,   137,   138,   139,   140,   141,   142,   143,   144,   145,
     146,   147,   148,   149,   150,   151,   152,   153,   154,   155,
     156,   157,   158,   159,   160,   161,   162,   163,   164,   165,
     166,   167,   168,   169,   170,   171,   172,   173,   174,   175,
     176,   177,   178,   179,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   197,   198,   199,   200,   201,   202,   203,   204,   205,
       0,   206,    29,    30,   207,   208,   131,   132,   133,   134,
     135,   136,   137,   138,   139,   140,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,   151,   152,   153,   154,
     155,   156,   157,   158,   159,   160,   161,   162,   163,   164,
     165,   166,   167,   168,   169,   170,   171,   172,   173,   174,
     175,   176,   177,   178,   179,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,   197,   198,   199,   200,     0,     0,     0,     0,
       0,     0,   220,   221,    30,   131,   132,   133,   134,   135,
     136,   137,   138,   139,   140,   141,   142,   143,   144,   145,
     146,   147,   148,   149,   150,   151,   152,   153,   154,   155,
     156,   157,   158,   159,   160,   161,   162,   163,   164,   165,
     166,   167,   168,   169,   170,   171,   172,   173,   174,   175,
     176,   177,   178,   179,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   197,   198,   199,   200,     0,     0,     0,     0,     0,
       0,   220,   300,   131,   132,   133,   134,   135,   136,   137,
     138,   139,   140,   141,   142,   143,   144,   145,   146,   147,
     148,   149,   150,   151,   152,   153,   154,   155,   156,   157,
     158,   159,   160,   161,   162,   163,   164,   165,   166,   167,
     168,   169,   170,   171,   172,   173,   174,   175,   176,   177,
     178,   179,   180,   181,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,   197,
     198,   199,   200,     0,     0,     0,     0,     0,     0,   220
};

static const yytype_int16 yycheck[] =
{
       3,    88,   128,    25,   130,    66,     0,     1,     1,     1,
      32,     4,    99,   100,     1,    20,   226,     1,    12,    17,
      23,    41,     1,     0,    26,    18,     5,    20,    22,    27,
      28,    29,    30,    20,    56,     8,    58,    16,    60,     1,
      42,    63,    78,     1,    38,    26,    40,    67,     0,    47,
      34,    70,     6,    72,    52,    13,    50,    31,    61,    53,
     270,    42,   272,    42,   125,    71,    64,    59,    90,    35,
      64,    59,    34,    80,    81,    80,    81,    15,    26,    45,
      59,    84,    80,    81,    78,    88,    59,    80,    81,    99,
     100,    42,    11,    80,    81,    65,    99,   100,   120,    76,
      44,    78,    46,    63,    48,    49,    50,    76,     1,    53,
       3,    55,   238,   116,     7,   118,     9,    10,    73,    76,
      60,    14,    83,    76,   127,   128,    19,   130,    74,   218,
      36,    59,   274,    39,   129,    41,    42,    43,    64,   226,
      33,    47,   226,    36,    37,    51,    52,   272,    54,    -1,
      43,    44,    -1,    46,    -1,    -1,    49,    -1,    51,    -1,
      -1,    -1,    55,    56,    57,    58,    -1,    -1,    61,    62,
      -1,    -1,    -1,    66,    -1,    68,    69,    21,    -1,    23,
      24,    -1,    -1,   270,    -1,   272,    92,    21,    32,    23,
      24,    -1,    -1,    -1,    -1,    39,    -1,    -1,    32,    12,
      13,    14,    15,    47,    -1,    39,    19,    -1,    -1,    -1,
      54,    -1,    -1,    47,   217,    -1,    -1,    -1,    -1,    -1,
      54,    -1,    21,   226,    23,    24,    25,    -1,    -1,    -1,
      -1,    -1,    -1,    32,    -1,   238,    -1,    -1,    -1,   242,
     243,   244,    -1,   246,   247,   248,    -1,    -1,    47,    48,
      -1,    -1,    -1,    52,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   267,     3,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      -1,    79,    80,    81,    82,    83,     3,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    -1,    -1,    -1,    -1,
      -1,    -1,    79,    80,    81,     3,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    -1,    -1,    -1,    -1,    -1,
      -1,    79,    80,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    -1,    -1,    -1,    -1,    -1,    -1,    79
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     1,    12,    22,    38,    40,    50,    53,    64,    78,
      85,    86,    87,    96,    97,    98,   105,   114,   117,   138,
     139,   141,     1,    59,    88,     1,     4,    18,    20,    80,
      81,   103,     1,     3,     7,     9,    10,    14,    19,    33,
      36,    37,    43,    44,    46,    49,    51,    55,    56,    57,
      58,    61,    62,    66,    68,    69,     1,    34,     1,    34,
       1,    13,   104,     1,     5,    16,    59,   118,   119,   120,
     121,   124,   125,   126,   131,     0,    86,    86,    86,    86,
      86,    99,    20,   103,     6,    91,    99,    99,    31,   142,
      71,   140,    15,   142,   142,   142,   142,   142,   140,     8,
      59,   143,   144,   140,   142,   140,   140,   140,   142,   142,
     140,   142,   140,    99,    26,    42,   106,    99,   106,    99,
       1,    20,   103,    99,    59,   131,    26,   122,   122,   128,
      42,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    79,    82,    83,   100,
     101,   102,   103,    70,    72,    89,   103,    65,    92,    93,
      79,    80,   101,   103,   145,   146,   147,   149,   150,   151,
     152,    99,   142,   146,   146,   103,   103,    99,   122,   103,
     123,   123,    21,    23,    24,    25,    32,    47,    48,    52,
     109,   110,   129,   130,   133,   134,   135,   136,   137,   123,
     127,    11,   103,    89,    41,    67,    94,    95,   145,   150,
      73,   148,    76,   107,   115,   123,   103,   103,   103,    35,
      45,   132,    17,    27,    28,    29,    30,    47,    52,    64,
     103,   111,   103,   103,    60,    63,    90,   103,   145,    74,
      80,   145,   151,    39,    54,   108,   109,   112,    39,   108,
     116,    83,   113
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_uint8 yyr1[] =
{
       0,    84,    85,    85,    85,    85,    85,    85,    85,    85,
      85,    85,    85,    85,    85,    85,    85,    85,    85,    86,
      86,    87,    87,    88,    89,    89,    89,    90,    90,    91,
      91,    92,    92,    93,    94,    95,    95,    96,    96,    96,
      96,    97,    98,    99,    99,   100,   100,   101,   101,   101,
     101,   101,   101,   101,   101,   101,   101,   101,   101,   101,
     101,   101,   101,   101,   101,   101,   101,   101,   101,   101,
     101,   101,   101,   101,   101,   101,   101,   101,   101,   101,
     101,   101,   101,   101,   101,   101,   101,   101,   101,   101,
     101,   101,   101,   101,   101,   101,   101,   101,   101,   101,
     101,   101,   101,   101,   101,   101,   101,   101,   101,   101,
     101,   101,   101,   101,   101,   101,   101,   102,   102,   102,
     102,   102,   102,   102,   102,   102,   103,   103,   104,   104,
     105,   106,   106,   106,   107,   107,   108,   108,   109,   109,
     109,   109,   109,   110,   110,   111,   111,   111,   111,   111,
     111,   111,   111,   112,   113,   114,   115,   115,   116,   117,
     118,   118,   119,   119,   119,   120,   121,   122,   122,   123,
     124,   125,   126,   127,   128,   128,   129,   129,   130,   130,
     131,   131,   132,   132,   133,   134,   135,   136,   137,   138,
     138,   139,   139,   139,   139,   139,   139,   139,   139,   139,
     139,   139,   140,   140,   141,   141,   141,   141,   141,   141,
     141,   141,   141,   141,   141,   141,   141,   142,   143,   144,
     145,   145,   146,   146,   147,   148,   148,   149,   150,   150,
     151,   151,   152,   152
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     2,     2,     2,     2,     2,     1,     1,
       1,     3,     3,     3,     3,     3,     4,     3,     1,     0,
       1,     4,     6,     2,     0,     1,     3,     0,     1,     0,
       2,     0,     2,     2,     2,     1,     1,     2,     2,     2,
       2,     2,     3,     0,     2,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     2,     2,
       6,     0,     1,     1,     0,     2,     1,     1,     1,     1,
       1,     1,     1,     2,     2,     1,     1,     1,     1,     1,
       1,     1,     1,     2,     1,     6,     0,     2,     1,     4,
       1,     1,     1,     1,     1,     1,     3,     0,     1,     1,
       3,     4,     3,     1,     0,     2,     1,     1,     1,     2,
       0,     1,     1,     1,     2,     2,     2,     2,     2,     1,
       1,     3,     3,     3,     3,     3,     3,     2,     2,     3,
       3,     2,     0,     2,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     4,     2,     2,     2,
       1,     1,     1,     2,     3,     0,     2,     1,     1,     3,
       1,     1,     1,     1
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)




# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  yy_symbol_value_print (yyo, yykind, yyvaluep);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp,
                 int yyrule)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)]);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif






/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep)
{
  YY_USE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/* Lookahead token kind.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;




/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */

  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */


  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      goto yyerrlab1;
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 2: /* statement: opt_semicolon  */
#line 200 "connecty.y"
        { stmt_type = STMT_NONE; }
#line 1831 "y.tab.c"
    break;

  case 3: /* statement: connect opt_semicolon  */
#line 202 "connecty.y"
        { stmt_type = STMT_CONNECT; }
#line 1837 "y.tab.c"
    break;

  case 4: /* statement: disconnect opt_semicolon  */
#line 204 "connecty.y"
        { stmt_type = STMT_DISCONN; }
#line 1843 "y.tab.c"
    break;

  case 5: /* statement: setconnect opt_semicolon  */
#line 206 "connecty.y"
        { stmt_type = STMT_SETCONN; }
#line 1849 "y.tab.c"
    break;

  case 6: /* statement: setother opt_semicolon  */
#line 208 "connecty.y"
        { stmt_type = STMT_SETOTHER; }
#line 1855 "y.tab.c"
    break;

  case 7: /* statement: info opt_semicolon  */
#line 210 "connecty.y"
        { stmt_type = (StmtType) yyvsp[-1].toktype; /*=C++=*/ }
#line 1861 "y.tab.c"
    break;

  case 8: /* statement: load  */
#line 212 "connecty.y"
        { stmt_type = STMT_LOAD; YYACCEPT; }
#line 1867 "y.tab.c"
    break;

  case 9: /* statement: unload  */
#line 214 "connecty.y"
        { stmt_type = STMT_UNLOAD; YYACCEPT; }
#line 1873 "y.tab.c"
    break;

  case 10: /* statement: reload  */
#line 216 "connecty.y"
        { stmt_type = STMT_RELOAD; YYACCEPT; }
#line 1879 "y.tab.c"
    break;

  case 11: /* statement: K_UNLOAD error token_list  */
#line 218 "connecty.y"
        { stmt_type = STMT_UNLOADERR; }
#line 1885 "y.tab.c"
    break;

  case 12: /* statement: K_RELOAD error token_list  */
#line 220 "connecty.y"
        { stmt_type = STMT_RELOADERR; }
#line 1891 "y.tab.c"
    break;

  case 13: /* statement: K_LOAD error token_list  */
#line 222 "connecty.y"
        { stmt_type = STMT_LOADERR; }
#line 1897 "y.tab.c"
    break;

  case 14: /* statement: K_DISCONNECT error token_list  */
#line 224 "connecty.y"
        { stmt_type = STMT_DISCONNERR; }
#line 1903 "y.tab.c"
    break;

  case 15: /* statement: K_CONNECT error token_list  */
#line 226 "connecty.y"
        { stmt_type = STMT_CONNECTERR; }
#line 1909 "y.tab.c"
    break;

  case 16: /* statement: K_SET K_CONNECTION error token_list  */
#line 228 "connecty.y"
        { stmt_type = STMT_SETCONNERR; }
#line 1915 "y.tab.c"
    break;

  case 17: /* statement: K_INFO error token_list  */
#line 230 "connecty.y"
        { stmt_type = INFO_ERROR; }
#line 1921 "y.tab.c"
    break;

  case 18: /* statement: error  */
#line 232 "connecty.y"
        {
            if (stmt_type >= INFO_BASE)
                stmt_type = INFO_ERROR;
            else if (stmt_type > STMT_NONE)
                stmt_type = (StmtType)(-stmt_type); /*=C++=*/
            else
                stmt_type = STMT_ERROR;
        }
#line 1934 "y.tab.c"
    break;

  case 21: /* connect: K_CONNECT K_TO K_DEFAULT opt_wct  */
#line 249 "connecty.y"
        { conn->ctype = CONN_DEFAULT; }
#line 1940 "y.tab.c"
    break;

  case 23: /* dbenv: K_TO string  */
#line 255 "connecty.y"
        { set_database(conn, yyvsp[0].start, yyvsp[0].end); }
#line 1946 "y.tab.c"
    break;

  case 24: /* opt_wct: %empty  */
#line 260 "connecty.y"
        { conn->wct = 0; }
#line 1952 "y.tab.c"
    break;

  case 25: /* opt_wct: K_WCT  */
#line 262 "connecty.y"
        { conn->wct = 1; }
#line 1958 "y.tab.c"
    break;

  case 26: /* opt_wct: K_WITH K_CONCURRENT K_TRANSACTIONS  */
#line 264 "connecty.y"
        { conn->wct = 1; }
#line 1964 "y.tab.c"
    break;

  case 27: /* opt_trusted: %empty  */
#line 269 "connecty.y"
        { conn->trusted = 0; }
#line 1970 "y.tab.c"
    break;

  case 28: /* opt_trusted: K_TRUSTED  */
#line 271 "connecty.y"
        { conn->trusted = 1; }
#line 1976 "y.tab.c"
    break;

  case 29: /* opt_conn: %empty  */
#line 276 "connecty.y"
        { conn->ctype = CONN_NONE; }
#line 1982 "y.tab.c"
    break;

  case 30: /* opt_conn: K_AS string  */
#line 278 "connecty.y"
        { conn->ctype = CONN_STRING;
          set_connection(conn, yyvsp[0].start, yyvsp[0].end);
        }
#line 1990 "y.tab.c"
    break;

  case 31: /* opt_user: %empty  */
#line 285 "connecty.y"
        { conn->username = 0; conn->password= 0; }
#line 1996 "y.tab.c"
    break;

  case 33: /* username: K_USER string  */
#line 291 "connecty.y"
        { set_username(conn, yyvsp[0].start, yyvsp[0].end); }
#line 2002 "y.tab.c"
    break;

  case 34: /* password: using_or_password string  */
#line 296 "connecty.y"
        { set_password(conn, yyvsp[0].start, yyvsp[0].end); }
#line 2008 "y.tab.c"
    break;

  case 37: /* disconnect: K_DISCONNECT K_CURRENT  */
#line 306 "connecty.y"
        { conn->ctype = CONN_CURRENT; }
#line 2014 "y.tab.c"
    break;

  case 38: /* disconnect: K_DISCONNECT K_ALL  */
#line 308 "connecty.y"
        { conn->ctype = CONN_ALL; }
#line 2020 "y.tab.c"
    break;

  case 39: /* disconnect: K_DISCONNECT K_DEFAULT  */
#line 310 "connecty.y"
        { conn->ctype = CONN_DEFAULT; }
#line 2026 "y.tab.c"
    break;

  case 40: /* disconnect: K_DISCONNECT string  */
#line 312 "connecty.y"
        { conn->ctype = CONN_STRING;
          set_connection(conn, yyvsp[0].start, yyvsp[0].end);
        }
#line 2034 "y.tab.c"
    break;

  case 42: /* setother: K_SET error token_list  */
#line 323 "connecty.y"
        { yyerrok; }
#line 2040 "y.tab.c"
    break;

  case 44: /* token_list: token_list token  */
#line 329 "connecty.y"
        { yyval = yyvsp[0]; }
#line 2046 "y.tab.c"
    break;

  case 128: /* conntype: K_CONNECTION string  */
#line 429 "connecty.y"
        { conn->ctype = CONN_STRING;
          set_connection(conn, yyvsp[0].start, yyvsp[0].end);
        }
#line 2054 "y.tab.c"
    break;

  case 129: /* conntype: K_CONNECTION K_DEFAULT  */
#line 433 "connecty.y"
        { conn->ctype = CONN_DEFAULT; }
#line 2060 "y.tab.c"
    break;

  case 130: /* load: K_LOAD K_FROM opt_file_pipe string load_opt_list K_INSERT  */
#line 438 "connecty.y"
        {
            set_string("load file", load->file, sizeof(load->file), yyvsp[-2].start, yyvsp[-2].end);
            load->stmt = yyvsp[0].start;
        }
#line 2069 "y.tab.c"
    break;

  case 131: /* opt_file_pipe: %empty  */
#line 446 "connecty.y"
        { load->pipe_or_file = LD_FILE; }
#line 2075 "y.tab.c"
    break;

  case 132: /* opt_file_pipe: K_FILE  */
#line 448 "connecty.y"
        { load->pipe_or_file = LD_FILE; }
#line 2081 "y.tab.c"
    break;

  case 133: /* opt_file_pipe: K_PIPE  */
#line 450 "connecty.y"
        { load->pipe_or_file = LD_PIPE; }
#line 2087 "y.tab.c"
    break;

  case 143: /* format: K_FORMAT string  */
#line 473 "connecty.y"
        { set_string("format name", load->format, sizeof(load->format), yyvsp[0].start, yyvsp[0].end); }
#line 2093 "y.tab.c"
    break;

  case 144: /* format: K_FORMAT fmt_kwd  */
#line 475 "connecty.y"
        { set_symbol("format name", load->format, sizeof(load->format), yyvsp[0].start, yyvsp[0].end); }
#line 2099 "y.tab.c"
    break;

  case 153: /* skip: K_SKIP number  */
#line 492 "connecty.y"
        { set_symbol("skip", load->skip, sizeof(load->skip), yyvsp[0].start, yyvsp[0].end); }
#line 2105 "y.tab.c"
    break;

  case 155: /* reload: K_RELOAD K_FROM opt_file_pipe string reload_opt_list K_INSERT  */
#line 501 "connecty.y"
        {
            set_string("reload file", load->file, sizeof(load->file), yyvsp[-2].start, yyvsp[-2].end);
            load->stmt = yyvsp[0].start;
        }
#line 2114 "y.tab.c"
    break;

  case 160: /* unload_dst: unload_file  */
#line 537 "connecty.y"
        { load->pipe_or_file = LD_FILE; }
#line 2120 "y.tab.c"
    break;

  case 161: /* unload_dst: unload_pipe  */
#line 539 "connecty.y"
        { load->pipe_or_file = LD_PIPE; }
#line 2126 "y.tab.c"
    break;

  case 166: /* to_file: K_TO opt_file filename  */
#line 554 "connecty.y"
        { load->mode = FM_CREATE; }
#line 2132 "y.tab.c"
    break;

  case 169: /* filename: string  */
#line 564 "connecty.y"
        { set_string("unload file", load->file, sizeof(load->file), yyvsp[0].start, yyvsp[0].end); }
#line 2138 "y.tab.c"
    break;

  case 170: /* create_file: K_CREATE opt_file filename  */
#line 569 "connecty.y"
        { load->mode = FM_CREATE; }
#line 2144 "y.tab.c"
    break;

  case 171: /* append_file: K_APPEND opt_to opt_file filename  */
#line 574 "connecty.y"
        { load->mode = FM_APPEND; }
#line 2150 "y.tab.c"
    break;

  case 172: /* to_pipe: opt_to K_PIPE shell_command  */
#line 579 "connecty.y"
        { load->pipe_or_file = LD_PIPE; }
#line 2156 "y.tab.c"
    break;

  case 178: /* unload_src: K_SELECT  */
#line 599 "connecty.y"
        { load->stmt = yyvsp[0].start; }
#line 2162 "y.tab.c"
    break;

  case 179: /* unload_src: K_EXECUTE proc_or_func  */
#line 601 "connecty.y"
        { load->stmt = yyvsp[-1].start; }
#line 2168 "y.tab.c"
    break;

  case 184: /* xmlrectag: K_RECORDTAG string  */
#line 616 "connecty.y"
        { set_string("XML record tag", load->xmltag, sizeof(load->xmltag), yyvsp[0].start, yyvsp[0].end); }
#line 2174 "y.tab.c"
    break;

  case 185: /* delimiter: K_DELIMITER string  */
#line 621 "connecty.y"
        { set_string("delimiter", load->delim, sizeof(load->delim), yyvsp[0].start, yyvsp[0].end); }
#line 2180 "y.tab.c"
    break;

  case 186: /* quote: K_QUOTE string  */
#line 626 "connecty.y"
        { set_string("quote", load->quote, sizeof(load->quote), yyvsp[0].start, yyvsp[0].end); }
#line 2186 "y.tab.c"
    break;

  case 187: /* eor: K_EOR string  */
#line 631 "connecty.y"
        { set_string("eor", load->eor, sizeof(load->eor), yyvsp[0].start, yyvsp[0].end); }
#line 2192 "y.tab.c"
    break;

  case 188: /* escape: K_ESCAPE string  */
#line 636 "connecty.y"
        { set_string("escape", load->escape, sizeof(load->escape), yyvsp[0].start, yyvsp[0].end); }
#line 2198 "y.tab.c"
    break;

  case 191: /* info_unqualified: K_INFO K_TABLES opt_where  */
#line 646 "connecty.y"
        { yyval.toktype = INFO_TABLES; }
#line 2204 "y.tab.c"
    break;

  case 192: /* info_unqualified: K_INFO K_SYSTABLES opt_where  */
#line 648 "connecty.y"
        { yyval.toktype = INFO_SYSTABLES; }
#line 2210 "y.tab.c"
    break;

  case 193: /* info_unqualified: K_INFO K_BASETABLES opt_where  */
#line 650 "connecty.y"
        { yyval.toktype = INFO_BASETABLES; }
#line 2216 "y.tab.c"
    break;

  case 194: /* info_unqualified: K_INFO K_VIEWS opt_where  */
#line 652 "connecty.y"
        { yyval.toktype = INFO_VIEWS; }
#line 2222 "y.tab.c"
    break;

  case 195: /* info_unqualified: K_INFO K_SYNONYMS opt_where  */
#line 654 "connecty.y"
        { yyval.toktype = INFO_SYNONYMS; }
#line 2228 "y.tab.c"
    break;

  case 196: /* info_unqualified: K_INFO K_PROCEDURES opt_where  */
#line 656 "connecty.y"
        { yyval.toktype = INFO_PROCEDURES; }
#line 2234 "y.tab.c"
    break;

  case 197: /* info_unqualified: K_INFO K_DATABASES  */
#line 658 "connecty.y"
        { yyval.toktype = INFO_DATABASES; }
#line 2240 "y.tab.c"
    break;

  case 198: /* info_unqualified: K_INFO K_CONNECTIONS  */
#line 660 "connecty.y"
        { yyval.toktype = INFO_CONNECTIONS; }
#line 2246 "y.tab.c"
    break;

  case 199: /* info_unqualified: K_INFO K_USERS opt_where  */
#line 662 "connecty.y"
        { yyval.toktype = INFO_USERS; }
#line 2252 "y.tab.c"
    break;

  case 200: /* info_unqualified: K_INFO K_ROLES opt_where  */
#line 664 "connecty.y"
        { yyval.toktype = INFO_ROLES; }
#line 2258 "y.tab.c"
    break;

  case 201: /* info_unqualified: K_INFO K_HELP  */
#line 666 "connecty.y"
        { yyval.toktype = INFO_HELP; }
#line 2264 "y.tab.c"
    break;

  case 203: /* opt_where: K_WHERE token_list  */
#line 672 "connecty.y"
        { info->wh_start = yyvsp[-1].end; info->wh_end = yyvsp[0].end; }
#line 2270 "y.tab.c"
    break;

  case 204: /* info_qualified: K_INFO K_COLUMNS for_objname  */
#line 677 "connecty.y"
        { yyval.toktype = INFO_COLUMNS; }
#line 2276 "y.tab.c"
    break;

  case 205: /* info_qualified: K_INFO K_INDEXES for_objname  */
#line 679 "connecty.y"
        { yyval.toktype = INFO_INDEXES; }
#line 2282 "y.tab.c"
    break;

  case 206: /* info_qualified: K_INFO K_ACCESS for_objname  */
#line 681 "connecty.y"
        { yyval.toktype = INFO_ACCESS; }
#line 2288 "y.tab.c"
    break;

  case 207: /* info_qualified: K_INFO K_PRIVILEGES for_objname  */
#line 683 "connecty.y"
        { yyval.toktype = INFO_ACCESS; }
#line 2294 "y.tab.c"
    break;

  case 208: /* info_qualified: K_INFO K_STATUS for_objname  */
#line 685 "connecty.y"
        { yyval.toktype = INFO_STATUS; }
#line 2300 "y.tab.c"
    break;

  case 209: /* info_qualified: K_INFO K_PROCBODY for_objname  */
#line 687 "connecty.y"
        { yyval.toktype = INFO_PROCBODY; }
#line 2306 "y.tab.c"
    break;

  case 210: /* info_qualified: K_INFO K_FRAGMENTS for_objname  */
#line 689 "connecty.y"
        { yyval.toktype = INFO_FRAGMENTS; }
#line 2312 "y.tab.c"
    break;

  case 211: /* info_qualified: K_INFO K_REFERENCES to_table  */
#line 691 "connecty.y"
        { yyval.toktype = INFO_REFS_TO; }
#line 2318 "y.tab.c"
    break;

  case 212: /* info_qualified: K_INFO K_REFERENCES by_table  */
#line 693 "connecty.y"
        { yyval.toktype = INFO_REFS_BY; }
#line 2324 "y.tab.c"
    break;

  case 213: /* info_qualified: K_INFO K_VIEWBODY for_objname  */
#line 695 "connecty.y"
        { yyval.toktype = INFO_VIEWBODY; }
#line 2330 "y.tab.c"
    break;

  case 214: /* info_qualified: K_INFO K_TRIGGERS for_objname  */
#line 697 "connecty.y"
        { yyval.toktype = INFO_TRIGGERS; }
#line 2336 "y.tab.c"
    break;

  case 215: /* info_qualified: K_INFO K_TRIGBODY for_objname  */
#line 699 "connecty.y"
        { yyval.toktype = INFO_TRIGBODY; }
#line 2342 "y.tab.c"
    break;

  case 216: /* info_qualified: K_INFO K_CHECK K_CONSTRAINTS for_objname  */
#line 701 "connecty.y"
        { yyval.toktype = INFO_CONSTR_CHECK; }
#line 2348 "y.tab.c"
    break;

  case 226: /* opt_server: C_AT identifier_or_keyword  */
#line 733 "connecty.y"
        { set_symbol("dbserver", info->server, sizeof(info->server), yyvsp[0].start, yyvsp[0].end); }
#line 2354 "y.tab.c"
    break;

  case 227: /* obj_dbname: identifier_or_keyword  */
#line 738 "connecty.y"
        { set_symbol("database", info->dbase, sizeof(info->dbase), yyvsp[0].start, yyvsp[0].end); }
#line 2360 "y.tab.c"
    break;

  case 228: /* object: obj_name  */
#line 743 "connecty.y"
        { set_symbol("table", info->table, sizeof(info->table), yyvsp[0].start, yyvsp[0].end); }
#line 2366 "y.tab.c"
    break;

  case 229: /* object: obj_owner C_PERIOD obj_name  */
#line 745 "connecty.y"
        {
            /* Must preserve quotes or no-quotes around */
            /* owner to work with MODE ANSI databases!! */
            set_symbol("owner", info->owner, sizeof(info->owner), yyvsp[-2].start, yyvsp[-2].end);
            set_symbol("table", info->table, sizeof(info->table), yyvsp[0].start, yyvsp[0].end);
        }
#line 2377 "y.tab.c"
    break;


#line 2381 "y.tab.c"

      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      yyerror (YY_("syntax error"));
    }

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;
  ++yynerrs;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  YY_ACCESSING_SYMBOL (yystate), yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif

  return yyresult;
}

#line 763 "connecty.y"


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
