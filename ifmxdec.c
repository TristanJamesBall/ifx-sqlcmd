/*
@(#)File:           $RCSfile: ifmxdec.c,v $
@(#)Version:        $Revision: 1.11 $
@(#)Last changed:   $Date: 2016/01/18 01:37:19 $
@(#)Purpose:        Test and Set DECIMAL values to Zero or SQL NULL
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 2004-05,2008,2016
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

/*TABSTOP=4*/

#include <stddef.h>
#include "decimal.h"
#include "ifmxdec.h"
#include "decsci.h"     /* Initializers! */

const ifx_dec_t dec_zero = DECZERO_INITIALIZER;
const ifx_dec_t dec_null = DECNULL_INITIALIZER;
/* An exponent of 0 implies a fraction 0.01..0.99; an exponent of 1 multiplies by 100 */
const ifx_dec_t dec_one      = { 1, DECPOSPOS, 1, { 1 } };
const ifx_dec_t dec_two      = { 1, DECPOSPOS, 1, { 2 } };
const ifx_dec_t dec_ten      = { 1, DECPOSPOS, 1, { 10 } };
const ifx_dec_t dec_sixty    = { 1, DECPOSPOS, 1, { 60 } };
const ifx_dec_t dec_hundred  = { 2, DECPOSPOS, 1, { 1 } };
/* Per 'bc -l' at scale=36 --  e(1)   = 2.718281828459045235360287471352662497 */
const ifx_dec_t dec_e        = { 1, DECPOSPOS, 16, { 2, 71, 82, 81, 82, 84, 59,  4, 52, 35, 36,  2, 87, 47, 13, 53 } };
/* Per 'bc -l' at scale=36 --  4*a(1) = 3.141592653589793238462643383279502884 */
const ifx_dec_t dec_pi       = { 1, DECPOSPOS, 16, { 3, 14, 15, 92, 65, 35, 89, 79, 32, 38, 46, 26, 43, 38, 32, 80 } };

#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
extern const char jlss_id_ifmxdec_c[];
const char jlss_id_ifmxdec_c[] = "@(#)$Id: ifmxdec.c,v 1.11 2016/01/18 01:37:19 jleffler Exp $";
#endif /* lint */

/* Set DECIMAL to Informix's SQL NULL */
void (dec_setnull)(ifx_dec_t *dp)
{
    *dp = dec_null;
}

/* Test whether DECIMAL is NULL */
int (dec_eq_null)(const ifx_dec_t *d)
{
    return(d->dec_pos == DECPOSNULL);
}

/* Set DECIMAL to zero */
void (dec_setzero)(ifx_dec_t *dp)
{
    *dp = dec_zero;
}

/* Test whether DECIMAL is zero */
int (dec_eq_zero)(const ifx_dec_t *d)
{
    return(d->dec_ndgts == 0 && d->dec_exp == DECEXPZERO);
}
