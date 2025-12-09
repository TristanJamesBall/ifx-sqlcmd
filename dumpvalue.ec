/*
@(#)File:           $RCSfile: dumpvalue.ec,v $
@(#)Version:        $Revision: 2008.5 $
@(#)Last changed:   $Date: 2008/04/07 06:14:44 $
@(#)Purpose:        Dump the contents of a value structure
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 1995,1997-98,2001,2003-04,2008
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

/*TABSTOP=4*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include "dumpesql.h"
#include "esqlutil.h"
#include "esqltype.h"
#include "dumpconfig.h"

#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
const char jlss_id_dumpvalue_ec[] = "@(#)$Id: dumpvalue.ec,v 2008.5 2008/04/07 06:14:44 jleffler Exp $";
#endif /* lint */

void dump_value(FILE *fp, const char *tag, const value_t *vp)
{
    char buffer[SQLTYPENAME_BUFSIZ];

    if (tag == (char *)0)
        tag = "";
    dump_print(fp, "VALUE: %s -- address 0x%08" PRIXPTR "\n", tag, (uintptr_t)vp);
    dump_print(fp, "Type: %3d, Indicator %2d, Precision: %4d -- type %s\n",
            vp->v_type, vp->v_ind, vp->v_prec,
            sqltypename(vp->v_type, vp->v_prec, buffer, sizeof(buffer)));
    if (vp->v_ind == -1)
        dump_print(fp, "Value is NULL\n");
    else
    {
        switch (vp->v_type)
        {

        case SQLCHAR:
#ifdef SQLNCHAR
        case SQLNCHAR:
#endif /* SQLNCHAR */
        case CCHARTYPE:
            if (vp->v_len > 20)
                dump_print(fp, "CHAR: '%20.20s...'\n", vp->v_charp);
            else
                dump_print(fp, "CHAR: '%s'\n", vp->v_charp);
            break;

        case CFIXCHARTYPE:
            if (vp->v_len > 20)
                dump_print(fp, "FIXC: '%20.20s...'\n", vp->v_charp);
            else
                dump_print(fp, "FIXC: '%s'\n", vp->v_charp);
            break;

        case CSTRINGTYPE:
            if (vp->v_len > 20)
                dump_print(fp, "STRING: '%20.20s...'\n", vp->v_charp);
            else
                dump_print(fp, "STRING: '%s'\n", vp->v_charp);
            break;

        case SQLSMINT:
        case CSHORTTYPE:
            dump_print(fp, "C short: %" PRId_ixInt2 "\n", vp->v_int);
            break;

        case CINTTYPE:
            dump_print(fp, "C int: %" PRId_ixMint "\n", vp->v_int);
            break;

        case SQLINT:
        case SQLSERIAL:
        case CLONGTYPE:
            dump_print(fp, "C long: %" PRId_ixInt4 "\n", vp->v_long);
            break;

        case SQLFLOAT:
        case CDOUBLETYPE:
            dump_print(fp, "C double: %21.14e\n", vp->v_double);
            break;

        case SQLSMFLOAT:
        case CFLOATTYPE:
            dump_print(fp, "C float: %13.6e\n", vp->v_float);
            break;

        case SQLDECIMAL:
        case CDECIMALTYPE:
            dump_decimal(fp, "", &vp->v_decimal);
            break;

        case SQLMONEY:
        case CMONEYTYPE:
            dump_decimal(fp, "", &vp->v_decimal);
            break;

        case SQLDATE:
        case CDATETYPE:
            {
            /* Nasty combination of C++ and non-const ESQL/C library functions */
            char fdate[] = "dd mmm yyyy";   /*=C++=*/
            char sdate[sizeof("dd mmm yyyy")];
            rfmtdate(vp->v_long, fdate, sdate);
            dump_print(fp, "DATE: %" PRId_ixInt4 " = '%s'\n", vp->v_long, sdate);
            }
            break;

        case SQLNULL:
            dump_print(fp, "<<NULL>>\n");
            break;

        case SQLDTIME:
        case CDTIMETYPE:
            {
            dtime_t d;
            d.dt_qual = vp->v_prec;
            d.dt_dec = vp->v_decimal;
            dump_datetime(fp, "", &d);
            }
            break;

        case SQLBYTES:
        case SQLTEXT:
        case CFILETYPE:
        case CLOCATORTYPE:
            dump_blob(fp, "", vp->v_blocator);
            break;

        case SQLVCHAR:
#ifdef SQLNVCHAR
        case SQLNVCHAR:
#endif /* SQLNVCHAR */
        case CVCHARTYPE:
            if (vp->v_len > 20)
                dump_print(fp, "VARCHAR: '%20.20s...'\n", vp->v_charp);
            else
                dump_print(fp, "VARCHAR: '%s'\n", vp->v_charp);
            break;

        case SQLINTERVAL:
        case CINVTYPE:
            {
            intrvl_t i;
            i.in_qual = vp->v_prec;
            i.in_dec = vp->v_decimal;
            dump_interval(fp, "", &i);
            }
            break;

        default:
            dump_print(fp, "*** UNKNOWN TYPE %d ***\n", vp->v_type);
            ESQLC_VERSION_CHECKER();
            break;
        }
    }
}
