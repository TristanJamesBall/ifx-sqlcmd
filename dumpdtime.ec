/*
@(#)File:           $RCSfile: dumpdtime.ec,v $
@(#)Version:        $Revision: 2008.3 $
@(#)Last changed:   $Date: 2008/04/06 21:25:40 $
@(#)Purpose:        Dump DATETIME value
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
#include "decsci.h"     /* dec_fix() */
#include "esqlutil.h"   /* sqltypename() */
#include "esqltype.h"   /* PRId_ixInt2 */
#include "dumpconfig.h"

#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
const char jlss_id_dumpdtime_ec[] = "@(#)$Id: dumpdtime.ec,v 2008.3 2008/04/06 21:25:40 jleffler Exp $";
#endif /* lint */

void dump_datetime(FILE *fp, const char *tag, const dtime_t *dp)
{
    char buffer[SQLTYPENAME_BUFSIZ];

    if (tag == (char *)0)
        tag = "";
    dump_print(fp, "DATETIME: %s -- address 0x%08" PRIXPTR "\n", tag, (uintptr_t)dp);
    dump_print(fp, "Qualifier: %4" PRId_ixInt2 " -- type %s\n",
            dp->dt_qual, sqltypename(SQLDTIME, dp->dt_qual, buffer, sizeof(buffer)));
    dec_fix(&dp->dt_dec, TU_FRACDIGITS(dp->dt_qual), 1, buffer, sizeof(buffer) - 1);
    dump_decimal(fp, buffer, &dp->dt_dec);
}

#ifdef TEST

struct Tests
{
    const char *src;
    int         dt_qual;
};

static const struct Tests test[] =
{
    { "1900-01-01 00:00:00",       TU_DTENCODE(TU_YEAR, TU_SECOND)  },
    { "1970-01-01 12:34:56.78901", TU_DTENCODE(TU_YEAR, TU_F5)      },
    { "1970-01-01",                TU_DTENCODE(TU_YEAR, TU_DAY)     },
    { "01-01 12:34",               TU_DTENCODE(TU_MONTH, TU_MINUTE) },
    { "12:34:56.78901",            TU_DTENCODE(TU_HOUR, TU_F5)      },
    { "12:34:56",                  TU_DTENCODE(TU_HOUR, TU_SECOND)  },
};

int main(int argc, char **argv)
{
    size_t n = DIM(test);
    size_t i;
    mint rc;
    dtime_t dt;
    const char *str;

    for (i = 0; i < n; i++)
    {
        dt.dt_qual = test[i].dt_qual;
        str = test[i].src;
        if ((rc = dtcvasc(CONST_CAST(char *, test[i].src), &dt)) != 0)
            fprintf(stderr, "conversion of %s (qual %d) failed: rc = %d\n", str, dt.dt_qual, rc);
        else
            dump_datetime(stdout, str, &dt);
        puts("");
    }
    return(0);
}

#endif /* TEST */
