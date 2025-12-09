/*
@(#)File:           $RCSfile: dumpdec.ec,v $
@(#)Version:        $Revision: 2008.3 $
@(#)Last changed:   $Date: 2008/04/06 21:25:40 $
@(#)Purpose:        Dump decimal structure
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 1995,1997-99,2002-03,2005,2007-08
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

/*TABSTOP=4*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include "dumpesql.h"
#include "dumpconfig.h"

#ifndef DECEXPZERO
#define DECEXPZERO -64
#endif /* DECEXPZERO */

#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
const char jlss_id_dumpdec_ec[] = "@(#)$Id: dumpdec.ec,v 2008.3 2008/04/06 21:25:40 jleffler Exp $";
#endif /* lint */

static const char *dec_sign(int dec_pos)
{
    if (dec_pos == 1)
        return("+");
    else if (dec_pos == 0)
        return("-");
    else if (dec_pos == -1)
        return("NULL");
    else
        return("<<value out of range -1..+1>>");
}

void dump_decimal(FILE *fp, const char *tag, const dec_t *dp)
{
    int i;
    static const char dec_corrupt[] = "decimal structure is corrupt!";

    if (tag == (char *)0)
        tag = "";
    dump_print(fp, "DECIMAL: %s -- address 0x%08" PRIXPTR "\n", tag, (uintptr_t)dp);
    dump_print(fp, "E: %+4d, S = %d (%s), N = %2d, M =",
            dp->dec_exp, dp->dec_pos, dec_sign(dp->dec_pos), dp->dec_ndgts);
    if (dp->dec_pos == -1)
        fprintf(fp, " value is NULL\n");
    else if (dp->dec_exp == DECEXPZERO)
        fprintf(fp, " value is ZERO%s\n", (dp->dec_ndgts == 0) ? "" : " but N != 0 as it should!");
    else
    {
        int n = dp->dec_ndgts;
        if (dp->dec_ndgts < 0)
        {
            n = DECSIZE;
            dump_print(fp, "Number of digits (N = %2d) is negative; %s\n", dp->dec_ndgts, dec_corrupt);
        }
        else if (dp->dec_ndgts == 0)
        {
            n = DECSIZE;
            dump_print(fp, "Number of digits (N = %2d) is zero but exponent %d is not set to %d; %s\n", dp->dec_ndgts, dp->dec_exp, DECEXPZERO, dec_corrupt);
        }
        else if (dp->dec_ndgts > DECSIZE)
        {
            n = DECSIZE;
            dump_print(fp, "Number of digits (N = %2d) is bigger than %d; %s\n", dp->dec_ndgts, DECSIZE, dec_corrupt);
        }
        for (i = 0; i < DECSIZE; i++)
        {
            if (i == n)
                fprintf(fp, " [ unused: ");
            fprintf(fp, " %02d", dp->dec_dgts[i]);
            /* First condition could generate compiler warning when plain char is unsigned */
            if (dp->dec_dgts[i] < 0 || dp->dec_dgts[i] > 99)
                fprintf(fp, " <bogus>");
        }
        fprintf(fp, "%s\n", (n < DECSIZE) ? " ]" : "");
    }
}

#ifdef TEST

#include <string.h>
#include <stdlib.h>

static const char *decimals[] =
{
    "-1.0",
    "0.0",
    "123.456789",
    "1.9035454309834442222e-120",
    "-2.23423423424243409854e-120",
    "+8.2342809547395234794355555539722e+120",
    "-8.2342809547395234794355555539722e+120",
    "+8.2342809547395234794355555539722e+121",
    "-8.2342809547395234794355555539722e+121"
};

int main(void)
{
    int     i;
    int     j;
    dec_t   num;

    for (j = 0; j < DIM(decimals); j++)
    {
        if ((i = deccvasc(CONST_CAST(char *, decimals[j]), strlen(decimals[j]), &num)) != 0)
        {
            fprintf(stderr, "error %d from deccvasc processing %s\n",
                    i, decimals[j]);
            exit(0);
        }
        dump_decimal(stdout, decimals[j], &num);
    }

    return(0);
}

#endif  /* TEST */
