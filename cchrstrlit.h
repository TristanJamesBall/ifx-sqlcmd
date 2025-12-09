/*
@(#)File:           $RCSfile: cchrstrlit.h,v $
@(#)Version:        $Revision: 1.1 $
@(#)Last changed:   $Date: 2016/07/08 00:31:16 $
@(#)Purpose:        Converting to/from C character/string literal notation
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 2016
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

/*TABSTOP=4*/

#ifndef JLSS_ID_CCHRSTRLIT_H
#define JLSS_ID_CCHRSTRLIT_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef MAIN_PROGRAM
#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
extern const char jlss_id_cchrstrlit_h[];
const char jlss_id_cchrstrlit_h[] = "@(#)$Id: cchrstrlit.h,v 1.1 2016/07/08 00:31:16 jleffler Exp $";
#endif /* lint */
#endif /* MAIN_PROGRAM */

#include <stddef.h>     /* size_t */

/* Convert C String Literal in (str..end] (excluding surrounding quotes) */
/* to string, returning length of string, or -1 if conversion error, or */
/* -2 if there is not enough room for the output */
extern int cstrlit_str(const char *str, const char *end, char *buffer, size_t buflen);

/* Convert C Character Literal in (str..end] (excluding surrounding quotes) */
/* to character, returning converted char or -1 if string is invalid. */
/* If non-null, eptr is set to first non-converted (or non-convertible) character */
extern int cstrlit_chr(const char *str, const char *end, char const ** const eptr);

/* Convert character to C Character Literal. */
/* buffer[0] = '\0' if there isn't enough room in buffer */
extern void chr_cstrlit(unsigned char c, char *buffer, size_t buflen);

/* Convert string to C String Literal */
extern void str_cstrlit(const char *str, char *buffer, size_t buflen);

#ifdef __cplusplus
}
#endif

#endif /* JLSS_ID_CCHRSTRLIT_H */
