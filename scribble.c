/*
@(#)File:           $RCSfile: scribble.c,v $
@(#)Version:        $Revision: 1.1 $
@(#)Last changed:   $Date: 2007/10/27 21:17:36 $
@(#)Purpose:        Scribble X's over a buffer
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 2007
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

/*TABSTOP=4*/

#include "jlsstools.h"
#include <string.h>

#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
extern const char jlss_id_scribble_c[];
const char jlss_id_scribble_c[] = "@(#)$Id: scribble.c,v 1.1 2007/10/27 21:17:36 jleffler Exp $";
#endif /* lint */

/* scribble_buffer - overwrite buffer with string of X's to prevent later reading */ 
/* In a separate file to avoid optimization etc. */
void  scribble_buffer(char *buffer, size_t buflen)
{
    if (buflen != 0)
    {
        memset(buffer, 'X', buflen - 1);
        buffer[buflen-1] = '\0';
    }
}
