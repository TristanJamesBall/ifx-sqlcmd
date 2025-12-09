/*
@(#)File:           $RCSfile: memory.c,v $
@(#)Version:        $Revision: 2015.1 $
@(#)Last changed:   $Date: 2015/07/10 16:05:12 $
@(#)Purpose:        Handle memory allocation
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 1992-93,1997-2000,2003,2005,2015
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "sqlerr.h"
#include "emalloc.h"
#include "memory.h"

#define NIL(x)      ((x)0)

#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
extern const char jlss_id_memory_c[];
const char jlss_id_memory_c[] = "@(#)$Id: memory.c,v 2015.1 2015/07/10 16:05:12 jleffler Exp $";
#endif /* lint */

#ifdef NDEBUG
#define mem_chk(m)  ((void)0)
#else
static void mem_chk(Memory *m)
{
    assert(m != 0);
    if (m->mem_base == 0)
    {
        assert(m->mem_size == 0);
        assert(m->mem_endp == 0);
        assert(m->mem_next == 0);
        assert(m->mem_read == 0);
    }
    else
    {
        assert(m->mem_size != 0);
        assert(m->mem_endp != 0);
        assert(m->mem_next != 0);
        assert(m->mem_read != 0);
        assert(m->mem_endp == m->mem_base + m->mem_size);
        assert(m->mem_next >= m->mem_base);
        assert(m->mem_next <= m->mem_endp);
        assert(m->mem_read >= m->mem_base);
        assert(m->mem_read <= m->mem_next);
    }
}
#endif /* NDEBUG */

/* Allocate initial resources. */
void    mem_new(Memory *m)
{
    m->mem_size = MEM_BLOCK_SIZE1;
    m->mem_base = (Uchar *)MALLOC(m->mem_size);
    m->mem_next = m->mem_base;
    m->mem_read = m->mem_base;
    jb_register(m->mem_base);
    m->mem_endp = m->mem_base + m->mem_size;
    mem_chk(m);
}

/* Reset pointers so structure is empty */
void    mem_rst(Memory *m)
{
    mem_chk(m);
    if (m->mem_base == NIL(Uchar *))
        mem_new(m);
    m->mem_next = m->mem_base;
    m->mem_read = m->mem_base;
}

/* Mark end of memory (normally with '\0'), but do not include mark in length */
void    mem_end(Memory *m, Uchar c)
{
    mem_add(m, c);
    m->mem_next--;
    mem_chk(m);
}

/* Append single character */
void    mem_add(Memory *m, Uchar c)
{
    mem_chk(m);
    if (m->mem_next >= m->mem_endp)
    {
        size_t n = m->mem_next - m->mem_base;
        size_t r = m->mem_read - m->mem_base;
        assert(m->mem_next == m->mem_endp);
        assert(m->mem_next >= m->mem_base);
        assert((size_t)(m->mem_next - m->mem_base) == m->mem_size);
        /* Alternative, given mem_size > 0, is m->mem_size *= 2 */
        m->mem_size += MEM_BLOCK_SIZE2;
        jb_unregister(m->mem_base);
        m->mem_base = (Uchar *)REALLOC(m->mem_base, m->mem_size);
        jb_register(m->mem_base);
        m->mem_endp = m->mem_base + m->mem_size;
        m->mem_next = m->mem_base + n;
        m->mem_read = m->mem_base + r;
        mem_chk(m);
    }
    *m->mem_next++ = c;
}

/* Zero all pointers, etc */
void    mem_zap(Memory *m)
{
    m->mem_base = m->mem_endp = m->mem_next = m->mem_read = NIL(Uchar *);
    m->mem_size = 0;
}

/* Free all resources */
void    mem_del(Memory *m)
{
    mem_chk(m);
    if (m->mem_base != NIL(Uchar *))
    {
        jb_unregister(m->mem_base);
        FREE(m->mem_base);
    }
    mem_zap(m);
    mem_chk(m);
}

/* Length of data */
size_t mem_len(Memory *m)
{
    return(m->mem_next - m->mem_base);
}

/* Start of data as Uchar pointer */
Uchar *mem_data(Memory *m)
{
    return(m->mem_base);
}

/* Start of data as char pointer */
char *mem_cdata(Memory *m)
{
    return((char *)m->mem_base);
}

/* Append string */
void mem_addstr(Memory *m, const Uchar *s, size_t len)
{
    mem_chk(m);
    if (m->mem_next + len >= m->mem_endp)
    {
        size_t n = m->mem_next - m->mem_base;
        size_t r = m->mem_read - m->mem_base;
        m->mem_size += ((len + MEM_BLOCK_SIZE2 - 1) / MEM_BLOCK_SIZE2) * MEM_BLOCK_SIZE2;
        jb_unregister(m->mem_base);
        m->mem_base = (Uchar *)REALLOC(m->mem_base, m->mem_size);
        jb_register(m->mem_base);
        m->mem_endp = m->mem_base + m->mem_size;
        m->mem_next = m->mem_base + n;
        m->mem_read = m->mem_base + r;
        mem_chk(m);
    }
    memcpy(m->mem_next, s, len);
    m->mem_next += len;
    mem_chk(m);
}

/* Reset pointers for reading */
void mem_scan(Memory *m)
{
    m->mem_read = m->mem_base;
}

/* Peek at next character (or EOF) */
int mem_peek(Memory *m)
{
   if (m->mem_read >= m->mem_next)
      return EOF;
   else
      return *m->mem_read;
}

/* Read next character (or EOF) */
int mem_char(Memory *m)
{
    if (m->mem_read >= m->mem_next)
        return EOF;
    else
        return *m->mem_read++;
}

/* Return and discard last character */
int mem_pop(Memory *m)
{
    assert(m->mem_next > m->mem_base);
    return *--m->mem_next;
}

/* Return last character */
Uchar mem_last(Memory *m)
{
    mem_chk(m);
    assert(m->mem_next > m->mem_base);
    return(*(m->mem_next - 1));
}
