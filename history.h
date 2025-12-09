/*
@(#)File:           $RCSfile: history.h,v $
@(#)Version:        $Revision: 2015.1 $
@(#)Last changed:   $Date: 2015/07/10 16:05:12 $
@(#)Purpose:        Interface to history system for SQLCMD
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 1992-93,1997-98,2002-03,2005,2009,2015
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

#ifndef HISTORY_H
#define HISTORY_H

/*
** This history mechanism assumes commands are saved in the history file.
** Strictly, they are simply null terminated (possibly multi-line) strings.
*/

#include "stldint.h"

#ifdef MAIN_PROGRAM
#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
extern const char jlss_id_history_h[];
const char jlss_id_history_h[] = "@(#)$Id: history.h,v 2015.1 2015/07/10 16:05:12 jleffler Exp $";
#endif /* lint */
#endif  /* MAIN_PROGRAM */

/* Flag values for hist_output */
enum { H_COMMAND = 1 };
enum { H_NUMBERS = 2 };

/* Flag values for hist_open */
typedef enum { H_READONLY = 1, H_MODIFY   = 2 } HistOpenMode;

/* History error numbers */
enum
{
    H_CANTOPEN  = -1,
    H_BADMAGIC  = -2,
    H_BADFORMAT = -3,
    H_OLDMAGIC  = -4,
    H_LOCKERR   = -5
};

/* Check whether history is initialized, returning 1 if it is, 0 if it is not */
extern int hist_chkinit(void);
/* Read command numbered... */
extern char *hist_read(Sint4 cmdnum);
/* Name of current history file */
extern const char *hist_file(void);
/* Number of commands saved in history file */
extern Uint4 hist_getsize(void);
/* Read command from given file and log it in history file */
extern Uint4 hist_input(FILE *fp);
/* Open history file */
extern Sint4 hist_open(HistOpenMode mode);
/* Write command to history file */
extern Uint4 hist_write(const char *cmd);
/* Close history file */
extern void hist_close(void);
/* Write list of commands (numbered c1..c2) to given file in text format */
extern void hist_output(FILE *fp, Sint4 c1, Sint4 c2, Uint4 flag);
/* Range of history numbers available */
extern void hist_range(Sint4 *lower, Sint4 *upper);
/* Set name of history file */
extern void hist_setfile(const char *name);
/* Set number of commands saved in history file */
extern void hist_setsize(Uint4 newsize);
/* Get current command number */
extern Uint4 hist_getcmdnum(void);
/* Erase range of history numbers */
extern void hist_erase(Sint4 c1, Sint4 c2);

/* Dump entire history file to given file */
extern void hist_dump(FILE *fp);

#endif  /* HISTORY_H */
