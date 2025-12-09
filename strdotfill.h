/*
@(#)File:           $RCSfile: strdotfill.h,v $
@(#)Version:        $Revision: 1.9 $
@(#)Last changed:   $Date: 2008/02/23 22:14:10 $
@(#)Purpose:        Produce string with trailing ellipsis when needed.
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 2004-06,2008
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

/*TABSTOP=4*/

#ifndef STRDOTFILL_H
#define STRDOTFILL_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stddef.h> /* size_t */

#ifdef MAIN_PROGRAM
#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
const char jlss_id_strdotfill_h[] = "@(#)$Id: strdotfill.h,v 1.9 2008/02/23 22:14:10 jleffler Exp $";
#endif /* lint */
#endif /* MAIN_PROGRAM */

/*
** When compiled with assertions, the code asserts on null pointers or a
** zero length output buffer; when compiled without assertions, it
** handles them safely.  The buffer is null terminated unless its length
** is zero or the pointer is null.  The code handles undersize output
** buffers (buf_len too short for "..." or "[...nn characters...]").
** NB: the code returns the pointer passed to it, even if it is null.
*/

/*
** Copy string to buffer, truncating extra characters using ellipsis
** "..." to indicate truncation.  It always returns buffer.
** Given str = "abcdef", str_len = 6, buf_len > 6, generates "abcdef" in buffer
** Given str = "abcdef", str_len = 6, buf_len = 6, generates "ab..."  in buffer.
** Archetypal usage: printf("%s", strdotfill(buffer, sizeof(buffer), str, strlen(str)));
*/
extern char *strdotfill(char *buffer, size_t buf_len, const char *str, size_t str_len);

/*
** Copy string to buffer, showing the number of truncated characters,
** with some number of trailing chars.  It always returns buffer.
** Given str = "abcdefghijklmnopqrstuvwxyz9876543210", str_len = 36, buf_len > 36, trail 10
**   generates "abcdefghijklmnopqrstuvwxyz9876543210" in buffer
** Given str = "abcdefghijklmnopqrstuvwxyz9876543210", str_len = 36, buf_len = 36, trail 10
**   generates "abcdefghi[...16 bytes...]9876543210" in buffer
** Archetypal usage: printf("%s", strdotomit(buffer, sizeof(buffer), str, strlen(str), 10));
*/
extern char *strdotomit(char *buffer, size_t buf_len, const char *str, size_t str_len, size_t trail);

#endif /* STRDOTFILL_H */
