%{
/*
@(#)File:           $RCSfile: grammar.y,v $
@(#)Version:        $Revision: 2.16 $
@(#)Last changed:   $Date: 2016/07/08 01:22:47 $
@(#)Purpose:        Explore grammar of LOAD, UNLOAD, RELOAD and UPLOAD syntax
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 2001,2008,2010,2016
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

/*TABSTOP=4*/

/*
** This grammar recognizes CONNECT, SET CONNECTION, DISCONNECT,
** INFO, UNLOAD, LOAD, RELOAD, and UPLOAD statements.
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "grammar.h"
#include "cchrstrlit.h"
#include "context.h"
#include "esqlutil.h"
#include "internal.h"
#include "jlss.h"
#include "kludge.h"
#include "sqltoken.h"

#ifndef DO_NOT_USE_JLSSYACC_DEFINES
/*
** If you are using JLSSYACC, these defines hide all the YACC symbols which
** are normally visible (see grammar.c.std).  These defines do not normally
** cause problems with a standard version of YACC.  If they do cause you
** problems, add -DDO_NOT_USE_JLSSYACC_DEFINES to the C compiler command
** line to remove them, and let Jonathan Leffler know which version of YACC
** you are using, and which platform you are using it on.
*/
#define YYERROR_UNUSED
#define YYBACKUP_UNUSED
#define YY_SC_PARSE     static /* Storage class for yyparse() */
#define YY_SC_LEX       static /* Storage class for yylex() */
#define YY_SC_SYNTAX    static /* Storage class for yyerror() */
#define YYGLOBAL        static
#define YYSTATIC        static
#else
#define YY_SC_PARSE     /*nothing*/ /* Storage class for yyparse() */
#define YY_SC_LEX       /*nothing*/ /* Storage class for yylex() */
#define YY_SC_SYNTAX    /*nothing*/ /* Storage class for yyerror() */
#define YYSTATIC        extern
#endif /* DO_NOT_USE_JLSSYACC_DEFINES */

struct Token
{
    int     toktype;
    const char   *start;
    const char   *end;
};
typedef struct Token Token;

#define YYSTYPE Token

static StmtType stmt_type;
static ConnInfo *conn;
static InfoInfo *info;
static LoadInfo *load;
static IntlInfo *intl;

/* num and file are used instead of %union - it simplifies the grammar specification */
static ScaledNumber num;
static LoadFile file;

static char *c_token;       /* Where to start next token search */

static void set_cstrlit_string(char *output, size_t outlen, const char *s, const char *e);
static void set_string(char *output, size_t outlen, const char *s, const char *e);
static void set_symbol(char *output, size_t outlen, const char *s, const char *e);
static mode_t octal(const char *s, const char *e);
static void loadfile_setempty(LoadFile *f);

#ifndef YYJLSSYACC
/* Primary entry point */
YYSTATIC int yyparse(void);
YY_SC_LEX int yylex(void);
YY_SC_SYNTAX void yyerror(const char *s);
#endif /* YYJLSSYACC */

#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
extern const char jlss_id_grammar_y[];
const char jlss_id_grammar_y[] = "@(#)$Id: grammar.y,v 2.16 2016/07/08 01:22:47 jleffler Exp $";
#endif /* lint */
%}

/* Keywords */
%token K_ABORT
%token K_ACCESS
%token K_ADJUST
%token K_AFTER
%token K_ALGORITHM
%token K_ALL
%token K_APPEND
%token K_AS
%token K_AUTOMATIC
%token K_B          /* Letter B */
%token K_BASE64
%token K_BASETABLES
%token K_BENCHMARK
%token K_BLOBDIR
%token K_BLOBS
%token K_BY
%token K_BYE
%token K_BZIP2
%token K_CHECK
%token K_CLOBBER
%token K_CLOCK
%token K_COLUMNS
%token K_COMPRESS
%token K_CONCURRENT
%token K_CONNECT
%token K_CONNECTION
%token K_CONNECTIONS
%token K_CONSTRAINTS
%token K_CONTEXT
%token K_CONTINUE
%token K_CREATE
%token K_CSV
%token K_MARKDOWN
%token K_CURRENT
%token K_DATABASES
%token K_DATE
%token K_DBNAMES
%token K_DEFAULT
%token K_DELIM
%token K_DELIMIT
%token K_DELIMITER
%token K_DIRECTORY
%token K_DISCONNECT
%token K_E          /* Letter E */
%token K_ECHO
%token K_EDIT
%token K_EOL
%token K_EOR
%token K_ERROR
%token K_ERRORS
%token K_ESCAPE
%token K_EXCLUSIVE
%token K_EXECUTE
%token K_EXIT
%token K_EXTENDED
%token K_EXTERNAL
%token K_FIELDS
%token K_FILE
%token K_FIXED
%token K_FOR
%token K_FORMAT
%token K_FRAGMENTS
%token K_FROM
%token K_FUNCTION
%token K_G          /* Letter G */
%token K_GZIP
%token K_HEADING
%token K_HEADINGS
%token K_HELP
%token K_HEURISTIC
%token K_HEX
%token K_HISTORY
%token K_IBASE
%token K_IN
%token K_INDEXES
%token K_INFO
%token K_INPUT
%token K_INSERT
%token K_INTERNAL
%token K_K          /* Letter K */
%token K_KEY
%token K_L          /* Letter L */
%token K_LIST
%token K_LOAD
%token K_LOCK
%token K_LOG
%token K_M          /* Letter M */
%token K_METADATA
%token K_MODE
%token K_NO
%token K_NONUNIQUE
%token K_OFF
%token K_OK
%token K_ON
%token K_OUTPUT
%token K_PASSWORD
%token K_PIPE
%token K_POP
%token K_PRIVILEGES
%token K_PROCBODY
%token K_PROCEDURE
%token K_PROCEDURES
%token K_PUSH
%token K_Q          /* Letter Q */
%token K_QLIMIT
%token K_QUERYLIMIT
%token K_QUIT
%token K_QUOTE
%token K_R          /* Letter R */
%token K_REFERENCES
%token K_REJECT
%token K_RELOAD
%token K_RERUN
%token K_ROLES
%token K_ROWS
%token K_SELECT
%token K_SET
%token K_SHARE
%token K_SILENCE
%token K_SIZE
%token K_SKIP
%token K_SLEEP
%token K_SQLEXIT
%token K_SQLSTART
%token K_START
%token K_STATUS
%token K_STOP
%token K_SYNONYMS
%token K_SYSTABLES
%token K_T
%token K_TABLE
%token K_TABLES
%token K_TIME
%token K_TO
%token K_TOGGLE
%token K_TRACE
%token K_TRANS
%token K_TRANSACTION
%token K_TRANSACTIONS
%token K_TRANSIZE
%token K_TRIGBODY
%token K_TRIGGERS
%token K_TRUSTED
%token K_TYPE
%token K_TYPES
%token K_UNIQUE
%token K_UNLOAD
%token K_UPDATE
%token K_UPLOAD
%token K_USE
%token K_USER
%token K_USERS
%token K_USING
%token K_V          /* Letter V */
%token K_VERBOSE
%token K_VERSION
%token K_VI
%token K_VIEW
%token K_VIEWBODY
%token K_VIEWS
%token K_WCT
%token K_WHERE
%token K_WITH
%token K_X          /* Letter X */
%token K_XML

/* Single punctuation characters */
%token C_AT         /* At-sign '@' */
%token C_COLON      /* Colon ':' */
%token C_COMMA      /* Comma ',' */
%token C_LPAREN     /* Left parenthesis '(' */
%token C_PERIOD     /* Period '.' */
%token C_RPAREN     /* Right parenthesis ')' */
%token C_SEMICOLON  /* Semi-colon ';' */

/* Sequences of characters */
%token S_IDENTIFIER /* Any other sequence of alphabetics - K_SYMBOL */
%token S_NUMBER     /* General number */
%token S_DQSTRING   /* Double-quoted string */
%token S_SQSTRING   /* Single-quoted string */
%token S_ERROR      /* Any other punctuation string (eg <, >, =, etc) */

%start statement

%%

statement
    :   stmt_body opt_semicolon
    ;

stmt_body
    :   /* Nothing */
        { stmt_type = STMT_NONE; }
    |   load
        { stmt_type = STMT_LOAD; }
    |   unload
        { stmt_type = STMT_UNLOAD; }
    |   reload
        { stmt_type = STMT_RELOAD; }
    |   upload
        { stmt_type = STMT_UPLOAD; }
    |   connect
        { stmt_type = STMT_CONNECT; }
    |   disconnect
        { stmt_type = STMT_DISCONN; }
    |   setconnect
        { stmt_type = STMT_SETCONN; }
    |   setother
        { stmt_type = STMT_SETOTHER; }
    |   info
        { stmt_type = (StmtType) $1.toktype; /*=C++=*/ }
    |   internal_cmd
        { intl->cmd = $$.toktype; }
    |   error_recovery
    ;

error_recovery
    :   K_UPLOAD error token_list
        { stmt_type = STMT_UPLOADERR; }
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

unload
    :   K_UNLOAD output_target unload_option_list select_or_exec_proc token_list
    ;

output_target
    :   init_file output_specification
        { load->file = file; loadfile_setempty(&file); }
    ;

init_file
    :   /* Nothing */
        { loadfile_setempty(&file); }
    ;

output_specification
    :   to_file
        { file.filetype = UNLOAD_CRFILE; }
    |   create_file
        { file.filetype = UNLOAD_CRFILE; }
    |   append_file
        { file.filetype = UNLOAD_APFILE; }
    |   to_pipe
        { file.filetype = UNLOAD_PIPE; }
    ;

to_file
    :   K_TO opt_file filename opt_clobber_control opt_file_perms   /* Backwards compatibility */
    ;

filename
    :   string
        { set_string(file.filename, sizeof(file.filename), $1.start, $1.end); }
    ;

string
    :   S_DQSTRING
    |   S_SQSTRING
    ;

create_file
    :   K_CREATE opt_file filename opt_clobber_control opt_file_perms
    ;

append_file
    :   K_APPEND opt_to opt_file filename opt_create_control
    ;

to_pipe
    :   opt_to K_PIPE shell_command
    ;

shell_command
    :   string
        { set_string(file.filename, sizeof(file.filename), $1.start, $1.end); }
    ;

opt_file
    :   /* Nothing */
    |   K_FILE
    ;

opt_clobber_control
    :   /* Nothing */
        { file.clobber = UNLOAD_CLOBBER; }
    |   K_NO K_CLOBBER
        { file.clobber = UNLOAD_NOCLOBBER; }
    |   K_CLOBBER K_OK
        { file.clobber = UNLOAD_CLOBBER; }
    ;

opt_file_perms
    :   /* Nothing */
    |   file_perms
    ;

file_perms
    :   K_MODE S_NUMBER
        { file.fileperms = octal($2.start, $2.end); }
    ;

opt_to
    :   /* Nothing */
    |   K_TO
    ;

opt_create_control
    :   /* Nothing */
        { file.create = UNLOAD_CREATE; }
    |   K_NO K_CREATE
        { file.create = UNLOAD_NOCREATE; }
    |   K_CREATE K_OK opt_file_perms
        { file.create = UNLOAD_CREATE; }
    ;

unload_option_list
    :   /* Nothing */
    |   unload_option_list unload_option
    ;

unload_option
    :   special_character_control
    |   unload_format
        { load->load_fmt = $1.toktype; }
    |   blob_location
    |   unld_external_field_spec
    |   internal_field_spec
    |   with_metadata
    ;

special_character_control
    :   K_DELIMITER string
        { set_cstrlit_string(load->delim, sizeof(load->delim), $2.start, $2.end); }
    |   K_ESCAPE string
        { set_cstrlit_string(load->escape, sizeof(load->escape), $2.start, $2.end); }
    |   K_QUOTE string
        { set_cstrlit_string(load->quote, sizeof(load->quote), $2.start, $2.end); }
    |   K_EOL string
        { set_cstrlit_string(load->eol, sizeof(load->eol), $2.start, $2.end); }
    ;

unload_format
    :   K_FORMAT unload_format_name
        { $$ = $2; }
    ;

unload_format_name
    :   load_format_name
    |   K_XML
	|	K_MARKDOWN
    ;

load_format_name
    :   K_UNLOAD
    |   K_EXTENDED
    |   K_FIXED
    |   K_CSV
    ;

blob_location
    :   K_BLOBS K_TO K_FILE blob_control_list
    ;

blob_control_list
    :   /* Nothing */
    |   blob_control_list blob_control
    ;

blob_control
    :   in_directory
    |   keycol
    |   compression
    |   blobcol_file_spec
    ;

opt_in_directory
    :   /* Nothing */
    |   in_directory
    ;

in_directory
    :   K_IN opt_directory dirname
    ;

opt_directory
    :   /* Nothing */
    |   K_DIRECTORY
    ;

dirname
    :   string
    ;

blobcol_file_spec
    :   C_LPAREN blobcol_file_speclist C_RPAREN
    ;

blobcol_file_speclist
    :   blobcol_file
    |   blobcol_file_speclist C_COMMA blobcol_file
    ;

blobcol_file
    :   column_name K_TO formatstring opt_keycol opt_compression
    ;

formatstring
    :   string
    ;

opt_keycol
    :   /* Nothing */
    |   keycol
    ;

keycol
    :   K_KEY column_name
    ;

with_metadata
    :   opt_with K_METADATA
        { load->metadata = 1; }
    ;

opt_with
    :   /* Nothing */
    |   K_WITH
    ;

internal_field_spec
    :   K_INTERNAL fields C_LPAREN internal_fieldlist C_RPAREN
    ;

fields
    :   K_COLUMNS
    |   K_FIELDS
    ;

internal_fieldlist
    :   internal_field
    |   internal_fieldlist C_COMMA internal_field
    ;

internal_field
    :   column_name K_FORMAT binary_format opt_compression
    |   column_name K_FORMAT formatstring
    ;

binary_format
    :   K_HEX
    |   K_BASE64
    ;

opt_compression
    :   /* Nothing */
    |   compression
    ;

compression
    :   opt_compress compression_algorithm
    ;

opt_compress
    :   /* Nothing */
    |   K_COMPRESS
    ;

compression_algorithm
    :   K_GZIP
    |   K_BZIP2
    ;

unld_external_field_spec
    :   K_EXTERNAL fields C_LPAREN unld_external_fieldlist C_RPAREN
    ;

unld_external_fieldlist
    :   unld_external_field
    |   unld_external_fieldlist C_COMMA unld_external_field
    ;

unld_external_field
    :   blobcol_file_spec
    ;

select_or_exec_proc
    :   K_SELECT
    |   K_EXECUTE K_FUNCTION
    |   K_EXECUTE K_PROCEDURE
    ;

load
    :   K_LOAD load_details
    ;

reload
    :   K_RELOAD load_details
    ;

load_details
    :   input_source load_option_list K_INSERT token_list
        { load->stmt.start = $3.start; load->stmt.end = $4.end; }
    ;

input_source
    :   K_FROM opt_file filename
        {
            set_string(load->file.filename, sizeof(load->file.filename), $3.start, $3.end);
            load->file.filetype = LOAD_FILE;
        }
    |   K_FROM K_PIPE shell_command
        {
            set_string(load->file.filename, sizeof(load->file.filename), $3.start, $3.end);
            load->file.filetype = LOAD_PIPE;
        }
    ;

load_option_list
    :   /* Nothing */
    |   load_option_list load_option
    ;

load_option
    :   special_character_control
    |   K_BLOBS K_FROM K_FILE opt_in_directory
        {   set_string(load->blobdir, sizeof(load->blobdir), $4.start, $4.end); }
    |   K_WITH opt_no K_AUTOMATIC K_TRANSACTION opt_tx_size
        { load->tx_size = num; }
    |   K_SKIP scaled_number opt_rows
        { load->nm_skip = num; }
    |   K_LOAD scaled_number opt_rows
        { load->nm_load = num; }
    |   K_ABORT K_AFTER scaled_number opt_errors
        { load->nm_abrt = num; }
    |   K_LOCK opt_table opt_in share_exclusive opt_mode
        { load->lock = ($4.toktype == K_EXCLUSIVE) ? LOCK_EXCLUSIVE : LOCK_SHARED; }
    |   load_format
        { load->load_fmt = $1.toktype; }
    |   reld_external_field_spec
    |   internal_field_spec
    |   errorlog_to_file
    |   rejectlog_to_file
    ;

load_format
    :   K_FORMAT load_format_name
        { $$ = $2; }
    ;

scaled_number
    :   S_NUMBER opt_scale
        { num.number = pos_number($1.start, $1.end); }
    ;

opt_scale
    :   /* Nothing */
        { num.scale = Byte; }
    |   K_K     /* Kilobytes - 1024^1 */
        { num.scale = KiloByte; }
    |   K_M     /* Megabytes - 1024^2 */
        { num.scale = MegaByte; }
    |   K_G     /* Gigabytes - 1024^3 */
        { num.scale = GigaByte; }
    |   K_T     /* Terabytes - 1024^4 */
        { num.scale = TeraByte; }
    ;

opt_table
    :   /* Nothing */
    |   K_TABLE
    ;

opt_mode
    :   /* Nothing */
    |   K_MODE
    ;

share_exclusive
    :   K_SHARE
    |   K_EXCLUSIVE
    ;

opt_in
    :   /* Nothing */
    |   K_IN
    ;

opt_no
    :   /* Nothing */
    |   K_NO
    ;

opt_tx_size
    :   /* Nothing */
    |   K_SIZE scaled_number opt_rows
    ;

opt_rows
    :   /* Nothing */
    |   K_ROWS
    ;

opt_errors
    :   /* Nothing */
    |   K_ERRORS
    ;

reld_external_field_spec
    :   K_EXTERNAL fields C_LPAREN reld_external_fieldlist C_RPAREN
    ;

reld_external_fieldlist
    :   reld_external_field
    |   reld_external_fieldlist C_COMMA reld_external_field
    ;

reld_external_field
    :   column_name opt_compression
    ;

errorlog_to_file
    :   K_ERROR opt_log log_target
        { load->errfile = file; loadfile_setempty(&file); }
    ;

opt_log
    :   /* Nothing */
    |   K_LOG
    ;

log_target
    :   filename
    |   output_target
    ;

rejectlog_to_file
    :   K_REJECT opt_log log_target
        { load->rejfile = file; loadfile_setempty(&file); }
    ;

upload
    :   K_UPLOAD tablename input_source upload_column_spec upload_key upload_option_list
    ;

tablename
    :   object_name
    ;

upload_column_spec
    :   K_COLUMNS C_LPAREN upload_column_list C_RPAREN
    ;

upload_column_list
    :   upload_column_name
    |   upload_column_list C_COMMA upload_column_name
    ;

upload_column_name
    :   column_name
    |   K_KEY C_PERIOD column_name
    ;

upload_key
    :   unique_key
    |   nonunique_key
    ;

unique_key
    :   K_UNIQUE opt_key
    ;

opt_key
    :   /* Nothing */
    |   K_KEY
    ;

nonunique_key
    :   K_NONUNIQUE opt_key
    ;

upload_option_list
    :   /* Nothing */
    |   upload_option_list upload_option
    ;

upload_option
    :   load_option
    |   K_USE upload_algorithm opt_algorithm opt_tx_size
    ;

upload_algorithm
    :   K_HEURISTIC
        { load->algorithm = H_HEURISTIC; }
    |   K_INSERT
        { load->algorithm = H_INSERT; }
    |   K_SELECT
        { load->algorithm = H_SELECT; }
    |   K_UPDATE
        { load->algorithm = H_UPDATE; }
    ;

opt_algorithm
    :   /* Nothing */
    |   K_ALGORITHM
    ;

column_name
    :   S_IDENTIFIER
    |   keyword
    ;

keyword
    :   K_ABORT
    |   K_ACCESS
    |   K_ADJUST
    |   K_AFTER
    |   K_ALGORITHM
    |   K_ALL
    |   K_APPEND
    |   K_AS
    |   K_AUTOMATIC
    |   K_B         /* Letter B */
    |   K_BASE64
    |   K_BASETABLES
    |   K_BENCHMARK
    |   K_BLOBDIR
    |   K_BLOBS
    |   K_BY
    |   K_BYE
    |   K_BZIP2
    |   K_CHECK
    |   K_CLOBBER
    |   K_CLOCK
    |   K_COLUMNS
    |   K_COMPRESS
    |   K_CONCURRENT
    |   K_CONNECT
    |   K_CONNECTION
    |   K_CONNECTIONS
    |   K_CONSTRAINTS
    |   K_CONTEXT
    |   K_CONTINUE
    |   K_CREATE
    |   K_CSV
	|	K_MARKDOWN
    |   K_CURRENT
    |   K_DATABASES
    |   K_DATE
    |   K_DBNAMES
    |   K_DEFAULT
    |   K_DELIM
    |   K_DELIMIT
    |   K_DELIMITER
    |   K_DIRECTORY
    |   K_DISCONNECT
    |   K_E         /* Letter E */
    |   K_ECHO
    |   K_EDIT
    |   K_EOL
    |   K_EOR
    |   K_ERROR
    |   K_ERRORS
    |   K_ESCAPE
    |   K_EXCLUSIVE
    |   K_EXECUTE
    |   K_EXIT
    |   K_EXTENDED
    |   K_EXTERNAL
    |   K_FIELDS
    |   K_FILE
    |   K_FIXED
    |   K_FOR
    |   K_FORMAT
    |   K_FRAGMENTS
    |   K_FROM
    |   K_FUNCTION
    |   K_G         /* Letter G */
    |   K_GZIP
    |   K_HEADING
    |   K_HEADINGS
    |   K_HELP
    |   K_HEURISTIC
    |   K_HEX
    |   K_HISTORY
    |   K_IBASE
    |   K_IN
    |   K_INDEXES
    |   K_INFO
    |   K_INPUT
    |   K_INSERT
    |   K_INTERNAL
    |   K_K         /* Letter K */
    |   K_KEY
    |   K_L         /* Letter L */
    |   K_LIST
    |   K_LOAD
    |   K_LOCK
    |   K_LOG
    |   K_M         /* Letter M */
    |   K_METADATA
    |   K_MODE
    |   K_NO
    |   K_NONUNIQUE
    |   K_OFF
    |   K_OK
    |   K_ON
    |   K_OUTPUT
    |   K_PASSWORD
    |   K_PIPE
    |   K_POP
    |   K_PRIVILEGES
    |   K_PROCBODY
    |   K_PROCEDURE
    |   K_PROCEDURES
    |   K_PUSH
    |   K_Q
    |   K_QLIMIT
    |   K_QUERYLIMIT
    |   K_QUIT
    |   K_QUOTE
    |   K_R         /* Letter R */
    |   K_REFERENCES
    |   K_REJECT
    |   K_RELOAD
    |   K_RERUN
    |   K_ROLES
    |   K_ROWS
    |   K_SELECT
    |   K_SET
    |   K_SHARE
    |   K_SILENCE
    |   K_SIZE
    |   K_SKIP
    |   K_SLEEP
    |   K_SQLEXIT
    |   K_SQLSTART
    |   K_START
    |   K_STATUS
    |   K_STOP
    |   K_SYNONYMS
    |   K_SYSTABLES
    |   K_TABLE
    |   K_TABLES
    |   K_TIME
    |   K_TO
    |   K_TOGGLE
    |   K_TRACE
    |   K_TRANS
    |   K_TRANSACTION
    |   K_TRANSACTIONS
    |   K_TRANSIZE
    |   K_TRIGBODY
    |   K_TRIGGERS
    |   K_TYPE
    |   K_TYPES
    |   K_UNIQUE
    |   K_UNLOAD
    |   K_UPDATE
    |   K_UPLOAD
    |   K_USE
    |   K_USER
    |   K_USERS
    |   K_USING
    |   K_V         /* Letter V */
    |   K_VERBOSE
    |   K_VERSION
    |   K_VI
    |   K_VIEW
    |   K_VIEWBODY
    |   K_VIEWS
    |   K_WCT
    |   K_WHERE
    |   K_WITH
    |   K_X         /* Letter X */
    |   K_XML
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
        { set_symbol(info->server, sizeof(info->server), $2.start, $2.end); }
    ;

obj_dbname
    :   identifier_or_keyword
        { set_symbol(info->dbase, sizeof(info->dbase), $1.start, $1.end); }
    ;

object
    :   obj_name
        { set_symbol(info->table, sizeof(info->table), $1.start, $1.end); }
    |   obj_owner C_PERIOD obj_name
        {
            /* Must preserve quotes or no-quotes around */
            /* owner to work with MODE ANSI databases!! */
            set_symbol(info->owner, sizeof(info->owner), $1.start, $1.end);
            set_symbol(info->table, sizeof(info->table), $3.start, $3.end);
        }
    ;

/*
** Note that object name in Informix can only be in double quotes (not single
** quotes), and technically only if DELIMIDENT is set.
*/
obj_name
    :   identifier_or_keyword
    |   S_DQSTRING
    ;

/* Note that owner in Informix can be in single or double quotes */
obj_owner
    :   identifier_or_keyword
    |   string
    ;

connect
    :   K_CONNECT K_TO K_DEFAULT opt_wct
        { conn->ctype = CONN_DEFAULT; }
    |   K_CONNECT dbenv opt_conn opt_user opt_wct opt_trusted
    ;

dbenv
    :   K_TO string
        { set_string(conn->dbase, sizeof(conn->dbase), $2.start, $2.end); }
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
        { conn->ctype = CONN_STRING; set_string(conn->uconn, sizeof(conn->uconn), $2.start, $2.end); }
    ;

opt_user
    :   /* Nothing */
        { conn->uname[0] = '\0'; conn->upass[0] = '\0'; }
    |   username password
    ;

username
    :   K_USER string
        { set_string(conn->uname, sizeof(conn->uname), $2.start, $2.end); }
    ;

password
    :   using_or_password string
        { set_string(conn->upass, sizeof(conn->upass), $2.start, $2.end); }
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
        { conn->ctype = CONN_STRING; set_string(conn->uconn, sizeof(conn->uconn), $2.start, $2.end); }
    ;

setconnect
    :   K_SET conntype
    ;

setother
    :   K_SET error token_list
    ;

token_list
    :   /* Nothing */
    |   token_list token
        { $$.start = $1.start; $$.end = $2.end; }
    ;

token
    :   keyword
    |   non_keyword
    ;

non_keyword
    :   C_AT
    |   C_COLON
    |   C_PERIOD
    |   C_LPAREN
    |   C_RPAREN
    |   C_COMMA
    |   S_ERROR
    |   S_IDENTIFIER
    |   string
    ;

conntype
    :   K_CONNECTION string
        { conn->ctype = CONN_STRING; set_string(conn->uconn, sizeof(conn->uconn), $2.start, $2.end); }
    |   K_CONNECTION K_DEFAULT
        { conn->ctype = CONN_DEFAULT; }
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
        { info->where.start = $2.start; info->where.end = $2.end; }
    ;

info_qualified
    :   K_INFO K_COLUMNS for_obj_name
        { $$.toktype = INFO_COLUMNS; }
    |   K_INFO K_INDEXES for_obj_name
        { $$.toktype = INFO_INDEXES; }
    |   K_INFO K_ACCESS for_obj_name
        { $$.toktype = INFO_ACCESS; }
    |   K_INFO K_PRIVILEGES for_obj_name
        { $$.toktype = INFO_ACCESS; }
    |   K_INFO K_STATUS for_obj_name
        { $$.toktype = INFO_STATUS; }
    |   K_INFO K_PROCBODY for_obj_name
        { $$.toktype = INFO_PROCBODY; }
    |   K_INFO K_FRAGMENTS for_obj_name
        { $$.toktype = INFO_FRAGMENTS; }
    |   K_INFO K_REFERENCES to_table
        { $$.toktype = INFO_REFS_TO; }
    |   K_INFO K_REFERENCES by_table
        { $$.toktype = INFO_REFS_BY; }
    |   K_INFO K_VIEWBODY for_obj_name
        { $$.toktype = INFO_VIEWBODY; }
    |   K_INFO K_TRIGGERS for_obj_name
        { $$.toktype = INFO_TRIGGERS; }
    |   K_INFO K_TRIGBODY for_obj_name
        { $$.toktype = INFO_TRIGBODY; }
    |   K_INFO K_CHECK K_CONSTRAINTS for_obj_name
        { $$.toktype = INFO_CONSTR_CHECK; }
    ;

for_obj_name
    :   K_FOR object_name
    ;

to_table
    :   K_TO object_name
    ;

by_table
    :   K_BY object_name
    ;

/* -- INTERNAL COMMANDS -- */

internal_cmd
    :   blobdir_command
    |   clock_command
    |   controlchar_command
    |   date_command
    |   echo_command
    |   edit_command
    |   exit_command
    |   format_command
    |   ibase_command
    |   io_command
    |   list_command
    |   qlimit_command
    |   rerun_command
    |   sleep_command
    |   stack_command
    |   txsize_command
    |   unary_command
    ;

unary_command
    :   K_CONTEXT
    |   K_DBNAMES
    |   K_TIME
    |   K_VERSION
    |   K_SQLEXIT
    |   K_SQLSTART
    ;

controlchar_command
    :   delimiter_command
    |   eor_command
    |   escape_command
    |   quote_command
    ;

qlimit_command
    :   qlimit_keyword scaled_number
    ;

qlimit_keyword
    :   K_QLIMIT
    |   K_QUERYLIMIT
        { $$.toktype = K_QLIMIT; }
    ;

eor_command
    :   K_EOR string
        { set_cstrlit_string(intl->str, sizeof(intl->str), $2.start, $2.end); }
    ;

quote_command
    :   K_QUOTE string
        { set_cstrlit_string(intl->str, sizeof(intl->str), $2.start, $2.end); }
    ;

escape_command
    :   K_ESCAPE string
        { set_cstrlit_string(intl->str, sizeof(intl->str), $2.start, $2.end); }
    ;

echo_command
    :   K_ECHO token_list
        { intl->echo.start = $2.start; intl->echo.end = $2.end; }
    ;

stack_command
    :   adjust_command
    |   benchmark_command
    |   continue_command
    |   headings_command
    |   history_command
    |   silence_command
    |   trace_command
    |   types_command
    |   verbose_command
    ;

adjust_command
    :   K_ADJUST
    |   K_ADJUST S_NUMBER
    ;

stack_operation
    :   K_ON
        { intl->op1 = OP_ON; }
    |   K_OFF
        { intl->op1 = OP_OFF; }
    |   K_PUSH
        { intl->op1 = OP_PUSH; }
    |   K_PUSH on_or_off        /* New: combine push with new state */
        { intl->op1 = OP_PUSH; }
    |   K_POP
        { intl->op1 = OP_POP; }
    |   K_TOGGLE
        { intl->op1 = OP_TOGGLE; }
    ;

on_or_off
    :   K_OFF
        { intl->op2 = OP_OFF; }
    |   K_ON
        { intl->op2 = OP_ON; }
    ;

benchmark_command
    :   K_BENCHMARK
    |   K_BENCHMARK stack_operation
    ;

continue_command
    :   K_CONTINUE
    |   K_CONTINUE stack_operation
    ;

history_command
    :   K_HISTORY
    |   K_HISTORY stack_operation
    ;

silence_command
    :   K_SILENCE
    |   K_SILENCE stack_operation
    ;

trace_command
    :   K_TRACE
    |   K_TRACE stack_operation
    ;

verbose_command
    :   K_VERBOSE
    |   K_VERBOSE stack_operation
    ;

blobdir_command
    :   K_BLOBDIR dirname
    ;

clock_command
    :   K_CLOCK stop_or_start
    ;

stop_or_start
    :   K_STOP
        { intl->op1 = K_STOP; KLUDGE("Should be OP_STOP"); }
    |   K_START
        { intl->op1 = K_START; KLUDGE("Should be OP_START"); }
    ;

io_command
    :   input_command
    |   output_command
    |   error_command
    ;

input_command
    :   K_INPUT
    |   K_INPUT filename    /* String - backwards compatibility break */
    |   K_INPUT input_source    /* New and improved */
    ;

error_command
    :   K_ERROR
    |   K_ERROR filename    /* String - backwards compatibility break */
    |   K_ERROR output_target   /* New and improved */
    ;

output_command
    :   K_OUTPUT
    |   K_OUTPUT filename   /* String - backwards compatibility break */
    |   K_OUTPUT output_target  /* New and improved */
    ;

rerun_command
    :   rerun_keyword cmd_range
    ;

rerun_keyword
    :   K_R         /* Letter R */
        { $$.toktype = K_RERUN; }
    |   K_RERUN
    ;

cmd_range
    :   /* Nothing */
    |   S_NUMBER
        { intl->num1 = neg_number($1.start, $1.end); }
    |   S_NUMBER S_NUMBER
        {
            intl->num1 = neg_number($1.start, $1.end);
            intl->num2 = neg_number($2.start, $2.end);
        }
    ;

list_command
    :   list_keyword cmd_range
    ;

list_keyword
    :   K_L         /* Letter L */
        { $$.toktype = K_LIST; }
    |   K_LIST
    ;

headings_command
    :   headings_keyword stack_operation
    ;

headings_keyword
    :   K_HEADING
        { $$.toktype = K_HEADINGS; }
    |   K_HEADINGS
    ;

types_command
    :   types_keyword stack_operation
    ;

types_keyword
    :   K_TYPE
        { $$.toktype = K_TYPES; }
    |   K_TYPES
    ;

delimiter_command
    :   delimiter_keyword string
        { set_cstrlit_string(intl->str, sizeof(intl->str), $2.start, $2.end); }
    ;

delimiter_keyword
    :    K_DELIM
        { $$.toktype = K_DELIMITER; }
    |    K_DELIMIT
        { $$.toktype = K_DELIMITER; }
    |    K_DELIMITER
    ;

edit_command
    :   edit_keyword cmd_range
    ;

edit_keyword
    :    K_EDIT
    |    K_V            /* Letter V */
        { $$.toktype = K_EDIT; }
    |    K_VI
        { $$.toktype = K_EDIT; }
    |    K_VIEW
        { $$.toktype = K_EDIT; }
    ;

exit_command
    :    K_B            /* Letter B */
        { $$.toktype = K_EXIT; }
    |    K_BYE
        { $$.toktype = K_EXIT; }
    |    K_E            /* Letter E */
        { $$.toktype = K_EXIT; }
    |    K_EXIT
    |    K_Q            /* Letter Q */
        { $$.toktype = K_EXIT; }
    |    K_QUIT
        { $$.toktype = K_EXIT; }
    |    K_X            /* Letter X */
        { $$.toktype = K_EXIT; }
    ;

txsize_command
    :   txsize_keyword scaled_number
        { intl->num3 = num; }
    ;

txsize_keyword
    :   K_TRANS
        { $$.toktype = K_TRANSIZE; }
    |   K_TRANSIZE
    ;

date_command
    :   K_DATE
    |   K_DATE string
        { intl->echo.start = $2.start; intl->echo.end = $2.end; }
    ;

format_command
    :   K_FORMAT
    |   K_FORMAT string
        { intl->echo.start = $2.start; intl->echo.end = $2.end; }
    ;

ibase_command
    :   K_IBASE
    |   K_IBASE S_NUMBER
        { intl->num1 = neg_number($2.start, $2.end); }
    ;

sleep_command
    :   K_SLEEP S_NUMBER
        { intl->num1 = neg_number($2.start, $2.end); }
    ;

%%

/*
** EXTENDED format is the DB-Access -X UNLOAD format where \xy with xy
** as a hex encoding is treated as the corresponding hex character.
*/

/*
** Semantics for UPLOAD:
**  UPLOAD tabname FROM "file"
**      COLUMNS(col1, col2, KEY.col3, col3, KEY.col4, col5, col6)
**      {UNIQUE|NONUNIQUE} [KEY]
**      USE {SELECT|INSERT|UPDATE|HEURISTIC} [ALGORITHM] [SIZE nnn [ROWS]]
**      LOCK [TABLE][IN] {SHARE|EXCLUSIVE} [MODE]
**
** NB: 'UPLOAD opt_to tabname' causes a shift/reduce conflict.
**
** The COLUMNS list specifies the columns that appear in the data file
** in the sequence that they appear.  Note that all columns cited as KEY
** in COLUMNS are in the key.  If a KEY.colN entry has a matching
** unqualified colN, then the key column is to be updated; if it has no
** matching unqualified colN, then that key column is not updated.  You
** cannot specify SIZE unless you use HEURISTIC algorithm.  If you use a
** non-unique key, then the algorithm used must be UPDATE or SELECT (and
** you need to visit a psychologist).
**
** !!Need to check that blobs in file, etc, are OK!!
*/

/*
** SQLCMD is flexible in the formats it will generate as output from
** UNLOAD statements.  SQLCMD is less flexible about what formats it
** will accept as input via LOAD, RELOAD or UPLOAD statements.  However,
** SQLCMD will accept as input anything that can be produced using the
** UNLOAD command and the UNLOAD (delimited field and record) format.
**
** In particular, SQLCMD will not accept XML format data for input; it
** is up to the user to convert XML data into UNLOAD format.
**
** It is up to the user to convert CSV data into UNLOAD format (but this
** might be handled by a later version of SQLCMD).
**
** It is up to the user to convert fixed format data into UNLOAD format
** because it is hard to express how to divide fixed format data up for
** subsequent data processing.  Issues like implicit decimal points and
** converting date and time formats are surprisingly tricky.  And there
** often need to be substitutions for encodings, etc.  It soon becomes
** more cost-effective to have specialized reformatting tools than to
** modify SQLCMD to handle such twists and turns.
** Nevertheless, this assertion will be reviewed when appropriate, so a
** future version of SQLCMD might conceivably handle fixed format input.
*/

/*
** blob_location: K_BLOBS K_TO K_FILE opt_blob_control
** This option controls all blobs automatically, rather than requiring
** the user to specify controls for each blob separately, via the
** opt_keycol and opt_compression options, or you can control individual
** blob fields with the opt_blobcol_file_speclist.
** If the optional in_directory is not specified, then the blobs will
** all be located in the directory specified by ctxt_getblobdir(), which
** defaults to /tmp.  Further, the file names would be based on the blob
** column name and the output row number.  You can change the ID number
** with the KEY keycol.  If compression is enabled, you can compress all
** blobs with the COMPRESSION option.
** Alternatively, you can control blob columns separately by specifying
** a blobcol_file_speclist.  This allows you to specify individual blob
** control specifications for any blobs requiring it -- any blob columns
** not listed will be treated using the default rules.
** Any non-blob columns listed in the blobcol_file_speclist will generate
** an error.
*/


/*
** Valid LOAD statements:

LOAD FROM "somefile" INSERT INTO SomeTable;
LOAD FROM "somefile" INSERT INTO SomeTable(Col01, Col02, Col03);
LOAD FROM PIPE "someprogram" INSERT INTO SomeTable(Col01, Col02, Col03);
LOAD FROM PIPE "someprogram" DELIMITER "\n" INSERT INTO SomeTable(Col01, Col02, Col03);

** Valid RELOAD statements (LOAD and RELOAD are interchangeable):
RELOAD FROM "somefile" INSERT INTO SomeTable;
RELOAD FROM "somefile" INSERT INTO SomeTable(Col01, Col02, Col03);
RELOAD FROM PIPE "someprogram" INSERT INTO SomeTable(Col01, Col02, Col03);
RELOAD FROM PIPE "someprogram" DELIMITER "\n" INSERT INTO SomeTable(Col01, Col02, Col03);
RELOAD FROM PIPE "someprogram"
    WITH AUTOMATIC TRANSACTION SIZE 4K
    SKIP 16K ROWS
    ABORT AFTER 256 ERRORS
    LOCK TABLE IN EXCLUSIVE MODE
    ERROR LOG "/tmp/error.log"
    REJECT LOG "/tmp/reject.log"
    FORMAT CSV
    QUOTE '"'
    DELIMITER "\n"
    ESCAPE '\\' EOL '\r\n'
    INSERT INTO SomeTable(Col01, Col02, Col03);

** Valid UNLOAD statements:
UNLOAD TO "somefile" SELECT * FROM SomeTable;
UNLOAD APPEND TO FILE "somefile" SELECT * FROM SomeTable;
UNLOAD CREATE FILE "somefile" EXECUTE PROCEDURE SomeProc(Arg1, Arg2);
UNLOAD CREATE "somefile" EXECUTE FUNCTION SomeProc(Arg1, Arg2);
UNLOAD TO PIPE "someprogram" SELECT Col01, Col02, Col03 FROM SomeTable;
UNLOAD PIPE "someprogram" DELIMITER "\n" SELECT Col1, Col2, Col3 FROM SomeTable;

UNLOAD TO FILE "somefile" NO CLOBBER MODE 0644 SELECT * FROM SomeTable;
UNLOAD APPEND TO FILE "somefile" NO CREATE SELECT * FROM SomeTable;
UNLOAD APPEND "somefile" CREATE OK SELECT * FROM SomeTable;
UNLOAD APPEND "somefile" CREATE OK MODE 644 SELECT * FROM SomeTable;
UNLOAD CREATE "somefile" CLOBBER OK MODE 644 SELECT * FROM SomeTable;

** Valid UPLOAD statements:
UPLOAD tabname FROM "file" COLUMNS(col1, col2, KEY.col3, col3, KEY.col4, col5, col6)
UNIQUE KEY USE SELECT ALGORITHM LOCK TABLE IN EXCLUSIVE MODE;

UPLOAD tabname FROM "file" COLUMNS(col1, col2, KEY.col3, col3, KEY.col4, col5, col6)
NONUNIQUE USE UPDATE LOCK EXCLUSIVE;

UPLOAD tabname FROM "file" COLUMNS(col1, col2, KEY.col3, col3, KEY.col4, col5, col6)
UNIQUE USE HEURISTIC SIZE nnn ROWS;

UPLOAD tabname FROM "file" COLUMNS(col1, col2, KEY.col3, col3, KEY.col4, col5, col6)
USE HEURISTIC;

UPLOAD tabname FROM "file" COLUMNS(col1, col2, KEY.col3, col3, KEY.col4, col5, col6)
LOCK SHARE MODE;

UPLOAD tabname FROM "file" COLUMNS(col1, col2, KEY.col3, col3, KEY.col4, col5, col6);

*/

struct Keyword
{
    char    *keyword;
    int      token;
};

typedef struct Keyword Keyword;

/* keylist to be maintained in alphabetic order of keyword */

#include "kwlist.c"

static const ScaledNumber zero = { Byte, 0 };

#define DIM(x)  (sizeof(x)/sizeof(*(x)))

/* Convert general digit string to number */
/* Silent about errors in digit string */
static ssize_t neg_number(const char *s, const char *e)
{
    char buffer[13];    /* Big enough for 32-bit octal number with leading zero and trailing null */
    size_t space = e - s + 1;
    if (space > sizeof(buffer))
        space = sizeof(buffer);
    nstrcpy(buffer, space, s);
    return(strtol(buffer, 0, 0));
}

/* Convert general digit string to number */
/* Silent about errors in digit string */
static mode_t pos_number(const char *s, const char *e)
{
    char buffer[13];    /* Big enough for 32-bit octal number with leading zero and trailing null */
    size_t space = e - s + 1;
    if (space > sizeof(buffer))
        space = sizeof(buffer);
    nstrcpy(buffer, space, s);
    return(strtoul(buffer, 0, 0));
}

/* Convert octal digit string to number */
/* Silent about errors in digit string */
static mode_t octal(const char *s, const char *e)
{
    char buffer[13];    /* Big enough for 32-bit octal number with leading zero and trailing null */
    size_t space = e - s + 1;
    if (space > sizeof(buffer))
        space = sizeof(buffer);
    nstrcpy(buffer, space, s);
    return(strtoul(buffer, 0, 8));
}

static int kw_compare(const void *p1, const void *p2)
{
    const Keyword *k1 = (Keyword *)p1;  /*=C++=*/
    const Keyword *k2 = (Keyword *)p2;  /*=C++=*/
    return(cistrcmp(k1->keyword, k2->keyword));
}

static void set_symbol(char *output, size_t outlen, const char *s, const char *e)
{
    size_t space = e - s + 1;
    if (outlen < space)
        space = outlen;
    nstrcpy(output, space, s);
}

static void set_string(char *output, size_t outlen, const char *s, const char *e)
{
    set_symbol(output, outlen, s + 1, e - 1);
}

static void set_cstrlit_string(char *output, size_t outlen, const char *s, const char *e)
{
    KLUDGE("This ignores the possible error return from cstrlit_str()");
    cstrlit_str(s + 1, e - 1, output, outlen);
}

#ifndef NDEBUG
static void chk_kwlist(void)
{
    /* If assert is enabled, verify that keylist is in sorted order */
    static int done_once = 0;
    if (done_once == 0)
    {
        size_t i;
        for (i = 0; i < DIM(keylist) - 1; i++)
            assert(kw_compare(&keylist[i], &keylist[i+1]) < 0);
        done_once = 1;
    }
}
#endif /* NDEBUG */

static void set_statement(char *stmt_str)
{
#ifndef NDEBUG
    chk_kwlist();
#endif /* NDEBUG */
    c_token = stmt_str;
}

YY_SC_LEX int yylex(void)
{
    char        buffer[80];
    Keyword     kw;
    const char *start;

    if (c_token == 0)
        abort();

    start = sqltoken(c_token, &c_token);
    yylval.start = start;
    yylval.end = c_token;
    if (*start == '\0')
    {
        yylval.toktype = 0;
        return yylval.toktype;
    }
    set_symbol(buffer, sizeof(buffer), start, c_token);
    /* printf("yylex(): token = %s\n", buffer); */
    kw.keyword = buffer;
    if (isalpha((unsigned char)buffer[0]) || buffer[0] == '_')
    {
        Keyword *p = 0;
        p = (Keyword *)bsearch(&kw, keylist, DIM(keylist), sizeof(Keyword),
                                kw_compare);    /*=C++=*/
        if (p == 0)
            yylval.toktype = S_IDENTIFIER;
        else
            yylval.toktype = p->token;
    }
    else if (isdigit(buffer[0]))
    {
        /* Lazy scanning  - buggy! */
        yylval.toktype = S_NUMBER;
    }
    else if (buffer[0] == '\'')
    {
        yylval.toktype = S_SQSTRING;
    }
    else if (buffer[0] == '"')
    {
        yylval.toktype = S_DQSTRING;
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
    else if (buffer[0] == ',')
    {
        assert(buffer[1] == '\0');
        yylval.toktype = C_COMMA;
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

YY_SC_SYNTAX void yyerror(const char *s)
{
    /* Do nothing here -- it will be reported elsewhere */
}

void conninfo_setempty(ConnInfo *c)
{
    c->dbase[0]  = '\0';
    c->uconn[0]  = '\0';
    c->uname[0]  = '\0';
    c->upass[0]  = '\0';
    c->ctype     = CONN_NONE;
    c->wct       = 0;
    c->trusted   = 0;
    c->online    = 0;
    c->logged    = 0;
    c->current   = 0;
    c->mode_ansi = 0;
}

static void substring_setempty(SubString *s)
{
    s->start = 0;
    s->end = 0;
}

static void infoinfo_setempty(InfoInfo *i)
{
    i->dbase[0] = '\0';
    i->owner[0] = '\0';
    i->server[0] = '\0';
    i->table[0] = '\0';
    substring_setempty(&i->where);
}

static void loadfile_setempty(LoadFile *f)
{
    f->filename[0] = '\0';
    f->filetype = LOAD_UNDEF;
    f->clobber = UNLOAD_CLOBBER;
    f->create = UNLOAD_CREATE;
    f->fileperms = 0644;
}

static void loadinfo_setempty(LoadInfo *l)
{
    substring_setempty(&l->stmt);
    loadfile_setempty(&l->file);
    loadfile_setempty(&l->rejfile);
    loadfile_setempty(&l->errfile);
    l->blobdir[0] = '\0';
    l->delim[0] = '\0';
    l->algorithm = H_UNDEFINED;
    l->quote[0] = '\0';
    l->escape[0] = '\0';
    l->eol[0] = '\0';
    l->metadata = 0;
    l->load_fmt = K_UNLOAD;
    l->tx_size = zero;
    l->nm_load = zero;
    l->nm_skip = zero;
    l->nm_abrt = zero;
}

static void intlinfo_setempty(IntlInfo *l)
{
    loadfile_setempty(&l->io);
    l->op1 = OP_NONE;
    l->op2 = OP_NONE;
    l->cmd = 0;
    substring_setempty(&l->echo);
    l->str[0] = '\0';
    l->num1 = 0;
    l->num2 = 0;
    l->num3 = zero;
}

static StmtType parse_sqlstmt(char *str, InfoInfo *p_info, ConnInfo *p_conn, LoadInfo *p_load,
                                IntlInfo *p_intl)
{
    info = p_info;
    conn = p_conn;
    load = p_load;
    intl = p_intl;
    stmt_type = STMT_NONE;
    loadinfo_setempty(load);
    infoinfo_setempty(info);
    conninfo_setempty(conn);
    intlinfo_setempty(intl);
    set_statement(str);
    (void)yyparse();
    set_statement(0);
    return stmt_type;
}

StmtType parse_connstmt(char *str, ConnInfo *p_conn)
{
    InfoInfo l_info;
    LoadInfo l_load;
    IntlInfo l_intl;
    return(parse_sqlstmt(str, &l_info, p_conn, &l_load, &l_intl));
}

StmtType parse_infostmt(char *str, InfoInfo *p_info)
{
    ConnInfo l_conn;
    LoadInfo l_load;
    IntlInfo l_intl;
    return(parse_sqlstmt(str, p_info, &l_conn, &l_load, &l_intl));
}

StmtType parse_loadstmt(char *str, LoadInfo *p_load)
{
    ConnInfo l_conn;
    InfoInfo l_info;
    IntlInfo l_intl;
    return(parse_sqlstmt(str, &l_info, &l_conn, p_load, &l_intl));
}

StmtType parse_intlstmt(char *str, IntlInfo *p_intl)
{
    ConnInfo l_conn;
    LoadInfo l_load;
    InfoInfo l_info;
    return(parse_sqlstmt(str, &l_info, &l_conn, &l_load, p_intl));
}

void set_dbase(ConnInfo *c, char *s)
{
    set_symbol(c->dbase, sizeof(c->dbase), s, s + strlen(s));
}

void set_uname(ConnInfo *c, char *s)
{
    set_symbol(c->uname, sizeof(c->uname), s, s + strlen(s));
}

void set_upass(ConnInfo *c, char *s)
{
    set_symbol(c->upass, sizeof(c->upass), s, s + strlen(s));
}

/*
** BISON only defines yydebug if YYDEBUG is set.
** BYACC defines yydebug unconditionally.
** JLSSYACC defines yydebug unless explicitly suppressed with YYDEBUG_UNUSED.
** Unix YACC defines yydebug unconditionally (JL 2001-01-15 to be verified).
*/
#ifndef YYDEBUGVAR
#define YYDEBUGVAR yydebug
#endif
#if YYBISON && !YYDEBUG
int YYDEBUGVAR = 0;
#endif

int set_yydebug(int newval)
{
#if !YYJLSSYACC && !YYBYACC && !YYBISON
    extern int YYDEBUGVAR;
#endif /* !YYJLSSYACC && !YYBYACC && !YYBISON */
    int oldval = YYDEBUGVAR;
    YYDEBUGVAR = newval;
    return(oldval);
}

#ifdef TEST

#include "stderr.h"

static void pr_conninfo(ConnInfo *p_conn)
{
    printf("Database = %s, Connection = %s, Username = %s, "
        "Password = %s WCT = %d, Trusted = %d, ConnType = %d, MODE ANSI = %d\n",
        p_conn->dbase, p_conn->uconn, p_conn->uname, p_conn->upass,
        p_conn->wct, p_conn->trusted, p_conn->ctype, p_conn->mode_ansi);
}

static void pr_loadfile(const char *what, LoadFile *f)
{
    if (f->filename[0] != '\0')
    {
        printf("%s file: %s, ", what, f->filename);
        switch (f->filetype)
        {
        case LOAD_FILE: printf("Read file, "); break;
        case LOAD_PIPE: printf("Read pipe, "); break;
        case UNLOAD_PIPE: printf("Write pipe, "); break;
        case UNLOAD_APFILE: printf("Append file, "); break;
        case UNLOAD_CRFILE: printf("Create file, "); break;
        default:    printf("*** UNDEFINED *** - %d\n", (int)f->filetype);  break;
        }
        switch (f->clobber)
        {
        case UNLOAD_CLOBBER: printf("Clobber OK, "); break;
        case UNLOAD_NOCLOBBER: printf("No Clobber, "); break;
        default: assert(0); break;
        }
        switch (f->create)
        {
        case UNLOAD_CREATE: printf("Create OK, "); break;
        case UNLOAD_NOCREATE: printf("No Create, "); break;
        default: assert(0); break;
        }
        printf(" permissions %o\n", (int)f->fileperms);
    }
}

static void pr_substring(const char *what, SubString *s)
{
    if (s->start != 0)
        printf("%s: %.*s\n", what, (int)(s->end - s->start), s->start);
}

static void pr_ctlstring(const char *what, CtlString s)
{
    if (*s != '\0')
    {
        char buffer[32];
        str_cstrlit(s, buffer, sizeof(buffer));
        printf("%s: %s\n", what, buffer);
     }
}

static void pr_loadinfo(const char *what, LoadInfo *c_load)
{
    printf("%s statement:\n", what);
    pr_loadfile("Data", &c_load->file);
    pr_loadfile("Error Log", &c_load->errfile);
    pr_loadfile("Reject Data", &c_load->rejfile);
    pr_substring("SQL Statement", &c_load->stmt);
    pr_ctlstring("Delimiter", c_load->delim);
    pr_ctlstring("Escape", c_load->escape);
    pr_ctlstring("Quote", c_load->quote);
    pr_ctlstring("EOL", c_load->eol);
    /* Missing lots of details! */
}

static void pr_where(InfoInfo *p_info)
{
    if (p_info->where.start)
    {
        int len = p_info->where.end - p_info->where.start;
        printf(" WHERE (%.*s)", len, p_info->where.start);
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

/**
** Simple minded command reader.
** Looks for complete lines, and assumes command is complete when string
** contains semi-colon.  It does not understand strings, comments, etc.
** DO NOT USE SEMI-COLONS except at end of statement when using this
** command reader.
*/
static int readcmd(char *buffer, size_t buflen, FILE *fp)
{
    while (fgets(buffer, buflen, fp) != 0)
    {
        if (strchr(buffer, ';') != 0)
            return(1);      /* Found a semi-colon */
        if (strchr(buffer, ';') == 0)
        {
            size_t len = strlen(buffer);
            assert(buflen >= len);
            buffer += len;
            buflen -= len;
            if (buflen < 10)
                return(0);  /* Deemed out of space */
        }
    }
    return(0);
}

int main(int argc, char **argv)
{
    char buffer[4096];
    StmtType n;
    InfoInfo c_info;
    ConnInfo c_conn;
    LoadInfo c_load;
    IntlInfo c_intl;

    err_setarg0(argv[0]);
    yydebug = (argc > 1) ? 1 : 0;

    fputs("SQL? ", stdout);
    fflush(stdout);
    while (readcmd(buffer, sizeof(buffer), stdin) != 0)
    {
        buffer[strlen(buffer) - 1] = '\0';
        printf("Input: << %s >>\n", buffer);
        fflush(stdout);
        n = parse_sqlstmt(buffer, &c_info, &c_conn, &c_load, &c_intl);
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
        case STMT_UPLOAD:
            pr_loadinfo("UPLOAD", &c_load);
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
            printf("<<%s>>\nSyntax error in RELOAD statement\n", buffer);
            break;

        case STMT_UPLOADERR:
            printf("<<%s>>\nSyntax error in UPLOAD statement\n", buffer);
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
