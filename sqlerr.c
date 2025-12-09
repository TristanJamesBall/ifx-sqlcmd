/*
@(#)File:           $RCSfile: sqlerr.c,v $
@(#)Version:        $Revision: 2015.1 $
@(#)Last changed:   $Date: 2015/06/24 07:21:31 $
@(#)Purpose:        Error handling for SQLCMD
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 1992-93,1995,1997-2008,2012,2014-15
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

/*TABSTOP=4*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "context.h"
#include "emalloc.h"
#include "esqlc.h"
#include "sqlcmd.h"
#include "sqlerr.h"
#include "jlsstools.h"
#include "esnprintf.h"
#include "stderr.h"
#include "esqlutil.h"

#define CACHESIZE   512

typedef struct ErrMap
{
    Error       errnum;
    const char *errmsg;
}   ErrMap;

static ErrMap errmap[] =
{
    { E_BADFORMAT,          "invalid format in history file" },
    { E_BADMAGIC,           "invalid magic number in history file" },
    { E_BLOBFIXED,          "blobs are always printed in variable width fields" },
    { E_CANTOPEN,           "cannot open file" },
    { E_CHARCONFLICT,       "cannot set two of delimiter, escape and quote to the same value" },
    { E_CMDNOTALLOWED,      "command or statement not allowed:" },
    { E_CTXTOVERFLOW,       "context stack overflow" },
    { E_CTXTUNDERFLOW,      "context stack underflow" },
    { E_DUPTABLE,           "invalid specification of two tables" },
    { E_EDITORFAIL,         "editor exited with status" },
    { E_EOFWONL,            "EOF without terminating newline" },
    { E_FAILCREATETMPFILE,  "cannot create temporary file for editing" },
    { E_FAILREOPENTMPFILE,  "cannot reopen editing file for reading" },
    { E_HISTORYOFF,         "history is not turned on" },
    { E_INTERNAL,           "internal error --" },
    { E_INVCLOCK,           "invalid clock command (not start or stop)" },
    { E_INVCLQUOTE,         "invalid closing quote character for string" },
    { E_INVCMD,             "invalid internal command --" },
    { E_INVDEBUG,           "invalid debug level" },
    { E_INVFIFO,            "invalid use of FIFO option" },
    { E_INVOPERATOR,        "invalid stack operator" },
    { E_INVOPQUOTE,         "invalid opening quote character for string" },
    { E_INVOPTION,          "invalid option" },
    { E_INVQLIMIT,          "invalid query limit" },
    { E_INVSIZE,            "invalid transaction size" },
    { E_INVSKIP,            "invalid skip size" },
    { E_INVSLEEP,           "invalid sleep duration (more than 1 year)" },
    { E_INVSTRLIT,          "invalid character string literal" },
    { E_INVSUBOPT,          "invalid sub-option" },
    { E_LOCKERR,            "file locking error" },
    { E_MISSINGSUBOPT,      "value missing for sub-option" },
    { E_MUTEXOPTS,          "mutually exclusive options specified" },
    { E_NODBASE,            "no database specified" },
    { E_NOTABLE,            "no table specified" },
    { E_NOTFIFO,            "specified file is not a FIFO" },
    { E_OLDMAGIC,           "old format history file - please remove" },
    { E_OPTNOTALLOWED,      "option not allowed" },
    { E_STRAYARG,           "extra arguments specified on command line" },
    { E_STRTOOLONG,         "string is too long" },
    { E_SYSTEMFAIL,         "system() function failed with status" },
    { E_UNIMPLEMENTED,      "unimplemented feature" },
};

static void *sjb_cache[CACHESIZE];
static int sjb_sp = 0;

#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
extern const char jlss_id_sqlerr_c[];
const char jlss_id_sqlerr_c[] = "@(#)$Id: sqlerr.c,v 2015.1 2015/06/24 07:21:31 jleffler Exp $";
#endif /* lint */

/* Resume execution at the appropriate point after an error */
static void sql_resume(void)
{
    switch (pflag)
    {
    case SQLFIFO:
    case SQLCMD:
    case DBACCESS:
        if (ctxt_getcontinue() == OP_OFF)
            sql_longjmp(1);
        return;
    case SQLUPLOAD:
    case SQLRELOAD:
    case SQLUNLOAD:
        break;
    case SQLEARG:
    case SQLFARG:
        if (ctxt_getcontinue() == OP_ON)
            return;
        break;
    default:
        err_remark("pflag out of control in sql_resume\n");
        break;
    }
    sql_exit(1);
    /* NOTREACHED */
}

/* Validate the error message map - when assert is enabled */
/* Error numbers should be contiguous from zero */
#ifdef NDEBUG
#define CHK_ERRMAP()    ((void)0)
#else
#define CHK_ERRMAP()    chk_errmap()
static void chk_errmap(void)
{
    size_t i;
    assert(errmap[0].errnum == 0);
    assert(DIM(errmap) > 2);
    for (i = 1; i < DIM(errmap); i++)
    {
        assert(errmap[i].errnum == i);
        assert(errmap[i].errnum == errmap[i-1].errnum + 1);
    }
}
#endif /* NDEBUG */

static int cmp_errmap(const void *v1, const void *v2)
{
    const ErrMap *e1 = (const ErrMap *)v1;
    const ErrMap *e2 = (const ErrMap *)v2;
    return(e1->errnum - e2->errnum);
}

/* Initialize error message map array */
static void init_errmap(void)
{
    /* Sort the error message map */
    qsort(errmap, DIM(errmap), sizeof(ErrMap), cmp_errmap);
    CHK_ERRMAP();
}

static const char *err_message(Error e)
{
    const char *m;
    static char buffer[80];
    static int  init = 0;

    if (init == 0)
    {
        init_errmap();
        init = 1;
    }

    if ((int)e < 0 || e >= DIM(errmap))
    {
        esnprintf(buffer, sizeof(buffer),
            "<<INTERNAL ERROR - UNKNOWN SQLCMD ERROR NUMBER %d>>", (int)e);
        m = buffer;
    }
    else
        m = errmap[e].errmsg;
    return m;
}

static void prt_errmsg(const char *msg, const char *aux)
{
    const char *in_file = ctxt_infile();
    unsigned long in_line = ctxt_getlinenum();
    fprintf(ctxt_error(), "%s (at %s:%lu): %s %s\n", err_getarg0(), in_file, in_line, msg, aux);
}

/* Internally generated warning */
void cmd_warning(Error e, const char *aux)
{
    if (ctxt_getsilence() == OP_OFF)
    {
        const char *msg = err_message(e);
        prt_errmsg(msg, aux);
    }
}

/* Print internally generated warning, then resume */
void cmd_resume(Error e, const char *aux)
{
    cmd_warning(e, aux);
    sql_resume();
}

/* Internally generated error */
void cmd_error(Error e, const char *s)
{
    cmd_warning(e, s);
    if (e == E_INTERNAL)
        abort();
    sql_exit(1);
}

/* Internally generated warning */
void cmd_syswarning(Error e, const char *s)
{
    /* ctxt_getsilence() probably doesn't clobber errno, but make sure */
    int syserr = errno;

    if (ctxt_getsilence() == OP_OFF)
    {
        const char *m = err_message(e);
        prt_errmsg(m, s);
        if (syserr != 0)
        {
            const char *sysstr = strerror(syserr);
            const char *arg0 = err_getarg0();
            int arg0len = strlen(arg0) + 1;
            fprintf(ctxt_error(), "%*s errno %d - %s\n", arg0len, "", syserr, sysstr);
        }
    }
}

/* Internally generated error */
void cmd_syserror(Error e, const char *s)
{
    cmd_syswarning(e, s);
    if (e == E_INTERNAL)
        abort();
    sql_exit(1);
}

void set_isam_err(long errnum, const char *str)
{
    nstrcpy(sqlca.sqlerrm, sizeof(sqlca.sqlerrm), str);
    sqlca.sqlerrd[1] = errnum;
}

void error_746(const char *str)
{
    sqlca.sqlcode = -746;
    sqlca.sqlerrd[1] = 0;
    nstrcpy(sqlca.sqlerrm, sizeof(sqlca.sqlerrm), str);
    sql_error();
}

/* Database generated error */
void sql_error(void)
{
    fflush(ctxt_output());
    if (ctxt_getsilence() == OP_OFF)
    {
        sql_printerror(ctxt_error());
#ifdef ESQLC_SQLSTATE
        if (strcmp(SQLSTATE, "00000") != 0)
            fprintf(ctxt_error(), "SQLSTATE: ESQL %s ", SQLSTATE);
#endif /* ESQLC_SQLSTATE */
        fprintf(ctxt_error(), "at BUILTIN %s:%lu\n", ctxt_infile(), (unsigned long)ctxt_getlinenum());
    }
    sql_resume();
}

/* Exit from program, doing all clean up */
void sql_exit(int status)
{
    /* 2003-11-18 -- hist_close(); -- done by atexit() */
    ctxt_free();
    jb_release();
#ifdef DEBUG_MALLOC
    dump_malloc();
#endif  /* DEBUG_MALLOC */
    exit(status);
    /* NOTREACHED */
}

/*
** Register a piece of memory for release
*/
void jb_register(void *s)
{
    if (sjb_sp < CACHESIZE)
        sjb_cache[sjb_sp++] = s;
    else
        cmd_error(E_INTERNAL, "cannot register memory");
}

/*
** Unregister a piece of memory for release
** Cannot be combined wih FREE: code in output.c uses REALLOC!
*/
void jb_unregister(void *s)
{
    int     i;

    for (i = 0; i < sjb_sp; i++)
    {
        if (sjb_cache[i] == s)
        {
            for (; i < sjb_sp; i++)
                sjb_cache[i] = sjb_cache[i + 1];
            sjb_sp--;
            return;
        }
    }
    cmd_error(E_INTERNAL, "unregistering non-registered memory");
}

/*
** Release all registered memory
** Used after an interrupt.
*/
void jb_release(void)
{
    while (sjb_sp > 0)
        FREE(sjb_cache[--sjb_sp]);
}
