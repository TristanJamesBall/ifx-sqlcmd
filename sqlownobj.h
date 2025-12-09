/*
@(#)File:           $RCSfile: sqlownobj.h,v $
@(#)Version:        $Revision: 2007.1 $
@(#)Last changed:   $Date: 2007/03/04 02:23:17 $
@(#)Purpose:        Determine Identifier for an owned object (tables, procedures, triggers)
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 2006-07
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

/*TABSTOP=4*/

#ifndef JLSS_ID_SQLOWNOBJ_H
#define JLSS_ID_SQLOWNOBJ_H

#ifdef  __cplusplus
extern "C" {
#endif

#ifdef MAIN_PROGRAM
#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
extern const char jlss_id_sqlownobj_h[];
const char jlss_id_sqlownobj_h[] = "@(#)$Id: sqlownobj.h,v 2007.1 2007/03/04 02:23:17 jleffler Exp $";
#endif /* lint */
#endif	/* MAIN_PROGRAM */

#include <stddef.h>     /* size_t */

/*
** sql_tabid -- return tabid of table, regardless of database type, etc.
**
** NB: returns -1 on any error; SQL error info is in sqlca record.  The
** owner name can be in quotes or not, and the results may differ
** depending on whether the owner is quoted or not.  It does not matter
** whether the quotes are single or double.  If the first character is a
** quote, the last character is assumed to be the matching quote.  The
** table name must be a valid string; the other parts can be empty strings or
** null pointers.  This code does now handle delimited identifiers for table
** names, requiring strictly double quotes around delimited names.  It uses
** statement IDs p_sql_tabid_q001 and c_sql_tabid_q001.
** The function uses functions vstrcpy(), strlower(), strupper() from jlss.h.
**
** sql_procid -- return procid of procedure, regardless of database type, etc.
** This code handles delimited identifiers for procedure names (and, to be bug
** compatible with IDS.2000, accepts both single and double quotes around the
** procedure names).  This is analogous to sql_tabid() and uses statement IDs
** p_sql_procid_q001 and c_sql_procid_q001.
**
** sql_trigid -- return trigid of trigger, regardless of database type, etc.
** This code does handle delimited identifiers for trigger names, requiring
** strictly double quotes around delimited names.  This is analogous to
** sql_procid() and uses statement IDs p_sql_trigid_q001 and c_sql_trigid_q001.
**
** The sql_mktablename() and sqlmkdbasename() functions format the
** components of a table and database name into a string.  If the owner,
** or dbase or server information is not available, pass a null pointer.
** The functions place the data in the buffer identified by output and
** outlen, and return a pointer to the output buffer if successful, or a
** pointer to null if there is not enough room or some other failure.
*/

extern long     sql_tabid(const char *table, const char *owner,
						  const char *dbase, const char *server, int mode_ansi);
extern long     sql_procid(const char *proc, const char *owner,
						  const char *dbase, const char *server, int mode_ansi);
extern long     sql_trigid(const char *trigger, const char *owner,
						  const char *dbase, const char *server, int mode_ansi);
extern char    *sql_mktablename(const char *table, const char *owner,
								const char *dbase, char *output, size_t outlen);
extern char    *sql_mkdbasename(const char *dbase, const char *server,
								char *output, size_t outlen);

#ifdef  __cplusplus
}
#endif

#endif	/* JLSS_ID_SQLOWNOBJ_H */
