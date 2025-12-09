/*
@(#)File:           $RCSfile: internal.h,v $
@(#)Version:        $Revision: 2008.1 $
@(#)Last changed:   $Date: 2008/07/12 22:54:59 $
@(#)Purpose:        Function declarations for internal.c
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 1992-93,1997-99,2003-05,2007-08
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

/*TABSTOP=4*/

#ifndef INTERNAL_H
#define INTERNAL_H

#ifdef MAIN_PROGRAM
#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
const char jlss_id_internal_h[] = "@(#)$Id: internal.h,v 2008.1 2008/07/12 22:54:59 jleffler Exp $";
#endif /* lint */
#endif /* MAIN_PROGRAM */

#include <stdio.h>

extern char    *skipalphanum(char *s);
extern char    *skipblanks(char *s);
extern char    *skipword(char *s);
extern void     do_command(const char *str);
extern void     hist_init(void);
extern void     sql_file(void);
extern void     sql_filename(char *file);
extern void     sql_info(char *str);
extern void     sql_monitor(char *fifo);
extern void     sql_reload(char *tname);
extern void     sql_unload(char *tname, char *order_by);
extern void     sql_version(FILE *fp);

#endif /* INTERNAL_H */
