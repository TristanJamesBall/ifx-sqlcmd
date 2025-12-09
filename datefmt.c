/*
@(#)File:           $RCSfile: datefmt.c,v $
@(#)Version:        $Revision: 2012.1 $
@(#)Last changed:   $Date: 2012/06/22 04:27:20 $
@(#)Purpose:        Convert DBDATE into date format
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 1992-93,1995,1997-98,2000,2003,2005-06,2012
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

#include "datefmt.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

/*
** JL 2006-01-06: After some experimentation, it appears that DB-Access
** interprets DBDATE="Y4MD" as equivalent to "Y4MD/", so the default
** punctuator is '/'.
*/

enum
{
    DB_YEAR  = 0x01,                        /* Seen year */
    DB_MONTH = 0x02,                        /* Seen month */
    DB_DAY   = 0x04,                        /* Seen day */
    DB_DELIM = 0x08,                        /* Seen delimiter */
    DB_DATE  = (DB_YEAR|DB_MONTH|DB_DAY),   /* Seen whole date */
    DB_DONE  = (DB_DATE|DB_DELIM)           /* Seen full DBDATE */
};

static const char default_format[] = "mm/dd/yyyy";

#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
const char jlss_id_datefmt_c[] = "@(#)$Id: datefmt.c,v 2012.1 2012/06/22 04:27:20 jleffler Exp $";
#endif /* lint */

/* Overwrite the blank punctuation markers */
static void fmt_overwrite(char *retval, char c)
{
    char *p;
    for (p = retval; *p != '\0'; p++)
    {
        if (*p == ' ')
            *p = c;
    }
}

const char *cvt_dbdate(const char *dbdate)
{
    static char     retval[11];
    char           *p;
    const char     *d;
    unsigned char   c;
    int             state;
    int             i;
    int             n;
    size_t          len;

    if (dbdate == 0)
        return(default_format);
    len = strlen(dbdate);
    if (len != 4 && len != 5)
        return(default_format);
    p = retval;
    d = dbdate;
    state = 0;
    while ((c = *d++) != '\0')
    {
        switch (c)
        {
        case 'y':
        case 'Y':
            if (state & DB_YEAR)
                return(default_format);
            switch (*d++)
            {
            case '2':
                n = 2;
                break;
            case '4':
                n = 4;
                break;
            default:
                return(default_format);
            }
            for (i = 0; i < n; i++)
                *p++ = 'y';
            *p++ = ' ';
            state |= DB_YEAR;
            break;
        case 'm':
        case 'M':
            if (state & DB_MONTH)
                return(default_format);
            *p++ = 'm';
            *p++ = 'm';
            *p++ = ' ';
            state |= DB_MONTH;
            break;
        case 'd':
        case 'D':
            if (state & DB_DAY)
                return(default_format);
            *p++ = 'd';
            *p++ = 'd';
            *p++ = ' ';
            state |= DB_DAY;
            break;
        default:
            if (state & DB_DELIM)
                return(default_format);
            if (!ispunct(c) && c != '0')
                return(default_format);
            state |= DB_DELIM;
            if ((state & DB_DONE) != DB_DONE)
                return(default_format);
            *--p = '\0';
            if (c != '0')
                fmt_overwrite(retval, c);
            else
            {
                /* No punctuation -- copy string without the blanks */
                char *src;
                char *dst;
                for (src = dst = retval; *src != '\0'; src++)
                {
                    if (*src != ' ')
                        *dst++ = *src;
                }
                *dst = '\0';
            }
            break;
        }
    }
    if ((state & DB_DATE) != DB_DATE)
        return(default_format);
    else if ((state & DB_DONE) != DB_DONE)
    {
        /* Zap trailing blank */
        *--p = '\0';
        fmt_overwrite(retval, '/');
    }
    return(retval);
}

const char *fmt_dbdate(void)
{
    char *dbdate = getenv("DBDATE");
    return(cvt_dbdate(dbdate));
}

#ifdef TEST

#include <stdio.h>

#define DIM(x)  (sizeof(x)/sizeof(*(x)))

typedef struct testcase
{
    const char *dbdate;
    const char *expect;
} testcase;

static const testcase dbd[] =
{
    {   "dmy4/",    "dd/mm/yyyy"    },  /* OK */
    {   "mdy4/",    "mm/dd/yyyy"    },  /* OK */
    {   "my4d/",    "mm/yyyy/dd"    },  /* OK */
    {   "dy4m/",    "dd/yyyy/mm"    },  /* OK */
    {   "y4dm/",    "yyyy/dd/mm"    },  /* OK */
    {   "Y4MD-",    "yyyy-mm-dd"    },  /* OK */
    {   "Y4MDZ",    "mm/dd/yyyy"    },  /* Invalid punctuator */
    {   "y4md~",    "yyyy~mm~dd"    },  /* OK */
    {   "dmy2.",    "dd.mm.yy"      },  /* OK */
    {   "y2md-",    "yy-mm-dd"      },  /* OK */
    {   "Z4md-",    "mm/dd/yyyy"    },  /* Invalid letter */
    {   "y1md-",    "mm/dd/yyyy"    },  /* Invalid year qualifier */
    {   "y4md",     "yyyy/mm/dd"    },  /* No punctuation */
    {   "y4m/",     "mm/dd/yyyy"    },  /* No day */
    {   "y4d/",     "mm/dd/yyyy"    },  /* No month */
    {   "md/",      "mm/dd/yyyy"    },  /* No year */
    {   "y2Md-",    "yy-mm-dd"      },  /* OK */
    {   "y2mD-",    "yy-mm-dd"      },  /* OK */
    {   "Y2md-",    "yy-mm-dd"      },  /* OK */
    {   "Y2md0",    "yymmdd"        },  /* OK */
    {   "Y4MD0",    "yyyymmdd"      },  /* OK */
    {   "Y4MD",     "yyyy/mm/dd"    },  /* OK */
    {   "MDY4",     "mm/dd/yyyy"    },  /* OK */
    {   "DMY4",     "dd/mm/yyyy"    },  /* OK */
    {   "MDY2",     "mm/dd/yy"      },  /* OK */
};

int main(void)
{
    const testcase *s;
    const testcase *e = dbd + DIM(dbd);
    size_t fail = 0;
    int    rc = EXIT_SUCCESS;

    printf("Environment variable $DBDATE=%s; date format = %s\n", getenv("DBDATE"), fmt_dbdate());

    for (s = dbd; s < e; s++)
    {
        const char  *fmt = s->dbdate;
        const char  *exp = s->expect;
        const char  *res = cvt_dbdate(fmt);
        if (strcmp(res, exp) != 0)
        {
            printf("DBDATE=%s; wanted %s, got %s -- FAIL\n", fmt, exp, res);
            fail++;
        }
        else
            printf("DBDATE=%s; date format = %s -- PASS\n", fmt, res);
    }

    if (fail != 0)
    {
        rc = EXIT_FAILURE;
        printf("== FAIL == (%u out of %u tests failed)\n", (unsigned)fail, (unsigned)DIM(dbd));
    }
    else
        printf("** PASS ** (%u tests passed)\n", (unsigned)DIM(dbd));

    return(rc);
}

#endif /* TEST */
