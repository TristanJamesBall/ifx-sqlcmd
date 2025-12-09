/*
@(#)File:            $RCSfile: grammar.h,v $
@(#)Version:         $Revision: 2.4 $
@(#)Last changed:    $Date: 2010/12/18 22:36:05 $
@(#)Purpose:         Interface to CONNECT, DISCONNECT, SET CONNECTION parser
@(#)Author:          J Leffler
@(#)Copyright:       (C) JLSS 2001
@(#)Product:         SQLCMD Version 90.02 (2016-07-28)
*/

/*TABSTOP=4*/

#ifndef GRAMMAR_H
#define GRAMMAR_H

#include "esqlc.h"
#include "context.h"

#ifdef MAIN_PROGRAM
#ifndef lint
static const char grammar_h[] = "@(#)$Id: grammar.h,v 2.4 2010/12/18 22:36:05 jleffler Exp $";
#endif	/* lint */
#endif	/* MAIN_PROGRAM */

/* Current versions of Informix actually only support commands up to 64 KB */
#define SQL_CMDBUFSIZE		(4*65536)
#define MAX_DBASE_LENGTH	(128+1)
#define MAX_UNAME_LENGTH	(32+1)
#define MAX_FNAME_LENGTH	256
#define MAX_DELIM_LENGTH	4

/* Statement types -- avoid 0 */
enum StmtType
{
	INFO_ERROR = -9,	/* Must be most negative number */
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
	INFO_TABLES,		/* Tables, Views, Public Synonyms, Private Synonyms */
	INFO_SYSTABLES,		/* SQLCMD only */
	INFO_BASETABLES,	/* SQLCMD only */
	INFO_VIEWS,			/* SQLCMD only */
	INFO_SYNONYMS,		/* SQLCMD only */
	INFO_PROCEDURES,	/* SQLCMD only */
	INFO_PROCBODY,		/* SQLCMD only */
	INFO_VIEWBODY,		/* SQLCMD only */
	INFO_COLUMNS,
	INFO_INDEXES,
	INFO_ACCESS,		/* aka PRIVILEGES */
	INFO_REFS_TO,		/* SQLCMD only */
	INFO_REFS_BY,		/* SQLCMD only */
	INFO_STATUS,		/* */
	INFO_FRAGMENTS,		/* */
	INFO_CONSTR_CHECK,	/* not implemented */
	INFO_DATABASES,		/* SQLCMD only */
	INFO_CONNECTIONS,	/* SQLCMD only */
	INFO_TRIGGERS,		/* SQLCMD only */
	INFO_TRIGBODY,		/* SQLCMD only */
	INFO_USERS,			/* SQLCMD only */
	INFO_ROLES,			/* SQLCMD only */
	INFO_HELP			/* SQLCMD only */
};
typedef enum StmtType StmtType;

/*
** CONN_NONE    - no connection name was specified.
** CONN_STRING  - connection name is string
** CONN_DEFAULT - connection is default
** CONN_CURRENT - disconnect current (set connection current dormant)
** CONN_ALL     - disconnect all
*/
enum ConnType { CONN_NONE, CONN_STRING, CONN_DEFAULT, CONN_CURRENT, CONN_ALL };
typedef enum ConnType ConnType;

/*
** LOAD_FILE     - LOAD from a file
** LOAD_PIPE     - LOAD from a pipe
** UNLOAD_CRFILE - UNLOAD to a file
** UNLOAD_APFILE - UNLOAD and append to a file
** UNLOAD_PIPE   - UNLOAD to a pipe
*/
enum FileType { LOAD_UNDEF, LOAD_FILE, LOAD_PIPE, UNLOAD_CRFILE, UNLOAD_APFILE, UNLOAD_PIPE };
typedef enum FileType FileType;

enum Create { UNLOAD_CREATE, UNLOAD_NOCREATE };
typedef enum Create Create;

enum Clobber { UNLOAD_CLOBBER, UNLOAD_NOCLOBBER };
typedef enum Clobber Clobber;

enum Scale
{
        Byte = 0,   /* 1 */
    KiloByte = 1,   /* 1024 */
    MegaByte = 2,   /* 1024 * 1024 */
    GigaByte = 3,   /* 1024 * 1024 * 1024 */
    TeraByte = 4,   /* 1024 * 1024 * 1024 * 1024 */
};
typedef enum Scale Scale;

struct ScaledNumber
{
	Scale	scale;
	size_t	number;
};
typedef struct ScaledNumber ScaledNumber;

typedef char FileName[MAX_FNAME_LENGTH];
typedef char UserName[MAX_UNAME_LENGTH];
typedef char DbaseName[MAX_DBASE_LENGTH];
typedef char CtlString[MAX_DELIM_LENGTH];

struct LoadFile
{
	FileName	filename;
	FileType	filetype;
	Create		create;
	Clobber		clobber;
	mode_t	    fileperms;					/* Created file permissions */
};
typedef struct LoadFile LoadFile;

/* Name LockMode is a temporary measure until we fix upload.h */
enum LockMode { LOCK_EXCLUSIVE, LOCK_SHARED, LOCK_NONE };
typedef enum LockMode LockMode;

struct SubString
{
	const char    *start;		/* INFO - Start of condition for WHERE clause */
	const char    *end;		/* INFO - One beyond end of condition for WHERE clause */
};
typedef struct SubString SubString;

enum HMode
{
	H_HEURISTIC = 'H',
	H_INSERT = 'I',
	H_UPDATE = 'U',
	H_SELECT = 'S',
	H_UNDEFINED = 0,
	H_COMMIT = 'C',
	H_SQLERROR = '*',
	H_FMTERROR = '@',
	H_NEWLINE = '\n'
};
typedef enum HMode HMode;

/* Information for CONNECT, SET CONNECTION, DISCONNECT commands. */
struct ConnInfo
{
	DbaseName	 dbase;		/* Connection dbase@server */
	DbaseName	 uconn;		/* Connection name */
	UserName	 uname;		/* Connection Username */
	UserName	 upass;		/* Connection Password */
	ixUint1  ctype;			/* Connection type */
	ixUint1	 wct;			/* Connection with concurrent TX */
	ixUint1	 trusted;		/* Connection in trusted context */
	ixUint1  mode_ansi;		/* DBase is MODE ANSI (vs regular) */
	ixUint1  logged;		/* DBase is Logged (vs UnLogged) */
	ixUint1  online;		/* DBase is OnLine (vs SE) */
	ixUint1  current;		/* Connection is current */
};
typedef struct ConnInfo ConnInfo;

/* Information for INFO xyz commands. */
struct InfoInfo
{
	DbaseName	 dbase;		/* Database for DB-qualified object name */
	DbaseName	 server;	/* Server for DB-qualified object name */
	UserName	 owner;		/* Owner qualified object name */
	DbaseName	 table;		/* Object name */
	SubString	 where;		/* Condition in WHERE clause */
};
typedef struct InfoInfo InfoInfo;

/* Information for internal (to SQLCMD) commands. */
struct LoadInfo
{
	LoadFile	file;			/* LOAD/UNLOAD Data File Name */
	LoadFile	rejfile;		/* LOAD/UNLOAD Reject Log File Name */
	LoadFile	errfile;		/* LOAD/UNLOAD Error Log File Name */
	FileName	blobdir;		/* LOAD/UNLOAD Blob Directory */
	CtlString	delim;			/* LOAD/UNLOAD Delimiter */
	CtlString	escape;			/* LOAD/UNLOAD Escape */
	CtlString	quote;			/* LOAD/UNLOAD Quote */
	CtlString	eol;			/* LOAD/UNLOAD End Of Line */
	SubString	stmt;			/* SELECT statement for UNLOAD; INSERT statement for LOAD */
	LockMode	lock;			/* Table locking */
	HMode		algorithm;		/* Algorithm (Heuristic, Select, Insert, Update)  */
	int     	load_fmt;		/* Load format */
	ScaledNumber	tx_size;	/* Size of transaction */
	ScaledNumber	nm_skip;	/* Number of rows to skip */
	ScaledNumber	nm_abrt;	/* Abort after N errors */
	ScaledNumber	nm_load;	/* Load only N rows */
	int			metadata;		/* Metadata in unload file */
};
typedef struct LoadInfo LoadInfo;

typedef ssize_t SignedNumber;

struct IntlInfo
{
	LoadFile	io;		/* For extended output, input, error commands */
	Operator	op1;	/* OP_* values from context.h */
	Operator	op2;	/* OP_* values from context.h */
	CtlString	str;	/* String information for DELIMITER etc. */
	SubString	echo;	/* Text to echo */
	SignedNumber	num1;	/* First value in command range, or size, etc. */
	SignedNumber	num2;	/* Second value in command range */
	ScaledNumber	num3;	/* Scaled number for TX sizes */
	int         cmd;	/* K_* values from y.tab.h??? */
};
typedef struct IntlInfo IntlInfo;

extern StmtType parse_connstmt(char *str, ConnInfo *p_conn);
extern StmtType parse_infostmt(char *str, InfoInfo *p_info);
extern StmtType parse_loadstmt(char *str, LoadInfo *p_load);
extern StmtType parse_intlstmt(char *str, IntlInfo *p_intl);

extern void set_dbase(ConnInfo *conn, char *s);
extern void set_uname(ConnInfo *conn, char *s);
extern void set_upass(ConnInfo *conn, char *s);

extern void sql_connect(ConnInfo *conn);
extern void sql_disconn(ConnInfo *conn);
extern void sql_setconn(ConnInfo *conn);

extern int  conninfo_current_modeansi(void);
extern void conninfo_connect(ConnInfo *conn);
extern void conninfo_disconnect(ConnInfo *conn);
extern void conninfo_print(void);
extern void conninfo_setconnection(ConnInfo *conn);

extern void conninfo_setempty(ConnInfo *conn); /* Used in sqlstmt.ec; defined connecty.y */

extern int set_yydebug(int);

#endif	/* GRAMMAR_H */
