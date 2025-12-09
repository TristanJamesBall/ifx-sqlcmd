/*
@(#)File:           $RCSfile: tokencmp.h,v $
@(#)Version:        $Revision: 1.4 $
@(#)Last changed:   $Date: 2008/02/11 07:39:36 $
@(#)Purpose:        Compare two tokens (case insensitive)
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 2004-06,2008
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

/*TABSTOP=4*/

#ifndef TOKENCMP_H
#define TOKENCMP_H

#ifdef MAIN_PROGRAM
#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
const char jlss_id_tokencmp_h[] = "@(#)$Id: tokencmp.h,v 1.4 2008/02/11 07:39:36 jleffler Exp $";
#endif /* lint */
#endif /* MAIN_PROGRAM */

#include <stddef.h> /* size_t */

/*
**  Token comparison -- case-insensitive
**  Returns: 0 if equal, or positive if str > token, or negative if str < token.
*/
int tokencmp(const char *str, size_t len, const char *token, size_t toklen);

#endif /* TOKENCMP_H */
