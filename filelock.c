/*
@(#)File:           $RCSfile: filelock.c,v $
@(#)Version:        $Revision: 2015.1 $
@(#)Last changed:   $Date: 2015/07/10 16:05:12 $
@(#)Purpose:        Simple-minded advisory file locking
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 2003,2005-06,2015
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

/*TABSTOP=4*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include "filelock.h"
#include "stderr.h"

#include <sys/types.h>
#include <assert.h>
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif /* HAVE_FCNTL_H */
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */

#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
extern const char jlss_id_filelock_c[];
const char jlss_id_filelock_c[] = "@(#)$Id: filelock.c,v 2015.1 2015/07/10 16:05:12 jleffler Exp $";
#endif /* lint */

#if !defined(HAVE_FCNTL)

const char jlss_feature_filelock[] = "@(#)$Warning: File Locking is not implemented/supported $";

/*ARGSUSED*/
int flk_freelock(FILE *fp)
{
    int rc = 0;
    return rc;
}

/*ARGSUSED*/
int flk_testlock(FILE *fp, LockType lk_type)
{
    int rc = 0;
    return rc;
}

/*ARGSUSED*/
int flk_waitlock(FILE *fp, LockType lk_type)
{
    int rc = 0;
    return rc;
}

#else

typedef struct flock FileLock;

static const FileLock zero = { 0 };

static void flk_initlock(FileLock *lk, int lk_type)
{
    *lk = zero;
    lk->l_type   = lk_type;
    lk->l_whence = SEEK_SET;
}

static int locktype_map(LockType lk_type)
{
    int rc = 0;

    switch (lk_type)
    {
    case FLK_READLOCK:
        rc = F_RDLCK;
        break;
    case FLK_WRITELOCK:
        rc = F_WRLCK;
        break;
    default:
        /*CANTHAPPEN*/
        err_report(ERR_ABT, ERR_STAT,
                   "locktype_map(): lk_type out of control (%d)\n", (int)lk_type);
        break;
    }
    return(rc);
}

int flk_freelock(FILE *fp)
{
    FileLock lk;
    int fd;
    int rc;

    assert(fp != 0);
    fd = fileno(fp);

    flk_initlock(&lk, F_UNLCK);
    rc = fcntl(fd, F_SETLK, &lk);

    return rc;
}

int flk_testlock(FILE *fp, LockType lk_type)
{
    int rc = -1;

    assert(fp != 0);
    if (lk_type == FLK_READLOCK || lk_type == FLK_WRITELOCK)
    {
        FileLock lk;
        int fd = fileno(fp);
        flk_initlock(&lk, F_WRLCK);
        rc = fcntl(fd, F_GETLK, &lk);
    }

    return rc;
}

int flk_waitlock(FILE *fp, LockType lk_type)
{
    int rc = -1;

    assert(fp != 0);
    if (lk_type == FLK_READLOCK || lk_type == FLK_WRITELOCK)
    {
        FileLock lk;
        int fd = fileno(fp);
        flk_initlock(&lk, locktype_map(lk_type));
        rc = fcntl(fd, F_SETLK, &lk);
    }

    return rc;
}

#endif /* HAVE_FCNTL */
