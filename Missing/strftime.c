/*
@(#)File:           $RCSfile: strftime.c,v $
@(#)Version:        $Revision: 1.16 $
@(#)Last changed:   $Date: 2011/11/28 04:39:39 $
@(#)Purpose:        Simulate STRFTIME(3)
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 1990-91,1997,1999,2005,2011
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

/*TABSTOP=4*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#ifndef HAVE_STRFTIME

#define _POSIX_SOURCE 1
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <time.h>

/* -- Constant Definitions */

#define LONG_STRING		1024

#define LOCAL_TIME		"%H:%M:%S"
#ifdef STRICT_STRFTIME
#define LOCAL_DATE		"%d %B %Y"
#define LOCAL_DATETIME	"%H:%M:%S %d %B %Y"
#else
#define LOCAL_DATE		"%o %B %Y"
#define LOCAL_DATETIME	"%H:%M:%S %o %B %Y"
#endif	/* STRICT_STRFTIME */

#define NIL(x)			((x)0)

#ifdef _POSIX_SOURCE
#define TZNAME
#undef TM_NAME
#undef TM_ZONE
#endif

#ifdef TM_NAME
#define timezonename(x)       ((x)->tm_name)
#endif /* TM_NAME */
#ifdef TM_ZONE
#define timezonename(x)       ((x)->tm_zone)
#endif /* TM_ZONE */
#ifdef TZNAME
#define timezonename(x)       (tzname[(x)->tm_isdst])
#endif /* TZNAME */
#ifndef timezonename
error "timezonename is not defined"
#endif

/* -- Declarations */

static const char *day_of_week[] =
{
	"Sunday",
	"Monday",
	"Tuesday",
	"Wednesday",
	"Thursday",
	"Friday",
	"Saturday"
};

static const char *month_of_year[] =
{
	"January",
	"February",
	"March",
	"April",
	"May",
	"June",
	"July",
	"August",
	"September",
	"October",
	"November",
	"December"
};

#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
const char jlss_id_strftime_c[] = "@(#)$Id: strftime.c,v 1.16 2011/11/28 04:39:39 jleffler Exp $";
#endif /* lint */

static char    *ordinal(int n)
{
	int             x;
	static char    *suffix[4] = {"th", "st", "nd", "rd"};
	static char     string[15];

	x = n % 100;
	if (x == 11 || x == 12 || x == 13)
		x = 0;
	else if ((x = x % 10) > 3)
		x = 0;
	sprintf(string, "%d%s", n, suffix[x]);
	return (string);
}

/*
**	Calculating week numbers:
**	To give the correct answers, the LHS of any modulus operator must be a
**	non-negative number.
**	Given the day of the year (yd: 1..366) and the weekday it corresponds to
**	(wd: 0..6), the day of the week of 1st January is readily derivable using
**	j1 = (8 + wd - (yd % 7)) % 7 (equivalent to j1 = (372 + wd - yd) % 7).
**	The other requisite input for the week number calculation is the reference
**	day of the week (rd: 0..6), which is the day deemed to be the first day of
**	the week (normally either 0 Sunday or 1 Monday).  Week numbers have the
**	range 00..53, where week 0 occurs before the first instance of rd in the
**	year, week 1 starts on the first instance of rd in the year, and week 53
**	occurs when day 365 or 366 is the same day of the week as rd (or the day
**	after for yd = 366).  Given j1 and rd, the value ff = (j1 + 6 - rd) % 7
**	needs to be added to yd and the result divided by 7 to produce the week
**	number.  Combining these results and using the associative property of the
**	modulus operator yields a function:
**	wn(yd, wd, rd) = (yd + (378 + wd - rd - yd) % 7) / 7
**
**	To further complicate things, the value yd' supplied to the function below
**	is the value yd' = yd - 1.  The revised formula becomes:
**	wn(yd', wd, rd) = (yd' + 1 + (379 + wd - rd - yd') % 7) / 7
*/

static int      weekno(int yd, int wd, int rd)
/* yd -- Days since Jan 1st (0..365) */
/* wd -- Day of week (0 Sunday) */
/* rd -- First day of week (0 Sunday) */
{
	return (yd + 1 + (379 + wd - rd - yd) % 7) / 7;
}

size_t          strftime(char *s, size_t sz, const char *f, const struct tm *lt)
{
	register char  *dst;
	register char  *end;
	register int    slen;
	int             xxxx;
	char            c;
	char           *t;

	dst = s;
	end = s + sz;

	while ((c = *f++) && dst < end)
	{
#ifdef TEST
		fprintf(stderr, "FC = %c\n", c);
#endif /* TEST */
		if (c != '%')
			*dst++ = c;
		else
		{
#ifdef TEST
			fprintf(stderr, "FC = %c\n", *f);
#endif /* TEST */
			switch (*f++)
			{
			case 'a':			/* abbreviated weekday name */
				slen = 3;
				if (dst < end - slen)
					strncpy(dst, day_of_week[lt->tm_wday], slen);
				else
					dst = end;
				break;

			case 'A':			/* full weekday name */
				slen = strlen(day_of_week[lt->tm_wday]);
				if (dst < end - slen)
					strcpy(dst, day_of_week[lt->tm_wday]);
				else
					dst = end;
				break;

			case 'b':			/* abbreviated month name */
				slen = 3;
				if (dst < end - slen)
					strncpy(dst, month_of_year[lt->tm_mon], slen);
				else
					dst = end;
				break;

			case 'B':			/* full month name */
				slen = strlen(month_of_year[lt->tm_mon]);
				if (dst < end - slen)
					strcpy(dst, month_of_year[lt->tm_mon]);
				else
					dst = end;
				break;

			case 'c':			/* local date and time representation */
				slen = strftime(dst, end - dst, LOCAL_DATETIME, lt);
				break;

			case 'd':			/* day of month (01-31) */
				slen = 2;
				if (dst < end - slen)
					sprintf(dst, "%02d", lt->tm_mday);
				else
					dst = end;
				break;

			case 'e':			/* day of month (1-31) */
				slen = (lt->tm_mday < 10) ? 1 : 2;
				if (dst < end - slen)
					sprintf(dst, "%d", lt->tm_mday);
				else
					dst = end;
				break;

			case 'H':			/* hour (24-hour clock) (00-23) */
				slen = 2;
				if (dst < end - slen)
					sprintf(dst, "%02d", lt->tm_hour);
				else
					dst = end;
				break;

			case 'I':			/* hour (12-hour clock) (01-12) */
				slen = 2;
				xxxx = (lt->tm_hour) % 12;
				if (xxxx == 0)
					xxxx = 12;
				if (dst < end - slen)
					sprintf(dst, "%02d", xxxx);
				else
					dst = end;
				break;

			case 'j':			/* day of year (001-366) */
				slen = 3;
				if (dst < end - slen)
					sprintf(dst, "%03d", lt->tm_yday + 1);
				else
					dst = end;
				break;

			case 'm':			/* month (01-12) */
				slen = 2;
				if (dst < end - slen)
					sprintf(dst, "%02d", lt->tm_mon + 1);
				else
					dst = end;
				break;

			case 'M':			/* minute (00-59) */
				slen = 2;
				if (dst < end - slen)
					sprintf(dst, "%02d", lt->tm_min);
				else
					dst = end;
				break;

#ifndef STRICT_STRFTIME
			case 'o':			/* Ordinal day (1st, 2nd, ...) */
				slen = (lt->tm_mday < 10) ? 3 : 4;
				if (dst < end - slen)
					strcpy(dst, ordinal(lt->tm_mday));
				else
					dst = end;
				break;
#endif	/* STRICT_STRFTIME */

			case 'p':			/* local equivalent of AM or PM */
				slen = 2;
				if (dst < end - slen)
					strcpy(dst, (lt->tm_hour < 12) ? "AM" : "PM");
				else
					dst = end;
				break;

			case 'S':			/* second (00-59) */
				slen = 2;
				if (dst < end - slen)
					sprintf(dst, "%02d", lt->tm_sec);
				else
					dst = end;
				break;

			case 'U':			/* week number of the year (00-53) */
								/* Sunday as 1st day of week       */
				slen = 2;
				if (dst < end - slen)
					sprintf(dst, "%02d", weekno(lt->tm_yday, lt->tm_wday, 0));
				else
					dst = end;
				break;

			case 'w':			/* weekday (0-6) (Sunday is 0) */
				slen = 1;
				if (dst < end - slen)
					sprintf(dst, "%01d", lt->tm_wday);
				else
					dst = end;
				break;

			case 'W':			/* Week number of the year (00-53) */
								/* Monday as 1st day of week       */
				slen = 2;
				if (dst < end - slen)
					sprintf(dst, "%02d", weekno(lt->tm_yday, lt->tm_wday, 1));
				else
					dst = end;
				break;

			case 'x':			/* local date representation */
				slen = strftime(dst, end - dst, LOCAL_DATE, lt);
				break;

			case 'X':			/* Local time representation */
				slen = strftime(dst, end - dst, LOCAL_TIME, lt);
				break;

			case 'y':			/* Year without century (00-99) */
				slen = 2;
				if (dst < end - slen)
					sprintf(dst, "%02d", lt->tm_year % 100);
				else
					dst = end;
				break;

			case 'Y':			/* Year with century */
				slen = 4;
				if (dst < end - slen)
					sprintf(dst, "%04d", lt->tm_year + 1900);
				else
					dst = end;
				break;

			case 'Z':			/* Time zone name (if any) */
				t = timezonename(lt);
				if (t == NIL(char *))
					t = "";
				slen = strlen(t);
				if (dst < end - slen)
					strcpy(dst, t);
				else
					dst = end;
				break;

			case '%':			/* % */
				slen = 1;
				*dst = '%';
				break;

			default:			/* Format error */
				slen = 0;
				dst = end;
				break;

			}
			dst += slen;
		}
#ifdef TEST
		*dst = '\0';
		fprintf(stderr, "sz = %d, DS = <<%s>>\n", dst - s, s);
#endif /* TEST */
	}

	if (dst >= end)
		*s = '\0';
	else
		*dst = '\0';

	return ((dst >= end) ? 0 : dst - s);
}

#endif /* HAVE_STRFTIME */

#ifdef TEST

#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#ifndef NIL
#define NIL(x)			((x)0)
#endif /* NIL */
#ifndef LONG_STRING
#define LONG_STRING 1024
#endif /* LONG_STRING */

int
main(int argc, char **argv)
{
	time_t          now;
	struct tm      *lt;
	char            buffer[LONG_STRING];

#ifdef HAVE_STRFTIME
    puts("Warning: using system supplied strftime()");
#endif /* HAVE_STRFTIME */

	if (argc <= 1)
	{
		fprintf(stderr, "Usage: %s format [...]\n", argv[0]);
		exit(1);
	}
	now = time(NIL(long *));
	lt = localtime(&now);
	while (*++argv != NIL(char *))
	{
		strftime(buffer, sizeof(buffer), *argv, lt);
		printf("%s => %s\n", *argv, buffer);
	}
	return(0);
}

#endif	/* TEST */
