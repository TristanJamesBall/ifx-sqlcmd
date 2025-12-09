/*
@(#)File:           $RCSfile: sqlsignal.c,v $
@(#)Version:        $Revision: 2009.1 $
@(#)Last changed:   $Date: 2009/10/23 18:57:47 $
@(#)Purpose:        Signal handling for SQLCMD
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 2003,2005,2007-09
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

/*TABSTOP=4*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#if __STDC_VERSION__ >= 199901L
#define _XOPEN_SOURCE 600
#else
#define _XOPEN_SOURCE 500
#endif /* __STDC_VERSION__ */

#include <signal.h>

#include "sqlcmd.h"
#include "sqlerr.h"
#include "jlsstools.h"

typedef void (*Signal)(int sig);

#ifdef HAVE_SIGACTION
typedef struct sigaction SigAction;
#endif /* HAVE_SIGACTION */

/*
** At some point, this code needs to record the on-startup disposition
** of signals, so that we don't casually unignore SIGPIPE (in
** particular) when we should not do so.  It should probably, also, be
** more nearly table driven, so that the number of #ifdef SIGxxxx
** sections is reduced to the table.
*/

/*
** This code will use the more modern and reliable sigaction() interface
** if HAVE_SIGACTION is defined.  Otherwise, it will use the archaic and
** less reliable signal() interface.
**
** With sigaction(), you get to specify which signals are suppressed
** while you are in the signal handler, and you do not have to reset the
** signal handling action in the signal handler.
**
** Additional work is needed - lots of it.  Ideally, we need to remove
** the dependency on longjmp.  We need to have a mechanism for detecting
** that an output pipe has broken and then closing it, and abandoning
** whatever operation is in progress.  That might even involve backing
** up several context levels if it is the general output that has gone
** AWOL.  We need to ensure that the code all notices signals and
** abandons ship appropriately.
**
** Note: if the program does ignore SIGPIPE while writing to a pipe (for
** example, as part of an UNLOAD statement), then the write will just
** return with an error (presumably EINTR) and the general error
** handling code for output errors will deal with the issue.
** (See: Rochkind, Advanced UNIX Programming - amongst other sources).
** This has the extra merit of avoiding longjmp().
*/

/*
** 2007-01-27: Patch from Carsten Haese <carsten@uniqsys.com> [Thanks!]
*/

#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
const char jlss_id_sqlsignal_c[] = "@(#)$Id: sqlsignal.c,v 2009.1 2009/10/23 18:57:47 jleffler Exp $";
#endif /* lint */

#ifdef HAVE_SIGACTION
static void set_sigaction(SigAction *action, Signal handler)
{
    sigfillset(&action->sa_mask);
    action->sa_handler = handler;
    action->sa_flags = 0;
}
#endif /* HAVE_SIGACTION */

/* Signal handler: terminate after notifying engine */
static void sig_terminate(int sig)
{
#ifndef HAVE_SIGACTION
    signal(sig, SIG_IGN);
#endif /* HAVE_SIGACTION */
    (void)sqlbreak();
    sql_exit(1);
    /* NOTREACHED */
}

/* Signal handler: continue after notifying engine */
static void sig_continue(int sig)
{
#ifndef HAVE_SIGACTION
    signal(sig, SIG_IGN);
#endif       /* HAVE_SIGACTION */
    (void)sqlbreak();
#ifndef HAVE_SIGACTION
    signal(sig, sig_continue);
#endif /* HAVE_SIGACTION */
    sql_longjmp(-sig);
}

/* Conditionally set signal handler for signal */
static void set_handler(int sig, Signal handler)
{
#ifdef HAVE_SIGACTION
    SigAction old;
    if (sigaction(sig, 0, &old) != 0)
        return;
    if (old.sa_handler != SIG_IGN)
    {
        SigAction act;
        set_sigaction(&act, handler);
        sigaction(sig, &act, 0);
    }
#else
    if (signal(sig, SIG_IGN) != SIG_IGN)
        (void)signal(sig, handler);
#endif /* HAVE_SIGACTION */
}

void sql_ignore_sigpipe(void)
{
#ifdef SIGPIPE
#ifdef HAVE_SIGACTION
    set_handler(SIGPIPE, SIG_IGN);
#else
     Signal ignore = signal(SIGPIPE, SIG_IGN);
#endif /* HAVE_SIGACTION */
#endif /* SIGPIPE */
}

void sql_handle_sigpipe(void)
{
#ifdef SIGPIPE
#ifdef HAVE_SIGACTION
    SigAction act;
    set_sigaction(&act, sig_continue);
    sigaction(SIGPIPE, &act, 0);
#else
     Signal ignore = signal(SIGPIPE, SIG_IGN);
#endif /* HAVE_SIGACTION */
#endif /* SIGPIPE */
}

/* Set correct signal handler for relevant signals */
void    sql_signals(void)
{
#ifdef SIGHUP
    set_handler(SIGHUP, sig_terminate);
#endif
#ifdef SIGTERM
    set_handler(SIGTERM, sig_terminate);
#endif
    if (pflag == SQLFIFO)
    {
#ifdef SIGINT
        set_handler(SIGINT, sig_continue);
#endif
#ifdef SIGQUIT
        set_handler(SIGQUIT, sig_continue);
#endif
#ifdef SIGPIPE
        set_handler(SIGPIPE, sig_continue);
#endif
    }
    else
    {
#ifdef SIGQUIT
        set_handler(SIGQUIT, sig_terminate);
#endif
#ifdef SIGPIPE
        set_handler(SIGPIPE, sig_terminate);
#endif
#ifdef SIGINT
        if (pflag == SQLCMD)
            set_handler(SIGINT, sig_terminate);
        else
            set_handler(SIGINT, sig_continue);
#endif
    }
}
