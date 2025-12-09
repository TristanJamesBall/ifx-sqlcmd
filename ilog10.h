/*
@(#)File:           $RCSfile: ilog10.h,v $
@(#)Version:        $Revision: 1.4 $
@(#)Last changed:   $Date: 2008/02/11 07:39:36 $
@(#)Purpose:        Integer logarithm to base 10 of integers
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 2006-08
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

/*TABSTOP=4*/

#ifndef JLSS_ID_ILOG10_H
#define JLSS_ID_ILOG10_H

#ifdef  __cplusplus
extern "C" {
#endif

#ifdef MAIN_PROGRAM
#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
extern const char jlss_id_ilog10_h[];
const char jlss_id_ilog10_h[] = "@(#)$Id: ilog10.h,v 1.4 2008/02/11 07:39:36 jleffler Exp $";
#endif /* lint */
#endif /* MAIN_PROGRAM */

#include <stddef.h> /* size_t */

extern size_t ilog10_z(size_t x);

#ifdef  __cplusplus
}
#endif

#endif /* JLSS_ID_ILOG10_H */
