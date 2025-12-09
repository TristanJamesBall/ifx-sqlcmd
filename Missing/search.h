/*
@(#)File:            $RCSfile: search.h,v $
@(#)Version:         $Revision: 1.2 $
@(#)Last changed:    $Date: 2006/03/07 09:14:12 $
@(#)Purpose:         Incomplete surrogate for System V header <search.h>
@(#)Author:          J Leffler
@(#)Copyright:      (C) JLSS 1998,2006
@(#)Product:         SQLCMD Version 90.02 (2016-07-28)
*/

/*TABSTOP=4*/

#ifndef SEARCH_H
#define SEARCH_H

#ifdef MAIN_PROGRAM
#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
const char jlss_id_search_h[] = "@(#)$Id: search.h,v 1.2 2006/03/07 09:14:12 jleffler Exp $";
#endif /* lint */
#endif	/* MAIN_PROGRAM */

extern void *lfind(const void *, const void *, size_t *, size_t,
	    int (*)(const void *, const void *));

/*
** The complete System V <search.h> header defines several other functions.
** The following functions are missing from this surrogate header.
** Binary Search:
** -- bsearch() (use <stdlib.h>)
** Linear Search with update:
** -- lsearch()
** Tree Search:
** -- tsearch()
** -- tfind()
** -- tdelete()
** -- twalk()
** Hash Table Search:
** -- hcreate()
** -- hdestroy()
** -- hsearch()
*/

#endif	/* SEARCH_H */
