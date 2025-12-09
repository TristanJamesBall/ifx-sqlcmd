/*
@(#)File:           $RCSfile: nstrcpy.c,v $
@(#)Version:        $Revision: 2.7 $
@(#)Last changed:   $Date: 2016/07/08 00:28:04 $
@(#)Purpose:        Variant on strncpy() - guarantees null-terminated string
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 1998,2002-03,2005,2008,2011,2016
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

/*TABSTOP=4*/

#include "jlss.h"
#include <assert.h>
#include <string.h>

#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
extern const char jlss_id_nstrcpy_c[];
const char jlss_id_nstrcpy_c[] = "@(#)$Id: nstrcpy.c,v 2.7 2016/07/08 00:28:04 jleffler Exp $";
#endif /* lint */

#ifdef TIMING_TEST
#include "timer.h"
#define NSTRCPY_USES_STRNCAT
#define NSTRCPY_USES_STRNCPY
#define NSTRCPY_USES_LOOP
extern void NSTRCPY_STRNCAT(char *dst, size_t dstlen, const char *src);
extern void NSTRCPY_STRNCPY(char *dst, size_t dstlen, const char *src);
extern void NSTRCPY_LOOP(char *dst, size_t dstlen, const char *src);
#else
/* Will fail to compile if more than one of NSTRCPY_USES_* is defined */
#if !defined(NSTRCPY_USES_STRNCAT) && !defined(NSTRCPY_USES_STRNCPY) && !defined(NSTRCPY_USES_LOOP)
#define NSTRCPY_USES_STRNCAT
#endif /* !defined(NSTRCPY_USES_STRNCAT) && !defined(NSTRCPY_USES_STRNCPY) && !defined(NSTRCPY_USES_LOOP) */
#if defined(NSTRCPY_USES_STRNCAT)
#define NSTRCPY_STRNCAT nstrcpy
#endif
#if defined(NSTRCPY_USES_STRNCPY)
#define NSTRCPY_STRNCPY nstrcpy
#endif
#if defined(NSTRCPY_USES_LOOP)
#define NSTRCPY_LOOP    nstrcpy
#endif
#endif /* TIMING_TEST */

extern const char jlss_id_nstrcpy_variant[];
#if defined(TIMING_TEST)
const char jlss_id_nstrcpy_variant[] = "@(#)NSTRCPY - timing test only";
#elif defined(NSTRCPY_USES_STRNCAT)
const char jlss_id_nstrcpy_variant[] = "@(#)NSTRCPY - using strncat()";
#elif defined(NSTRCPY_USES_STRNCPY)
const char jlss_id_nstrcpy_variant[] = "@(#)NSTRCPY - using strncpy()";
#else
const char jlss_id_nstrcpy_variant[] = "@(#)NSTRCPY - using local loop";
#endif /* TIMING_TEST etc */

/*
** The version using strncpy() out performs the version with the loop
** if the source strings are long and the target strings are short.
** The version with the loop out performs the other if the source
** strings are short and the destination strings are large.  In
** general, programs are more likely to be dealing with short lines,
** on average, being stored in long buffers (in case there's a long
** line), rather than long lines and short buffers.
**
** Testing on a Sun ES-6000 under Solaris 2.5.1.
** The strncpy() version was called nstrcpy1 and the loop version was
** called nstrcpy2.  The tags short, medium and long refer to the
** length of the source buffer.  The buffer size values refer to the
** length of the target buffer.  The output from a test run was:
**
** The bare loop timing varied between 0.0004 and 0.0008 seconds, and the
** copies were iterated 100,000 times for each number displayed.
**
** strlen(short)  = 12
** strlen(medium) = 500
** strlen(long)   = 4898
**
** Buffer  Short                   Medium                  Long
** Size    strncpy()   loop        strncpy()   loop        strncpy()   loop
**  128    0.139370    0.040470    0.121481    0.320182    0.129494    0.364921
**  256    0.246652    0.040226    0.215903    0.797278    0.233185    0.652269
**  512    0.471067    0.039209    0.404455    1.444087    0.516838    1.512324
** 1024    1.029870    0.045345    1.015691    1.472599    0.813221    2.800626
** 2048    2.034544    0.036935    2.129954    1.424648    1.620638    5.446630
** 4096    3.791687    0.039668    3.694508    1.330972    3.482941   11.185609
**
** The strncpy() routine is probably written in assembler and heavily
** optimized, and it outperforms the loop code by 3:1 when the
** routines have to copy the same amount of data.  On the other hand,
** given a short string to copy, the loop code handily out performs
** the assembler code.  The difference is that strncpy() has to null
** pad the buffer (so it always writes as many characters as there are
** in the target buffer, which is why its times are uniform for a
** given buffer size regardless of source string size), whereas the
** loop code does not (which is why its performance is affected by
** source string size).
*/

#ifdef NSTRCPY_USES_STRNCAT

/* Copy no more than sz characters into dst, including terminal null */
void NSTRCPY_STRNCAT(char *dst, size_t sz, const char *src)
{
    assert(sz > 0);
    sz--;
    dst[0] = '\0';
    strncat(dst, src, sz);
    dst[sz] = '\0';
}

#endif /* NSTRCPY_USES_STRNCAT */

#ifdef NSTRCPY_USES_STRNCPY

/* Copy no more than sz characters into dst, including terminal null */
void NSTRCPY_STRNCPY(char *dst, size_t sz, const char *src)
{
    assert(sz > 0);
    sz--;
    strncpy(dst, src, sz);
    dst[sz] = '\0';
}

#endif /* NSTRCPY_USES_STRNCPY */

#ifdef NSTRCPY_USES_LOOP

/* Copy no more than sz characters into dst, including terminal null */
void NSTRCPY_LOOP(char *dst, size_t sz, const char *src)
{
    char *end = dst + sz - 1;

    assert(sz > 0);
    while (dst < end && *src)
        *dst++ = *src++;
    *dst = '\0';
}

#endif /* NSTRCPY_USES_LOOP */

#ifdef TIMING_TEST

#include <assert.h>
#include <stdio.h>

typedef void (*Function)(char *, size_t, const char *);

typedef struct FuncName
{
    Function     f_pointer;
    const char  *f_name;
} FuncName;

#define DIM(x) (sizeof(x)/sizeof(*(x)))

enum { TIMER_ITERATIONS = 1000000 };

/* Source of data */
static char source[5120];

/* Target for all copying */
static char buffer[5120];

static const FuncName funcs[] =
{
    { NSTRCPY_STRNCAT, "NSTRCPY_STRNCAT" },
    { NSTRCPY_STRNCPY, "NSTRCPY_STRNCPY" },
    { NSTRCPY_LOOP,    "NSTRCPY_LOOP"    },
};

static size_t sz_src[] = { 12, 500, 4898 };
static size_t sz_dst[] = { 1, 2, 128, 256, 512, 1024, 2048, 4096 };

static void time_test(char *dst, size_t dstlen, char *src, size_t srclen, const FuncName *f)
{
    Clock c;
    size_t i;
    char outbuf[60];
    clk_init(&c);
    clk_start(&c);
    for (i = 0; i < TIMER_ITERATIONS; i++)
    {
        (*f->f_pointer)(dst, dstlen, src);
    }
    clk_stop(&c);
    clk_elapsed_us(&c, outbuf, sizeof(outbuf));
    printf("FUNCTION = %15s, DSTLEN %4zu, SRCLEN = %4zu, TIME = %10s\n",
            f->f_name, dstlen, srclen, outbuf);
}

int main(void)
{
    size_t f;
    size_t s;
    size_t d;

    for (f = 0; f < DIM(funcs); f++)
    {
        for (s = 0; s < DIM(sz_src); s++)
        {
            memset(source, 'A', sz_src[s]);
            source[sz_src[s]] = '\0';
            for (d = 0; d < DIM(sz_dst); d++)
            {
                time_test(buffer, sz_dst[d], source, sz_src[s], &funcs[f]);
            }
        }
    }
    return(0);
}

#else

#ifdef TEST

#include <assert.h>
#include <stdio.h>

int main(void)
{
    const char str1[] = "First String";
    const char str2[] = "Very much longer second string";
    char buffer[20];
    size_t  bufsiz = sizeof(buffer) - 1;

    printf("buffer size = %zu\n", bufsiz);
    printf("strlen(str1) = %zu (%s)\n", strlen(str1), str1);
    assert(sizeof(str1) < sizeof(buffer) - 1);
    buffer[bufsiz] = '\177';
    nstrcpy(buffer, bufsiz, str1);
    assert(strcmp(buffer, str1) == 0);
    assert(buffer[bufsiz] == '\177');
    puts(buffer);

    printf("strlen(str2) = %zu (%s)\n", strlen(str2), str2);
    assert(sizeof(str2) > sizeof(buffer));
    buffer[bufsiz] = '\177';
    nstrcpy(buffer, bufsiz, str2);
    assert(strlen(buffer) == bufsiz - 1);
    assert(buffer[bufsiz] == '\177');
    puts(buffer);

    puts("OK");
    return(0);
}

#endif /* TEST */
#endif /* TIMING_TEST */
