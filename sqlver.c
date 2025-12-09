/*
@(#)File:           $RCSfile: sqlver.c,v $
@(#)Version:        $Revision: 2015.1 $
@(#)Last changed:   $Date: 2015/07/10 16:05:12 $
@(#)Purpose:        Print Command and ESQL/C Version Numbers
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 1998,2000-05,2008,2010,2013,2015
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

/*TABSTOP=4*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <assert.h>
#include <stdio.h>
#include "internal.h"
#include "jlsstools.h"
#include "esnprintf.h"
#include "gnureadline.h"
#include "stderr.h"

#ifndef ESQLC_STRING
/* This is primarily for use on Windows NT */
#ifndef ESQLC_VERSTR
#define ESQLC_VERSTR "??UNKNOWN??"
#endif
#ifndef ESQLC_PRDSTR
#define ESQLC_PRDSTR "Informix ESQL/C"
#endif
#define ESQLC_STRMAC(x) #x
#define ESQLC_VERMAC(p,v) p " Version " ESQLC_STRMAC(v)
#define ESQLC_STRING    ESQLC_VERMAC(ESQLC_PRDSTR, ESQLC_VERSTR)
#endif /* ESQLC_STRING */

static const char jlss_cr[] = "(C) Copyright Jonathan Leffler 1987-2016";

#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
extern const char jlss_id_sqlver_c[];
const char jlss_id_sqlver_c[] = "@(#)$Id: sqlver.c,v 2015.1 2015/07/10 16:05:12 jleffler Exp $";
#endif /* lint */

/* SQLCMD licence string */
static const char *sqlcmd_licence(void)
{
    return(&"@(#)GNU General Public Licence Version 2"[4]);
}

/* SQLCMD version string */
static const char *sqlcmd_version(void)
{
    return(&"@(#)SQLCMD Version 90.02 (2016-07-28)"[4]);
}

#if defined(HAVE_READLINE_READLINE_H) && defined(HAVE_LIBREADLINE)
static const char *readline_version(void)
{
    return rl_library_version;
}
#endif /* HAVE_READLINE_READLINE_H && HAVE_LIBREADLINE */

/* Write SQLCMD & ESQL/C version information */
void            sql_version(FILE *fp)
{
    FILE *olderr;
    char buffer1[128];
    char buffer2[128];
    char buffer3[128];
    const char *v1;
    const char *v2 = buffer2;
    const char *v3 = buffer3;

    v1 = err_rcs_string(sqlcmd_version(), buffer1, sizeof(buffer1));
    olderr = err_stderr(fp);
    buffer2[0] = '\0';
#if defined(HAVE_READLINE_READLINE_H) && defined(HAVE_LIBREADLINE)
    esnprintf(buffer2, sizeof(buffer2), "GNU Readline %s\n", readline_version());
#endif /* HAVE_READLINE_READLINE_H && HAVE_LIBREADLINE */
    esnprintf(buffer3, sizeof(buffer3), "Licenced under %s\n", sqlcmd_licence());
    err_remark("%s\n%s\n%s%s\n%s", v1, esqlc_version(), v2, jlss_cr, v3);
    olderr = err_stderr(olderr);
    assert(olderr == fp);
}
