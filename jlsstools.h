/*
@(#)File:           $RCSfile: jlsstools.h,v $
@(#)Version:        $Revision: 2008.4 $
@(#)Last changed:   $Date: 2008/06/08 20:51:28 $
@(#)Purpose:        Function Prototypes for SQLCMD tools
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 1992-93,1995,1997-2008
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

#ifndef JLSSTOOLS_H
#define JLSSTOOLS_H

#ifdef MAIN_PROGRAM
#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
const char jlss_id_jlsstools_h[] = "@(#)$Id: jlsstools.h,v 2008.4 2008/06/08 20:51:28 jleffler Exp $";
#endif /* lint */
#endif  /* MAIN_PROGRAM */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include "jlss.h"
#include "esqlc.h"
#include "memory.h"

/* -- Type Definitions */

struct RecLoc
{
    size_t  rownum;     /* Record number */
    size_t  line_1;     /* First line number */
    size_t  line_2;     /* Last  line number */
};

typedef struct RecLoc RecLoc;

/* -- Declarations */

extern char *quotify(const char *str, char *buffer, size_t buflen);
extern int   chk_fifo(const char *fifo);
extern int   count_blobs(Sqlda *u);
extern int   scanrecord(FILE *fp, Sqlda *idesc, RecLoc *rec, const char *name, Memory *line);
extern void  free_blobs(Sqlda *u);
extern void  rec_zero(RecLoc *rec);
extern void  reload(char *ins);

extern void  sql_command(char *cmd);
extern void  sql_dbconnect(char *dbase, char *uname, char *upass);
extern void  sql_dbinfo(void);
extern void  sql_dbnames(void);
extern void  sql_longjmp(int status);

extern FILE *sql_efopen(const char *file, const char *mode);
extern FILE *sql_epopen(const char *cmd, const char *mode);

extern const char *esqlc_version(void);
extern int   xml_arghandler(const char *argval);
extern void  scribble_buffer(char *buffer, size_t buflen);

#endif  /* JLSSTOOLS_H */
