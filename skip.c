/*
@(#)File:           $RCSfile: skip.c,v $
@(#)Version:        $Revision: 2015.1 $
@(#)Last changed:   $Date: 2015/07/10 16:05:12 $
@(#)Purpose:        Simple string parsing routines
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 1998,2003,2005,2008,2015
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

/*TABSTOP=4*/

#include <ctype.h>
#include "internal.h"

#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
extern const char jlss_id_skip_c[];
const char jlss_id_skip_c[] = "@(#)$Id: skip.c,v 2015.1 2015/07/10 16:05:12 jleffler Exp $";
#endif /* lint */

/* Skip over alphanumeric word */
char   *skipalphanum(char *s)
{
    while (isalnum((unsigned char)*s) || *s == '_')
        s++;
    return(s);
}

/* Skip over (non-blank) word */
char   *skipword(char *s)
{
    while (*s != '\0' && !isspace((unsigned char)*s))
        s++;
    return(s);
}

/* Skip over blanks */
char   *skipblanks(char *s)
{
    while (isspace((unsigned char)*s))
        s++;
    return(s);
}
