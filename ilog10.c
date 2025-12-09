/*
@(#)File:           $RCSfile: ilog10.c,v $
@(#)Version:        $Revision: 1.6 $
@(#)Last changed:   $Date: 2008/02/20 15:36:02 $
@(#)Purpose:        Integer logarithm to base 10 of integers
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 2006-07
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

/*TABSTOP=4*/

#include "ilog10.h"

#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
extern const char jlss_id_ilog10_c[];
const char jlss_id_ilog10_c[] = "@(#)$Id: ilog10.c,v 1.6 2008/02/20 15:36:02 jleffler Exp $";
#endif /* lint */

size_t ilog10_z(size_t x)
{
    size_t n = 0;
    while ((x /= 10) != 0)
        n++;
    return n;
}

#ifdef TEST

#define FMT_SIZE_T "%lu"

#include "phasedtest.h"

typedef struct Test1
{
    size_t    input;
    size_t    expected;
} Test1;

static const Test1 p1_test_cases[] =
{
    {          0U,  0   },
    {          1U,  0   },
    {          2U,  0   },
    {          9U,  0   },
    {         10U,  1   },
    {         11U,  1   },
    {         99U,  1   },
    {        100U,  2   },
    {        101U,  2   },
    {        999U,  2   },
    {       1000U,  3   },
    {       1010U,  3   },
    {       9999U,  3   },
    {      10000U,  4   },
    {      99999U,  4   },
    {     100000U,  5   },
    {     999999U,  5   },
    {    1000000U,  6   },
    {    9999999U,  6   },
    {   10000000U,  7   },
    {   99999999U,  7   },
    {  100000000U,  8   },
    {  999999999U,  8   },
    { 1000000000U,  9   },
    { 3999999999U,  9   },
};

static void p1_tester(const void *data)
{
    const Test1 *test = (const Test1 *)data;
    size_t rc = ilog10_z(test->input);
    if (rc != test->expected)
        pt_fail("<<" FMT_SIZE_T ">> - unexpected result " FMT_SIZE_T " (instead of " FMT_SIZE_T ")\n",
                test->input, rc, test->expected);
    else
        pt_pass("<<" FMT_SIZE_T ">>\n", test->input);
}

static const pt_auto_phase phases[] =
{
    { p1_tester, PT_ARRAYINFO(p1_test_cases), 0, "Test ilog10_z (size_t - 32-bit)" },
};

int main(int argc, char **argv)
{
    return pt_auto_harness(argc, argv, phases, DIM(phases));
}

#endif /* TEST */
