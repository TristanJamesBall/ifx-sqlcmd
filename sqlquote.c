/*
@(#)File:           $RCSfile: sqlquote.c,v $
@(#)Version:        $Revision: 2004.2 $
@(#)Last changed:   $Date: 2004/12/24 18:43:55 $
@(#)Purpose:        Convert unquoted string to SQL single-quote or double-quote enclosed string
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 2002-04
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

/*TABSTOP=4*/

#include "sqlquote.h"

#ifndef lint
static const char rcs[] = "@(#)$Id: sqlquote.c,v 2004.2 2004/12/24 18:43:55 jleffler Exp $";
#endif

#ifdef TEST
#include <stdio.h>
#define FAILURE(n, s)	printf("Failure %d - %s\n", n, s)
#else
#define FAILURE(n, s)	((void)0)
#endif /* TEST */

/*
** Convert unquoted string to SQL single-quote or double-quote enclosed string.
** For example, convert: anything to "anything" (or 'anything').
** Convert: "bastard" to """bastard""" or '"bastard"'.
** Convert: 'bastard' to '''bastard''' or "'bastard'".
** Returns: 0 if OK; -1 if failed (eg no space or format error).
** Output string is not null terminated on error!
*/
int sql_quote_string(char *dst, size_t dstlen, const char *src, char quote)
{
	char *end = dst + dstlen - 1;
	char  c = '\0';		/* Avoid accusations of might be used uninitialized */

	/* Minimal output string is three characters */
	if (dstlen < sizeof("''") || dst == 0 || src == 0)
	{
		FAILURE(0, "Invalid input parameters");
		return(-1);
	}

	if (quote != '"' && quote != '\'')
	{
		FAILURE(1, "Invalid quote!");
		return(-1);
	}

	*dst++ = quote;
	while (dst < end && (c = *src++) != '\0')
	{
		if (c == quote)
		{
			*dst++ = c;
			if (dst >= end)
			{
				FAILURE(2, "Too long for quote");
				/* Note that this test needs space for the quote and the NUL */
				return(-1);
			}
		}
		*dst++ = c;
	}
	if (c != '\0')
	{
		FAILURE(3, "Source string too long");
		return(-1);
	}
	*dst++ = quote;
	if (dst > end)
	{
		/* Failure 4 - too long */
		FAILURE(4, "No space for final null");
		/* Note that this test only needs space for the NUL */
		return(-1);
	}
	*dst = '\0';
	return(0);
}

#ifdef TEST

#include <string.h>

#define DIM(x)	(sizeof(x)/sizeof(*(x)))

/*
** Note that the 'too long' tests are critically dependent on the length
** of the variable output in the test function!
*/

#define MAXTESTSTRLEN	16

typedef struct TestCase
{
	char	quote;
	char	*input;
	char	*output;
} TestCase;

static TestCase tests[] =
{
	{ '"', "anything", "\"anything\"" },
	{ '\'', "anything", "'anything'" },
	{ '"', "double \" quote", "\"double \"\" quote\"" },
	{ '\'', "double \" quote", "'double \" quote'" },
	{ '\'', "single ' quote", "'single '' quote'" },
	{ '"', "single ' quote", "\"single ' quote\"" },
	{ '"', "\"bastard\"", "\"\"\"bastard\"\"\"" },
	{ '\'', "'bastard'", "'''bastard'''" },
	{ '"', "32 plain characters should be OK", "\"32 plain characters should be OK\"" },
	{ '"', "33 plain characters for failure 3", "" },
	{ '0', "malformed", "" },						/* Failure - 1 */
	{ '"', "\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"", "\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"" },
	{ '\'', "''''''''''''''''", "''''''''''''''''''''''''''''''''''" },
	{ '"', "\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"", "" },	/* Too long */
	{ '\'', "'''''''''''''''''", "" },	/* Too long */
};

static int test_function(TestCase *test)
{
	const char *src = test->input;
	const char *ans = test->output;
	char quote = test->quote;
	char    output[2*MAXTESTSTRLEN+3];
	int estat = 0;
	int rc = sql_quote_string(output, sizeof(output), src, quote);

	printf("Test: input <<%s>> quote %c (expected %s) got rc = %d\n", src, quote,
			((*ans == '\0') ? "fail" : "pass"), rc);
	if (rc == 0 && *ans == '\0')
	{
		estat = 1;
		printf("FAIL: <<%s>> was expected to fail but passed (<<%s>>)\n", src, output);
	}
	else if (rc == 0 && strcmp(output, ans) == 0)
		printf("PASS: <<%s>> -> <<%s>>\n", src, ans);
	else if (rc == 0)
	{
		estat = 1;
		printf("FAIL: <<%s>> -> <<%s>> (but should be <<%s>>)\n", src, output, ans);
	}
	else if (*ans == '\0')
		printf("PASS: <<%s>> should not be converted\n", src);
	else
	{
		estat = 1;
		printf("FAIL: <<%s>> is OK (should be <<%s>>) but was falsely treated as malformed\n", src, ans);
	}
	return(estat);
}

int main(void)
{
	size_t	i;
	size_t	n = DIM(tests);
	int estat = 0;

	for (i = 0; i < n; i++)
	{
		if (test_function(&tests[i]) != 0)
			estat = 1;
	}

	if (estat != 0)
		printf("***FAILED***\n");
	else
		printf("OK\n");
	return(estat);
}

#endif /* TEST */
