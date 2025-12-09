/*
@(#)File:           $RCSfile: lduint4.c,v $
@(#)Version:        $Revision: 1.6 $
@(#)Last changed:   $Date: 2007/12/25 03:47:13 $
@(#)Purpose:        C-ISAM style LD_UINT4(3)
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 2000,2005,2007
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

/*TABSTOP=4*/

#include "stldint.h"

#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
const char jlss_id_lduint4_c[] = "@(#)$Id: lduint4.c,v 1.6 2007/12/25 03:47:13 jleffler Exp $";
#endif /* lint */

/*
**  Convert 4-byte character array into unsigned 4-byte integer.
*/
Uint4   ld_uint4(const char *s)
{
    int i;
    Uint4   j = 0;

    for (i = 0; i < 4; i++)
    {
        j = (j << 8) | (*s++ & 0xFF);
    }
    return(j);
}

#ifdef TEST

#include <stdio.h>

#define DIM(x)  (sizeof(x)/sizeof(*(x)))

static const char values[][4] =
{
    "\000\000\000\000",
    "\001\002\003\004",
    "\177\377\377\377",
    "\200\000\000\000",
    "\327\147\167\377",
    "\377\377\377\377",
};

int main(void)
{
    int             i;

    puts("LOAD Unsigned 4-Byte Integer");
    for (i = 0; i < DIM(values); i++)
    {
        printf("0x%02X%02X%02X%02X => 0x%08lX\n",
               (unsigned char)values[i][0], (unsigned char)values[i][1],
               (unsigned char)values[i][2], (unsigned char)values[i][3],
               ld_uint4(values[i]));
    }
    return(0);
}

#endif /* TEST */
