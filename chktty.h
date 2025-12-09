/*
@(#)File:           $RCSfile: chktty.h,v $
@(#)Version:        $Revision: 1.4 $
@(#)Last changed:   $Date: 2015/07/10 16:04:04 $
@(#)Purpose:        Check whether file pointer refers to a tty.
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 2004-05,2015
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

/*TABSTOP=4*/

#ifndef CHKTTY_H
#define CHKTTY_H

#ifdef MAIN_PROGRAM
#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
extern const char jlss_id_chktty_h[];
const char jlss_id_chktty_h[] = "@(#)$Id: chktty.h,v 1.4 2015/07/10 16:04:04 jleffler Exp $";
#endif /* lint */
#endif  /* MAIN_PROGRAM */

#include <stdio.h>

/* Like isatty() - returns true if fp refers to a tty device */
extern int chk_tty(FILE *fp);

#endif  /* CHKTTY_H */
