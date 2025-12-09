/*
@(#)File:           $RCSfile: dumpintvl.ec,v $
@(#)Version:        $Revision: 2008.3 $
@(#)Last changed:   $Date: 2008/04/06 21:25:40 $
@(#)Purpose:        Dump INTERVAL structure
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 1995,1997-98,2001,2003-04,2008
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

/*TABSTOP=4*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include "dumpesql.h"
#include "datetime.h"   /* TU_* */
#include "esqltype.h"   /* PRId_ixInt2 */
#include "esqlutil.h"   /* sqltypename() */
#include "decsci.h"     /* dec_fix() */
#include "dumpconfig.h"

#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
const char jlss_id_dumpintvl_ec[] = "@(#)$Id: dumpintvl.ec,v 2008.3 2008/04/06 21:25:40 jleffler Exp $";
#endif /* lint */

void dump_interval(FILE *fp, const char *tag, const intrvl_t *ip)
{
    char buffer[SQLTYPENAME_BUFSIZ];

    if (tag == (char *)0)
        tag = "";
    dump_print(fp, "INTERVAL: %s -- address 0x%08" PRIXPTR "\n", tag, (uintptr_t)ip);
    dump_print(fp, "Qualifier: %4" PRId_ixInt2 " -- type %s\n",
            ip->in_qual, sqltypename(SQLINTERVAL, ip->in_qual, buffer, sizeof(buffer)));
    dec_fix(&ip->in_dec, TU_FRACDIGITS(ip->in_qual), 1, buffer, sizeof(buffer) - 1);
    dump_decimal(fp, buffer, &ip->in_dec);
}

#ifdef TEST

struct Tests
{
    const char *src;
    int         in_qual;
};

static const struct Tests test[] =
{
    { "1900-01",                TU_IENCODE(9, TU_YEAR, TU_MONTH)  },
    { "19700101",               TU_IENCODE(9, TU_YEAR, TU_YEAR)   },
    { "2341212 12:34:56.78901", TU_IENCODE(9, TU_DAY,  TU_F5)     },
    { "12:34:56",               TU_IENCODE(9, TU_HOUR, TU_SECOND) },
};

int main(int argc, char **argv)
{
    size_t       n = DIM(test);
    size_t       i;
    ifx_errnum_t rc;
    ifx_intrvl_t iv;

    dump_setindent(2);

    for (i = 0; i < n; i++)
    {
        iv.in_qual = test[i].in_qual;
        if ((rc = incvasc(test[i].src, &iv)) != 0)
            fprintf(stderr, "conversion of %s (qual %d) failed: rc = %d\n",
                    test[i].src, test[i].in_qual, rc);
        else
            dump_interval(stdout, test[i].src, &iv);
        puts("");
    }
    return(0);
}

#endif /* TEST */
