/*
@(#)File:           $RCSfile: describe.h,v $
@(#)Version:        $Revision: 1.12 $
@(#)Last changed:   $Date: 2008/02/11 07:39:36 $
@(#)Purpose:        Header file for use with describe
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 1993,1997-98,2000,2003,2006,2008
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

/*TABSTOP=4*/

#ifndef DESCRIBE_H
#define DESCRIBE_H

#ifdef MAIN_PROGRAM
#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
const char jlss_id_describe_h[] = "@(#)$Id: describe.h,v 1.12 2008/02/11 07:39:36 jleffler Exp $";
#endif /* lint */
#endif /* MAIN_PROGRAM */

/*
** sql_describe() allocates space for a given sqlda structure.
** The return value should be freed via sql_release() when the work is complete.
** The del_blob_file argument indicates whether any blobs in files should be deleted.
*/
extern void *sql_describe(Sqlda *desc);
extern void sql_release(Sqlda *desc, void *buffer, int del_blob_file);

#endif /* DESCRIBE_H */
