/*
@(#)File:           $RCSfile: esqlver.c,v $
@(#)Version:        $Revision: 2008.1 $
@(#)Last changed:   $Date: 2008/02/11 04:54:33 $
@(#)Purpose:        Print Command and ESQL/C Version Numbers
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 2000,2003,2005,2008
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

/*TABSTOP=4*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <assert.h>
#include <stdio.h>
#include "jlsstools.h"

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

#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
const char jlss_id_esqlver_c[] = "@(#)$Id: esqlver.c,v 2008.1 2008/02/11 04:54:33 jleffler Exp $";
#endif /* lint */

/* ESQL/C version string */
const char *esqlc_version(void)
{
    return ESQLC_STRING;
}
