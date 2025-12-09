/*
@(#)File:            $RCSfile: varchar.h,v $
@(#)Version:         $Revision: 1.1 $
@(#)Last changed:    $Date: 1998/05/15 20:40:26 $
@(#)Purpose:         Surrogate for Informix header varchar.h
@(#)Author:          J Leffler
@(#)Copyright:       (C) JLSS 1998
@(#)Product:         SQLCMD Version 90.02 (2016-07-28)
*/

/*TABSTOP=4*/

#ifndef VARCHAR_H
#define VARCHAR_H

#ifdef MAIN_PROGRAM
#ifndef lint
static const char varchar_h[] = "@(#)$Id: varchar.h,v 1.1 1998/05/15 20:40:26 jleffler Exp $";
#endif	/* lint */
#endif	/* MAIN_PROGRAM */

#define MAXVCLEN		(255)
#define VCLENGTH(len)	(VCMAX(len) + 1)
#define VCMIN(size)		(((size) >> 8) & 0xFF)
#define VCMAX(size)		((size) & 0xFF)
#define VCSIZ(max, min)	((((min) & 0xFF) << 8) | ((max) & 0xFF))

#endif	/* VARCHAR_H */
