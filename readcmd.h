/*
@(#)File:           $RCSfile: readcmd.h,v $
@(#)Version:        $Revision: 2008.1 $
@(#)Last changed:   $Date: 2008/01/30 16:35:55 $
@(#)Purpose:        Read SQL Statements and Commands for SQLCMD
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 2004-05,2008
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

/*TABSTOP=4*/

#ifndef READCMD_H
#define READCMD_H

#include <stdio.h>

#ifdef MAIN_PROGRAM
#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
const char jlss_id_readcmd_h[] = "@(#)$Id: readcmd.h,v 2008.1 2008/01/30 16:35:55 jleffler Exp $";
#endif /* lint */
#endif /* MAIN_PROGRAM */

extern int      cmd_read(FILE *fp, char *buffer, int length);
extern size_t   cmd_get_linenum(void);
extern void     cmd_add_to_readline_history(const char *cmd);
extern void     cmd_set_linenum(size_t newnum);
extern void     cmd_set_promptnum(long num);
extern void     hit_return_to_continue(void);

#endif /* READCMD_H */
