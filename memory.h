/*
@(#)File:           $RCSfile: memory.h,v $
@(#)Version:        $Revision: 2015.1 $
@(#)Last changed:   $Date: 2015/07/10 16:05:12 $
@(#)Purpose:        Definitions for memory blocks
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 1992,1997-2000,2003,2005,2015
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

#ifndef MEMORY_H
#define MEMORY_H

#ifdef MAIN_PROGRAM
#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
extern const char jlss_id_memory_h[];
const char jlss_id_memory_h[] = "@(#)$Id: memory.h,v 2015.1 2015/07/10 16:05:12 jleffler Exp $";
#endif /* lint */
#endif  /* MAIN_PROGRAM */

#include <stddef.h>

/* -- Constant Definitions */

#define MEM_BLOCK_SIZE1     512
#define MEM_BLOCK_SIZE2     2048

/* -- Type Definitions */

typedef unsigned char Uchar;

struct Memory
{
    size_t   mem_size;  /* Number of bytes allocated */
    Uchar   *mem_base;  /* Start of buffer */
    Uchar   *mem_endp;  /* Pointer one beyond last allocated byte */
    Uchar   *mem_next;  /* Next location to write to */
    Uchar   *mem_read;  /* Next location to read from */
};

typedef struct Memory Memory;

/* -- Declarations */

extern Uchar *mem_data(Memory *m);  /* Start of data as Uchar pointer */
extern Uchar  mem_last(Memory *m);  /* Last character */
extern char  *mem_cdata(Memory *m); /* Start of data as char pointer */
extern int    mem_char(Memory *m);      /* Read next character (or EOF) */
extern int    mem_peek(Memory *m);      /* Peek at next character (or EOF) */
extern int    mem_pop(Memory *m);       /* Remove and return last char */
extern size_t mem_len(Memory *m);   /* Number of characters */
extern void   mem_add(Memory *m, Uchar c);  /* Append single character */
extern void   mem_addstr(Memory *m, const Uchar *s, size_t l);  /* Append multiple characters */
extern void   mem_del(Memory *m);       /* Release all resources */
extern void   mem_end(Memory *m, Uchar c);  /* Append character to end, but don't count it */
extern void   mem_new(Memory *m);       /* Initialize */
extern void   mem_rst(Memory *m);       /* Reset pointers to start */
extern void   mem_scan(Memory *m);  /* Reset read pointer */
extern void   mem_zap(Memory *m);       /* Set all pointers zero */

#endif  /* MEMORY_H */
