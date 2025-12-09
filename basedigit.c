/*
@(#)File:           $RCSfile: basedigit.c,v $
@(#)Version:        $Revision: 1.4 $
@(#)Last changed:   $Date: 2015/11/15 00:57:47 $
@(#)Purpose:        Convert Character to Digit in Given Base
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 2001,2005,2015
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

/*TABSTOP=4*/

#include "jlss.h"

#if !defined(lint)
/* Prevent over-aggressive optimizers from eliminating ID string */
extern const char jlss_id_basedigit_c[];
const char jlss_id_basedigit_c[] = "@(#)$Id: basedigit.c,v 1.4 2015/11/15 00:57:47 jleffler Exp $";
#endif /* lint */

#if ('z' - 'a') != 25 || ('Z' - 'A') != 25
#error Faulty Assumption
This code assumes the code set is ASCII, ISO 646, ISO 8859, or something similar.
#endif /* Alphabet test */

/*
** Convert character to digit in given base,
** returning -1 for invalid bases and characters.
*/
int basedigit(char c, int base)
{
    int             i;

    if (base < 2 || base > 36)
        i = -1;
    else if (c >= '0' && c <= '9')
        i = c - '0';
    else if (c >= 'A' && c <= 'Z')
        i = c - 'A' + 10;
    else if (c >= 'a' && c <= 'z')
        i = c - 'a' + 10;
    else
        i = -1;
    return((i < base) ? i : -1);
}

#if defined(TEST)

#include <stdio.h>
#include <stdlib.h>

#define DIM(x)  (sizeof(x)/sizeof(*(x)))

typedef struct Test
{
    char    in;         /* Character to test */
    int     minbase;    /* Minimum base to test */
    int     maxbase;    /* Maximum base to test */
    int     digit;      /* Expected digit value */
} Test;

static Test test[] =
{
    {   ' ',    2,  2,  -1  },
    {   '0',    0,  37, 0   },
    {   '1',    2,  10, 1   },
    {   '2',    2,  10, 2   },
    {   '3',    4,  10, 3   },
    {   '4',    0,  10, 4   },
    {   '5',    4,  10, 5   },
    {   '6',    4,  10, 6   },
    {   '7',    0,  37, 7   },
    {   '8',    4,  10, 8   },
    {   '9',    4,  10, 9   },
    {   'a',    4,  12, 10  },
    {   'b',    0,  16, 11  },
    {   'c',    8,  16, 12  },
    {   'D',    8,  16, 13  },
    {   'E',    10, 16, 14  },
    {   'F',    10, 16, 15  },
    {   'G',    10, 26, 16  },
    {   'H',    10, 26, 17  },
    {   'i',    10, 26, 18  },
    {   'J',    10, 26, 19  },
    {   'k',    10, 26, 20  },
    {   'L',    10, 26, 21  },
    {   'm',    10, 26, 22  },
    {   'N',    10, 26, 23  },
    {   'o',    10, 26, 24  },
    {   'p',    20, 36, 25  },
    {   'Q',    20, 36, 26  },
    {   'r',    20, 36, 27  },
    {   'S',    20, 36, 28  },
    {   't',    20, 36, 29  },
    {   'U',    20, 36, 30  },
    {   'v',    20, 36, 31  },
    {   'W',    20, 36, 32  },
    {   'x',    30, 36, 33  },
    {   'y',    30, 36, 34  },
    {   'Z',    30, 37, 35  },
    {   '@',    8,  8,  -1  },
    {   '[',    8,  8,  -1  },
    {   '`',    8,  8,  -1  },
    {   '{',    8,  8,  -1  },
};

int main(void)
{
    int base;
    int count = 0;
    int failed = 0;
    int digit;
    int rc = EXIT_SUCCESS;

    for (size_t i = 0; i < DIM(test); i++)
    {
        for (base = test[i].minbase; base <= test[i].maxbase; base++)
        {
            count++;
            digit = basedigit(test[i].in, base);
            if (digit == test[i].digit)
                continue;
            if (base <= test[i].digit && digit == -1)
                continue;
            if ((base < 2 || base > 36) && digit == -1)
                continue;
            failed++;
            printf("** FAIL ** %d: input %c base %d expected %d but got %d\n",
                    count, test[i].in, base, test[i].digit, digit);
            rc = EXIT_FAILURE;
        }
    }

    if (rc == EXIT_SUCCESS)
        printf("== PASS == %d tests\n", count);
    else
        printf("** FAIL ** failed %d of %d tests\n", failed, count);
    return(rc);
}

#endif /* TEST */
