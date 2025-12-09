/*
@(#)File:           $RCSfile: fcopy.c,v $
@(#)Version:        $Revision: 1.11 $
@(#)Last changed:   $Date: 2008/02/11 07:28:06 $
@(#)Purpose:        Copy the rest of file1 to file2
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 1991,1997,2000,2003,2005,2008
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

/*TABSTOP=4*/

#include "jlss.h"
#include "stderr.h"

#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
const char jlss_id_fcopy_c[] = "@(#)$Id: fcopy.c,v 1.11 2008/02/11 07:28:06 jleffler Exp $";
#endif /* lint */

void fcopy(FILE *f1, FILE *f2)
{
    char            buffer[BUFSIZ];
    size_t          n;

    while ((n = fread(buffer, sizeof(char), sizeof(buffer), f1)) > 0)
    {
        if (fwrite(buffer, sizeof(char), n, f2) != n)
            err_syserr("write failed\n");
    }
}

#ifdef TEST

int main(int argc, char **argv)
{
    FILE *fp1;
    FILE *fp2;

    err_setarg0(argv[0]);
    if (argc != 3)
        err_usage("from to");
    if ((fp1 = fopen(argv[1], "rb")) == 0)
        err_syserr("cannot open file %s for reading\n", argv[1]);
    if ((fp2 = fopen(argv[2], "wb")) == 0)
        err_syserr("cannot open file %s for writing\n", argv[2]);
    fcopy(fp1, fp2);
    return(0);
}

#endif /* TEST */
