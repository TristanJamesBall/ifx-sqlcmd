/*
@(#)File:           $RCSfile: openfile.h,v $
@(#)Version:        $Revision: 1.5 $
@(#)Last changed:   $Date: 2008/05/18 16:50:24 $
@(#)Purpose:        Open files in various secure ways.
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 2005-06,2008
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

/*TABSTOP=4*/

#ifndef OPENFILE_H
#define OPENFILE_H

#ifdef  __cplusplus
extern "C" {
#endif

#ifdef MAIN_PROGRAM
#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
const char jlss_id_openfile_h[] = "@(#)$Id: openfile.h,v 1.5 2008/05/18 16:50:24 jleffler Exp $";
#endif /* lint */
#endif /* MAIN_PROGRAM */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include "sysstat.h"    /* mode_t for Windows - <sys/types.h> for POSIX */

/* Open named (text) file stream for append writing, creating safely */
extern FILE *fopen_logfile(const char *file);

/* Open named (text) file for append writing, creating safely */
extern int open_logfile(const char *file);

/* Open new named (text) file stream for read/write with 664 perms */
extern FILE *fopen_tmpfile(const char *file);

/* Open new named (text) file for read/write with 664 perms */
extern int open_tmpfile(const char *file);

/* Open new named (text) file stream for read/write with specified perms */
extern FILE *fopen_tmpfileperms(const char *file, mode_t perms);

/* Open new named (text) file for read/write with specified perms */
extern int open_tmpfileperms(const char *file, mode_t perms);

/* Open named (text) file stream for append writing, creating safely with specified perms */
extern FILE *fopen_logfileperms(const char *file, mode_t perms);

/* Open named (text) file for append writing, creating safely with specified perms */
extern int open_logfileperms(const char *file, mode_t perms);

#ifdef  __cplusplus
}
#endif

#endif /* OPENFILE_H */
