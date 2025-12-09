/*
@(#)File:           $RCSfile: kludge.h,v $
@(#)Version:        $Revision: 1.16 $
@(#)Last changed:   $Date: 2016/01/17 15:48:53 $
@(#)Purpose:        Provide support for KLUDGE macro
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 1995,1997-2000,2003,2005,2008,2015-16
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

/*TABSTOP=4*/

#ifndef KLUDGE_H
#define KLUDGE_H

#ifdef MAIN_PROGRAM
#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
extern const char jlss_id_kludge_h[];
const char jlss_id_kludge_h[] = "@(#)$Id: kludge.h,v 1.16 2016/01/17 15:48:53 jleffler Exp $";
#endif /* lint */
#endif /* MAIN_PROGRAM */

/*
 * The KLUDGE macro is enabled by default.
 * It can be disabled by specifying -DKLUDGE_DISABLE
 */

#ifdef KLUDGE_DISABLE

#define KLUDGE(x)   ((void)0)
#define FEATURE(x)  ((void)0)

#else

/*
** The Solaris C compiler without either -O or -g removes unreferenced
** strings, which defeats the purpose of the KLUDGE macro.
** If your compiler is lenient enough, you can use -DKLUDGE_NO_FORCE,
** but most modern compilers make KLUDGE_FORCE preferable.
**
** The GNU C Compiler will complain about unused variables if told to
** do so.  Setting KLUDGE_FORCE ensures that it doesn't complain about
** any kludges.
*/

#undef KLUDGE_FORCE
#ifndef KLUDGE_NO_FORCE
#define KLUDGE_FORCE
#endif /* KLUDGE_NO_FORCE */

#ifdef KLUDGE_FORCE
#define KLUDGE_USE(x)   kludge_use(x)
#else
#define KLUDGE_USE(x)   ((void)0)
#endif /* KLUDGE_FORCE */

/*
** Example use: KLUDGE("Fix macro to accept arguments with commas");
** Note that the argument is now a string.  An alternative (and
** previously used) design is to have the argument as a non-string:
**              KLUDGE(Fix macro to accept arguments with commas);
** This allows it to work with traditional compilers but runs foul of
** the absence of string concatenation, and you have to avoid commas
** in the reason string, etc.
*/

#define KLUDGE_DEC  static const char kludge[]
#define FEATURE_DEC static const char feature[]
#define ONCE_ONLY   static int once = 0; if (once++ == 0)

#define KLUDGE(x)   { ONCE_ONLY KLUDGE_USE("@(#)KLUDGE: " x); }
#define FEATURE(x)  { ONCE_ONLY KLUDGE_USE("@(#)Feature: " x); }

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

extern void kludge_use(const char *str);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* KLUDGE_DISABLE */

#endif /* KLUDGE_H */
