/*
@(#)File:           $RCSfile: datefmt.h,v $
@(#)Version:        $Revision: 2015.1 $
@(#)Last changed:   $Date: 2015/07/10 16:05:12 $
@(#)Purpose:        Convert DBDATE values to rfmtdate() format
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 2006-07,2015
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

/*TABSTOP=4*/

#ifndef JLSS_ID_DATEFMT_H
#define JLSS_ID_DATEFMT_H

#ifdef  __cplusplus
extern "C" {
#endif

#ifdef MAIN_PROGRAM
#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
extern const char jlss_id_datefmt_h[];
const char jlss_id_datefmt_h[] = "@(#)$Id: datefmt.h,v 2015.1 2015/07/10 16:05:12 jleffler Exp $";
#endif /* lint */
#endif  /* MAIN_PROGRAM */

extern const char *cvt_dbdate(const char *dbdate);
extern const char *fmt_dbdate(void);

#ifdef  __cplusplus
}
#endif

#endif  /* JLSS_ID_DATEFMT_H */
