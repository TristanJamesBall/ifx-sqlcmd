/*
@(#)File:           $RCSfile: sqlerr.h,v $
@(#)Version:        $Revision: 2014.1 $
@(#)Last changed:   $Date: 2014/07/28 03:05:10 $
@(#)Purpose:        Error Codes and Functions for SQLCMD
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 1992-93,1997-2003,2005-06,2008,2012
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

#ifndef SQLERR_H
#define SQLERR_H

#ifdef MAIN_PROGRAM
#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
const char jlss_id_sqlerr_h[] = "@(#)$Id: sqlerr.h,v 2014.1 2014/07/28 03:05:10 jleffler Exp $";
#endif /* lint */
#endif /* MAIN_PROGRAM */

enum Error
{
    E_BADFORMAT,
    E_BADMAGIC,
    E_BLOBFIXED,
    E_CANTOPEN,
    E_CHARCONFLICT,
    E_CMDNOTALLOWED,
    E_CTXTOVERFLOW,
    E_CTXTUNDERFLOW,
    E_DUPTABLE,
    E_EDITORFAIL,
    E_EOFWONL,
    E_FAILCREATETMPFILE,
    E_FAILREOPENTMPFILE,
    E_HISTORYOFF,
    E_INTERNAL,
    E_INVCLOCK,
    E_INVCLQUOTE,
    E_INVCMD,
    E_INVDEBUG,
    E_INVFIFO,
    E_INVOPERATOR,
    E_INVOPQUOTE,
    E_INVOPTION,
    E_INVQLIMIT,
    E_INVSIZE,
    E_INVSKIP,
    E_INVSLEEP,
    E_INVSTRLIT,
    E_INVSUBOPT,
    E_LOCKERR,
    E_MISSINGSUBOPT,
    E_MUTEXOPTS,
    E_NODBASE,
    E_NOTABLE,
    E_NOTFIFO,
    E_OLDMAGIC,
    E_OPTNOTALLOWED,
    E_STRAYARG,
    E_STRTOOLONG,
    E_SYSTEMFAIL,
    E_UNIMPLEMENTED,
    E_ZZZZZZ
};

typedef enum Error Error;

extern void cmd_error(Error errnum, const char *str);
extern void cmd_resume(Error e, const char *aux);
extern void cmd_syserror(Error errnum, const char *str);
extern void cmd_syswarning(Error errnum, const char *str);
extern void cmd_warning(Error errnum, const char *str);

extern void error_746(const char *str);
extern void jb_register(void *s);
extern void jb_release(void);
extern void jb_unregister(void *s);
extern void set_isam_err(long errnum, const char *str);
extern void sql_error(void);
extern void sql_exit(int status);
extern void sql_signals(void);
extern void sql_ignore_sigpipe(void);
extern void sql_handle_sigpipe(void);

#endif /* SQLERR_H */
