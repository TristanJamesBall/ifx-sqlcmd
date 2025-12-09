/*
@(#)File:           $RCSfile: cstrlitchr.c,v $
@(#)Version:        $Revision: 1.10 $
@(#)Last changed:   $Date: 2016/07/08 01:17:53 $
@(#)Purpose:        Convert C Character Literal to Character
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 2001,2005,2007,2013,2015-16
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

/*TABSTOP=4*/

#include "cchrstrlit.h"
#include "jlss.h"       /* basedigit() */
#include <ctype.h>

#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
extern const char jlss_id_cstrlitchr_c[];
const char jlss_id_cstrlitchr_c[] = "@(#)$Id: cstrlitchr.c,v 1.10 2016/07/08 01:17:53 jleffler Exp $";
#endif /* lint */

/* Convert C Character Literal in (str..end] (excluding surrounding quotes) */
/* to character, returning converted char or -1 if string is invalid. */

/* Convert string containing C character literal to character value */
/* Returns -1 if character literal is invalid, otherwise 0x00..0xFF */
/* Does not support extension \E for ESC \033. */
/* Does not support any extension for DEL \177. */
/* Does not support control-char notation ^A for CTRL-A \001. */
/* Accepts \z as valid z when z is not otherwise special. */
/* Accepts \038 as valid CTRL-C \003; next character starts with the 8. */
/* Accepts \x3Z as valid CTRL-C \003; next character starts with the Z. */
int cstrlit_chr(const char *str, const char *end, char const ** const eptr)
{
    char c;
    int rv;

    if (str >= end)
        rv = -1;    /* String contains no data */
    else if ((c = *str++) != '\\')
        rv = c;
    else if (str == end)
        rv = -1;    /* Just a backslash - invalid */
    else if ((c = *str++) == 'x')
    {
        /**
        ** Hex character constant - \xHH or \xH, where H is a hex digit.
        ** Technically, can also be \xHHH or \xHHHH (or longer), if
        ** CHAR_BIT > 8; this nicety is being studiously ignored.
        */
        int x1;
        int x2;
        if (str == end)
            rv = -1;
        else if ((x1 = basedigit(*str++, 16)) < 0)
        {
            rv = -1;        /* Invalid hex constant */
            str--;
        }
        else if (str == end)
            rv = x1;        /* Single digit hex constant */
        else if ((x2 = basedigit(*str++, 16)) < 0)
        {
            rv = x1;        /* Single-digit hex constant */
            str--;
        }
        else
            rv = (x1 << 4) | x2;    /* Double-digit hex constant */
    }
    else if (isdigit((unsigned char)c))
    {
        /**
        ** Octal character constant - \O or \OO or \OOO, where O is an
        ** octal digit.  Treat \8 as 8 and \9 as 9.
        */
        int o1;
        int o2;
        int o3;
        if ((o1 = basedigit(c, 8)) < 0)
            rv = c; /* Invalid octal constant (\8 or \9) */
        else if (str == end)
            rv = o1;    /* Single-digit octal constant */
        else if ((o2 = basedigit(*str++, 8)) < 0)
        {
            rv = o1;    /* Single-digit octal constant */
            str--;
        }
        else if (str == end)
            rv = (o1 << 3) | o2;    /* Double-digit octal constant */
        else if ((o3 = basedigit(*str++, 8)) < 0)
        {
            rv = (o1 << 3) | o2;    /* Double-digit octal constant */
            str--;
        }
        else if (o1 >= 4)
            rv = -1;                /* Out of range 0x00..0xFF (\000..\377) */
        else
            rv = (((o1 << 3) | o2) << 3) | o3;
    }
    else
    {
        /* Presumably \a, \b, \f, \n, \r, \t, \v, \', \", \? or \\ - or an error */
        switch (c)
        {
        case 'a':
            rv = '\a';
            break;
        case 'b':
            rv = '\b';
            break;
        case 'f':
            rv = '\f';
            break;
        case 'n':
            rv = '\n';
            break;
        case 'r':
            rv = '\r';
            break;
        case 't':
            rv = '\t';
            break;
        case 'v':
            rv = '\v';
            break;
        case '\"':
            rv = '\"';
            break;
        case '\'':
            rv = '\'';
            break;
        case '\?':
            rv = '\?';
            break;
        case '\\':
            rv = '\\';
            break;
        case '\0':  /* Malformed: solitary backslash followed by NUL */
            rv = -1;
            break;
        default:
            rv = c; /* Nominally invalid: \X but X not special; return X. */
            break;
        }
    }
    if (eptr != 0)
        *eptr = str;
    return(rv);
}

#ifdef TEST

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DIM(x)  (sizeof(x)/sizeof(*(x)))

typedef struct Test
{
    const char *str;
    int         val;
    int         nxt;
} Test;

static const Test test[] =
{
    {   "",         -1,     '\0'    },  /* Empty string */
    {   "aZ",       'a',    'Z'     },  /* Regular character */
    {   "0Z",       '0',    'Z'     },  /* Regular digit */
    {   "a",        'a',    '\0'    },  /* Regular character */
    {   "\"Z",      '"',    'Z'     },  /* Just a double quote */
    {   "\\\"Z",    '"',    'Z'     },  /* Escaped double quote */
    {   "\\'Z",     '\'',   'Z'     },  /* Escaped single quote */
    {   "'Z",       '\'',   'Z'     },  /* Just a single quote */
    {   "\\\\Z",    '\\',   'Z'     },  /* Double backslash */
    {   "\\",       -1,     '\0'    },  /* Single backslash */
    {   "\\Z",      'Z',    '\0'    },  /* Backslash non-escape */
    {   "\\aZ",     '\a',   'Z'     },  /* Alert */
    {   "\\bZ",     '\b',   'Z'     },  /* Backspace */
    {   "\\fZ",     '\f',   'Z'     },  /* Form-feed */
    {   "\\nZ",     '\n',   'Z'     },  /* Newline */
    {   "\\rZ",     '\r',   'Z'     },  /* Carriage return */
    {   "\\tZ",     '\t',   'Z'     },  /* Horizontal tab */
    {   "\\vZ",     '\v',   'Z'     },  /* Vertical tab */
    {   "\\?Z",     '\?',   'Z'     },  /* Question mark - trigraphs! */
    {   "\\zZ",     'z',    'Z'     },  /* Invalid escape */
    {   "\\xZ",     -1,     'Z'     },  /* Invalid hex constant */
    {   "\\xAZ",    0xA,    'Z'     },  /* Single-digit hex constant */
    {   "\\xAAZ",   0xAA,   'Z'     },  /* Double-digit hex constant */
    {   "\\xAZ",    10,     'Z'     },  /* Single-digit hex constant plus extra */
    {   "\\xFF",    0xFF,   '\0'    },  /* Single-digit hex constant plus extra */
    {   "\\0Z",     0,      'Z'     },  /* Single-digit octal constant */
    {   "\\00Z",    0,      'Z'     },  /* Double-digit octal constant */
    {   "\\000Z",   0,      'Z'     },  /* Triple-digit octal constant */
    {   "\\7Z",     7,      'Z'     },  /* Single-digit octal constant */
    {   "\\07Z",    7,      'Z'     },  /* Double-digit octal constant */
    {   "\\377Z",   0xFF,   'Z'     },  /* Triple-digit octal constant */
    {   "\\378Z",   037,    '8'     },  /* Double-digit octal constant */
    {   "\\38Z",    03,     '8'     },  /* Single-digit octal constant */
    {   "\\8Z",     '8',    'Z'     },  /* Invalid octal constant */
    {   "\\9Z",     '9',    'Z'     },  /* Invalid octal constant */
    {   "\\400Z",   -1,     'Z'     },  /* Invalid octal constant */
    {   "\\777Z",   -1,     'Z'     },  /* Invalid octal constant */
};

int main(void)
{
    size_t  i;
    int     result;
    char buffer[8];
    const char *eptr;
    const char *str;
    char obuff[6];
    int     rc = EXIT_SUCCESS;

    for (i = 0; i < DIM(test); i++)
    {
        str = test[i].str;
        result = cstrlit_chr(str, str + strlen(str), &eptr);
        sprintf(buffer, "'%s'", test[i].str);
        chr_cstrlit((unsigned char)*eptr, obuff, sizeof(obuff));
        if (result == test[i].val && *eptr == test[i].nxt)
        {
            printf("== PASS == %-7s => %3d (next = %-4s)\n",
                buffer, result, obuff);
        }
        else
        {
            printf("** FAIL ** %-7s => %3d (next = %-4s) (expected %d)\n",
                buffer, result, obuff, test[i].val);
            rc = EXIT_FAILURE;
        }
    }
    puts((rc == EXIT_SUCCESS) ? "== PASS ==" : "** FAIL **");
    return(rc);
}

#endif /* TEST */
