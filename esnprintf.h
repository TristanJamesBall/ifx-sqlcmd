/*
@(#)File:           $RCSfile: esnprintf.h,v $
@(#)Version:        $Revision: 1.2 $
@(#)Last changed:   $Date: 2009/03/06 03:32:18 $
@(#)Purpose:        Error-checking cover for snprintf(), vsnprintf()
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 2008-09
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

/*TABSTOP=4*/

#ifndef JLSS_ID_ESNPRINTF_H
#define JLSS_ID_ESNPRINTF_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef MAIN_PROGRAM
#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
extern const char jlss_id_esnprintf_h[];
const char jlss_id_esnprintf_h[] = "@(#)$Id: esnprintf.h,v 1.2 2009/03/06 03:32:18 jleffler Exp $";
#endif /* lint */
#endif /* MAIN_PROGRAM */

#include <stddef.h>
#include <stdarg.h>

extern int evsnprintf(char *buffer, size_t buflen, const char *format, va_list args);
extern int  esnprintf(char *buffer, size_t buflen, const char *format, ...);

#ifdef __cplusplus
}
#endif

#endif /* JLSS_ID_ESNPRINTF_H */
