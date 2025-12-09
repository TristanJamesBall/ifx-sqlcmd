/*
@(#)File:           $RCSfile: dumpconfig.h,v $
@(#)Version:        $Revision: 1.6 $
@(#)Last changed:   $Date: 2008/06/02 14:08:47 $
@(#)Purpose:        Internal header for Informix ESQL/C Type Dump Functions
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 2008
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

/*TABSTOP=4*/

#ifndef JLSS_ID_DUMPCONFIG_H
#define JLSS_ID_DUMPCONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef MAIN_PROGRAM
#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
extern const char jlss_id_dumpconfig_h[];
const char jlss_id_dumpconfig_h[] = "@(#)$Id: dumpconfig.h,v 1.6 2008/06/02 14:08:47 jleffler Exp $";
#endif /* lint */
#endif /* MAIN_PROGRAM */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if !defined(HAVE_INTTYPES_H) && __STDC_VERSION__ >= 199901L
#define HAVE_INTTYPES_H
#endif /* __STDC_VERSION__ */

#ifdef RECORD_DUMPCONFIG_CONFIG
#define DUMPCONFIG(var, str)   static const char var[] = "@(#)" str;
#else
#define DUMPCONFIG(var, str)   /* As nothing */
#endif /* RECORD_DUMPCONFIG_CONFIG */

#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
DUMPCONFIG(jlss_id_dumpconfig_inttypes_h, "Header inttypes.h present - presume uintptr_t is there")
#undef HAVE_UINTPTR_T
#define HAVE_UINTPTR_T
#endif /* HAVE_INTTYPES_H */

#include <limits.h>

#ifndef HAVE_INTTYPES_H
#if LONG_MAX >  INT_MAX
#define uintptr_t unsigned long
DUMPCONFIG(jlss_id_dumpconfig_uintptr, "Header inttypes.h missing - #define uintptr_t unsigned long")
#else
#define uintptr_t unsigned int
DUMPCONFIG(jlss_id_dumpconfig_uintptr, "Header inttypes.h missing - #define uintptr_t unsigned int")
#endif
#undef HAVE_UINTPTR_T
#define HAVE_UINTPTR_T
#endif /* HAVE_INTTYPES_H */

/*
** The only definitions used from INTTYPES_H are uintptr_t and PRIXPTR.
** Solaris 9 has <inttypes.h> but does not have PRIXPTR; it was tracking
** a draft of the standard and does not include this definition.
** Solaris  9 uses 'unsigned int'  for 32-bit; 'unsigned long' for 64-bit.
** Solaris 10 uses 'unsigned long' for 32-bit; 'unsigned long' for 64-bit.
** -- 32-bit: int  is 32-bits, the same as a pointer, so X is OK.
** -- 64-bit: long is 64-bits (except for 64-bit Windows), so lX is OK.
** -- For LP64 systems such as Unix, LONG_MAX > INT_MAX.
*/
#ifndef PRIXPTR
#if LONG_MAX > INT_MAX
DUMPCONFIG(jlss_id_dumpconfig_prixptr, "PRIXPTR undefined - choose lX")
#define PRIXPTR "lX"
#else
DUMPCONFIG(jlss_id_dumpconfig_prixptr, "PRIXPTR undefined - choose X")
#define PRIXPTR "X"
#endif
#else
#define dmpcfg_str(x) #x
#define dmpcfg_dbl(x) dmpcfg_str(x)
DUMPCONFIG(jlss_id_dumpconfig_prixptr, "PRIXPTR defined - value " dmpcfg_dbl(PRIXPTR))
#undef dmpcfg_str
#undef dmpcfg_dbl
#endif /* PRIXPTR */

#ifndef HAVE_UINTPTR_T
DUMPCONFIG(jlss_id_dumpconfig_uintptr, "uintptr_t missing - typedef unsigned long uintptr_t")
typedef unsigned long uintptr_t;
#endif /* HAVE_UINTPTR_T */

#ifdef __cplusplus
}
#endif

#endif /* JLSS_ID_DUMPCONFIG_H */
