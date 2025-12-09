/*
@(#)File:           $RCSfile: connblob.h,v $
@(#)Version:        $Revision: 2015.1 $
@(#)Last changed:   $Date: 2015/07/10 16:04:31 $
@(#)Purpose:        Connect to database for blob manipulation programs
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 2004-05,2008,2015
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

/*TABSTOP=4*/

#ifndef CONNBLOB_H
#define CONNBLOB_H

#ifdef  __cplusplus
extern "C" {
#endif

#ifdef MAIN_PROGRAM
#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
extern const char jlss_id_connblob_h[];
const char jlss_id_connblob_h[] = "@(#)$Id: connblob.h,v 2015.1 2015/07/10 16:04:31 jleffler Exp $";
#endif /* lint */
#endif  /* MAIN_PROGRAM */

#include <stddef.h> /* size_t */

extern int  blob_connect(int xflag, char *dbase, char *uname, char *upass);
extern void sql_error(const char *s1, const char *s2);
extern void build_where(size_t nkeycols, char **keycol, char **keyval, char *where, size_t size);

#ifdef  __cplusplus
}
#endif

#endif  /* CONNBLOB_H */
