/*
@(#)File:           $RCSfile: sqlunquote.c,v $
@(#)Version:        $Revision: 2004.1 $
@(#)Last changed:   $Date: 2004/12/24 18:33:37 $
@(#)Purpose:        Convert SQL single-quote or double-quote enclosed string to unquoted value
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 2000,2002-04
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

/*TABSTOP=4*/

#include "sqlquote.h"

#ifndef lint
static const char rcs[] = "@(#)$Id: sqlunquote.c,v 2004.1 2004/12/24 18:33:37 jleffler Exp $";
#endif

/*
** Convert SQL single-quote or double-quote enclosed string to unquoted value.
** For example, convert: "anything" to anything.
** Convert: """bastard""" to "bastard".
** Returns: 0 if OK; -1 if failed (eg no space or format error).
** Output string is not null terminated on error!
*/
int sql_unquote_string(char *dst, size_t dstlen, const char *src)
{
	char *end = dst + dstlen - 1;
	char  quote;
	char  c;

	if (dstlen <= 1 || dst == 0 || src == 0)
	{
		/* Failure 0 - Invalid input parameters */
		return(-1);
	}

	if (src[0] != '"' && src[0] != '\'')
	{
		/* Failure 1 - No leading quote! */
		return(-1);
	}

	quote = *src++;

	while ((c = *src++) != '\0')
	{
		if (c == quote)
		{
			if (*src == '\0')
			{
				/* Reached EOS */
				*dst = '\0';
				return(0);
			}
			else if (*src == quote)
			{
				if (dst < end)
					*dst++ = c;
				else
				{
					/* Failure 2 - Not enough space for doubled quote */
					return(-1);
				}
				src++;
			}
			else
			{
				/* Failure 3 - Quote not followed by EOS or another quote */
				return(-1);
			}
		}
		else if (dst < end)
			*dst++ = c;
		else
		{
			/* Failure 4 - Not enough space */
			return(-1);
		}
	}
	/* Failure 5 - Did not find closing quote - malformed string */
	return(-1);
}

#ifdef TEST

#include <assert.h>
#include <stdio.h>
#include <string.h>

#define DIM(x)	(sizeof(x)/sizeof(*(x)))

/*
** Note that the 'too long' tests are critically dependent on the length
** of the variable output in the test function!
** Note that the strings are modified in situ to test single-quote handling.
*/

static char strings[][24] =
{
	"\"anything\"", "anything",
	"\"double \"\" quote\"", "double \" quote",
	"\"\"\"bastard\"\"\"", "\"bastard\"",
	"malformed", "",						/* Failure - 1 */
	"\"malformed", "",					/* Failure - 5 */
	"\"malformed\"!", "",				/* Failure - 3 */
	"\"malformed\"\"", "",				/* Failure - 5 */
	"\"too long to fit!\"", "",			/* Failure - 4 */
	"\"a bit too long!\"\"\"", "",		/* Failure - 2 */
	"\"only just fits!\"", "only just fits!",
	"\"just about OK!\"\"\"", "just about OK!\"",
};

static int test_function(const char *src, const char *ans)
{
	char    output[16];
	int estat = 0;
	int rc = sql_unquote_string(output, sizeof(output), src);

	printf("Test: <<%s>> (expected to %s) - rc = %d\n", src, ((*ans == '\0') ? "fail" : "pass"), rc);
	if (rc == 0 && *ans == '\0')
	{
		estat = 1;
		printf("FAIL: <<%s>> is malformed but this was not detected (<<%s>>)\n", src, output);
	}
	else if (rc == 0 && strcmp(output, ans) == 0)
		printf("PASS: <<%s>> -> <<%s>>\n", src, ans);
	else if (rc == 0)
	{
		estat = 1;
		printf("FAIL: <<%s>> -> <<%s>> (but should be <<%s>>)\n", src, output, ans);
	}
	else if (*ans == '\0')
		printf("PASS: <<%s>> is malformed\n", src);
	else
	{
		estat = 1;
		printf("FAIL: <<%s>> is OK (should be <<%s>>) but was falsely treated as malformed\n", src, ans);
	}
	return(estat);
}

static void strmap(char *src, char oc, char nc)
{
	char c;

	while ((c = *src) != '\0')
	{
		if (c == oc)
			*src = nc;
		src++;
	}
}

int main(void)
{
	size_t	i;
	size_t	n = DIM(strings);
	int estat = 0;

	assert((n % 2) == 0);

	printf("Test: double-quote handling\n");
	for (i = 0; i < n; i += 2)
	{
		if (test_function(strings[i+0], strings[i+1]) != 0)
			estat = 1;
	}

	printf("Test: single-quote handling\n");
	for (i = 0; i < n; i += 2)
	{
		strmap(strings[i+0], '"', '\'');
		strmap(strings[i+1], '"', '\'');
		if (test_function(strings[i+0], strings[i+1]) != 0)
			estat = 1;
	}

	if (estat != 0)
		printf("***FAILED***\n");
	else
		printf("OK\n");
	return(estat);
}

#endif /* TEST */
