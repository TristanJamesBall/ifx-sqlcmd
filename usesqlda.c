/*
@(#)File:           $RCSfile: usesqlda.c,v $
@(#)Version:        $Revision: 1.3 $
@(#)Last changed:   $Date: 2015/07/10 15:40:39 $
@(#)Purpose:        Manipulate SQLDA Structures
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 2010,2012,2015
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

/*TABSTOP=4*/

#include "posixver.h"
#include "usesqlda.h"
#include <assert.h>
#include <stdlib.h>

#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
extern const char jlss_id_usesqlda_c[];
const char jlss_id_usesqlda_c[] = "@(#)$Id: usesqlda.c,v 1.3 2015/07/10 15:40:39 jleffler Exp $";
#endif /* lint */

/* -- SQLDA Management Routines -- */

Sqlda *alloc_sqlda(size_t nvar)
{
    Sqlda *idesc = 0;
    void *space;

    space = calloc(1, sizeof(Sqlda) + nvar * sizeof(Sqlva));
    if (space != 0)
    {
        idesc = (Sqlda *)space;
        space = (void *)((char *)space + sizeof(Sqlda));
        idesc->sqld = nvar;
        idesc->sqlvar = (Sqlva *)space;
    }
    return idesc;
}

void dealloc_sqlda(Sqlda *idesc)
{
    assert((void *)idesc->sqlvar == (void *)((char *)idesc + sizeof(Sqlda)));
    free(idesc);
}
