/*
@(#)File:           $RCSfile: stldint.h,v $
@(#)Version:        $Revision: 2.2 $
@(#)Last changed:   $Date: 2008/02/11 07:39:36 $
@(#)Purpose:        C-ISAM Compatible load and store functions - for integer types
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 1997,2000,2003,2005-08
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

/*TABSTOP=4*/

#ifndef STLDINT_H
#define STLDINT_H

#ifdef MAIN_PROGRAM
#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
const char jlss_id_stldint_h[] = "@(#)$Id: stldint.h,v 2.2 2008/02/11 07:39:36 jleffler Exp $";
#endif /* lint */
#endif /* MAIN_PROGRAM */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <limits.h>     /* LONG_MAX */
#include <stddef.h>     /* size_t */

#ifndef JLSS_ULONG_TYPE     /* Legacy only */
typedef unsigned long Ulong;
#define JLSS_ULONG_TYPE
#endif

#ifndef JLSS_USHORT_TYPE        /* Legacy only */
typedef unsigned short Ushort;
#define JLSS_USHORT_TYPE
#endif

/* Old-fashioned C-ISAM compatibility functions */
extern Ushort ldint(const char *s);
extern Ulong  ldlong(const char *s);
extern void   stint(short int i, char *s);
extern void   stlong(long int l, char *s);

/* Spiffy new code (in 2000-05-30) */

#define IX_UINT1_MAX    ((Uint1)0xFF)
#define IX_UINT2_MAX    ((Uint2)0xFFFF)
#define IX_UINT4_MAX    ((Uint4)0xFFFFFFFF)

#define IX_SINT1_MAX    ((Sint1)0x7F)
#define IX_SINT2_MAX    ((Sint2)0x7FFF)
#define IX_SINT4_MAX    ((Sint4)0x7FFFFFFF)

#define IX_SINT1_MIN    (-IX_SINT1_MAX)
#define IX_SINT2_MIN    (-IX_SINT2_MAX)
#define IX_SINT4_MIN    (-IX_SINT4_MAX)

#define IX_SINT1_NULL   ((Sint1)0x80)
#define IX_SINT2_NULL   ((Sint2)0x8000)
#define IX_SINT4_NULL   ((Sint4)0x80000000)

/* Almost tautologous */
enum { IX_SINT1_SIZE  = 1 };
enum { IX_SINT2_SIZE  = 2 };
enum { IX_SINT4_SIZE  = 4 };

enum { IX_UINT1_SIZE  = 1 };
enum { IX_UINT2_SIZE  = 2 };
enum { IX_UINT4_SIZE  = 4 };

#ifndef NO_EIGHTBYTE_INTEGERS
enum { IX_SINT8_SIZE  = 8 };
enum { IX_UINT8_SIZE  = 8 };
#if defined(HAVE_64BIT_LONG)
#define IX_SINT8_NULL   ((Sint8)0x8000000000000000L)
#define IX_SINT8_MAX    ((Sint8)0x7FFFFFFFFFFFFFFFL)
#define IX_SINT8_MIN    (-IX_SINT8_MAX)
#define IX_UINT8_MAX    ((Uint8)0xFFFFFFFFFFFFFFFFL)
#else
/* Assume 64-bit type is long long and LL is valid suffix */
#define IX_SINT8_NULL   ((Sint8)0x8000000000000000LL)
#define IX_SINT8_MAX    ((Sint8)0x7FFFFFFFFFFFFFFFLL)
#define IX_SINT8_MIN    (-IX_SINT8_MAX)
#define IX_UINT8_MAX    ((Uint8)0xFFFFFFFFFFFFFFFFLL)
#endif /* HAVE_64BIT_LONG */
#endif /* NO_EIGHTBYTE_INTEGERS */

#if defined(HAVE_INTTYPES_H)

#include <inttypes.h>

typedef int8_t  Sint1;
typedef int16_t Sint2;
typedef int32_t Sint4;
#ifndef NO_EIGHTBYTE_INTEGERS
typedef int64_t Sint8;
#endif /* NO_EIGHTBYTE_INTEGERS */

typedef uint8_t  Uint1;
typedef uint16_t Uint2;
typedef uint32_t Uint4;
#ifndef NO_EIGHTBYTE_INTEGERS
typedef uint64_t Uint8;
#endif /* NO_EIGHTBYTE_INTEGERS */

#define PRIX_Sint1  PRIX8
#define PRIx_Sint1  PRIx8
#define PRIo_Sint1  PRIo8
#define PRIu_Sint1  PRIu8
#define PRIi_Sint1  PRIi8
#define PRId_Sint1  PRId8

#define PRIX_Uint1  PRIX8
#define PRIx_Uint1  PRIx8
#define PRIo_Uint1  PRIo8
#define PRIu_Uint1  PRIu8
#define PRIi_Uint1  PRIi8
#define PRId_Uint1  PRId8

#define PRIX_Sint2  PRIX16
#define PRIx_Sint2  PRIx16
#define PRIo_Sint2  PRIo16
#define PRIu_Sint2  PRIu16
#define PRIi_Sint2  PRIi16
#define PRId_Sint2  PRId16

#define PRIX_Uint2  PRIX16
#define PRIx_Uint2  PRIx16
#define PRIo_Uint2  PRIo16
#define PRIu_Uint2  PRIu16
#define PRIi_Uint2  PRIi16
#define PRId_Uint2  PRId16

#define PRIX_Sint4  PRIX32
#define PRIx_Sint4  PRIx32
#define PRIo_Sint4  PRIo32
#define PRIu_Sint4  PRIu32
#define PRIi_Sint4  PRIi32
#define PRId_Sint4  PRId32

#define PRIX_Uint4  PRIX32
#define PRIx_Uint4  PRIx32
#define PRIo_Uint4  PRIo32
#define PRIu_Uint4  PRIu32
#define PRIi_Uint4  PRIi32
#define PRId_Uint4  PRId32

#define PRIX_Sint8  PRIX64
#define PRIx_Sint8  PRIx64
#define PRIo_Sint8  PRIo64
#define PRIu_Sint8  PRIu64
#define PRIi_Sint8  PRIi64
#define PRId_Sint8  PRId64

#define PRIX_Uint8  PRIX64
#define PRIx_Uint8  PRIx64
#define PRIo_Uint8  PRIo64
#define PRIu_Uint8  PRIu64
#define PRIi_Uint8  PRIi64
#define PRId_Uint8  PRId64

#elif LONG_MAX > 0x7FFFFFFFL

/* Platform with 64-bit 'long' assumed to have 32-bit int */
#if INT_MAX != 0x7FFFFFFF
error sizeof(int) != 4
#endif

typedef signed char  Sint1;
typedef signed short Sint2;
typedef signed int   Sint4;
typedef signed long  Sint8;

typedef unsigned char  Uint1;
typedef unsigned short Uint2;
typedef unsigned int   Uint4;
typedef unsigned long  Uint8;

#define PRIX_Sint1  "X"
#define PRIx_Sint1  "x"
#define PRIo_Sint1  "o"
#define PRIu_Sint1  "u"
#define PRIi_Sint1  "i"
#define PRId_Sint1  "d"

#define PRIX_Uint1  "X"
#define PRIx_Uint1  "x"
#define PRIo_Uint1  "o"
#define PRIu_Uint1  "u"
#define PRIi_Uint1  "i"
#define PRId_Uint1  "d"

#define PRIX_Sint2  "X"
#define PRIx_Sint2  "x"
#define PRIo_Sint2  "o"
#define PRIu_Sint2  "u"
#define PRIi_Sint2  "i"
#define PRId_Sint2  "d"

#define PRIX_Uint2  "X"
#define PRIx_Uint2  "x"
#define PRIo_Uint2  "o"
#define PRIu_Uint2  "u"
#define PRIi_Uint2  "i"
#define PRId_Uint2  "d"

#define PRIX_Sint4  "X"
#define PRIx_Sint4  "x"
#define PRIo_Sint4  "o"
#define PRIu_Sint4  "u"
#define PRIi_Sint4  "i"
#define PRId_Sint4  "d"

#define PRIX_Uint4  "X"
#define PRIx_Uint4  "x"
#define PRIo_Uint4  "o"
#define PRIu_Uint4  "u"
#define PRIi_Uint4  "i"
#define PRId_Uint4  "d"

#define PRIX_Sint8  "lX"
#define PRIx_Sint8  "lx"
#define PRIo_Sint8  "lo"
#define PRIu_Sint8  "lu"
#define PRIi_Sint8  "li"
#define PRId_Sint8  "ld"

#define PRIX_Uint8  "lX"
#define PRIx_Uint8  "lx"
#define PRIo_Uint8  "lo"
#define PRIu_Uint8  "lu"
#define PRIi_Uint8  "li"
#define PRId_Uint8  "ld"

#else

/* 32-bit platforms with 64-bit long long */

typedef signed char  Sint1;
typedef signed short Sint2;
typedef signed long  Sint4;
#ifndef NO_EIGHTBYTE_INTEGERS
typedef signed long long Sint8;
#endif /* NO_EIGHTBYTE_INTEGERS */

typedef unsigned char  Uint1;
typedef unsigned short Uint2;
typedef unsigned long  Uint4;
#ifndef NO_EIGHTBYTE_INTEGERS
typedef unsigned long long Uint8;
#endif /* NO_EIGHTBYTE_INTEGERS */

#define PRIX_Sint1  "X"
#define PRIx_Sint1  "x"
#define PRIo_Sint1  "o"
#define PRIu_Sint1  "u"
#define PRIi_Sint1  "i"
#define PRId_Sint1  "d"

#define PRIX_Uint1  "X"
#define PRIx_Uint1  "x"
#define PRIo_Uint1  "o"
#define PRIu_Uint1  "u"
#define PRIi_Uint1  "i"
#define PRId_Uint1  "d"

#define PRIX_Sint2  "X"
#define PRIx_Sint2  "x"
#define PRIo_Sint2  "o"
#define PRIu_Sint2  "u"
#define PRIi_Sint2  "i"
#define PRId_Sint2  "d"

#define PRIX_Uint2  "X"
#define PRIx_Uint2  "x"
#define PRIo_Uint2  "o"
#define PRIu_Uint2  "u"
#define PRIi_Uint2  "i"
#define PRId_Uint2  "d"

#define PRIX_Sint4  "lX"
#define PRIx_Sint4  "lx"
#define PRIo_Sint4  "lo"
#define PRIu_Sint4  "lu"
#define PRIi_Sint4  "li"
#define PRId_Sint4  "ld"

#define PRIX_Uint4  "lX"
#define PRIx_Uint4  "lx"
#define PRIo_Uint4  "lo"
#define PRIu_Uint4  "lu"
#define PRIi_Uint4  "li"
#define PRId_Uint4  "ld"

#define PRIX_Sint8  "llX"
#define PRIx_Sint8  "llx"
#define PRIo_Sint8  "llo"
#define PRIu_Sint8  "llu"
#define PRIi_Sint8  "lli"
#define PRId_Sint8  "lld"

#define PRIX_Uint8  "llX"
#define PRIx_Uint8  "llx"
#define PRIo_Uint8  "llo"
#define PRIu_Uint8  "llu"
#define PRIi_Uint8  "lli"
#define PRId_Uint8  "lld"

#endif

/* New-fangled C-ISAM non-compatible functions */
extern Sint1 ld_sint1(const char *s);
extern Sint2 ld_sint2(const char *s);
extern Sint4 ld_sint4(const char *s);
extern Uint1 ld_uint1(const char *s);
extern Uint2 ld_uint2(const char *s);
extern Uint4 ld_uint4(const char *s);

extern void  st_sint1(Sint1 i, char *s);
extern void  st_sint2(Sint2 i, char *s);
extern void  st_sint4(Sint4 i, char *s);
extern void  st_uint1(Uint1 i, char *s);
extern void  st_uint2(Uint2 i, char *s);
extern void  st_uint4(Uint4 i, char *s);

#ifndef NO_EIGHTBYTE_INTEGERS
extern Sint8 ld_sint8(const char *s);
extern Uint8 ld_uint8(const char *s);
extern void  st_sint8(Sint8 i, char *s);
extern void  st_uint8(Uint8 i, char *s);
#endif /* NO_EIGHTBYTE_INTEGERS */

extern size_t ld_char(const char *from, size_t size, char *to);
extern void st_char(const char *from, char *to, size_t size);

/* Comparison functions for canonical integers in non-aligned storage */
extern int cmp_sint1(const void *s1, const void *s2);
extern int cmp_sint2(const void *s1, const void *s2);
extern int cmp_sint4(const void *s1, const void *s2);
extern int cmp_uint1(const void *s1, const void *s2);
extern int cmp_uint2(const void *s1, const void *s2);
extern int cmp_uint4(const void *s1, const void *s2);

#ifndef NO_EIGHTBYTE_INTEGERS
extern int cmp_sint8(const void *s1, const void *s2);
extern int cmp_uint8(const void *s1, const void *s2);
#endif /* NO_EIGHTBYTE_INTEGERS */

#endif /* STLDINT_H */
