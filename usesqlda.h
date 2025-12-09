/*
@(#)File:           $RCSfile: usesqlda.h,v $
@(#)Version:        $Revision: 1.1 $
@(#)Last changed:   $Date: 2010/05/26 22:28:08 $
@(#)Purpose:        Manipulate SQLDA Structures
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 2010
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

/*TABSTOP=4*/

#ifndef JLSS_ID_USESQLDA_H
#define JLSS_ID_USESQLDA_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef MAIN_PROGRAM
#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
extern const char jlss_id_usesqlda_h[];
const char jlss_id_usesqlda_h[] = "@(#)$Id: usesqlda.h,v 1.1 2010/05/26 22:28:08 jleffler Exp $";
#endif /* lint */
#endif /* MAIN_PROGRAM */

#include "esqlc.h"
#include <stddef.h>

extern void dealloc_sqlda(Sqlda *idesc);
extern Sqlda *alloc_sqlda(size_t nvar);

#ifdef __cplusplus
}
#endif

#endif /* JLSS_ID_USESQLDA_H */
