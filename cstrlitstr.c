/*
@(#)File:           $RCSfile: cstrlitstr.c,v $
@(#)Version:        $Revision: 1.9 $
@(#)Last changed:   $Date: 2016/07/08 01:17:53 $
@(#)Purpose:        Convert C String Literal to String
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 2001,2005,2011,2015-16
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

/*TABSTOP=4*/

#include "cchrstrlit.h"

#if !defined(lint)
/* Prevent over-aggressive optimizers from eliminating ID string */
extern const char jlss_id_cstrlitstr_c[];
const char jlss_id_cstrlitstr_c[] = "@(#)$Id: cstrlitstr.c,v 1.9 2016/07/08 01:17:53 jleffler Exp $";
#endif /* lint */

/*
** Convert C String Literal in (str..end] (excluding surrounding quotes)
** to string, returning length if conversion OK, -1 if string is invalid,
** -2 if conversion is not complete (not enough output space).  Output is
** null terminated, but if input contains \0 or \00 or \000, then output
** contains embedded ASCII NUL too.
*/
int cstrlit_str(const char *src, const char *src_end, char *buffer, size_t buflen)
{
    char *dst = buffer;
    char *dst_end = buffer + buflen - 1;
    const char *eptr;
    int c = 0;

    while (src < src_end && dst < dst_end)
    {
        if ((c = cstrlit_chr(src, src_end, &eptr)) == -1)
        {
            *dst = '\0';
            return -1;
        }
        *dst++ = (char)c;
        src = eptr;
    }
    *dst = '\0';
    return((src == src_end) ? (int)(dst - buffer) : -2);
}

#if defined(TEST)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DIM(x)  (sizeof(x)/sizeof(*(x)))
#define TESTBUFFLEN 32  /* Do not change without changing test cases */

struct Test
{
    const char *str;    /* String to convert */
    const char *val;    /* Expected output */
    ssize_t vallen;     /* Length of expected output */
};

static struct Test test[] =
{
    {   ".@=",      ".@=", sizeof(".@=")-1  },
    {   "\\003",    "\3", sizeof("\3")-1    },
    {   "\\''\\\"\"\\011",  "\'\'\"\"\t", sizeof("\'\'\"\"\t")-1    },
    {   "\\''\\\"\"\\t",    "\'\'\"\"\t", sizeof("\'\'\"\"\t")-1    },
    {   "\\040 \\x20\\40",  "    ", sizeof("    ")-1    },
    {   "\\000\\00\\0\\x00\\x0",    "\0\0\0\0\0", sizeof("\0\0\0\0\0")-1    },
    {   "\\400",    0, -1   },
    {   "\\400AAAZ",    0, -1   },
    {   "\\xZZ",    0, -1   },
    /* Much longer than TESTBUFFLEN */
    {   "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz", 0, -2   },
    /* Exactly TESTBUFFLEN */
    {   "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdef", 0, -2   },
    /* One shorter than TESTBUFFLEN */
    {   "ABCDEFGHIJKLMNOPQRSTUVWXYZabcde",  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcde",
            sizeof("ABCDEFGHIJKLMNOPQRSTUVWXYZabcde")-1 },
    /* Two shorter than TESTBUFFLEN */
    {   "ABCDEFGHIJKLMNOPQRSTUVWXYZabcd",   "ABCDEFGHIJKLMNOPQRSTUVWXYZabcd",
            sizeof("ABCDEFGHIJKLMNOPQRSTUVWXYZabcd")-1  },
    {   "", "", 0   },
};

int main(void)
{
    size_t i;
    int rc = EXIT_SUCCESS;
    char buffer[TESTBUFFLEN];
    const char *str;
    int rv;

    for (i = 0; i < DIM(test); i++)
    {
        str = test[i].str;
        rv = cstrlit_str(str, str + strlen(str), buffer, sizeof(buffer));
        if (rv < 0 && rv != test[i].vallen)
        {
            rc = EXIT_FAILURE;
            printf("** FAIL ** input \"%s\"; unexpected conversion failure %d", test[i].str, rv);
            if (test[i].str != 0)
                printf("; expected output %s", test[i].val);
            putchar('\n');
        }
        else if (rv < 0 && rv == test[i].vallen)
            printf("== PASS == input \"%s\"; got expected failure %zd\n", test[i].str,
                    test[i].vallen);
        else if (test[i].val == 0)
        {
            rc = EXIT_FAILURE;
            printf("** FAIL ** input \"%s\"; unexpected success (got \"%s\")\n",
                    test[i].str, buffer);
        }
        else if (rv != test[i].vallen)
        {
            rc = EXIT_FAILURE;
            printf("** FAIL ** input \"%s\"; length mismatch (got %d, wanted %zd)\n",
                    test[i].str, rv, test[i].vallen);
        }
        else if (memcmp(buffer, test[i].val, (size_t)test[i].vallen) == 0)
            printf("== PASS == input \"%s\"; got expected output \"%s\"\n",
                    test[i].str, test[i].val);
        else
        {
            rc = EXIT_FAILURE;
            printf("** FAIL ** input \"%s\"; comparison failure (expected \"%s\" but got \"%s\")\n",
                    test[i].str, test[i].val, buffer);
        }
    }
    puts((rc == EXIT_SUCCESS) ? "OK" : "** FAIL **");
    return(rc);
}

#endif /* TEST */
