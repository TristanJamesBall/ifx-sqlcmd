/*
@(#)File:           $RCSfile: output.h,v $
@(#)Version:        $Revision: 2015.1 $
@(#)Last changed:   $Date: 2015/07/10 16:05:12 $
@(#)Purpose:        Printing routines for SQLCMD
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 1992-93,1997-98,2000,2003,2005,2011,2015
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

#ifndef OUTPUT_H
#define OUTPUT_H

#ifdef MAIN_PROGRAM
#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
extern const char jlss_id_output_h[];
const char jlss_id_output_h[] = "@(#)$Id: output.h,v 2015.1 2015/07/10 16:05:12 jleffler Exp $";
#endif /* lint */
#endif  /* MAIN_PROGRAM */

#include "esqlc.h"

extern int  print_header(const Sqlda *desc);
extern void print_trailer(void);
extern int  print_record(const Sqlda *desc);

#endif  /* OUTPUT_H */
