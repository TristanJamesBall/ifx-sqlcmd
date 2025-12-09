/*
@(#)File:           $RCSfile: upload.h,v $
@(#)Version:        $Revision: 2008.1 $
@(#)Last changed:   $Date: 2008/02/11 15:30:59 $
@(#)Purpose:        Control Information for SQLUPLOAD
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 1998,2000,2003,2005-06,2008
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

/*TABSTOP=4*/

#ifndef UPLOAD_H
#define UPLOAD_H

#ifdef MAIN_PROGRAM
#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
const char jlss_id_upload_h[] = "@(#)$Id: upload.h,v 2008.1 2008/02/11 15:30:59 jleffler Exp $";
#endif /* lint */
#endif  /* MAIN_PROGRAM */

#include "sqlconn.h"

enum Volume
{
    V_QUIET, V_NORMAL, V_VERBOSE
};
typedef enum Volume Volume;

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

typedef enum KeyType
{
    K_UNIQUE, K_DUPS, K_UNDEF
}       KeyType;

typedef const char *LockMode;

typedef char DbsType;

enum DbsTypeFlags
{
    IS_ONLINE_DB = 0x01,
    IS_LOGGED_DB = 0x02,
    IS_MODE_ANSI = 0x04
};

/* UPLOAD data input data specification */
struct Control
{
    char *keycols;          /* Key columns specified */
    char *columns;          /* Data columns specified */
    char *owner;            /* Table owner */
    char *table;            /* Table name */
    LockMode lockmode;      /* Locking mode for table */
    DbsType dbs_type;       /* Type of DBS */
    HMode hmode;            /* Heuristic mode */
    KeyType keytype;        /* Type of key (unique or non-unique) */
    Volume volume;          /* Verbosity of error reporting */
    size_t skip_first;      /* Rows to skip before starting UPLOAD */
    size_t stop_after;      /* Errors allowed before stopping */
    char *inpfile;          /* Input file name */
    char *logfile;          /* Log file name */
    char *rejfile;          /* Reject data file name */
    const char *delim;      /* Delimiter */
    const char *escape;     /* Escape */
    const char *txsize;     /* Size of transaction */
};
typedef struct Control Control;

extern HMode get_heuristic(void);
extern void prt_heuristic(FILE *fp);
extern void set_heuristic(HMode mode, size_t size);
extern void upd_heuristic(char c);

extern HMode char_to_HMode(char c);
extern char HMode_to_char(HMode m);

extern int db_connect(ConnInfo *conn);
extern int db_disconnect(ConnInfo *conn);

extern int upload_stmt(Control *ctrl);

#endif  /* UPLOAD_H */
