/*
@(#)File:           $RCSfile: filelock.h,v $
@(#)Version:        $Revision: 2015.1 $
@(#)Last changed:   $Date: 2015/07/10 16:05:12 $
@(#)Purpose:        Simple-minded advisory file locking
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 2003,2005-06,2015
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

/*TABSTOP=4*/

#ifndef FILELOCK_H
#define FILELOCK_H

#ifdef MAIN_PROGRAM
#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
extern const char jlss_id_filelock_h[];
const char jlss_id_filelock_h[] = "@(#)$Id: filelock.h,v 2015.1 2015/07/10 16:05:12 jleffler Exp $";
#endif /* lint */
#endif  /* MAIN_PROGRAM */

#include <stdio.h>

typedef enum { FLK_READLOCK, FLK_WRITELOCK } LockType;

extern int flk_freelock(FILE *fp);
extern int flk_testlock(FILE *fp, LockType lk_type);
extern int flk_waitlock(FILE *fp, LockType lk_type);

#endif  /* FILELOCK_H */
