/*
@(#)File:           $RCSfile: openfile.c,v $
@(#)Version:        $Revision: 2.1 $
@(#)Last changed:   $Date: 2016/01/19 06:48:28 $
@(#)Purpose:        Open files in various secure ways
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 2003,2005,2007-08,2015-16
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

/*TABSTOP=4*/

#include "posixver.h"
#include "openfile.h"
#include "sysstat.h"        /* For Windows */
#include <fcntl.h>          /* O_WRONLY, etc */
#include <stdio.h>          /* fdopen(), etc */
#include <unistd.h>         /* open() */

#define O_RW_RW_R__PERMS    (S_IWUSR|S_IRUSR|S_IWGRP|S_IRGRP|S_IROTH)
#define O_EXCL_CREAT        (O_EXCL|O_CREAT)

#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
extern const char jlss_id_openfile_c[];
const char jlss_id_openfile_c[] = "@(#)$Id: openfile.c,v 2.1 2016/01/19 06:48:28 jleffler Exp $";
#endif /* lint */

/* Open named file without truncate or create safely */
static int open_or_create_file(const char *file, int flags, mode_t perms)
{
    int fd = open(file, flags & ~O_EXCL_CREAT, perms);
    if (fd < 0)
        fd = open(file, flags | O_EXCL_CREAT, perms);
    return(fd);
}

/* Open new named file for read/write with specified perms */
int open_tmpfileperms(const char *file, mode_t perms)
{
    int fd = open(file, O_RDWR | O_EXCL | O_CREAT, perms);
    return(fd);
}

/* Open new named file for read/write */
int open_tmpfile(const char *file)
{
    int fd = open_tmpfileperms(file, O_RW_RW_R__PERMS);
    return(fd);
}

/* Open new named file stream for read/write */
FILE *fopen_tmpfile(const char *file)
{
    FILE *fp = fopen_tmpfileperms(file, O_RW_RW_R__PERMS);
    return(fp);
}

/* Open new named file stream for read/write with specified perms */
FILE *fopen_tmpfileperms(const char *file, mode_t perms)
{
    int fd = open_tmpfileperms(file, perms);
    FILE *fp = 0;
    if (fd >= 0)
        fp = fdopen(fd, "r+");
    return(fp);
}

/* Open named file for append writing, creating safely with specified perms */
int open_logfileperms(const char *file, mode_t perms)
{
    int fd = open_or_create_file(file, O_WRONLY | O_APPEND, perms);
    return(fd);
}

/* Open named file stream for append writing, creating safely with specified perms */
FILE *fopen_logfileperms(const char *file, mode_t perms)
{
    int fd = open_logfileperms(file, perms);
    FILE *fp = 0;
    if (fd >= 0)
        fp = fdopen(fd, "a");
    return(fp);
}

/* Open named file for append writing, creating safely with 664 perms */
int open_logfile(const char *file)
{
    int fd = open_logfileperms(file, O_RW_RW_R__PERMS);
    return(fd);
}

/* Open named file stream for append writing, creating safely with 664 perms */
FILE *fopen_logfile(const char *file)
{
    FILE *fp = fopen_logfileperms(file, O_RW_RW_R__PERMS);
    return(fp);
}
